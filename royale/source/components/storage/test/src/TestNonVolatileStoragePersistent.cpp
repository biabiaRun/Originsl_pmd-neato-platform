/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/NonVolatileStoragePersistent.hpp>

#include <common/MakeUnique.hpp>

#include <gtest/gtest.h>

using namespace royale::common;
using namespace royale::storage;
using namespace testing;

// This class is also tested by tests that request JUST_CALIBRATION from NonVolatileStorageFactory

/**
 * Tests that, if opened with a non-existent file, either the constructor throws or all of the
 * getters throw.
 */
TEST (TestNonVolatileStoragePersistent, ThrowsForNonexistentFile)
{
    std::unique_ptr<NonVolatileStoragePersistent> storage;
    try
    {
        storage = makeUnique<NonVolatileStoragePersistent> ("this file does not exist");
    }
    catch (...)
    {
        // The constructor threw, which means the test passes
    }
    if (storage)
    {
        ASSERT_ANY_THROW (storage->getModuleIdentifier());
        ASSERT_ANY_THROW (storage->getModuleSuffix());
        ASSERT_ANY_THROW (storage->getModuleSerialNumber());
        ASSERT_ANY_THROW (storage->getCalibrationData());
    }
}

#ifndef TEST_FILE_FOR_NONVOLATILESTORAGEPERSISTENT
#error No definition for the filename to test NonVolatileStoragePersistent
#endif
/**
 * If the file exists, then getCalibrationData() should return data, and the other methods should
 * return the expected values for "I/O worked, but this data isn't there".
 */
TEST (TestNonVolatileStoragePersistent, ReadsDataFromFile)
{
    std::unique_ptr<NonVolatileStoragePersistent> storage;
    ASSERT_NO_THROW (storage = makeUnique<NonVolatileStoragePersistent> (TEST_FILE_FOR_NONVOLATILESTORAGEPERSISTENT));

    ASSERT_FALSE (storage->getCalibrationData().empty());
    ASSERT_TRUE (storage->getModuleIdentifier().empty());
    ASSERT_TRUE (storage->getModuleSuffix().empty());
    ASSERT_TRUE (storage->getModuleSerialNumber().empty());
}
