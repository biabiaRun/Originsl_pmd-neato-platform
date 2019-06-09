/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/StorageFormatZwetschge.hpp>

#include <StorageTestUtils.hpp>
#include <ZwetschgeTestUtils.hpp>

#include <MockRandomAccessStorage.hpp>

#include <common/EndianConversion.hpp>
#include <common/exceptions/DataNotFound.hpp>
#include <common/exceptions/WrongDataFormatFound.hpp>
#include <common/IteratorBoundsCheck.hpp>
#include <processing/ProcessingParameterId.hpp>
#include <royale/StreamId.hpp>
#include <usecase/ExposureGroup.hpp>
#include <usecase/UseCaseIdentifier.hpp>

#include <imager/WrapperImagerExternalConfig.hpp>

#include <gmock/gmock.h>

#include <algorithm>
#include <cassert>
#include <memory>

using namespace royale::common;
using namespace royale::imager;
using namespace royale::pal;
using namespace royale::processing;
using namespace royale::storage;
using namespace royale::stub::storage;
using namespace royale::test::utils;
using namespace royale::test::utils::zwetschge;
using namespace royale::usecase;

namespace
{
    const auto tocMagic = stringlikeMagicNumber ("ZWETSCHGE");

    /**
     * Returns an otherwise-valid Zwetschge image, but with the start of the ToC overwritten with
     * the given data.
     *
     * This ensures that the data used for corrupt-image tests is large enough that the expected
     * data can be read.
     */
    std::vector<uint8_t> getImageWithCorruptMagic (const std::vector<uint8_t> &magic)
    {
        assert (magic.size() > 0);

        const std::vector<uint8_t> calib {1, 2, 3, 4};
        auto image = getZwetschgeImage (calib);

        const auto zwetschgeMagicOffset = std::size_t (0x2000u);
        assert (image[zwetschgeMagicOffset] == 'Z');

        assert (zwetschgeMagicOffset + magic.size() < image.size());
        std::copy (magic.begin(), magic.end(), image.begin() + zwetschgeMagicOffset);

        return image;
    }

    /**
     * This data matches the Zwetschge file created from ExampleDevice.py, which is accessible via
     * the ZwetschgeTestUtils' getZwetschgeExampleDevice().
     *
     * There's a subtle difference in the registers between the ExampleDevice and ExampleFlashImage,
     * for this example the value 0x1234 is put in register 0xaaaa.
     */
    const ExampleUseCaseList zwetschgeExampleDeviceUseCaseList
    {
        {
            "example 9 phase",
            172, 120, // image size
            UseCaseIdentifier { UseCaseIdentifier::datatype {{0x91, 0x3d, 0x36, 0x91, 0x65, 0x95, 0x51, 0xa1, 0x84, 0x37, 0xd0, 0x5e, 0x81, 0x57, 0x34, 0x98}} },
            { 9 }, // block sizes
            { 30000000u, 80000000u, 80000000u, 80000000u, 80000000u },
            SequentialRegisterBlock {},
            TimedRegisterList
            {
                {0x9000, 0x0001, 0},
                {0x9001, 0x0002, 50000},
                {0xaaaa, 0x1234, 0},
            },
            50, 1, 50, // start, min, max fps
            ProcessingParameterId { ProcessingParameterId::datatype {{0xbf, 0x10, 0x8e, 0x41, 0xb4, 0x9f, 0x54, 0x15, 0x91, 0x97, 0x2b, 0x28, 0x3a, 0xdf, 0x67, 0xb9}} },
            std::chrono::microseconds (10u), // wait time
            {0x1234}, // stream id
            {{"gray", {1, 800}, 200}, {"mod", {1, 1000}, 250}, {"mod", {1, 1000}, 250}},
            {{1, 30000000u, 0}, {4, 80000000u, 1}, {4, 80000000u, 2}},
            royale::CameraAccessLevel::L1
        },
    };

