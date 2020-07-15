#include <common/StringFunctions.hpp>
#include <royale/String.hpp>
#include <royale/Vector.hpp>
#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>

using namespace royale;
using namespace royale::common;

TEST (TestStringFunctions, TestStof)
{
    {
        String testStr ("3");
        auto val = stofRoyale (testStr);
        EXPECT_EQ (val, 3.0f);
    }

    {
        String testStr ("3.3");
        auto val = stofRoyale (testStr);
        EXPECT_EQ (val, 3.3f);
    }

    {
        String testStr ("zzz");
        EXPECT_THROW (stofRoyale (testStr), std::invalid_argument);
    }
}

TEST (TestStringFunctions, TestStoi)
{
    {
        String testStr ("3");
        auto val = stoiRoyale (testStr);
        EXPECT_EQ (val, 3);
    }

    {
        String testStr ("3.3");
        auto val = stoiRoyale (testStr);
        EXPECT_EQ (val, 3);
    }

    {
        String testStr ("zzz");
        EXPECT_THROW (stoiRoyale (testStr), std::invalid_argument);
    }
}
