/****************************************************************************\
 * Copyright (C) 2016 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __COMPUTILS_HPP__
#define __COMPUTILS_HPP__

#include <iterator>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sstream>
#include <stdint.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace spectre
{
    inline float stof (const std::string &str)
    {
        char *end = nullptr;
        auto ret = std::strtod (str.c_str (), &end);
        if (!std::strncmp (str.c_str (), end, str.size ()))
        {
            throw std::invalid_argument (str);
        }
        return static_cast<float> (ret);
    }

    inline long stoi (const std::string &str)
    {
        char *end = nullptr;
        auto ret = std::strtol (str.c_str (), &end, 10);
        if (!std::strncmp (str.c_str (), end, str.size ()))
        {
            throw std::invalid_argument (str);
        }
        return ret;
    }

    /**
     * @brief Wrapper for strncpy
     *
     * Ensures that the string is always terminated, and works around
     * MSVC incompatible secure CRT implementation.
     *
     * @param dst destination buffer
     * @param src source buffer
     * @param dstSize size of destination
     */
    inline void strncpy (char *dst, const char *src, size_t dstSize)
    {
#ifdef _MSC_VER
        strncpy_s (dst, dstSize, src, _TRUNCATE);
#else
        ::strncpy (dst, src, dstSize);
        dst[dstSize - 1] = '\0';
#endif
    }

    /**
     * @brief Converts a value to a std::string
     *
     * @param value value to convert
     *
     * @return string representation of value
     */
    template<typename T>
    std::string to_string (T value)
    {
        std::ostringstream os;
        os << value;
        return os.str ();
    }

    /**
     * @brief Converts a value to a std::string
     *
     * @param value value to convert
     *
     * @return string representation of value
     */
    inline std::string to_string (uint8_t value)
    {
        std::ostringstream os;
        os << (unsigned)value;
        return os.str ();
    }

    /**
     * @brief Converts a value to a std::string
     *
     * @param value value to convert
     *
     * @return string representation of value
     */
    inline std::string to_string (int8_t value)
    {
        std::ostringstream os;
        os << (int)value;
        return os.str ();
    }

    /**
     * @brief Counts the leading zeros of a word
     *
     * A builtin is utilized if possible.
     *
     * @param x word
     *
     * @return leading zeros of word
     */
    inline int countLeadingZeros (unsigned x)
    {
#ifdef __GNUC__
        return __builtin_clz (x);
#elif _MSC_VER
        return __lzcnt (x);
#else
#error "No implementation for count_leading_zeros available"
#endif
    }

    /**
     * @brief Counts the leading zeros of an uint16_t
     *
     * @param x uint16_t
     *
     * @return leading zeros of x
     */
    inline int countLeadingZeros (uint16_t x)
    {
        return countLeadingZeros (static_cast<unsigned> (x)) -
               8 * (sizeof (unsigned) - sizeof (uint16_t));
    }

#if defined(_MSC_VER)
    inline bool hasAvx2SupportMsc ()
    {
        int data[4];
        __cpuid (data, 0);
        auto num = data[0];
        if (num >= 7)
        {
            __cpuidex (data, 7, 0);
            auto supportsAvx2 = (data[1] & 0x20);
            return static_cast<bool> (supportsAvx2);
        }

        return false;
    }
#endif

#if defined(__x86_64__) || defined(_MSC_VER)

    /**
     * @brief Checks if the CPU supports AVX2 instructions
     *
     * @return true if AVX2 is supported
     */
    inline bool hasAvx2Support ()
    {
#if defined(__GNUC__)
        static const bool avx2Support = __builtin_cpu_supports ("avx2");
#elif defined(_MSC_VER) && defined(_WIN64)
        static const bool avx2Support = hasAvx2SupportMsc ();
#else
        static const bool avx2Support = false;
#endif
        return avx2Support;
    }
#else
    inline bool hasAvx2Support ()
    {
        return false;
    }
#endif
} // namespace spectre

#ifdef _MSC_VER
namespace spectre
{
    /**
     * @brief Gets a checked iterator
     *
     * @param x output iterator (or pointer)
     * @param n number of elements in x
     *
     * @return checked iterator
     */
    template<typename T>
    inline stdext::checked_array_iterator<T> getCheckedArrayIterator (T x,
                                                                      size_t n)
    {
        return stdext::checked_array_iterator<T> (x, n);
    }
} // namespace spectre
/**
 * @brief Gets a checked iterator or a normal iterator
 *
 * @param X pointer to first element (output iterator)
 * @param N number of elements
 *
 * @return checked iterator (or X itself)
 */
#define PMD_CHECKED_ITERATOR(X, N) spectre::getCheckedArrayIterator (X, N)
#else
/**
 * @brief Gets a checked iterator or a normal iterator
 *
 * @param X pointer to first element (output iterator)
 * @param N number of elements
 *
 * @return checked iterator (or X itself)
 */
#define PMD_CHECKED_ITERATOR(X, N) (X)
#endif

#ifdef _MSC_VER
#define SPECTRE_TEMPLATE_PREFIX
#else
#define SPECTRE_TEMPLATE_PREFIX template
#endif

#ifndef _MSC_VER
#define SPECTRE_STRCPY(DEST, SIZE, SRC) strcpy (DEST, SRC);
#else
#define SPECTRE_STRCPY(DEST, SIZE, SRC) strcpy_s (DEST, SIZE, SRC);
#endif

#endif /*__COMPUTILS_HPP__*/