    /**
     * This data matches the Zwetschge file created from ExampleFlashImage.py, which is accessible
     * via the ZwetschgeTestUtils' getZwetschgeExampleFlashImage().
     *
     * There's a subtle difference in the registers between the ExampleDevice and ExampleFlashImage,
     * for this example the value 0x1234 is put in register 0x9002.
     */
    const ExampleUseCaseList zwetschgeExampleFlashImageUseCaseList
    {
        {
            "example 9 phase",
            172, 120, // image size
            UseCaseIdentifier { UseCaseIdentifier::datatype {{0x91, 0x3d, 0x36, 0x91, 0x65, 0x95, 0x51, 0xa1, 0x84, 0x37, 0xd0, 0x5e, 0x81, 0x57, 0x34, 0x98}} },
            { 9 }, // block sizes
            { 30000000u, 80000000u, 80000000u, 80000000u, 80000000u },
            SequentialRegisterBlock
            {
                {0x0001, 0x0002, 0x1234},
                0x9000
            },
            TimedRegisterList{},
            50, 1, 50, // start, min, max fps
            ProcessingParameterId { ProcessingParameterId::datatype {{0xbf, 0x10, 0x8e, 0x41, 0xb4, 0x9f, 0x54, 0x15, 0x91, 0x97, 0x2b, 0x28, 0x3a, 0xdf, 0x67, 0xb9}} },
            std::chrono::microseconds (10u), // wait time
            {0x1234}, // stream id
            {{"gray", {1, 800}, 200}, {"mod", {1, 1000}, 250}, {"mod", {1, 1000}, 250}},
            {{1, 30000000u, 0}, {4, 80000000u, 1}, {4, 80000000u, 2}},
            royale::CameraAccessLevel::L1
        },
    };

    /**
     * This data matches the Zwetschge file created from ExampleFlashImage.py, which is accessible
     * via the ZwetschgeTestUtils' getZwetschgeExampleFlashImage().
     *
     * It is used to test the conversion of sequential register maps to timed register lists.
     */
    const ExampleUseCaseList zwetschgeExampleDeviceUseCaseListSeqConverted
    {
        {
            "example 9 phase",
            172, 120, // image size
            UseCaseIdentifier{ UseCaseIdentifier::datatype{ { 0x91, 0x3d, 0x36, 0x91, 0x65, 0x95, 0x51, 0xa1, 0x84, 0x37, 0xd0, 0x5e, 0x81, 0x57, 0x34, 0x98 } } },
            { 9 }, // block sizes
            { 30000000u, 80000000u, 80000000u, 80000000u, 80000000u },
            SequentialRegisterBlock{},
            TimedRegisterList
            {
                { 0x9000, 0x0001, 0 },
                { 0x9001, 0x0002, 0 },
                { 0x9002, 0x1234, 0 },
            },
            50, 1, 50, // start, min, max fps
            ProcessingParameterId{ ProcessingParameterId::datatype{ { 0xbf, 0x10, 0x8e, 0x41, 0xb4, 0x9f, 0x54, 0x15, 0x91, 0x97, 0x2b, 0x28, 0x3a, 0xdf, 0x67, 0xb9 } } },
            std::chrono::microseconds (10u), // wait time
            { 0x1234 }, // stream id
            { { "gray", { 1, 800 }, 200 }, { "mod", { 1, 1000 }, 250 }, { "mod", { 1, 1000 }, 250 } },
            { { 1, 30000000u, 0 }, { 4, 80000000u, 1 }, { 4, 80000000u, 2 } },
            royale::CameraAccessLevel::L1
        },
    };


    /**
     * GUID used in both ExampleDevice and ExampleFlashImage
     */
    const royale::Vector<uint8_t> zwetschgeExampleDeviceProductCode { 0xfc, 0xde, 0x3c, 0x85, 0x22, 0x70, 0x59, 0x0f, 0x9e, 0x7c, 0xee, 0x00, 0x3d, 0x65, 0xe0, 0xe2 };
}


/**
 * Tests that a valid TOC with calibration is read.
 */
TEST (TestStorageFormatZwetschgeNoFixture, ZwetschgeWithCalibration)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    const auto image = getZwetschgeImage (calib);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (bridge)));

    royale::Vector<uint8_t> data;
    ASSERT_NO_THROW (data = reader->getExternalConfig().calibration->getCalibrationData());
    ASSERT_EQ (calib.size(), data.size());
    for (std::size_t i = 0; i < calib.size() ; i++)
    {
        ASSERT_EQ (data[i], calib[i]);
    }
}

/**
 * Tests that an image with Polar format gets a specific exception, if the Polar is written as it
 * would be on a normal device (at offset zero).
 */
TEST (TestStorageFormatZwetschgeNoFixture, PolarAtOffsetZero)
{
    // start with an image of the right size
    auto image = getImageWithCorruptMagic ({0});

    // make it contain the magic for Polar
    const auto magic = stringlikeMagicNumber ("PMDTEC");
    std::copy (magic.begin(), magic.end(), image.begin());

    auto bridge = std::make_shared<MockRandomAccessStorage> (image);
    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (bridge)));

    ASSERT_THROW (reader->getExternalConfig(), WrongDataFormatFound);
}

/**
 * Lena version of the PolarAtOffsetZero test.
 */
