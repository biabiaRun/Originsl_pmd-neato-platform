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
#include <royale.hpp>
#include <limits>

/* Note: files in this directory that are named UnitTest* can run without hardware */

TEST (UnitTestRoyale, TestVersion)
{
    unsigned major, minor, patch, build;
    royale::String scmRevision;
    major = minor = patch = build = std::numeric_limits<uint16_t>::max();
    royale::getVersion (major, minor, patch, build, scmRevision);
    EXPECT_NE (major, std::numeric_limits<uint16_t>::max());
    EXPECT_NE (minor, std::numeric_limits<uint16_t>::max());
    EXPECT_NE (patch, std::numeric_limits<uint16_t>::max());
    EXPECT_NE (build, std::numeric_limits<uint16_t>::max());
    EXPECT_EQ (scmRevision.empty(), false);

    unsigned major1, minor1, patch1, build1;
    major1 = minor1 = patch1 = build1 = std::numeric_limits<uint16_t>::max();
    royale::getVersion (major1, minor1, patch1, build1);
    EXPECT_NE (major1, std::numeric_limits<uint16_t>::max());
    EXPECT_NE (minor1, std::numeric_limits<uint16_t>::max());
    EXPECT_NE (patch1, std::numeric_limits<uint16_t>::max());
    EXPECT_NE (build1, std::numeric_limits<uint16_t>::max());
    EXPECT_EQ (major1, major);
    EXPECT_EQ (minor1, minor);
    EXPECT_EQ (patch1, patch);
    EXPECT_EQ (build1, build);

    major1 = minor1 = patch1 = build1 = std::numeric_limits<uint16_t>::max();
    royale::getVersion (major1, minor1, patch1);
    EXPECT_NE (major1, std::numeric_limits<uint16_t>::max());
    EXPECT_NE (minor1, std::numeric_limits<uint16_t>::max());
    EXPECT_NE (patch1, std::numeric_limits<uint16_t>::max());
    EXPECT_EQ (build1, std::numeric_limits<uint16_t>::max());
    EXPECT_EQ (major1, major);
    EXPECT_EQ (minor1, minor);
    EXPECT_EQ (patch1, patch);
}
