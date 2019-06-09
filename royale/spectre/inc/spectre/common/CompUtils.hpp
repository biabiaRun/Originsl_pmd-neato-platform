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

namespace spectre
{
    inline float stof (const std::string &str)
    {
        char *end = nullptr;
        auto ret = std::strtod (str.c_str(), &end);
        if (!std::strncmp (str.c_str(), end, str.size()))
        {
            throw std::invalid_argument (str);
        }
        return static_cast<float> (ret);
    }

    inline long stoi (const std::string &str)
    {
        char *end = nullptr;
        auto ret = std::strtol (str.c_str(), &end, 10);
        if (!std::strncmp (str.c_str(), end, str.size()))
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
    template <typename T>
    std::string to_string (T value)
    {
        std::ostringstream os;
        os << value;
        return os.str();
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
        os << (unsigned) value;
        return os.str();
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
        os << (int) value;
        return os.str();
    }
}


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
    inline stdext::checked_array_iterator<T> getCheckedArrayIterator (T x, size_t n)
    {
        return stdext::checked_array_iterator<T> (x, n);
    }
}
/**
 * @brief Gets a checked iterator or a normal iterator
 *
 * @param X pointer to first element (output iterator)
 * @param N number of elements
 *
 * @return checked iterator (or X itself)
 */
#define PMD_CHECKED_ITERATOR(X,N) spectre::getCheckedArrayIterator(X, N)
#else
/**
 * @brief Gets a checked iterator or a normal iterator
 *
 * @param X pointer to first element (output iterator)
 * @param N number of elements
 *
 * @return checked iterator (or X itself)
 */
#define PMD_CHECKED_ITERATOR(X,N) (X)
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