TEST (TestStorageFormatZwetschgeNoFixture, LenaAtOffsetZero)
{
    // start with an image of the right size
    auto image = getImageWithCorruptMagic ({0});

    // make it contain the magic for Lena
    const auto magic = stringlikeMagicNumber ("ROYALE-IMAGER-LENA-FILE");
    std::copy (magic.begin(), magic.end(), image.begin());

    auto bridge = std::make_shared<MockRandomAccessStorage> (image);
    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (bridge)));

    ASSERT_THROW (reader->getExternalConfig(), WrongDataFormatFound);
}

/**
 * Tests that an image that does not have Zwetschge at the expected location, but does have
 * Zwetschge magic at offset zero (so it's missing the expected firmware pages) causes a
 * WrongDataFormatFound.
 */
TEST (TestStorageFormatZwetschgeNoFixture, ZwetschgeAtOffsetZero)
{
    // start with an image of the right size
    auto image = getImageWithCorruptMagic ({0});
    std::copy (tocMagic.begin(), tocMagic.end(), image.begin());

    auto bridge = std::make_shared<MockRandomAccessStorage> (image);
    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (bridge)));

    ASSERT_THROW (reader->getExternalConfig(), WrongDataFormatFound);
}

/**
 * Tests that a valid Zwetschge image that additionally has Zwetschge magic at offset zero is
 * accepted.
 *
 * For this test a complete valid TOC is added at offset zero, but the StorageFormatZwetschge class
 * should ignore all of that.
 */
TEST (TestStorageFormatZwetschgeNoFixture, ZwetschgePlusZwetschgeAtOffsetZero)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    auto image = getZwetschgeImage (calib);

    const auto zwetschgeMagicOffset = std::size_t (0x2000u);
    assert (image[zwetschgeMagicOffset] == 'Z');

    auto copyLength = zwetschgeMagicOffset;
    if (zwetschgeMagicOffset + copyLength > image.size())
    {
        copyLength = image.size() - zwetschgeMagicOffset;
    }
    std::copy_n (image.begin() + zwetschgeMagicOffset, copyLength, image.begin());

    auto bridge = std::make_shared<MockRandomAccessStorage> (image);
    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (bridge)));
    ASSERT_NO_THROW (reader->getExternalConfig());
}

/**
 * Tests that an image with Polar format gets a specific exception, if the Polar is written after
 * the non-writable area at offset 0x2000u, the same offset as Zwetschge's magic.
 */
TEST (TestStorageFormatZwetschgeNoFixture, PolarAtOffset2000)
{
    const auto magic = stringlikeMagicNumber ("PMDTEC");
    auto image = getImageWithCorruptMagic (magic);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (bridge)));
    ASSERT_THROW (reader->getExternalConfig(), WrongDataFormatFound);
}

/**
 * Lena version of the PolarAtOffset2000 test.
 */
TEST (TestStorageFormatZwetschgeNoFixture, LenaAtOffset2000)
{
    const auto magic = stringlikeMagicNumber ("ROYALE-IMAGER-LENA-FILE");
    auto image = getImageWithCorruptMagic (magic);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (bridge)));
    ASSERT_THROW (reader->getExternalConfig(), WrongDataFormatFound);
}

/**
 * Tests that an otherwise valid TOC with incorrect CRC for the TOC causes an exception.
 */
TEST (TestStorageFormatZwetschgeNoFixture, ZwetschgeInvalidTocCrc)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    auto image = getZwetschgeImage (calib);
    // corrupt the CRC
    image[0x200a]++;
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (bridge)));

    ASSERT_ANY_THROW (reader->getExternalConfig());
}

/**
 * Tests that a valid TOC with corrupted calibration behaves as documented (getCalibrationData()
 * throws, getExternalConfig() doesn't).
 */
TEST (TestStorageFormatZwetschgeNoFixture, ZwetschgeInvalidCaliCrc)
{
    const std::vector<uint8_t> calib {1, 2, 3, 4};
    auto image = getZwetschgeImage (calib);
    const auto offsetOfCalibrationPointer = 0x2000 + 52;
    // here a failing assert means that the offsetOfCalibrationPointer is wrong
    assert (calib.size() == bufferToHost24 (image.data() + offsetOfCalibrationPointer + 3));
    auto calibrationStart = bufferToHost24 (image.data() + offsetOfCalibrationPointer);
    assert (calibrationStart + 4 <= image.size());
    image[calibrationStart]++;
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (bridge)));

    decltype (reader->getExternalConfig()) externalConfig;
    ASSERT_NO_THROW (externalConfig = reader->getExternalConfig());
    ASSERT_NE (nullptr, externalConfig.calibration);
    ASSERT_ANY_THROW (externalConfig.calibration->getCalibrationData());
    ASSERT_EQ (16u, externalConfig.calibration->getModuleIdentifier().size());
    ASSERT_NE (royale::Vector<uint8_t> (16u), externalConfig.calibration->getModuleIdentifier());
}

