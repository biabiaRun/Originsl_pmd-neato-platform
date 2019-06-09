/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <common/exceptions/RuntimeError.hpp>

namespace royale
{
    namespace usb
    {
        namespace bridge
        {
            /**
             * Simple cleanup for an IUnknown pointer, as the CComPtr class is only available with the
             * ATL.
             *
             * Takes a pointer (which is assumed to have already had AddRef called, probably by the
             * library which returned the pointer), and decrements the ref when it goes out of
             * scope.
             */
            template<typename T> class WrappedComPtr
            {
            public:
                /**
                 * Create a wrapper for a pointer.  If WrapperComPtr::getRef() will be
                 * used to hold the result of a CoCreateInstance call, then the default
                 * constructor values (initialize to nullptr) should be used.
                 *
                 * If given a non-nullptr, it will always add a reference.
                 *
                 * @param ptr the pointer to take ownership of
                 */
                explicit WrappedComPtr (T *ptr = nullptr) :
                    m_ptr{ptr}
                {
                    if (ptr != nullptr)
                    {
                        m_ptr->AddRef();
                    }
                }

                WrappedComPtr (const WrappedComPtr<T> &other) :
                    m_ptr {other.m_ptr}
                {
                    m_ptr->AddRef();
                }

                WrappedComPtr<T> &operator= (WrappedComPtr<T> other)
                {
                    swap (*this, other);
                    return *this;
                }

                friend void swap (WrappedComPtr<T> &first, WrappedComPtr<T> &second)
                {
                    // The refcount is inside the pointed-to instance
                    std::swap (first.m_ptr, second.m_ptr);
                }

                void reset()
                {
                    if (m_ptr != nullptr)
                    {
                        m_ptr->Release();
                        m_ptr = nullptr;
                    }
                }

                ~WrappedComPtr()
                {
                    reset();
                }

                T *get()
                {
                    return m_ptr;
                }

                /**
                 * Provides the argument for a CoCreateInstance call.
                 *
                 * If this wrapper is already pointing to a non-nullptr, the previously pointed-to
                 * object will be forgotten without calling Release(), this will probably leak memory.
                 */
                void **getRef()
                {
                    if (m_ptr != nullptr)
                    {
                        throw royale::common::RuntimeError ("Reusing an already-assigned WrappedComPtr");
                    }
                    return reinterpret_cast<void **> (&m_ptr);
                }

                /**
                 * Returns the encapsulated pointer.  If this wrapper is already pointing to a
                 * non-nullptr, changing the pointer will leak the original pointer.
                 *
                 * This does not throw even if it's likely to be involved in a memory leak,
                 * because the only place it's used seems to be a leak in the MSDN documentation.
                 */
                T **getTypeRef()
                {
                    return &m_ptr;
                }

                T *operator-> ()
                {
                    return m_ptr;
                }

                operator bool() const
                {
                    return m_ptr != nullptr;
                }

            private:
                T *m_ptr;
            };
        }
    }
}
