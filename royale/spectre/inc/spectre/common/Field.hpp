/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include "ArrayAllocator.hpp"
#include "common/CommonConfig.hpp"
#include "CompUtils.hpp"
#include "Utils.hpp"

#include <cstdio>
#include <algorithm>
#include <cassert>

namespace spectre
{
    namespace common
    {
        /**
        * @brief The AbstractField class is an abstract helper class which encapsulates
        * an array.
        *
        * The class is a type-safe wrapper with an STL-like interface. It
        * allows for optional bounds checking if enabled during
        * compilation.
        *
        * Depending on the chosen implementation the class can act as a reference to an
        * existing array (ArrayReference<T>), or hold the memory itself (ArrayHolder<T>).
        */
        template <typename T>
        class AbstractField
        {
        protected:
            AbstractField() : m_data (nullptr), m_size (0) {}

            AbstractField (T *data, size_t size) : m_data (data), m_size (size) {}

        public:

            virtual ~AbstractField() {}
            /**
             * @brief virtual operation which returns a copy of the appropiate class
             * @return copy of current class
             */
            virtual AbstractField<T> *clone() const = 0;

            /**
             * @brief returns the number of the elements contained
             * @return number of elements
             */
            inline size_t size() const
            {
                return m_size;
            }

            /**
             * @brief checks whether the container is empty
             * @return true if the container is empty, false otherwise
             */
            inline bool empty() const
            {
                return m_size == 0;
            }

            T &operator[] (size_t idx)
            {
                SPECTRE_ASSERT (idx < m_size, "Index out of bounds");

                return m_data[idx];
            }

            const T &operator[] (size_t idx) const
            {
                SPECTRE_ASSERT (idx < m_size, "Index out of bounds");

                return m_data[idx];
            }

            /**
             * @brief returns the underlying array
             * @return underlying array
             */
            inline T *data()
            {
                return m_data;
            }

            inline const T *data() const
            {
                return m_data;
            }

            /**
             * @brief returns an iterator to the beginning
             * @return iterator to the beginning
             */
            inline T *begin()
            {
                return m_data;
            }

            /**
             * @brief returns an iterator to the beginning
             * @return iterator to the beginning
             */
            inline const T *begin() const
            {
                return m_data;
            }


            /**
             * @brief returns an const_iterator to the beginning
             * @return const_iterator to the beginning
             */
            inline const T *cbegin() const
            {
                return m_data;
            }

            /**
             * @brief returns an iterator to the end
             * @return iterator to the end
             */
            inline T *end()
            {
                return m_data + m_size;
            }

            /**
             * @brief returns an iterator to the end
             * @return iterator to the end
             */
            inline const T *end() const
            {
                return m_data + m_size;
            }


            /**
             * @brief returns an const_iterator to the end
             * @return const_iterator to the end
             */
            inline const T *cend() const
            {
                return m_data + m_size;
            }

            /// Underlying type
            using value_type =  T;

            /// const iterator type
            using const_iterator = const T*;

            /// iterator type
            using iterator = T*;


        protected:
            T *m_data; ///< pointer to underlying array
            size_t m_size; ///< number of elements contained
        };

        /**
         * @brief Wrapper class around an AbstractField which makes it immutable
         */
        template <typename T>
        class ImmutableField final
        {
        public:
            explicit ImmutableField (const AbstractField<T> &other)
                : m_field (other.clone())
            {}

            ImmutableField (const ImmutableField<T> &other)
                : m_field (other.m_field->clone())
            {}

            /// Creates an invalid ImmutableField
            ImmutableField()
                : m_field (nullptr)
            {}

            ~ImmutableField()
            {
                details::callDelete (m_field);
            }

            ImmutableField<T> &operator= (const ImmutableField<T> &other)
            {
                callDelete (m_field);
                m_field = other.m_field->clone();
                return *this;
            }

            ImmutableField<T> &operator= (ImmutableField<T> &&other)
            {
                std::swap (m_field, other.m_field);
                return *this;
            }

            ImmutableField (ImmutableField<T> &&other)
            {
                m_field = other.m_field;
                other.m_field = nullptr;
            }

            inline const size_t size() const
            {
                return m_field->size();
            }

            /**
             * @brief checks whether the container is empty
             * @return true if the container is empty, false otherwise
             */
            inline bool empty() const
            {
                return m_field->empty();
            }

            const T &operator[] (size_t idx) const
            {
                return (*m_field) [idx];
            }

            /**
             * @brief returns the underlying array
             * @return underlying array
             */
            inline const T *data() const
            {
                return m_field->data();
            }

            /**
             * @brief returns an iterator to the beginning
             * @return iterator to the beginning
             */
            inline const T *begin() const
            {
                return m_field->begin();
            }

            /**
             * @brief returns an const_iterator to the beginning
             * @return const_iterator to the beginning
             */
            inline const T *cbegin() const
            {
                return m_field->cbegin();
            }

