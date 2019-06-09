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

#include <cstdint>
#include <vector>

#include <common/exceptions/OutOfBounds.hpp>

namespace royale
{
    namespace common
    {
        /** Read a 16-bit number from a big-endian buffer */
        inline uint16_t bufferToHostBe16 (const uint8_t *src)
        {
            return static_cast<uint16_t> (
                       (src[0] <<  8) |
                       (src[1] <<  0));
        }

        /** Read a 24-bit number from a big-endian buffer */
        inline uint32_t bufferToHostBe24 (const uint8_t *src)
        {
            return static_cast<uint32_t> (
                       (src[0] << 16) |
                       (src[1] <<  8) |
                       (src[2] <<  0));
        }

        /** Read a 32-bit number from a big-endian buffer */
        inline uint32_t bufferToHostBe32 (const uint8_t *src)
        {
            return static_cast<uint32_t> (
                       (src[0] << 24) |
                       (src[1] << 16) |
                       (src[2] <<  8) |
                       (src[3] <<  0));
        }

        /** Write a 32-bit number in host endian format in to the four bytes starting at the argument. The destination is little-endian */
        inline void hostToBuffer32 (uint8_t *dest, const uint32_t src)
        {
            dest[0] = static_cast<uint8_t> ( (src >>  0) & 0xff);
            dest[1] = static_cast<uint8_t> ( (src >>  8) & 0xff);
            dest[2] = static_cast<uint8_t> ( (src >> 16) & 0xff);
            dest[3] = static_cast<uint8_t> ( (src >> 24) & 0xff);
        }

        /** Read a 16-bit number from a little-endian buffer */
        inline uint16_t bufferToHost16 (const uint8_t *src)
        {
            return static_cast<uint16_t> (
                       (src[0] <<  0) |
                       (src[1] <<  8));
        }

        /** Read a 24-bit number from a little-endian buffer */
        inline uint32_t bufferToHost24 (const uint8_t *src)
        {
            return static_cast<uint32_t> (
                       (src[0] <<  0) |
                       (src[1] <<  8) |
                       (src[2] <<  16));
        }

        /** Read a 32-bit number from a little-endian buffer */
        inline uint32_t bufferToHost32 (const uint8_t *src)
        {
            return static_cast<uint32_t> (
                       (src[0] <<  0) |
                       (src[1] <<  8) |
                       (src[2] << 16) |
                       (src[3] << 24));
        }

        /**
         * Read an array of little-endian 16-bit data, T is expected to be royale::Vector<uint16_t>
         * or std::vector<uint16_t>, but vector<uint32_t> or vector<std::size_t> can also be used.
         */
        template<typename T>
        inline T bufferToHostVector16 (const uint8_t *src, const std::size_t count)
        {
            T ret;
            for (auto i = 0u; i < count; i++)
            {
                ret.emplace_back (bufferToHost16 (src + i * 2));
            }
            return ret;
        }

        /**
         * Read an array of little-endian 32-bit data, T is expected to be royale::Vector<uint32_t>
         * or std::vector<uint32_t>.
         */
        template<typename T>
        inline T bufferToHostVector32 (const uint8_t *src, const std::size_t count)
        {
            T ret;
            for (auto i = 0u; i < count; i++)
            {
                ret.emplace_back (bufferToHost32 (src + i * 4));
            }
            return ret;
        }

        /** Flip the byte order of a 32bit value */
        inline void flipByteOrder32 (uint32_t *val)
        {
            uint32_t temp = *val;
            uint8_t *ptrOut = reinterpret_cast<uint8_t *> (val);
            uint8_t *ptrIn = reinterpret_cast<uint8_t *> (&temp);
            ptrOut[0] = ptrIn[3];
            ptrOut[1] = ptrIn[2];
            ptrOut[2] = ptrIn[1];
            ptrOut[3] = ptrIn[0];
        }

        /** Append a 16-bit number to the back of a byte buffer, in big-endian format */
        inline void pushBackBe16 (std::vector<uint8_t> &buffer, const uint16_t src)
        {
            buffer.push_back (static_cast<uint8_t> ( (src >>  8) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >>  0) & 0xff));
        }