/**
 * Tests that a valid TOC with use cases is read.
 */
TEST (TestStorageFormatZwetschgeNoFixture, StorageFormatZwetschgeWithUseCases)
{
    const auto timedRegList = TimedRegisterList {{0x100, 0x0302, 0x00060504}};
    const ExampleUseCaseList exampleUseCases
    {
        {
            "EXAMPLE_5_PHASE",
            80, 40, // image size
            UseCaseIdentifier { "5 phase" },
            { 5 }, // block sizes
            { 30000000u, 80000000u, 80000000u, 80000000u, 80000000u },
            SequentialRegisterBlock {},
            timedRegList,
            50, 1, 50, // start, min, max fps
            ProcessingParameterId {"5 phase params"},
            std::chrono::milliseconds (10u), // wait time
            {0x1234}, // stream id
            {{"gray", {100, 500}, 400}, {"mod", {100, 600}, 550}},
            {{1, 30000000u, 0}, {4, 80000000u, 1}},
            royale::CameraAccessLevel::L1
        },
        {
            "EXAMPLE_CALIBRATION",
            80, 40, // image size
            UseCaseIdentifier { "11 phase" },
            { 5, 5, 1 }, // block sizes
            { 30000000u, 80000000u, 80000000u, 80000000u, 80000000u, 80000000u, 80000000u, 80000000u, 80000000u, 30000000u, 30000000u },
            SequentialRegisterBlock {},
            timedRegList,
            50, 1, 50, // start, min, max fps
            ProcessingParameterId {"calib params"},
            std::chrono::milliseconds (10u), // wait time
            {0x1234}, // stream id
            {{"gray", {100, 500}, 400}, {"mod", {100, 600}, 550}, {"mod", {100, 600}, 550}, {"gray", {100, 500}, 400}, {"gray", {100, 500}, 400}},
            {{1, 30000000u, 0}, {4, 80000000u, 1}, {4, 80000000u, 2}, {1, 30000000u, 3}, {1, 30000000u, 4}},
            royale::CameraAccessLevel::L3
        },
        // \todo ROYAL-3346 add an EXAMPLE_MIXED
    };

    const std::vector<uint8_t> exampleCalibration {1, 2, 3, 4};
    const auto image = getZwetschgeImage (exampleCalibration, exampleUseCases);
    auto bridge = std::make_shared<MockRandomAccessStorage> (image);

    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (bridge)));

    decltype (reader->getExternalConfig()) externalConfig;
    ASSERT_NO_THROW (externalConfig = reader->getExternalConfig());

    const auto &extConfig = externalConfig.imagerExternalConfig;
    ASSERT_NE (nullptr, extConfig);

    const auto initMap = extConfig->getInitializationMap();
    const auto startMap = extConfig->getStartMap();
    const auto stopMap = extConfig->getStopMap();
    ASSERT_EQ (size_t{ 0 }, initMap.size());
    ASSERT_EQ (size_t{ 0 }, startMap.size());
    ASSERT_EQ (size_t{ 0 }, stopMap.size());

    std::vector<IImagerExternalConfig::UseCaseData> imagerUcdList;
    ASSERT_NO_THROW (imagerUcdList = externalConfig.imagerExternalConfig->getUseCaseList());
    ASSERT_EQ (exampleUseCases.size(), imagerUcdList.size());
    auto &royaleUcdList = externalConfig.royaleUseCaseList;
    ASSERT_EQ (exampleUseCases.size(), royaleUcdList.size());

    for (auto i = std::size_t {0}; i < exampleUseCases.size(); i++)
    {
        const auto &example = exampleUseCases.at (i);
        const auto &imagerUcd = imagerUcdList.at (i);
        const auto &seqRegHeader = imagerUcd.sequentialRegisterHeader;
        if (example.seqRegBlock.values.empty())
        {
            ASSERT_EQ (0u, seqRegHeader.flashConfigAddress);
            ASSERT_EQ (0u, seqRegHeader.flashConfigSize);
            ASSERT_EQ (0u, seqRegHeader.imagerAddress);
        }
        else
        {
            ASSERT_NE (0u, seqRegHeader.flashConfigAddress);
            ASSERT_EQ (example.seqRegBlock.values.size() * sizeof (uint16_t), seqRegHeader.flashConfigSize);
            ASSERT_GE (image.size(), seqRegHeader.flashConfigAddress + seqRegHeader.flashConfigSize);
            ASSERT_EQ (0u, seqRegHeader.flashConfigAddress % 0x100) << "Use case flash config is not page-aligned, not useable by M2453 B11";
            auto placedData = iteratorBoundsCheck (image.data() + seqRegHeader.flashConfigAddress, image.data() + image.size(), seqRegHeader.flashConfigSize);
            for (auto j = std::size_t {0}; j < example.seqRegBlock.values.size(); j++)
            {
                ASSERT_EQ (example.seqRegBlock.values.at (j), bufferToHostBe16 (&placedData[j * sizeof (uint16_t)]));
            }
        }
        ASSERT_EQ (example.timedRegList.size(), imagerUcd.registerMap.size());
        for (auto j = std::size_t {0}; j < example.timedRegList.size(); j++)
        {
            const auto &a = example.timedRegList.at (j);
            const auto &b = imagerUcd.registerMap.at (j);

            ASSERT_EQ (a.address, b.address);
            ASSERT_EQ (a.value, b.value);
            ASSERT_NEAR (a.sleepTime, b.sleepTime, TIMED_REG_LIST_TIME_UNIT);
        }

        ASSERT_EQ (example.imageStreamBlockSizes.size(), imagerUcd.imageStreamBlockSizes.size());
        for (auto j = std::size_t {0}; j < example.imageStreamBlockSizes.size(); j++)
        {
            ASSERT_EQ (example.imageStreamBlockSizes.at (j), imagerUcd.imageStreamBlockSizes.at (j));
        }

        // In the format, the wait time is currently stored in exact microseconds, but we might
        // change to the same 16-bit compression as the TimedRegisterList. The test will accept
        // either option, but for eye safety the waitTime must be rounded up when compressed.
        ASSERT_LE (example.waitTime, imagerUcd.waitTime);
        ASSERT_GE (example.waitTime + std::chrono::microseconds (TIMED_REG_LIST_TIME_UNIT), imagerUcd.waitTime);

        const auto &royaleUc = royaleUcdList.at (i);
        ASSERT_EQ (example.name, royaleUc.getName().toStdString());
        ASSERT_EQ (example.accessLevel, royaleUc.getAccessLevel());
        ASSERT_EQ (example.procParamId, royaleUc.getProcessingParameterId());
        ASSERT_EQ (example.accessLevel, royaleUc.getAccessLevel());

        const auto royaleUcd = royaleUc.getDefinition();
        ASSERT_EQ (example.guid, royaleUcd->getIdentifier());
        ASSERT_EQ (example.minFps, royaleUcd->getMinRate());
        ASSERT_EQ (example.maxFps, royaleUcd->getMaxRate());
        ASSERT_EQ (example.startFps, royaleUcd->getTargetRate());

        // \todo ROYAL-3346 Support for stream IDs is postponed until the mixed-mode support, for
        // the current version the stream id can instead be assigned by
        // UseCaseDefinition::constructNonMixedUseCase()
        // ASSERT_EQ (example.streamIds, royaleUcd->getStreamIds().toStdVector());

        // The expo group's names aren't part of Zwetschge, and they may be different
        ASSERT_EQ (example.exposureGroups.size(), royaleUcd->getExposureGroups().size());
        for (auto j = std::size_t{ 0u }; j < example.exposureGroups.size(); ++j)
        {
            const auto &a = example.exposureGroups.at (j);
            const auto &b = royaleUcd->getExposureGroups().at (j);

            ASSERT_EQ (a.m_exposureLimits, b.m_exposureLimits);
            ASSERT_EQ (a.m_exposureTime, b.m_exposureTime);
        }

        uint16_t width, height;
        ASSERT_NO_THROW (royaleUcd->getImage (width, height));
        ASSERT_EQ (example.width, width);
        ASSERT_EQ (example.height, height);

        auto &calibration = externalConfig.calibration;
        ASSERT_NE (nullptr, calibration);
        ASSERT_EQ (exampleCalibration, calibration->getCalibrationData().toStdVector());
        ASSERT_EQ (idOfZwetschgeImage(), calibration->getModuleIdentifier().toStdVector());
    }
}

