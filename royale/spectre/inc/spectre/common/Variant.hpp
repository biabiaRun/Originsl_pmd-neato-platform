/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/


#ifndef __VARIANT_HPP__
#define __VARIANT_HPP__

#include <cstddef>
#include <iostream>
#include "CompUtils.hpp"
#include "TypeList.hpp"
#include "ArrayAllocator.hpp"

namespace spectre
{
    namespace common
    {

        // Implementation details which are not considered as part of the public API
        namespace details
        {
            template<typename T1, typename T2, typename... Tn>
            struct get_typeid
            {
                static const unsigned value = std::is_same<T1, T2>::value ? 0 : 1 + get_typeid<T1, Tn...>::value;
            };

            template<typename T1, typename T2>
            struct get_typeid<T1, T2>
            {
                static const unsigned value = std::is_same<T1, T2>::value ? 0 : 1;
            };

            template<unsigned N, typename T, typename ...Tn>
            struct apply_visitor
            {
                template<typename V>
                void operator() (unsigned typeId, V &visitor)
                {
                    if (get_typeid<T, T, Tn...>::value + N == typeId)
                    {
                        visitor.SPECTRE_TEMPLATE_PREFIX operator() <T>();
                    }
                    else
                    {
                        apply_visitor < N + 1, Tn... > () (typeId, visitor);
                    }
                }
            };

            template<unsigned N, typename T>
            struct apply_visitor<N, T>
            {
                template<typename V>
                void operator() (int typeId, V &visitor)
                {
                    if (get_typeid<T, T>::value + N == typeId)
                    {
                        visitor.SPECTRE_TEMPLATE_PREFIX operator() <T>();
                    }
                }
            };
        }

        /**
         * @brief A Variant<T1, T2, ..., Tn> stores a value of a type Ti, i = 1, ..., n.
         *
         * The Variant class is type-safe in the sense, that it is possible to store a value
         * for any type Ti, i = 1, ..., n, but it is only possible to call Variant::get<Ti>()
         * for the type Ti successfully.
         *
         * The Variant class requires that each type Ti, i = 1, ... ,n is copy constructable and
         * assignable. Additionally, it is assumed that neither the destructor, the copy constructor
         * or the assigment operator for any type Ti throws.
         */
        template <typename... Args>
        class Variant
        {
        public:
            /**
             * Constructs a new variant
             *
             * The Variant is marked as invalid, and it is not possible to call get.
             */
            Variant()
                : m_holder (allocArray<char> (get_size<Args ...>::value)), m_typeId (sizeof... (Args))
            { }

            /// Destructor
            ~Variant()
            {
                destruct();
                freeArray (m_holder);
            }

            /**
             * @brief Constructs a new Variant, and copies the value from other
             *
             * @param other other variant
             */
            Variant (const Variant<Args...> &other)
                : Variant()
            {
                if (this != &other)
                {
                    *this = other;
                }
            }

            /**
             * @brief Assigns the value of other to this Variant
             *
             * @param other other Variant
             *
             * @return *this
             */
            Variant<Args...> &operator= (const Variant<Args...> &other)
            {
                destruct();
                m_typeId = other.m_typeId;
                setter set = { m_holder, other.m_holder };
                applyVisitor (set);
                return *this;
            }

            /**
             * @brief Ctor which constructs a variant from a passed value
             *
             * @param val value to assign after construction
             */
            template < typename T, typename U = typename std::enable_if< (details::get_typeid<T, Args...>::value < sizeof... (Args)) >::type >
            explicit Variant (const T &val)
                : Variant()
            {
                set (val);
            }

            /**
             * @brief Sets the value of the Variant
             *
             *
             * @param val value to set
             */
            template < typename T, typename U = typename std::enable_if< (details::get_typeid<T, Args...>::value < sizeof... (Args)) >::type >
            void set (const T &val)
            {
                destruct();
                auto id = details::get_typeid<T, Args...>::value;
                m_typeId = id;
                m_holder = reinterpret_cast<char *> (new (m_holder) T (val));
            }

            /**
             * @brief Gets the value stored in the Variant
             *
             * The function sets the value of the passed reference val to the value of the stored Variant, if the type of
             * val is equal to the type of the stored value. If this not the case, the function will return false, and not
             * alter the value of val.
             *
             * @param val target where to the value
             *
             * @return true if the getter succeeded, false otherwise
             */
            template<typename T, typename U = typename std::enable_if<true>::type >
            bool get (T &val) const
            {
                if (details::get_typeid<T, Args...>::value == m_typeId)
                {
                    val = *reinterpret_cast<const T *> (m_holder);
                    return true;
                }

                return false;
            }

