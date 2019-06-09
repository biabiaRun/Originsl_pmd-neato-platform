/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/Crc32.hpp>
#include <gtest/gtest.h>
#include <map>

using namespace royale::common;

TEST (TestCrc32, testCalc)
{
    {
        const char *testString = "123456789";
        uint32_t testStringRes = calculateCRC32 (reinterpret_cast<const uint8_t *> (testString), 9);
        EXPECT_EQ (0xCBF43926u, testStringRes);
    }

    {
        const char *testString = "The quick brown fox jumps over the lazy dog";
        uint32_t testStringRes = calculateCRC32 (reinterpret_cast<const uint8_t *> (testString), 43);
        EXPECT_EQ (0x414FA339u, testStringRes);
    }

    {
        const char *testString = "0000000000000000000000000000000000000000000000000000000000000000";
        uint32_t testStringRes = calculateCRC32 (reinterpret_cast<const uint8_t *> (testString), 64);
        EXPECT_EQ (0x34B1E4CBu, testStringRes);
    }

    {
        const char *testString = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
        uint32_t testStringRes = calculateCRC32 (reinterpret_cast<const uint8_t *> (testString), 64);
        EXPECT_EQ (0xE1FD58B2u, testStringRes);
    }

    {
        const char *testString = "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F";
        uint32_t testStringRes = calculateCRC32 (reinterpret_cast<const uint8_t *> (testString), 64);
        EXPECT_EQ (0x973D64BBu, testStringRes);
    }

    {
        const char *testString = "This is a CRC test string";
        uint32_t testStringRes = calculateCRC32 (reinterpret_cast<const uint8_t *> (testString), 25);
        EXPECT_EQ (0xED73F663u, testStringRes);
    }
}

TEST (TestCrc32, testEmpty)
{
    {
        char *testString = 0;
        uint32_t testStringRes = calculateCRC32 (reinterpret_cast<uint8_t *> (testString), 9);
        EXPECT_EQ (0x0u, testStringRes);
    }

    {
        const char *testString = "123456789";
        uint32_t testStringRes = calculateCRC32 (reinterpret_cast<const uint8_t *> (testString), 0);
        EXPECT_EQ (0x0u, testStringRes);
    }
}