/**
 * This the equivalent of TestImagerLenaReader.ReadImagerRealFile, but using a Zwetschge file.
 *
 * The example device here uses only TimedRegisterLists, so could be in a file on the local
 * filesystem or could be in imager-accessible flash.
 */
TEST (TestStorageFormatZwetschgeNoFixture, LoadZwetschgeExampleDevice)
{
    // This data needs to match the data in ExampleDevice.py
    auto &productCode = zwetschgeExampleDeviceProductCode;
    auto &exampleUseCases = zwetschgeExampleDeviceUseCaseList;

    std::shared_ptr<royale::pal::IStorageReadRandom> storage;
    ASSERT_NO_THROW (storage = getZwetschgeExampleDevice());
    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (storage)));

    decltype (reader->getExternalConfig()) externalConfig;
    ASSERT_NO_THROW (externalConfig = reader->getExternalConfig());

    const auto &extConfig = externalConfig.imagerExternalConfig;
    ASSERT_NE (nullptr, extConfig);

    const auto initMap = extConfig->getInitializationMap();
    const auto startMap = extConfig->getStartMap();
    const auto stopMap = extConfig->getStopMap();
    ASSERT_EQ (size_t{ 2041 }, initMap.size());
    ASSERT_EQ (size_t{ 3 }, startMap.size());
    ASSERT_EQ (size_t{ 1 }, stopMap.size());

    std::vector<IImagerExternalConfig::UseCaseData> imagerUcdList;
    ASSERT_NO_THROW (imagerUcdList = externalConfig.imagerExternalConfig->getUseCaseList());
    ASSERT_EQ (exampleUseCases.size(), imagerUcdList.size());
    auto &royaleUcdList = externalConfig.royaleUseCaseList;
    ASSERT_EQ (exampleUseCases.size(), royaleUcdList.size());

    for (auto i = std::size_t {0}; i < exampleUseCases.size(); i++)
    {
        const auto &example = exampleUseCases.at (i);
        const auto &imagerUcd = imagerUcdList.at (i);

        // This test uses a device that doesn't use sequential register blocks
        const auto &seqRegHeader = imagerUcd.sequentialRegisterHeader;
        ASSERT_TRUE (example.seqRegBlock.values.empty());
        ASSERT_EQ (0u, seqRegHeader.flashConfigAddress);
        ASSERT_EQ (0u, seqRegHeader.flashConfigSize);
        ASSERT_EQ (0u, seqRegHeader.imagerAddress);

        ASSERT_EQ (example.timedRegList.size(), imagerUcd.registerMap.size());
        for (auto j = std::size_t {0}; j < example.timedRegList.size(); j++)
        {
            const auto &a = example.timedRegList.at (j);
            const auto &b = imagerUcd.registerMap.at (j);

            ASSERT_EQ (a.address, b.address);
            ASSERT_EQ (a.value, b.value);
            ASSERT_NEAR (a.sleepTime, b.sleepTime, TIMED_REG_LIST_TIME_UNIT);
        }

        ASSERT_EQ (example.imageStreamBlockSizes.size(), imagerUcd.imageStreamBlockSizes.size());
        for (auto j = std::size_t {0}; j < example.imageStreamBlockSizes.size(); j++)
        {
            ASSERT_EQ (example.imageStreamBlockSizes.at (j), imagerUcd.imageStreamBlockSizes.at (j));
        }

        // In the format, the wait time is currently stored in exact microseconds, but we might
        // change to the same 16-bit compression as the TimedRegisterList. The test will accept
        // either option, but for eye safety the waitTime must be rounded up when compressed.
        ASSERT_LE (example.waitTime, imagerUcd.waitTime);
        ASSERT_GE (example.waitTime + std::chrono::microseconds (TIMED_REG_LIST_TIME_UNIT), imagerUcd.waitTime);

        const auto &royaleUc = royaleUcdList.at (i);
        ASSERT_EQ (example.name, royaleUc.getName().toStdString());
        ASSERT_EQ (example.accessLevel, royaleUc.getAccessLevel());
        ASSERT_EQ (example.procParamId, royaleUc.getProcessingParameterId());
        ASSERT_EQ (example.accessLevel, royaleUc.getAccessLevel());

        const auto royaleUcd = royaleUc.getDefinition();
        ASSERT_EQ (example.guid, royaleUcd->getIdentifier());
        ASSERT_EQ (example.minFps, royaleUcd->getMinRate());
        ASSERT_EQ (example.maxFps, royaleUcd->getMaxRate());
        ASSERT_EQ (example.startFps, royaleUcd->getTargetRate());

        // \todo ROYAL-3346 Support for stream IDs is postponed until the mixed-mode support, for
        // the current version the stream id can instead be assigned by
        // UseCaseDefinition::constructNonMixedUseCase()
        // ASSERT_EQ (example.streamIds, royaleUcd->getStreamIds().toStdVector());

        // The expo group's names aren't part of Zwetschge, and they may be different
        ASSERT_EQ (example.exposureGroups.size(), royaleUcd->getExposureGroups().size());
        for (auto j = std::size_t{ 0u }; j < example.exposureGroups.size(); ++j)
        {
            const auto &a = example.exposureGroups.at (j);
            const auto &b = royaleUcd->getExposureGroups().at (j);

            ASSERT_EQ (a.m_exposureLimits, b.m_exposureLimits);
            ASSERT_EQ (a.m_exposureTime, b.m_exposureTime);
        }

        uint16_t width, height;
        ASSERT_NO_THROW (royaleUcd->getImage (width, height));
        ASSERT_EQ (example.width, width);
        ASSERT_EQ (example.height, height);

        auto &calibration = externalConfig.calibration;
        ASSERT_NE (nullptr, calibration);
        ASSERT_THROW (calibration->getCalibrationData(), DataNotFound);

        ASSERT_EQ (productCode, calibration->getModuleIdentifier());
        ASSERT_EQ (16u, calibration->getModuleIdentifier().size());
        ASSERT_NE (royale::Vector<uint8_t> (16), calibration->getModuleIdentifier());
    }
}

