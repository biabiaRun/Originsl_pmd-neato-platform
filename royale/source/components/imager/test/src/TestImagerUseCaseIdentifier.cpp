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
#include <imager/ImagerUseCaseIdentifier.hpp>
#include <usecase/UseCaseIdentifier.hpp>

#include <functional>

using namespace royale;
using namespace royale::imager;

namespace
{
    const auto idA = std::string {"{69F0E9EA-0DFF-47C9-8311-E236F35AB6FF}"};
    const auto idB = std::string {"{40864D9B-F844-4CCB-9975-8CF820292B03}"};
}

/**
 * Checking whether an identifier is the default value.
 *
 * Also check that "ImagerUseCaseIdentifier {}" compiles, that it isn't an ambiguous statement.
 */
TEST (TestImagerUseCaseIdentifier, DefaultValue)
{
    ASSERT_NO_THROW (ImagerUseCaseIdentifier {});
    ASSERT_EQ (ImagerUseCaseIdentifier {}, ImagerUseCaseIdentifier {});
    ASSERT_EQ (ImagerUseCaseIdentifier {}, (ImagerUseCaseIdentifier {std::array<uint8_t, 16> {}}));
    ASSERT_NE (ImagerUseCaseIdentifier {}, ImagerUseCaseIdentifier {idA});
}

/**
 * Test that two identifiers created from the same string are equal,
 * and two identifiers constructed from different strings are not-equal.
 */
TEST (TestImagerUseCaseIdentifier, ConstructFromString)
{
    ASSERT_NO_THROW (ImagerUseCaseIdentifier {idA});
    ASSERT_NO_THROW (ImagerUseCaseIdentifier {idB});
    ASSERT_EQ (ImagerUseCaseIdentifier {idA}, ImagerUseCaseIdentifier {idA});
    ASSERT_EQ (ImagerUseCaseIdentifier {idB}, ImagerUseCaseIdentifier {idB});
    ASSERT_NE (ImagerUseCaseIdentifier {idA}, ImagerUseCaseIdentifier {idB});
}

/**
 * Test equality and inequality when the string identifier was converted to a
 * royale::usecase::UseCaseIdentifier first.
 */
TEST (TestImagerUseCaseIdentifier, ConstructFromUseCaseIdentifier)
{
    using namespace royale::usecase;

    ASSERT_EQ (ImagerUseCaseIdentifier {idA}, toImagerUseCaseIdentifier (UseCaseIdentifier {idA}));
    ASSERT_EQ (ImagerUseCaseIdentifier {idB}, toImagerUseCaseIdentifier (UseCaseIdentifier {idB}));
    ASSERT_NE (ImagerUseCaseIdentifier {idA}, toImagerUseCaseIdentifier (UseCaseIdentifier {idB}));
}

/**
 * Test that ids can be stable-sorted when put in a list, and that std::less is equivalent to the
 * less-than operator.
 */
TEST (TestImagerUseCaseIdentifier, Inequalities)
{
    ASSERT_FALSE (ImagerUseCaseIdentifier {idA} < ImagerUseCaseIdentifier {idA});
    ASSERT_FALSE (std::less<ImagerUseCaseIdentifier>() (ImagerUseCaseIdentifier {idA}, ImagerUseCaseIdentifier {idA}));

    if (ImagerUseCaseIdentifier {idA} < ImagerUseCaseIdentifier {idB})
    {
        ASSERT_TRUE (std::less<ImagerUseCaseIdentifier>() (ImagerUseCaseIdentifier {idA}, ImagerUseCaseIdentifier {idB}));
        ASSERT_LT (ImagerUseCaseIdentifier {idA}, ImagerUseCaseIdentifier {idB});
        ASSERT_FALSE (std::less<ImagerUseCaseIdentifier>() (ImagerUseCaseIdentifier {idB}, ImagerUseCaseIdentifier {idA}));
        ASSERT_FALSE (ImagerUseCaseIdentifier {idB} < ImagerUseCaseIdentifier {idA});
    }
    else
    {
        ASSERT_TRUE (std::less<ImagerUseCaseIdentifier>() (ImagerUseCaseIdentifier {idB}, ImagerUseCaseIdentifier {idA}));
        ASSERT_LT (ImagerUseCaseIdentifier {idB}, ImagerUseCaseIdentifier {idA});
        ASSERT_FALSE (std::less<ImagerUseCaseIdentifier>() (ImagerUseCaseIdentifier {idA}, ImagerUseCaseIdentifier {idB}));
        ASSERT_FALSE (ImagerUseCaseIdentifier {idA} < ImagerUseCaseIdentifier {idB});
    }
}
