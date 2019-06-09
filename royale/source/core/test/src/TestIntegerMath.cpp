/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <common/IntegerMath.hpp>

#include <cstdint>
#include <vector>

using namespace royale::common;

TEST (TestIntegerMath, TestRoundUpToUnit)
{
    ASSERT_EQ (50u, roundUpToUnit (41, 10));
    ASSERT_EQ (40u, roundUpToUnit (40, 10));
}

TEST (TestIntegerMath, TestDivideRoundUp)
{
    ASSERT_EQ (5u, divideRoundUp (41, 10));
    ASSERT_EQ (4u, divideRoundUp (40, 10));
    ASSERT_EQ (41u, divideRoundUp (401, 10));
    ASSERT_EQ (40u, divideRoundUp (400, 10));
}