/**
 * This the equivalent of TestImagerLenaReader.ReadImagerRealFile, but using a Zwetschge flash
 * image.
 *
 * The example here is meant to be a flash image for storage on imager-accessible flash, so can
 * (and should) use SequentialRegisterBlocks instead of TimedRegisterLists.
 */
TEST (TestStorageFormatZwetschgeNoFixture, LoadZwetschgeExampleFlashImage)
{
    // This data needs to match the data in ExampleDevice.py, and the test relies on the
    // version using sequential register blocks to match the TimedRegisterList version.
    auto &productCode = zwetschgeExampleDeviceProductCode;
    auto &exampleUseCases = zwetschgeExampleFlashImageUseCaseList;

    std::shared_ptr<royale::pal::IStorageReadRandom> storage;
    ASSERT_NO_THROW (storage = getZwetschgeExampleFlashImage());
    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (storage)));

    decltype (reader->getExternalConfig()) externalConfig;
    ASSERT_NO_THROW (externalConfig = reader->getExternalConfig());

    const auto &extConfig = externalConfig.imagerExternalConfig;
    ASSERT_NE (nullptr, extConfig);

    const auto initMap = extConfig->getInitializationMap();
    const auto startMap = extConfig->getStartMap();
    const auto stopMap = extConfig->getStopMap();
    ASSERT_EQ (size_t{ 2041 }, initMap.size());
    ASSERT_EQ (size_t{ 3 }, startMap.size());
    ASSERT_EQ (size_t{ 1 }, stopMap.size());

    std::vector<IImagerExternalConfig::UseCaseData> imagerUcdList;
    ASSERT_NO_THROW (imagerUcdList = externalConfig.imagerExternalConfig->getUseCaseList());
    ASSERT_EQ (exampleUseCases.size(), imagerUcdList.size());
    auto &royaleUcdList = externalConfig.royaleUseCaseList;
    ASSERT_EQ (exampleUseCases.size(), royaleUcdList.size());

    for (auto i = std::size_t {0}; i < exampleUseCases.size(); i++)
    {
        const auto &example = exampleUseCases.at (i);
        const auto &imagerUcd = imagerUcdList.at (i);

        const auto &seqRegHeader = imagerUcd.sequentialRegisterHeader;
        ASSERT_LT (0x2000u, seqRegHeader.flashConfigAddress);
        ASSERT_EQ (0u, seqRegHeader.flashConfigAddress % 0x100) << "Use case flash config is not page-aligned, not useable by M2453 B11";
        ASSERT_EQ (example.seqRegBlock.values.size() * 2, seqRegHeader.flashConfigSize);
        ASSERT_EQ (example.seqRegBlock.imagerAddress, seqRegHeader.imagerAddress);

        // The readStorage call will throw if the image is too small for the data
        std::vector<uint8_t> flashData (seqRegHeader.flashConfigSize);
        ASSERT_NO_THROW (storage->readStorage (seqRegHeader.flashConfigAddress, flashData));

        for (auto j = std::size_t {0}; j < example.timedRegList.size(); j++)
        {
            ASSERT_EQ (example.seqRegBlock.values.at (j), bufferToHostBe16 (&flashData[2 * j]));
        }

        ASSERT_EQ (0u, imagerUcd.registerMap.size());

        ASSERT_EQ (example.imageStreamBlockSizes.size(), imagerUcd.imageStreamBlockSizes.size());
        for (auto j = std::size_t {0}; j < example.imageStreamBlockSizes.size(); j++)
        {
            ASSERT_EQ (example.imageStreamBlockSizes.at (j), imagerUcd.imageStreamBlockSizes.at (j));
        }

        // In the format, the wait time is currently stored in exact microseconds, but we might
        // change to the same 16-bit compression as the TimedRegisterList. The test will accept
        // either option, but for eye safety the waitTime must be rounded up when compressed.
        ASSERT_LE (example.waitTime, imagerUcd.waitTime);
        ASSERT_GE (example.waitTime + std::chrono::microseconds (TIMED_REG_LIST_TIME_UNIT), imagerUcd.waitTime);

        const auto &royaleUc = royaleUcdList.at (i);
        ASSERT_EQ (example.name, royaleUc.getName().toStdString());
        ASSERT_EQ (example.accessLevel, royaleUc.getAccessLevel());
        ASSERT_EQ (example.procParamId, royaleUc.getProcessingParameterId());
        ASSERT_EQ (example.accessLevel, royaleUc.getAccessLevel());

        const auto royaleUcd = royaleUc.getDefinition();
        ASSERT_EQ (example.guid, royaleUcd->getIdentifier());
        ASSERT_EQ (example.minFps, royaleUcd->getMinRate());
        ASSERT_EQ (example.maxFps, royaleUcd->getMaxRate());
        ASSERT_EQ (example.startFps, royaleUcd->getTargetRate());

        // \todo ROYAL-3346 Support for stream IDs is postponed until the mixed-mode support, for
        // the current version the stream id can instead be assigned by
        // UseCaseDefinition::constructNonMixedUseCase()
        // ASSERT_EQ (example.streamIds, royaleUcd->getStreamIds().toStdVector());

        // The expo group's names aren't part of Zwetschge, and they may be different
        ASSERT_EQ (example.exposureGroups.size(), royaleUcd->getExposureGroups().size());
        for (auto j = std::size_t{ 0u }; j < example.exposureGroups.size(); ++j)
        {
            const auto &a = example.exposureGroups.at (j);
            const auto &b = royaleUcd->getExposureGroups().at (j);

            ASSERT_EQ (a.m_exposureLimits, b.m_exposureLimits);
            ASSERT_EQ (a.m_exposureTime, b.m_exposureTime);
        }

        uint16_t width, height;
        ASSERT_NO_THROW (royaleUcd->getImage (width, height));
        ASSERT_EQ (example.width, width);
        ASSERT_EQ (example.height, height);

        auto &calibration = externalConfig.calibration;
        ASSERT_NE (nullptr, calibration);
        ASSERT_THROW (calibration->getCalibrationData(), DataNotFound);

        ASSERT_EQ (productCode, calibration->getModuleIdentifier());
        ASSERT_EQ (16u, calibration->getModuleIdentifier().size());
        ASSERT_NE (royale::Vector<uint8_t> (16), calibration->getModuleIdentifier());
    }
}

