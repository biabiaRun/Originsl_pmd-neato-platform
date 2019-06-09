/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/StorageFormatPolar.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>

#include <MockRandomAccessStorage.hpp>
#include <StorageTestUtils.hpp>

#include <EndianConversion.hpp>
#include <common/Crc32.hpp>
#include <common/exceptions/DataNotFound.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <NarrowCast.hpp>

#include <gtest/gtest.h>

#include <RoyaleLogger.hpp>

#include <algorithm>
#include <memory>

using namespace royale::common;
using namespace royale::pal;
using namespace royale::storage;
using namespace royale::stub::storage;
using namespace royale::test::utils;

namespace
{
    const auto magic = stringlikeMagicNumber ("PMDTEC");

    /**
     * The raw data for an EEPROM with a v2 header.
     *
     * \param calibration the calibration data (can be empty)
     */
    std::vector<uint8_t> getV2Image (const std::vector<uint8_t> &calibration)
    {
        std::vector<uint8_t> image;
        pushBackIterable (image, magic);
        pushBack16 (image, 0x02); // version
        pushBack32 (image, 0);
        pushBack32 (image, 0);
        pushBack32 (image, 0);
        pushBack32 (image, 0);
        pushBack32 (image, narrow_cast<uint32_t> (calibration.size()));
        pushBackIterable (image, calibration);
        return image;
    }

    std::vector<uint8_t> getV3Image (const std::vector<uint8_t> &calibration)
    {
        std::vector<uint8_t> image;
        pushBackIterable (image, magic);
        pushBack16 (image, 3); // version
        auto checksum = calculateCRC32 (calibration.data(), calibration.size());
        pushBack32 (image, checksum);
        pushBack32 (image, narrow_cast<uint32_t> (calibration.size()));
        pushBackIterable (image, calibration);
        return image;
    }

    std::vector<uint8_t> getV0x101Image (const std::vector<uint8_t> &actualCalibration, uint32_t serial)
    {
        // 0x101 is similar to two v3 headers, as in "getV3Image (getV3Image (actualCalibration))"
        auto innerHeaderAndCalib = getV3Image (actualCalibration);
        std::vector<uint8_t> image;
        pushBackIterable (image, magic);
        pushBack16 (image, 0x101); // version
        pushBack32 (image, serial);
        pushBack32 (image, narrow_cast<uint32_t> (innerHeaderAndCalib.size()));
        pushBackIterable (image, innerHeaderAndCalib);
        return image;
    }

    std::vector<uint8_t> getV7Image (const std::vector<uint8_t> &calibration,
                                     const std::vector<uint8_t> &id,
                                     const std::string &suffix,
                                     const std::string &serial)
    {
        std::vector<uint8_t> image;
        pushBackIterable (image, magic);
        pushBack16 (image, 7); // version
        auto checksum = calculateCRC32 (calibration.data(), calibration.size());
        pushBack32 (image, checksum);
        pushBack32 (image, narrow_cast<uint32_t> (calibration.size()));
        pushBackPadded (image, id, 16);
        pushBackPadded (image, suffix, 16);
        pushBackPadded (image, serial, 19);
        pushBackIterable (image, calibration);
        return image;
    }

    /**
     * Creates a identifier starting with the given string, padded to the correct length.
     */
    royale::Vector<uint8_t> getV7Identifier (const std::string &id)
    {
        return royale::Vector<uint8_t> (stringlikeMagicNumber (id, 16));
    }
}

/**
 * Tests that a valid V2 header is read.
 */
TEST (TestStorageFormatPolarNoFixture, V2WithoutCalibration)
{
    const auto image = getV2Image ({});
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));
    ASSERT_ANY_THROW (polar->getCalibrationData());
}

/**
 * Tests that a valid V2 header is read.
 */
TEST (TestStorageFormatPolarNoFixture, V2WithCalibration)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    const auto image = getV2Image (calib);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    royale::Vector<uint8_t> data;
    ASSERT_NO_THROW (data = polar->getCalibrationData());
    ASSERT_EQ (data.size(), calib.size());
    for (std::size_t i = 0; i < calib.size() ; i++)
    {
        ASSERT_EQ (data[i], calib[i]);
    }
}

/**
 * Tests that an invalid V2 header with uninitialized size does not
 * cause out-of-memory.
 */