            /**
             * @brief returns an iterator to the end
             * @return iterator to the end
             */
            inline const T *end() const
            {
                return m_field->end();
            }


            /**
             * @brief returns an const_iterator to the end
             * @return const_iterator to the end
             */
            inline const T *cend() const
            {
                return m_field->cend();
            }

            /// Underlying type
            using value_type =  T;

            /// const iterator type
            using const_iterator = const T*;

        private:
            AbstractField<T> *m_field;
        };


        /**
         * @brief The ArrayReference class is an implementation of the AbstractField class that holds a reference to external memory
         */
        template <typename T>
        class ArrayReference : public AbstractField<T>
        {
        public:
            ArrayReference() : AbstractField<T> (nullptr, 0) {}

            /**
             * References memory passed by data.
             *
             * Note that ArrayReference is only valid as long as the referenced memory
             * is valid.
             *
             * @param data memory to be referenced
             * @param size number of elements in data
             */
            ArrayReference (T *data, size_t size) : AbstractField<T> (data, size) {}

            ArrayReference (const ArrayReference<T> &other) : AbstractField<T> (other.m_data, other.m_size) {}

            virtual ~ArrayReference() {}

            ArrayReference<T> &operator= (const ArrayReference<T> &other)
            {
                if (this != &other)
                {
                    this->m_data = other.m_data;
                    this->m_size = other.m_size;
                }
                return *this;
            }

            ArrayReference<T> *clone() const override
            {
                return details::callCopyCtor (*this);
            }

        };

        /**
         * @brief The ArrayHolder class is an implementation of the AbstractField class that holds and manages its own memory
         */
        template <typename T>
        class ArrayHolder : public AbstractField<T>
        {
        public:
            ArrayHolder() : AbstractField<T> (nullptr, 0) {}

            /**
             * Allocates memory of the given size
             *
             * @param size number of elements to allocate
             */
            explicit ArrayHolder (size_t size) : AbstractField<T> (nullptr, size)
            {
                if (size)
                {
                    this->m_data = allocArray<T> (size);
                }
            }

            /**
             * Allocates and copies memory passed by data.
             *
             * Note that ArrayHolder is only valid if the memory passed by data
             * is valid.
             *
             * @param data memory to be copied
             * @param size number of elements in data
             */
            ArrayHolder (const T *data, size_t size) : AbstractField<T> (nullptr, size)
            {
                if (size)
                {
                    this->m_data = allocArray<T> (this->m_size);
                    std::copy_n (PMD_CHECKED_ITERATOR (data, size), size, PMD_CHECKED_ITERATOR (this->m_data, size));
                }
            }

            ArrayHolder (const ArrayHolder<T> &other) : AbstractField<T> (nullptr, other.m_size)
            {
                if (this->m_size)
                {
                    this->m_data = allocArray<T> (this->m_size);
                    std::copy_n (PMD_CHECKED_ITERATOR (other.m_data, other.m_size), other.m_size, PMD_CHECKED_ITERATOR (this->m_data, other.m_size));
                }
            }

            ArrayHolder (ArrayHolder<T> &&other) : AbstractField<T> (nullptr, 0)
            {
                this->m_size = other.m_size;
                this->m_data = other.m_data;

                other.m_size = 0;
                other.m_data = nullptr;
            }

            ArrayHolder (std::initializer_list<T> init)
                : AbstractField<T> (allocArray<T> (init.size()), init.size())
            {
                if (this->m_size)
                {
                    std::copy (init.begin(), init.end(), PMD_CHECKED_ITERATOR (this->m_data, init.size()));
                }
            }

            virtual ~ArrayHolder()
            {
                freeArray (this->m_data);
            }

            ArrayHolder<T> &operator= (const ArrayHolder<T> &other)
            {
                if (this != &other)
                {
                    if (this->m_size != other.m_size)
                    {
                        this->m_size = other.m_size;
                        freeArray (this->m_data);
                        this->m_data = nullptr;
                        this->m_data = allocArray<T> (this->m_size);
                    }

                    if (this->m_size)
                    {
                        std::copy_n (PMD_CHECKED_ITERATOR (other.m_data, other.m_size), other.m_size, PMD_CHECKED_ITERATOR (this->m_data, other.m_size));
                    }
                }
                return *this;
            }

            ArrayHolder<T> &operator= (ArrayHolder<T> &&other)
            {
                if (this != &other)
                {
                    this->m_size = other.m_size;
                    freeArray (this->m_data);
                    this->m_data = other.m_data;

                    other.m_size = 0;
                    other.m_data = nullptr;
                }
                return *this;
            }

            ArrayHolder<T> *clone() const override
            {
                return details::callCopyCtor (*this); //TODO: undefined reference
            }
        };
    }
}
