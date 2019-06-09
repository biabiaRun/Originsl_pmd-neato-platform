/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __MEMORYHOLDER_HPP__
#define __MEMORYHOLDER_HPP__

#include "spectre/Deleter.hpp"
#include <utility>

namespace spectre
{
    /**
     * @brief Class which holds heap-allocated objects
     *
     * The MemoryHolder is a smart-pointer like class which is used to transfer
     * heap-allocated objects from the Spectre library to its users. The MemoryHolder
     * owns the objects, and ensures that it is free'd using the appropriate free function
     * from the Spectre library.
     */
    template<typename T>
    class MemoryHolder
    {
    public:
        explicit MemoryHolder (T *ptr)
            : m_ptr (ptr) {}

        MemoryHolder()
            : m_ptr (nullptr)
        {}

        MemoryHolder (MemoryHolder &&other)
            : MemoryHolder()
        {
            std::swap (m_ptr, other.m_ptr);
        }

        ~MemoryHolder()
        {
            spectre::details::Deleter<T>::free (m_ptr);
        }

        T &operator*()
        {
            return *m_ptr;
        };
        T *operator->()
        {
            return m_ptr;
        }

        const T &operator*() const
        {
            return *m_ptr;
        };
        const T *operator->() const
        {
            return m_ptr;
        };

        T *get()
        {
            return m_ptr;
        };
        const T *get() const
        {
            return m_ptr;
        }

        T *release()
        {
            T *released = m_ptr;
            m_ptr = nullptr;
            return released;
        }

        void swap (MemoryHolder &&other)
        {
            std::swap (m_ptr, other.m_ptr);
        }

        MemoryHolder (const MemoryHolder &) = delete;
        MemoryHolder &operator= (const MemoryHolder &) = delete;

    private:
        T *m_ptr;
    };

    template<typename T, typename ...Args>
    MemoryHolder<T> make_holder (Args &&...args)
    {
        return MemoryHolder<T> (new T (std::forward<Args> (args)...));
    }

}  // spectre

#endif /*__MEMORYHOLDER_HPP__*/