TEST (TestStorageFormatPolarNoFixture, V2JunkSize)
{
    auto image = getV2Image ({});
    image[24] = 0xff;
    image[25] = 0xff;
    image[26] = 0xff;
    image[27] = 0xff;

    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));
    ASSERT_THROW (polar->getCalibrationData(), Exception);
}

/**
 * Tests that a valid V3 header with no calibration data causes an exception.
 */
TEST (TestStorageFormatPolarNoFixture, V3WithoutCalibration)
{
    const auto image = getV3Image ({});
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));
    ASSERT_THROW (polar->getCalibrationData(), Exception);
}

/**
 * Tests that a valid V3 header with calibration data can be read.
 */
TEST (TestStorageFormatPolarNoFixture, V3WithCalibration)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    const auto image = getV3Image (calib);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    royale::Vector<uint8_t> data;
    ASSERT_NO_THROW (data = polar->getCalibrationData());
    ASSERT_EQ (data.size(), calib.size());
    for (std::size_t i = 0; i < calib.size() ; i++)
    {
        ASSERT_EQ (data[i], calib[i]);
    }
}

/**
 * Tests that a valid V3 header with calibration data but without a checksum is rejected.
 */
TEST (TestStorageFormatPolarNoFixture, V3WithoutChecksum)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    auto image = getV3Image (calib);
    image[8] = 0;
    image[9] = 0;
    image[10] = 0;
    image[11] = 0;
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    royale::Vector<uint8_t> data;
    ASSERT_THROW (data = polar->getCalibrationData(), Exception);
}

/**
 * Tests that an invalid V3 header with uninitialized size does not
 * cause out-of-memory.
 */
TEST (TestStorageFormatPolarNoFixture, V3JunkSize)
{
    auto image = getV3Image ({});
    image[8] = 0xff;
    image[9] = 0xff;
    image[10] = 0xff;
    image[11] = 0xff;

    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));
    ASSERT_THROW (polar->getCalibrationData(), Exception);
}

/**
 * Tests that an invalid V3 header is rejected.
 */
TEST (TestStorageFormatPolarNoFixture, CorruptionBug1076)
{
    // A Version 3 header that claims to be version 2
    auto image = getV3Image ({});
    image[6] = 2;

    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));
    ASSERT_THROW (polar->getCalibrationData(), Exception);
}

/**
 * Tests that a valid header is written (which should be a V3 header), and that getCalibrationData
 * either does not cache the data, or invalidates the cache when data is written.
 */
TEST (TestStorageFormatPolarNoFixture, V3WriteCalibrationData)
{
    // start with an empty image
    const std::vector<uint8_t> blank;
    auto bridge = std::make_shared<MockRandomAccessStorage> (blank);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    // Check that getCalibrationData() throws when there is no data.  This is done in the same test
    // as the successful reads, to check that a failure does not affect the successful reads.
    ASSERT_THROW (polar->getCalibrationData(), Exception);

    // Write the first version of the data
    royale::Vector<uint8_t> calibration;
    const auto firstCalSize = std::size_t (256);
    for (auto i = 0u; i < firstCalSize; i++)
    {
        calibration.emplace_back (static_cast<uint8_t> (i));
    }
    ASSERT_NO_THROW (polar->writeCalibrationData (calibration));

    // Read the data, and check that it matches what was written
    ASSERT_NO_THROW (calibration = polar->getCalibrationData());
    ASSERT_EQ (firstCalSize, calibration.size());
    for (auto i = 0u; i < firstCalSize; i++)
    {
        ASSERT_EQ (calibration [i], static_cast<uint8_t> (i));
    }

    // Now overwrite that data, and check that the new data is read back
    calibration.clear();
    const auto secondCalSize = std::size_t (127);
    const auto constant = unsigned (42);
    for (auto i = 0u; i < secondCalSize; i++)
    {
        calibration.emplace_back (static_cast<uint8_t> (i + constant));
    }
    ASSERT_NO_THROW (polar->writeCalibrationData (calibration));
    ASSERT_NO_THROW (calibration = polar->getCalibrationData());
    ASSERT_EQ (secondCalSize, calibration.size());
    for (auto i = 0u; i < secondCalSize; i++)
    {
        ASSERT_EQ (calibration [i], static_cast<uint8_t> (i + constant));
    }
}

