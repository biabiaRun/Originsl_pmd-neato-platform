/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <sstream>
#include <memory>
#include <string>
#include <cstdlib>
#include <array>

namespace spectre
{
    namespace common
    {
        /**
         * @brief Analog implementation of std::make_shared
         *
         * @param args arguments
         *
         * @return unique_ptr
         */
        template<typename T, typename ...Args>
        std::unique_ptr<T> make_unique (Args &&...args)
        {
            return std::unique_ptr<T> (new T (std::forward<Args> (args)...));
        }

        /**
         * @brief Overload for macros where no message should be printed
         */
        void printMessage();

        /**
         * @brief Prints the message to stderr using the given format string
         *
         * @param fmt format string
         */
        void printMessage (const char *fmt, ...);

        /**
         * @brief Converts a value to a std::string
         *
         * @param t value to convert
         *
         * @return string representation
         */
        template<typename T>
        std::string toString (const T &t)
        {
            std::stringstream sstr;
            sstr << t;
            return sstr.str();
        }

// TODO SK: Remove VS workaround after switch to >= VS 2015 (PP-1030)
#ifdef _MSC_VER
        template<typename T>
        std::array<T, 1> make_array (T &&k1)
        {
            return {{k1}};
        }

        template<typename T>
        std::array<T, 2> make_array (T &&k1, T &&k2)
        {
            return {{k1, k2}};
        }

        template<typename T>
        std::array<T, 3> make_array (T &&k1, T &&k2, T &&k3)
        {
            return{ { k1, k2, k3 } };
        }

        template<typename T>
        std::array<T, 4> make_array (T &&k1, T &&k2, T &&k3, T &&k4)
        {
            return{ { k1, k2, k3, k4 } };
        }

        template<typename T>
        std::array<T, 5> make_array (T &&k1, T &&k2, T &&k3, T &&k4, T &&k5)
        {
            return{ { k1, k2, k3, k4, k5 } };
        }

        template<typename T>
        std::array<T, 6> make_array (T &&k1, T &&k2, T &&k3, T &&k4, T &&k5, T &&k6)
        {
            return{ { k1, k2, k3, k4, k5, k6 } };
        }

        template<typename T>
        std::array<T, 7> make_array (T &&k1, T &&k2, T &&k3, T &&k4, T &&k5, T &&k6, T &&k7)
        {
            return{ { k1, k2, k3, k4, k5, k6, k7 } };
        }

        template<typename T>
        std::array<T, 8> make_array (T &&k1, T &&k2, T &&k3, T &&k4, T &&k5, T &&k6, T &&k7, T &&k8)
        {
            return{ { k1, k2, k3, k4, k5, k6, k7, k8 } };
        }

        template<typename T>
        std::array<T, 9> make_array (T &&k1, T &&k2, T &&k3, T &&k4, T &&k5, T &&k6, T &&k7, T &&k8, T &&k9)
        {
            return{ { k1, k2, k3, k4, k5, k6, k7, k8, k9 } };
        }

        template<typename T>
        std::array<T, 10> make_array (T &&k1, T &&k2, T &&k3, T &&k4, T &&k5, T &&k6, T &&k7, T &&k8, T &&k9, T &&k10)
        {
            return{ { k1, k2, k3, k4, k5, k6, k7, k8, k9, k10 } };
        }

#else
        template<typename ...T>
        auto make_array (T &&...t) -> std::array<typename std::common_type<T...>::type, sizeof... (T) >
        {
            return {{std::forward<T> (t)...}};
        }
#endif
        /**
         * @brief Checks if an element is contained in a range
         *
         * This function uses operator== of T.
         *
         * @param first iterator pointing to begin of the range
         * @param last iterator pointing behind the range
         * @param val element to check for
         *
         * @return true if the element is found, false otherwise
         */
        template<typename InIt, typename T>
        bool contains (InIt first, const InIt &last, const T &val)
        {
            for (; first != last; ++first)
            {
                if (*first == val)
                {
                    return true;
                }
            }

            return false;
        }

    } // common
}  // spectre

#ifdef SPECTRE_ENABLE_ADDITIONAL_CHECKS
/**
 * Asserts that the passed condition is true
 *
 * If SPECTRE_ENABLE_ADDITIONAL_CHECKS is defined
 * the assertion will terminate the program if the condition is false.
 * Otherwise the condition will not be evaluation
 *
 * The optional parameters will be forwarded to a printf
 * like function, and printed to stderr in case of a failing assertation.
 *
 * @param x condition
 */
#define SPECTRE_ASSERT(x,...) do {                              \
        bool res = x;                                           \
        if (!res) {                                             \
            spectre::common::printMessage (__VA_ARGS__);        \
            std::abort();                                       \
        }                                                       \
    }                                                           \
    while(0)
#else
/**
 * Asserts that the passed condition is true
 *
 * If SPECTRE_ENABLE_ADDITIONAL_CHECKS is defined
 * the assertion will terminate the program if the condition is false.
 * Otherwise the condition will not be evaluation
 *
 * The optional parameters will be forwarded to a printf
 * like function, and printed to stderr in case of a failing assertation.
 *
 * @param x condition
 */
#define SPECTRE_ASSERT(x,...)
#endif

#endif /*__UTILS_HPP__*/
