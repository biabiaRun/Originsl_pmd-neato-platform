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

#if __cplusplus >= 201402L
        using std::make_unique;
#else
        /**
         * @brief Analog implementation of std::make_shared
         *
         * @param args arguments
         *
         * @return unique_ptr
         */
        template<typename T, typename... Args>
        typename std::enable_if<!std::is_array<T>::value,
                                std::unique_ptr<T>>::type
        make_unique (Args &&... args)
        {
            return std::unique_ptr<T> (new T (std::forward<Args> (args)...));
        }

        /** @brief Allows 'make_unique<T[]>(10)'. (N3690 s20.9.1.4 p3-4) */
        template<typename T>
        typename std::enable_if<std::is_array<T>::value,
                                std::unique_ptr<T>>::type
        make_unique (const size_t n)
        {
            return std::unique_ptr<T> (
                new typename std::remove_extent<T>::type[n]());
        }

        /** @brief Disallows 'make_unique<T[10]>()'. (N3690 s20.9.1.4 p5) */
        template<typename T, typename... Args>
        typename std::enable_if<std::extent<T>::value != 0,
                                std::unique_ptr<T>>::type
        make_unique (Args &&...) = delete;
#endif
        /**
         * @brief Overload for macros where no message should be printed
         */
        void printMessage ();

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
            return sstr.str ();
        }

        template<typename... T>
        auto make_array (T &&... t)
            -> std::array<typename std::common_type<T...>::type, sizeof...(T)>
        {
            return {{std::forward<T> (t)...}};
        }

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

    } // namespace common
} // namespace spectre

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
#define SPECTRE_ASSERT(x, ...)                                                 \
    do                                                                         \
    {                                                                          \
        bool res = x;                                                          \
        if (!res)                                                              \
        {                                                                      \
            spectre::common::printMessage (__VA_ARGS__);                       \
            std::abort ();                                                     \
        }                                                                      \
    } while (0)
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
#define SPECTRE_ASSERT(x, ...)
#endif

#endif /*__UTILS_HPP__*/