/**
 * Tests that the single-argument version of writeCalibrationData throws if the constructor argument
 * corresponding to FlashMemoryConfig::identifierIsMandatory is set.
 */
TEST (TestStorageFormatPolarNoFixture, WriteWithoutIdentifier)
{
    // start with an empty image
    const std::vector<uint8_t> blank;
    auto bridge = std::make_shared<MockRandomAccessStorage> (blank);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge, true)));

    royale::Vector<uint8_t> calibration {1, 2, 3, 4};
    ASSERT_THROW (polar->writeCalibrationData (calibration), LogicError);
}

/**
 * Tests that a valid V7 header with calibration data can be read.
 */
TEST (TestStorageFormatPolarNoFixture, V7WithCalibration)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    const std::vector<uint8_t> id {10, 11, 12}; // will be padded to 16 bytes
    const std::string suffix {"Ice Flow"};
    const std::string serial {"1111-2222-3333-4444"};
    const auto image = getV7Image (calib, id, suffix, serial);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    royale::Vector<uint8_t> data;
    ASSERT_NO_THROW (data = polar->getModuleIdentifier());
    ASSERT_EQ (16u, data.size());
    for (auto i = 0u; i < id.size(); i++)
    {
        ASSERT_EQ (id[i], data[i]);
    }
    for (auto i = id.size(); i < 16; i++)
    {
        ASSERT_EQ (0u, data[i]);
    }

    ASSERT_NO_THROW (data = polar->getCalibrationData());
    ASSERT_EQ (calib.size(), data.size());
    for (std::size_t i = 0; i < calib.size(); i++)
    {
        ASSERT_EQ (calib[i], data[i]);
    }

    ASSERT_EQ (royale::String ("1111-2222-3333-4444"), polar->getModuleSerialNumber());
}

/**
 * Tests that a valid V7 header with a short serial number can be read, and also that the serial
 * "number" can contain non-numbers.
 */
TEST (TestStorageFormatPolarNoFixture, V7WithShortSerial)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    const std::vector<uint8_t> id {10, 11, 12}; // will be padded to 16 bytes
    royale::Vector<uint8_t> idPadded = id;
    idPadded.resize (16, 0);
    const std::string suffix {"suffix"};
    const std::string serial {"A-Za-z!@$%\""};
    const auto image = getV7Image (calib, id, suffix, serial);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    royale::Vector<uint8_t> data;

    ASSERT_NO_THROW (data = polar->getCalibrationData());
    ASSERT_EQ (calib.size(), data.size());
    for (std::size_t i = 0; i < calib.size(); i++)
    {
        ASSERT_EQ (calib[i], data[i]);
    }

    ASSERT_EQ (idPadded, polar->getModuleIdentifier());
    ASSERT_EQ (royale::String (suffix), polar->getModuleSuffix());

    // getModuleSerialNumber is defined to return an empty string if there's no serial number
    ASSERT_EQ (royale::String (serial), polar->getModuleSerialNumber());
}

/**
 * Tests that a valid V7 header with only the info from a V3 can be read.
 *
 * \todo ROYAL-2600 Decide whether writeCalibrationData should throw when given this data. But if
 * it's already written to a device, the reading code should still handle it.
 */
TEST (TestStorageFormatPolarNoFixture, V7WithoutNewData)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    const std::vector<uint8_t> id {}; // Will be padded to 16 bytes, but for this
    royale::Vector<uint8_t> idPadded {}; // all-zero id, getModuleIdentifier should return empty
    const std::string suffix {""};
    const std::string serial {""};
    const auto image = getV7Image (calib, id, suffix, serial);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    royale::Vector<uint8_t> data;

    ASSERT_NO_THROW (data = polar->getCalibrationData());
    ASSERT_EQ (calib.size(), data.size());
    for (std::size_t i = 0; i < calib.size(); i++)
    {
        ASSERT_EQ (calib[i], data[i]);
    }

    ASSERT_EQ (idPadded, polar->getModuleIdentifier());
    ASSERT_EQ (royale::String (suffix), polar->getModuleSuffix());

    // getModuleSerialNumber is defined to return an empty string if there's no serial number
    ASSERT_EQ (royale::String (""), polar->getModuleSerialNumber());
}

/**
 * Tests that the new V7 data can be read from a valid V7 header without calibration data.
 */
