/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/String.hpp>
#include <array>
#include <cstdint>

namespace royale
{
    namespace common
    {
        /**
         * Given a string in the specific UUID format used in Lena, (which is one of several string
         * formats defined in RFC4122), will construct the corresponding 16-byte binary data.
         *
         * This will throw if it doesn't parse it.
         */
        ROYALE_API std::array<uint8_t, 16> parseRfc4122AsUuid (const royale::String &s);

        /**
         * Given any string, will construct a 16-byte binary blob, as per the documentation of
         * UuidlikeIdentifier's from-string constructor.
         *
         * This function will not throw (except for conditions such as std::bad_alloc).
         */
        ROYALE_API std::array<uint8_t, 16> hashUuidlikeIdentifier (const royale::String &s);

        /**
         * This holds a UUID / GUID, or a similar object that can be used as a key for tables of use
         * cases, processing parameters, etc.
         *
         * It should be subclassed using the Curiously Recurring Template Pattern, so that the
         * compiler will warn if code tries to use a UseCaseIdentifier as a ProcessingParameterId,
         * etc.
         */
        template <typename T>
        class UuidlikeIdentifier
        {
        public:
            using datatype = std::array<uint8_t, 16>;

            /**
             * Returns a sentinel value, which should be used to indicate that an item doesn't have
             * a unique identifier assigned.
             */
            ROYALE_API UuidlikeIdentifier<T> () :
                m_data{}
            {
            }

            ROYALE_API UuidlikeIdentifier<T> (const datatype &data) :
                m_data (data)
            {
            }

            ROYALE_API explicit UuidlikeIdentifier<T> (const UuidlikeIdentifier<T> &other) :
                m_data (other.m_data)
            {
            }

            /**
             * Given any string, will construct a UUID-like identifier. The implementation is a hash
             * function, for which the behavior will be the same in future versions of Royale.
             *
             * Here, "uuidlike" means that it's a 16-byte binary blob, which can hold a UUID.  The
             * requirements of Royale are lower than general universal-uniqueness, and this
             * from-string constructor uses the CRC32 implementation which is already in Royale,
             * rather than the SHA1 recommended for version 5 UUIDs.
             *
             * The 16 bytes used are the first 12 characters of the string (padded with zeros if
             * required), followed by the CRC32 of the full string.
             *
             * This function will not throw (except for conditions such as std::bad_alloc).
             *
             * For the hash-method what is guaranteed is that, if two strings are equal, then they
             * will generate the same UuidlikeIdentifier; and two strings that aren't equal will
             * probably generate different UuidlikeIdentifier.
             *
             * Note that the hashing algorithm does not try to check for RFC4122's syntaxes, and the
             * return value will be different to that returned by parseRfc4122AsUuid().
             */
            ROYALE_API UuidlikeIdentifier<T> (const royale::String &s) :
                m_data (hashUuidlikeIdentifier (s))
            {
            }

            /**
             * Given a string containing precisely the RFC4122 format used in Lena, construct a
             * UuidlikeIdentifier. This will create a object with the corresponding UUID, or throw
             * if it can't parse it correctly.
             */
            ROYALE_API static T parseRfc4122 (const royale::String &s)
            {
                return T (parseRfc4122AsUuid (s));
            }

            ROYALE_API UuidlikeIdentifier<T> &operator= (const UuidlikeIdentifier<T> &rhs)
            {
                m_data = rhs.m_data;
                return *this;
            }

            ROYALE_API bool operator== (const UuidlikeIdentifier<T> &rhs) const
            {
                return m_data == rhs.m_data;
            }

            ROYALE_API bool operator!= (const UuidlikeIdentifier<T> &rhs) const
            {
                return m_data != rhs.m_data;
            }

            ROYALE_API bool operator< (const UuidlikeIdentifier<T> &rhs) const
            {
                return m_data < rhs.m_data;
            }

            /**
             * Returns a copy of the data.
             *
             * This matches the `(const datatype &data)` constructor, in both of these round-trip examples B == A:
             *
             * * datatype A = ...;
             * * datatype B = (UuidlikeIdentifier (A)).data();
             *
             * and
             *
             * * UuidlikeIdentifier A = ...;
             * * UuidlikeIdentifier B = UuidlikeIdentifier (A.data());
             */
            ROYALE_API const datatype &data() const
            {
                return m_data;
            }

            /**
             * Returns true if this UUID holds the sentinel value that's used by the no-argument
             * constructor.
             */
            ROYALE_API bool isSentinel() const
            {
                return *this == UuidlikeIdentifier<T> {};
            }

        private:
            datatype m_data;
        };
    }
}