            /**
             * @brief Gets the type id of the value stored
             *
             * For a Variant<T0, ..., Tn-1> is the type id the lowest index i
             * so that Ti equals to the type of the stored value. If i is >= n,
             * then the Variant does not hold any value.
             *
             * @return type id
             */
            unsigned typeId() const
            {
                return m_typeId;
            }

            /**
             * @brief Checks if the Variant holds any value
             *
             *
             * @return true if the Variant holds a value, false otherwise.
             */
            bool empty() const
            {
                return !isIdValid (m_typeId);
            }

            /**
             * @brief Equal operator for two Variants
             *
             * Two Variants are considered as equal if they contain the same type,
             * and if the equal operator for the values stored in both Variants return true.
             *
             * @param other variant to compare
             *
             * @return true if they are equal, false otherwise
             */
            bool operator== (const Variant<Args...> &other) const
            {
                if (m_typeId == other.m_typeId)
                {
                    if (empty())
                    {
                        return true;
                    }

                    comperatorEq cmp{ m_holder, other.m_holder, false };
                    applyVisitor (cmp);
                    return cmp.res;
                }

                return false;
            }

            /**
             * @brief Less operator for two Variants (used for mapping)
             *
             * One Variant is considered smaller then the other Variant if they contain
             * the same type, and if the less operator for the values stored in both
             * Variants returns true. If they contain different types however, their
             * typeIds will be compared.
             *
             * @param other variant to compare
             *
             * @return true if the left handed Variant is smaller then the right handed
             * Variant, false otherwise
             */
            bool operator< (const Variant<Args...> &other) const
            {
                if (m_typeId != other.m_typeId)
                {
                    return m_typeId < other.m_typeId;
                }

                if (empty())
                {
                    return true;
                }

                comperatorLess cmp{ m_holder, other.m_holder, false };
                applyVisitor (cmp);
                return cmp.res;
            }

            /**
             * @brief Prints the Variant in human-readable format to a stream
             *
             * @param os output stream
             */
            void print (std::ostream &os) const
            {
                printer prn { os, m_holder };
                applyVisitor (prn);
            }

            /// Possible value types of this Variant
            using value_types = TypeList<Args...>;

        private:
            template<typename T, typename ... Tn>
            struct get_size
            {
                static const size_t value = sizeof (T) < get_size<Tn...>::value ? get_size<Tn...>::value : sizeof (T);
                                                                                          };

            template<typename T>
            struct get_size<T>
            {
                static const size_t value = sizeof (T);
            };

            struct destructor
            {
                template<typename T>
                void operator() ()
                {
                    reinterpret_cast<T *> (holder)->~T();
                }
                char *holder;
            };

            struct setter
            {

                template<typename T>
                void operator() ()
                {
                    holder = reinterpret_cast<char *> (new (holder) T (*reinterpret_cast<const T *> (origin)));
                }

                char *holder;
                const char *origin;
            };

            struct comperatorEq
            {
                template<typename T>
                void operator() ()
                {
                    res = *reinterpret_cast<const T *> (lhs) == *reinterpret_cast<const T *> (rhs);
                }

                const char *lhs;
                const char *rhs;
                bool res;
            };

            struct comperatorLess
            {
                template<typename T>
                void operator() ()
                {
                    res = *reinterpret_cast<const T *> (lhs) < *reinterpret_cast<const T *> (rhs);
                }

                const char *lhs;
                const char *rhs;
                bool res;
            };

            struct printer
            {
                template<typename T>
                void operator() ()
                {
                    os << *reinterpret_cast<const T *> (holder);
                }

                std::ostream &os;
                const char *holder;
            };

            template<typename V>
            void applyVisitor (V &visitor)
            {
                details::apply_visitor<0, Args...>() (m_typeId, visitor);
            }

            template<typename V>
            void applyVisitor (V &visitor) const
            {
                details::apply_visitor<0, Args...>() (m_typeId, visitor);
            }

            static bool isIdValid (unsigned typeId)
            {
                return typeId < sizeof... (Args);
            }

            void destruct()
            {
                if (isIdValid (m_typeId))
                {
                    destructor dest = { m_holder };
                    applyVisitor (dest);
                }
            }

        private:
            char *m_holder;
            unsigned m_typeId;
        };

        template<>
        class Variant<> {};

        /**
         * @brief Prints a Variant in human-readble format
         *
         * @param v Variant to print
         * @param os output stream
         *
         * @return os
         */
        template<typename ...Args>
        std::ostream &operator<< (std::ostream &os, const Variant<Args...> &v)
        {
            if (v.empty())
            {
                os << "! Empty Variant !";
            }
            else
            {
                v.print (os);
            }
            return os;
        }
    }  // common

}  // spectre

#endif /*__VARIANT_HPP|SPECTRE__*/