TEST (TestStorageFormatPolarNoFixture, V7WithoutCalibration)
{
    const std::vector<uint8_t> calib {};
    const std::vector<uint8_t> id {10, 11, 12}; // will be padded to 16 bytes
    const std::string suffix {"Ice Flow"};
    const std::string serial {"1111-2222-3333-4444"};
    const auto image = getV7Image (calib, id, suffix, serial);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    royale::Vector<uint8_t> data;
    ASSERT_NO_THROW (data = polar->getModuleIdentifier());
    ASSERT_EQ (16u, data.size());
    for (auto i = 0u; i < id.size(); i++)
    {
        ASSERT_EQ (id[i], data[i]);
    }
    for (auto i = id.size(); i < 16; i++)
    {
        ASSERT_EQ (0u, data[i]);
    }

    ASSERT_THROW (polar->getCalibrationData(), DataNotFound);

    ASSERT_EQ (royale::String ("1111-2222-3333-4444"), polar->getModuleSerialNumber());
}

/**
 * Tests that a valid header with identifier is written (which should be a V7 header), and that
 * the getter functions either do not cache the data, or invalidate the cache when data is written.
 */
TEST (TestStorageFormatPolarNoFixture, V7WriteCalibrationData)
{
    // start with an empty image
    const std::vector<uint8_t> blank;
    auto bridge = std::make_shared<MockRandomAccessStorage> (blank);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    // Check that getCalibrationData() throws when there is no data.  This is done in the same test
    // as the successful reads, to check that a failure does not affect the successful reads.
    ASSERT_THROW (polar->getCalibrationData(), Exception);

    // Write the first version of the data and identifiers
    {
        royale::Vector<uint8_t> calibration;
        const auto firstCalSize = std::size_t (256);
        for (auto i = 0u; i < firstCalSize; i++)
        {
            calibration.emplace_back (static_cast<uint8_t> (i));
        }
        const auto identifier = getV7Identifier ("unofficial_id");
        const auto suffix = royale::String ("test ver. 1");
        const auto serialNumber = royale::String ("1234-5678-90ab-cdef");
        ASSERT_NO_THROW (polar->writeCalibrationData ( (calibration), identifier, suffix, serialNumber));

        auto checksum = calculateCRC32 (calibration.data(), calibration.size());

        // Read the data, and check that it matches what was written
        ASSERT_EQ (identifier, polar->getModuleIdentifier());
        ASSERT_EQ (suffix, polar->getModuleSuffix());
        ASSERT_EQ (serialNumber, polar->getModuleSerialNumber());
        ASSERT_EQ (checksum, polar->getCalibrationDataChecksum());
        ASSERT_NO_THROW (calibration = polar->getCalibrationData());
        ASSERT_EQ (firstCalSize, calibration.size());
        for (auto i = 0u; i < firstCalSize; i++)
        {
            ASSERT_EQ (calibration [i], static_cast<uint8_t> (i));
        }
    }

    // Now overwrite that data
    {
        royale::Vector<uint8_t> calibration;
        const auto secondCalSize = std::size_t (127);
        const auto constant = unsigned (42);
        for (auto i = 0u; i < secondCalSize; i++)
        {
            calibration.emplace_back (static_cast<uint8_t> (i + constant));
        }
        auto identifier = getV7Identifier ("abc");
        const auto suffix = royale::String ("test v.2");
        const auto serialNumber = royale::String ("1234-5678-90AB-CDEF");
        ASSERT_NO_THROW (polar->writeCalibrationData ( (calibration), identifier, suffix, serialNumber));

        // Read the data back, and check that it matches the new data
        ASSERT_EQ (identifier, polar->getModuleIdentifier());
        ASSERT_EQ (suffix, polar->getModuleSuffix());
        ASSERT_EQ (serialNumber, polar->getModuleSerialNumber());
        ASSERT_NO_THROW (calibration = polar->getCalibrationData());
        ASSERT_EQ (secondCalSize, calibration.size());
        for (auto i = 0u; i < secondCalSize; i++)
        {
            ASSERT_EQ (calibration [i], static_cast<uint8_t> (i + constant));
        }
    }
}

/**
 * Tests that attempting to write a header with an invalid-sized identifier throws. The identifier
 * can contain trailing zeros, so if written then it would effectively be padded to 16 characters,
 * but then wouldn't match the identifier that was requersted to be written.
 */
