#include <common/FileSystem.hpp>
#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>
#include <map>

using namespace royale;
using namespace royale::common;

TEST (TestFileSystem, TestWriteReadVector)
{
    Vector<uint32_t> testVector;

    testVector.push_back (1);
    testVector.push_back (2);
    testVector.push_back (3);
    testVector.push_back (4);

    EXPECT_EQ (testVector.size () * sizeof (uint32_t), writeVectorToFile ("testVector.dat", testVector));

    Vector<uint32_t> testVector2;

    EXPECT_EQ (testVector.size() * sizeof (uint32_t), readFileToVector ("testVector.dat", testVector2));

    EXPECT_EQ (testVector.size(), testVector2.size());

    for (size_t i = 0; i < testVector.size(); ++i)
    {
        EXPECT_EQ (testVector.at (i), testVector2.at (i));
    }

    remove ("testVector.dat");
}

TEST (TestFileSystem, TestFileExists)
{
    Vector<uint32_t> testVector;

    testVector.push_back (1);
    testVector.push_back (2);
    testVector.push_back (3);
    testVector.push_back (4);

    EXPECT_EQ (testVector.size() * sizeof (uint32_t), common::writeVectorToFile ("testVector.dat", testVector));

    EXPECT_TRUE (fileexists ("testVector.dat"));

    remove ("testVector.dat");

    EXPECT_FALSE (fileexists ("testVector.dat"));
}