        /** Append a 16-bit number to the back of a byte buffer, in little-endian format */
        inline void pushBack16 (std::vector<uint8_t> &buffer, const uint16_t src)
        {
            buffer.push_back (static_cast<uint8_t> ( (src >>  0) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >>  8) & 0xff));
        }

        /** Append a 24-bit number to the back of a byte buffer, in big-endian format */
        inline void pushBackBe24 (std::vector<uint8_t> &buffer, const uint32_t src)
        {
            buffer.push_back (static_cast<uint8_t> ( (src >> 16) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >>  8) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >>  0) & 0xff));
            if (src >> 24 != 0)
            {
                throw OutOfBounds ("Trying to fit 32-bit data in to 24 bits");
            }
        }

        /** Append a 24-bit number to the back of a byte buffer, in little-endian format */
        inline void pushBack24 (std::vector<uint8_t> &buffer, const uint32_t src)
        {
            buffer.push_back (static_cast<uint8_t> ( (src >>  0) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >>  8) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >> 16) & 0xff));
            if (src >> 24 != 0)
            {
                throw OutOfBounds ("Trying to fit 32-bit data in to 24 bits");
            }
        }

        /** Append a 32-bit number to the back of a byte buffer, in little-endian format */
        inline void pushBack32 (std::vector<uint8_t> &buffer, const uint32_t src)
        {
            buffer.push_back (static_cast<uint8_t> ( (src >>  0) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >>  8) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >> 16) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >> 24) & 0xff));
        }

        /** Append a 32-bit number to the back of a byte buffer, in big-endian format */
        inline void pushBackBe32 (std::vector<uint8_t> &buffer, const uint32_t src)
        {
            buffer.push_back (static_cast<uint8_t> ( (src >> 24) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >> 16) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >>  8) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >>  0) & 0xff));
        }

        /** Append a 48-bit number to the back of a byte buffer, in big-endian format */
        inline void pushBackBe48 (std::vector<uint8_t> &buffer, const uint64_t src)
        {
            buffer.push_back (static_cast<uint8_t> ( (src >> 40) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >> 32) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >> 24) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >> 16) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >>  8) & 0xff));
            buffer.push_back (static_cast<uint8_t> ( (src >>  0) & 0xff));
        }

        /**
         * Given an iterable containing uint8_t elements (T is likely vector<uint8_t> or
         * string<uint8_t>), push back the contents of that iterable.
         */
        template <typename T, typename = std::enable_if<std::is_same<typename T::value_type, uint8_t>::value> >
        void pushBackIterable (std::vector<uint8_t> &dest, const T &src)
        {
            for (uint8_t x : src)
            {
                dest.push_back (x);
            }
        }

        /**
         * Given an iterable containing uint8_t elements (T is likely vector<uint8_t> or
         * string<uint8_t>), push back the contents of that iterable, and then pad with zeros (or
         * NULs) to the size given in the size argument.
         */
        template <typename T, typename = std::enable_if<std::is_same<typename T::value_type, uint8_t>::value> >
        void pushBackPadded (std::vector<uint8_t> &dest, const T &src, std::size_t size)
        {
            if (src.size() > size)
            {
                throw OutOfBounds ("Padding data to a length shorter than the original length");
            }

            pushBackIterable (dest, src);
            for (auto i = src.size(); i < size ; i++)
            {
                dest.push_back (0);
            }
        }

        /**
         * Given an iterable containing uint8_t elements (T is likely vector<uint8_t> or
         * string<uint8_t>), append pairs of elements to the uint16_t dest.
         *
         * If src contains an odd number of bytes, the result is the same as if
         * a zero-byte had been appended.
         */
        template <typename T, typename = std::enable_if<std::is_same<typename T::value_type, uint8_t>::value> >
        void pushBackIterableAsHighFirst16 (std::vector<uint16_t> &dest, const T &src)
        {
            typename std::remove_const<T>::type paddedSrc = src;
            if (paddedSrc.size() % 2)
            {
                paddedSrc.push_back (0);
            }

            for (auto i = 0u; i < paddedSrc.size(); i += 2)
            {
                dest.push_back (bufferToHostBe16 (paddedSrc.data() + i));
            }
        }
    }
}