TEST (TestStorageFormatPolarNoFixture, V7WriteShortIdentifier)
{
    // start with an empty image
    const std::vector<uint8_t> blank;
    auto bridge = std::make_shared<MockRandomAccessStorage> (blank);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    const auto calibration = royale::Vector<uint8_t> {0, 1, 2, 3};
    const auto identifier = stringlikeMagicNumber ("short");
    const auto suffix = royale::String ("test ver. 1");
    const auto serialNumber = royale::String ("1234-5678-90ab-cdef");
    ASSERT_THROW (polar->writeCalibrationData ( (calibration), identifier, suffix, serialNumber), LogicError);
}

/**
 * Given a device that already has a module identifier, checks that the module identifier can be
 * preserved when new calibration data is written.
 *
 * \todo ROYAL-2600 Decide whether this is implemented in the INonVolatileStorage, or in the
 * CameraCore / CameraDevice.  This test is a prototype which will stay as a test if it is
 * implemented in INonVolatileStorage, or become the code in CameraCore / CameraDevice.
 */
TEST (TestStorageFormatPolarNoFixture, PreserveV7ModuleIdentifier)
{
    // Start with a v7 image prepared as in V7WriteCalibrationData
    const std::vector<uint8_t> blank;
    auto bridge = std::make_shared<MockRandomAccessStorage> (blank);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    const auto identifier = getV7Identifier ("unofficial_id");
    const auto suffix = royale::String ("test ver. 1");
    const auto serialNumber = royale::String ("1234-5678-90ab-cdef");

    {
        royale::Vector<uint8_t> calibration;
        const auto firstCalSize = std::size_t (256);
        for (auto i = 0u; i < firstCalSize; i++)
        {
            calibration.emplace_back (static_cast<uint8_t> (i));
        }
        ASSERT_NO_THROW (polar->writeCalibrationData ( (calibration), identifier, suffix, serialNumber));
    }
    // The lines until this point are the V7WriteCalibrationData test

    // Now overwrite that data
    {
        royale::Vector<uint8_t> calibration;
        const auto secondCalSize = std::size_t (127);
        const auto constant = unsigned (42);
        for (auto i = 0u; i < secondCalSize; i++)
        {
            calibration.emplace_back (static_cast<uint8_t> (i + constant));
        }
        ASSERT_NO_THROW (polar->writeCalibrationData (calibration));

        // Read the data back, and check that the calibration matches the new
        // data, but the others match the original data
        ASSERT_EQ (identifier, polar->getModuleIdentifier());
        ASSERT_EQ (suffix, polar->getModuleSuffix());
        ASSERT_EQ (serialNumber, polar->getModuleSerialNumber());
        ASSERT_NO_THROW (calibration = polar->getCalibrationData());
        ASSERT_EQ (secondCalSize, calibration.size());
        for (auto i = 0u; i < secondCalSize; i++)
        {
            ASSERT_EQ (calibration [i], static_cast<uint8_t> (i + constant));
        }
    }
}

/**
 * Tests that a header with an unsupported version is rejected.
 *
 * The V4 is not supported in Royale (and never will be, the V4 is now obsolete), this is just
 * checking that the version number is checked.
 */
TEST (TestStorageFormatPolarNoFixture, UnsupportedVersion)
{
    // A Version 3 header that claims to be version 4
    auto image = getV3Image ({});
    image[6] = 4;

    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));
    ASSERT_THROW (polar->getCalibrationData(), Exception);
}

/**
 * Tests that a version 0x101 header with calibration data can be read, and that the serial-number
 * handling is still the same.
 */
TEST (TestStorageFormatPolarNoFixture, v0x101WithCalibration)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    const auto image = getV0x101Image (calib, 0x00345678);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatPolar> polar;
    ASSERT_NO_THROW (polar.reset (new StorageFormatPolar (bridge)));

    royale::Vector<uint8_t> data;
    ASSERT_NO_THROW (data = polar->getCalibrationData());
    ASSERT_EQ (data.size(), calib.size());
    for (std::size_t i = 0; i < calib.size() ; i++)
    {
        ASSERT_EQ (data[i], calib[i]);
    }
    ASSERT_EQ (royale::String ("00345678"), polar->getModuleSerialNumber());
}