TEST (TestStorageFormatZwetschgeNoFixture, ConversionFromSeqToTimed)
{
    auto &exampleUseCases = zwetschgeExampleDeviceUseCaseListSeqConverted;

    std::shared_ptr<royale::pal::IStorageReadRandom> storage;
    ASSERT_NO_THROW (storage = getZwetschgeExampleFlashImage());
    std::unique_ptr<StorageFormatZwetschge> reader;
    ASSERT_NO_THROW (reader.reset (new StorageFormatZwetschge (storage)));

    decltype (reader->getExternalConfig()) externalConfig;
    ASSERT_NO_THROW (externalConfig = reader->getExternalConfig (true));

    const auto &extConfig = externalConfig.imagerExternalConfig;
    ASSERT_NE (nullptr, extConfig);

    std::vector<IImagerExternalConfig::UseCaseData> imagerUcdList;
    ASSERT_NO_THROW (imagerUcdList = externalConfig.imagerExternalConfig->getUseCaseList());
    ASSERT_EQ (exampleUseCases.size(), imagerUcdList.size());
    auto &royaleUcdList = externalConfig.royaleUseCaseList;
    ASSERT_EQ (exampleUseCases.size(), royaleUcdList.size());

    for (auto i = std::size_t{ 0 }; i < exampleUseCases.size(); i++)
    {
        const auto &example = exampleUseCases.at (i);
        const auto &imagerUcd = imagerUcdList.at (i);

        ASSERT_EQ (example.timedRegList.size(), imagerUcd.registerMap.size());
        for (auto j = std::size_t{ 0 }; j < example.timedRegList.size(); j++)
        {
            ASSERT_EQ (example.timedRegList.at (j).address, imagerUcd.registerMap.at (j).address);
            ASSERT_EQ (example.timedRegList.at (j).value, imagerUcd.registerMap.at (j).value);
            ASSERT_EQ (example.timedRegList.at (j).sleepTime, imagerUcd.registerMap.at (j).sleepTime);
        }
    }
}
