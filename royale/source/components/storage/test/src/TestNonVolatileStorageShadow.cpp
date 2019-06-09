/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/NonVolatileStorageShadow.hpp>
#include <common/Crc32.hpp>
#include <common/FileSystem.hpp>

#include <gmock/gmock.h>

using namespace royale::common;
using namespace royale::storage;
using namespace testing;

namespace
{
    /**
     * A mock for testing the copy-from-interface-constructor.  Testing NonVolatileStorageShadow is
     * probably the only task that will need this mock, as NonVolatileStorageShadow itself can be
     * used as the mock in most tests that need an INonVolatileStorage.
     */
    class MockNonVolatileStorage : public royale::hal::INonVolatileStorage
    {
    public:
        MOCK_METHOD0 (getModuleIdentifier, royale::Vector<uint8_t>());
        MOCK_METHOD0 (getModuleSuffix, royale::String());
        MOCK_METHOD0 (getModuleSerialNumber, royale::String());
        MOCK_METHOD0 (getCalibrationData, royale::Vector<uint8_t>());
        MOCK_METHOD0 (getCalibrationDataChecksum, uint32_t());
        MOCK_METHOD1 (writeCalibrationData, void (const royale::Vector<uint8_t> &));
        MOCK_METHOD4 (writeCalibrationData, void (const royale::Vector<uint8_t> &,
                      const royale::Vector<uint8_t> &,
                      const royale::String &,
                      const royale::String &));
    };
}

TEST (TestNonVolatileStorageShadow, ConstructWithCalibOnly)
{
    const royale::Vector<uint8_t> calib {1, 2, 3, 4};

    NonVolatileStorageShadow shadow (calib);

    auto checksum = calculateCRC32 (calib.data(), calib.size());

    ASSERT_EQ (calib, shadow.getCalibrationData());
    ASSERT_TRUE (shadow.getModuleIdentifier().empty());
    ASSERT_TRUE (shadow.getModuleSuffix().empty());
    ASSERT_TRUE (shadow.getModuleSerialNumber().empty());
    ASSERT_EQ (checksum, shadow.getCalibrationDataChecksum());
}

TEST (TestNonVolatileStorageShadow, ConstructWithV7Data)
{
    const royale::Vector<uint8_t> calib {1, 2, 3, 4};
    const royale::Vector<uint8_t> id {10, 11, 12}; // will not be padded
    const royale::String suffix {"suffix"};
    const royale::String serial {"1111-2222-3333-4444"};

    NonVolatileStorageShadow shadow (calib, id, suffix, serial);

    ASSERT_EQ (calib, shadow.getCalibrationData());
    ASSERT_EQ (id, shadow.getModuleIdentifier());
    ASSERT_EQ (suffix, shadow.getModuleSuffix());
    ASSERT_EQ (serial, shadow.getModuleSerialNumber());
}

TEST (TestNonVolatileStorageShadow, CopyConstructor)
{
    const royale::Vector<uint8_t> calib {1, 2, 3, 4};
    const royale::Vector<uint8_t> id {10, 11, 12}; // will not be padded
    const royale::String suffix {"suffix"};
    const royale::String serial {"1111-2222-3333-4444"};

    auto checksum = calculateCRC32 (calib.data(), calib.size());

    MockNonVolatileStorage mock;
    EXPECT_CALL (mock, getCalibrationData()).WillRepeatedly (Return (calib));
    EXPECT_CALL (mock, getModuleIdentifier()).WillRepeatedly (Return (id));
    EXPECT_CALL (mock, getModuleSuffix()).WillRepeatedly (Return (suffix));
    EXPECT_CALL (mock, getModuleSerialNumber()).WillRepeatedly (Return (serial));
    EXPECT_CALL (mock, getCalibrationDataChecksum()).WillRepeatedly (Return (checksum));

    NonVolatileStorageShadow shadow (mock, false);

    ASSERT_EQ (calib, shadow.getCalibrationData());
    ASSERT_EQ (id, shadow.getModuleIdentifier());
    ASSERT_EQ (suffix, shadow.getModuleSuffix());
    ASSERT_EQ (serial, shadow.getModuleSerialNumber());
    ASSERT_EQ (checksum, shadow.getCalibrationDataChecksum());
}

TEST (TestNonVolatileStorageShadow, Caching)
{
    const royale::Vector<uint8_t> calib{ 1, 2, 3, 4 };
    const royale::Vector<uint8_t> id{ 10, 11, 12 }; // will not be padded
    const royale::String suffix{ "suffix" };
    const royale::String serial{ "1111-2222-3333-4444" };

    auto checksum = calculateCRC32 (calib.data(), calib.size());

    MockNonVolatileStorage mock;
    EXPECT_CALL (mock, getCalibrationData()).WillRepeatedly (Return (calib));
    EXPECT_CALL (mock, getModuleIdentifier()).WillRepeatedly (Return (id));
    EXPECT_CALL (mock, getModuleSuffix()).WillRepeatedly (Return (suffix));
    EXPECT_CALL (mock, getModuleSerialNumber()).WillRepeatedly (Return (serial));
    EXPECT_CALL (mock, getCalibrationDataChecksum()).WillRepeatedly (Return (checksum));

    royale::String filename = serial + ".cal";
    if (royale::common::fileexists (filename))
    {
        remove (filename.c_str());
    }

    {
        NonVolatileStorageShadow shadow (mock, true);

        ASSERT_EQ (calib, shadow.getCalibrationData());
        ASSERT_EQ (id, shadow.getModuleIdentifier());
        ASSERT_EQ (suffix, shadow.getModuleSuffix());
        ASSERT_EQ (serial, shadow.getModuleSerialNumber());
        ASSERT_EQ (checksum, shadow.getCalibrationDataChecksum());
    }

    ASSERT_TRUE (royale::common::fileexists (filename));

    {
        NonVolatileStorageShadow shadow (mock, true);

        ASSERT_EQ (calib, shadow.getCalibrationData());
        ASSERT_EQ (id, shadow.getModuleIdentifier());
        ASSERT_EQ (suffix, shadow.getModuleSuffix());
        ASSERT_EQ (serial, shadow.getModuleSerialNumber());
        ASSERT_EQ (checksum, shadow.getCalibrationDataChecksum());
    }
}
