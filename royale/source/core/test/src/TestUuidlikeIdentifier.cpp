/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <common/UuidlikeIdentifier.hpp>

#include <functional>

using namespace royale;
using namespace royale::common;

namespace
{
    class UuidSubclass : public royale::common::UuidlikeIdentifier<UuidSubclass>
    {
    public:
        // \todo ROYAL-3355 all of the constructors could be added with just
        // using UuidlikeIdentifier<UuidSubclass>::UuidlikeIdentifier;
        // but only when we drop support for older compilers

        UuidSubclass () :
            UuidlikeIdentifier<UuidSubclass> ()
        {
        }

        UuidSubclass (const datatype &data) :
            UuidlikeIdentifier<UuidSubclass> (data)
        {
        }

        UuidSubclass (const royale::String &s) :
            UuidlikeIdentifier<UuidSubclass> (s)
        {
        }
    };

    const auto idA = royale::String {"{69F0E9EA-0DFF-47C9-8311-E236F35AB6FF}"};
    const auto idB = royale::String {"{40864D9B-F844-4CCB-9975-8CF820292B03}"};

    const auto arrayA = std::array<uint8_t, 16>
                        {
                            {
                                0x69, 0xF0, 0xE9, 0xEA, 0x0D, 0xFF, 0x47, 0xC9, 0x83, 0x11, 0xE2, 0x36, 0xF3, 0x5A, 0xB6, 0xFF
                            }
                        };
}

/**
 * Checking whether an identifier is the default value.
 *
 * Also check that "UuidSubclass {}" compiles, that it isn't an ambiguous statement.
 */
TEST (TestUuidlikeIdentifier, DefaultValue)
{
    ASSERT_NO_THROW (UuidSubclass {});
    ASSERT_EQ (UuidSubclass {}, UuidSubclass {});
    ASSERT_EQ (UuidSubclass {}, (UuidSubclass {std::array<uint8_t, 16> {}}));
    ASSERT_NE (UuidSubclass {}, UuidSubclass::parseRfc4122 (idA));
    ASSERT_TRUE (UuidSubclass {} .isSentinel());
}

/**
 * Test that two identifiers created from the same string are equal,
 * and two identifiers constructed from different strings are not-equal.
 */
TEST (TestUuidlikeIdentifier, ConstructFromRfc4122String)
{
    ASSERT_NO_THROW (UuidSubclass::parseRfc4122 (idA));
    ASSERT_NO_THROW (UuidSubclass::parseRfc4122 (idB));
    ASSERT_EQ (UuidSubclass::parseRfc4122 (idA), UuidSubclass::parseRfc4122 (idA));
    ASSERT_EQ (UuidSubclass::parseRfc4122 (idB), UuidSubclass::parseRfc4122 (idB));
    ASSERT_NE (UuidSubclass::parseRfc4122 (idA), UuidSubclass::parseRfc4122 (idB));

    ASSERT_ANY_THROW (UuidSubclass::parseRfc4122 ("This doesn't look like a GUID"));
    ASSERT_ANY_THROW (UuidSubclass::parseRfc4122 ("This string doesn't look like a GUID, and is longer than a GUID is expected to be"));
}

/**
 * Check that the fallback handles arbitrary strings.
 */
TEST (TestUuidlikeIdentifier, ConstructFromNonGuidString)
{
    ASSERT_NO_THROW (UuidSubclass {"a"});
    ASSERT_NE (UuidSubclass{}, UuidSubclass {"a"});
    ASSERT_NO_THROW (UuidSubclass {"This doesn't look like a GUID"});
    ASSERT_NE (UuidSubclass{}, UuidSubclass {"This doesn't look like a GUID"});

    const auto longString = royale::String {"This string doesn't look like a GUID, and is longer than a GUID is expected to be"};
    ASSERT_NO_THROW (UuidSubclass {longString});
    ASSERT_NE (UuidSubclass{}, UuidSubclass {longString});
    ASSERT_NE (UuidSubclass{longString}, UuidSubclass {longString + "2"});
}

/**
 * From Royale 3.20, the exact behavior of the fallback is documented, and must
 * return the expected values.
 */
TEST (TestUuidlikeIdentifier, ExpectedValuesFromNonGuidString)
{
    ASSERT_EQ (UuidSubclass::parseRfc4122 ("{61000000-0000-0000-0000-000043beb7e8}"), UuidSubclass ("a"));
    ASSERT_EQ (UuidSubclass::parseRfc4122 ("{6578616d-706c-6520-3520-70686485d74a}"), UuidSubclass ("example 5 phase"));
    ASSERT_EQ (UuidSubclass::parseRfc4122 ("{6578616d-706c-6520-3920-706810ef778d}"), UuidSubclass ("example 9 phase"));
    ASSERT_EQ (UuidSubclass::parseRfc4122 ("{54686973-2064-6f65-736e-2774212a3da4}"), UuidSubclass ("This doesn't look like a GUID"));
    ASSERT_EQ (UuidSubclass::parseRfc4122 ("{54686973-2073-7472-696e-67209920adb6}"), UuidSubclass ("This string doesn't look like a GUID, and is longer than a GUID is expected to be"));
}

TEST (TestUuidlikeIdentifier, FromStringMatchesFromInts)
{
    ASSERT_EQ (UuidSubclass::parseRfc4122 (idA), UuidSubclass {arrayA});
    ASSERT_EQ (arrayA, UuidSubclass::parseRfc4122 (idA) .data());
}

/**
 * Test that ids can be stable-sorted when put in a list, and that std::less is equivalent to the
 * less-than operator.
 */
TEST (TestUuidlikeIdentifier, Inequalities)
{
    const auto a = UuidSubclass::parseRfc4122 (idA);
    const auto b = UuidSubclass::parseRfc4122 (idB);

    ASSERT_FALSE (a < a);
    ASSERT_FALSE (std::less<UuidSubclass>() (a, a));

    if (a < b)
    {
        ASSERT_TRUE (std::less<UuidSubclass>() (a, b));
        ASSERT_LT (a, b);
        ASSERT_FALSE (std::less<UuidSubclass>() (b, a));
        ASSERT_FALSE (b < a);
    }
    else
    {
        ASSERT_TRUE (std::less<UuidSubclass>() (b, a));
        ASSERT_LT (b, a);
        ASSERT_FALSE (std::less<UuidSubclass>() (a, b));
        ASSERT_FALSE (a < b);
    }
}
