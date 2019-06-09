/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <StorageTestUtils.hpp>
#include <ZwetschgeTestUtils.hpp>

#include <storage/StorageFile.hpp>

#include <common/Crc32.hpp>
#include <common/EndianConversion.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/NarrowCast.hpp>
#include <imager/WrapperImagerExternalConfig.hpp>
#include <processing/ProcessingParameterId.hpp>
#include <royale/StreamId.hpp>
#include <usecase/ExposureGroup.hpp>
#include <usecase/UseCaseIdentifier.hpp>

#include <gmock/gmock.h>

#include <RoyaleLogger.hpp>

#include <algorithm>
#include <cassert>
#include <memory>

using namespace royale::common;
using namespace royale::imager;
using namespace royale::pal;
using namespace royale::processing;
using namespace royale::storage;
using namespace royale::test::utils;
using namespace royale::test::utils::zwetschge;
using namespace royale::usecase;

namespace
{
    /**
     * There is no alignment requirement for the M2453 itself, but the storage device might have
     * requirements, this depends on which flash chip the customer is using in the module.
     *
     * Assuming the code from the unit test will eventually become the code for creating Zwetschge
     * format images, put some alignment in.
     */
    const auto ALIGNMENT_SEQUENTIAL_REGISTER_BLOCK = 256;

    const auto tocMagic = stringlikeMagicNumber ("ZWETSCHGE");
    const auto toucMagic = stringlikeMagicNumber ("Elena");
    const auto tormMagic = stringlikeMagicNumber ("eLENA");
    const auto exampleProductIssuer = stringlikeMagicNumber ("TEST");
    const auto exampleProductCode = stringlikeMagicNumber ("identifier", 16u);
    const auto exampleModuleSerial = stringlikeMagicNumber ("1111-2222-3333-4444");

    /**
     * An ExampleUseCase which has the address for the seqRegBlock, or zeros if the ucd doesn't have
     * a seqRegBlock.  This is used for communication between getZwetschgeImage and getTableOfUseCases.
     *
     * The code that creates these stores the ucd.seqRegBlock.values data in the image, so
     * that reading from this address will return the expected data.
     */
    struct PlacedExampleUseCase
    {
        const ExampleUseCase &ucd;
        SequentialRegisterHeader seqRegHeader;
    };
}

royale::config::ExternalConfigFileConfig royale::test::utils::getNvsfGetterForZwetschgeExampleDevice()
{
#ifndef EXAMPLE_DEVICE_FOR_ZWETSCHGE
#error The test environment needs to define EXAMPLE_DEVICE_FOR_ZWETSCHGE via CMake
#endif
    return royale::config::ExternalConfigFileConfig::fromZwetschgeFile (EXAMPLE_DEVICE_FOR_ZWETSCHGE);
}

std::shared_ptr<royale::pal::IStorageReadRandom> royale::test::utils::getZwetschgeExampleDevice()
{
#ifndef EXAMPLE_DEVICE_FOR_ZWETSCHGE
#error The test environment needs to define EXAMPLE_DEVICE_FOR_ZWETSCHGE via CMake
#endif
    const royale::String zwetschgeFilename { EXAMPLE_DEVICE_FOR_ZWETSCHGE };
    const royale::config::FlashMemoryConfig storageFileConfig {};
    return std::make_shared<StorageFile> (storageFileConfig, zwetschgeFilename);
}

std::vector<uint8_t> royale::test::utils::idOfZwetschgeExampleDevice()
{
    // productCode = uuid.uuid5 (uuid.NAMESPACE_URL, "http://www.example.com/"),
    auto productCode = parseRfc4122AsUuid ("{fcde3c85-2270-590f-9e7c-ee003d65e0e2}");
    return std::vector<uint8_t> (productCode.cbegin(), productCode.cend());
}

std::shared_ptr<royale::pal::IStorageReadRandom> royale::test::utils::getZwetschgeExampleFlashImage()
{
#ifndef EXAMPLE_FLASH_IMAGE_FOR_ZWETSCHGE
#error The test environment needs to define EXAMPLE_FLASH_IMAGE_FOR_ZWETSCHGE via CMake
#endif
    const royale::String zwetschgeFilename { EXAMPLE_FLASH_IMAGE_FOR_ZWETSCHGE };
    const royale::config::FlashMemoryConfig storageFileConfig {};
    return std::make_shared<StorageFile> (storageFileConfig, zwetschgeFilename);
}

std::vector<uint8_t> royale::test::utils::idOfZwetschgeExampleFlashImage()
{
    return idOfZwetschgeExampleDevice();
}

namespace
{
    /**
     * Several tables have the structure:
     * magic
     * CRC of all data except the magic and CRC fields
     * rest of table (starts offset magic.size() + crc32.size())
     */
    void insertCrc (std::vector<uint8_t> &table, std::size_t magicLen)
    {
        const auto dataOffset = magicLen + 4u;
        std::vector<uint8_t> crc;
        pushBack32 (crc, calculateCRC32 (table.data() + dataOffset, table.size() - dataOffset));
        std::copy (crc.begin(), crc.end(), table.begin() + magicLen);
    }

    /**
     * "push back Table of Contents Entry", convenience method for adding the (24p + 24s) entries
     * used in the Table of Contents.
     */
    void pbTocEntry (std::vector<uint8_t> &table, std::size_t addr, std::size_t size)
    {
        // The pushBack24 function itself includes a check that the uint32_t values can be further
        // reduced by a narrow_cast_24 or narrow_cast<uint24_t>.
        pushBack24 (table, narrow_cast<uint32_t> (addr));
        pushBack24 (table, narrow_cast<uint32_t> (size));
    }

    /**
     * "push back SequentialRegisterHeader", convenience method for adding the (24p + 24s + 16a)
     * entries used for pointers to SequentialRegisterBlocks.
     *
     * If a SequentialRegisterBlock is also given, this will do a consistency check and assert if
     * the header doesn't seem to correspond to that block.
     */
    void pbSeqRegHeader (std::vector<uint8_t> &dest, const SequentialRegisterHeader &header,
                         const SequentialRegisterBlock *block = nullptr)
    {
        if (block)
        {
            // The size is in bytes, not a count of registers
            // These do not need to be read by Royale, the imager will load them itself
            // Thsee asserts check that either all numbers are zero, or all are non-zero
            if (block->values.empty())
            {
                assert (! block->imagerAddress);
                assert (! (header.flashConfigAddress || header.flashConfigSize || header.imagerAddress));
            }
            else
            {
                assert (block->imagerAddress);
                assert (header.flashConfigAddress && header.flashConfigSize && header.imagerAddress);
            }
        }

        pbTocEntry (dest, header.flashConfigAddress, header.flashConfigSize);
        pushBack16 (dest, header.imagerAddress);
    }

    /**
     * "push back TimedRegisterList", convenience method for adding the data used in the use cases
     * and in the Table of Register Maps. This does not record the size of the list, that data has
     * to be stored separately.
     */
    void pbTimedRegisterList (std::vector<uint8_t> &dest, const TimedRegisterList &timedRegList)
    {
        for (auto x : timedRegList)
        {
            pushBack16 (dest, x.address);
            pushBack16 (dest, x.value);
            pushBack16 (dest, narrow_cast<uint16_t> (x.sleepTime / TIMED_REG_LIST_TIME_UNIT));
        }
    }

    /**
     * "push back Firmware Page", convenience method for adding the contents of a
     * SequentialRegisterBlock to the image. The method returns a corresponding
     * SequentialRegisterHeader identifying where in the image the data is.
     *
     * If the block is empty, this method will return a corresponding header with all of the data
     * items zero.
     */
    SequentialRegisterHeader pbFirmwarePage (std::vector<uint8_t> &dest, const SequentialRegisterBlock &block)
    {
        SequentialRegisterHeader header {0u, 0u, 0u};
        if (block.values.empty())
        {
            return header;
        }

        if (dest.size() % ALIGNMENT_SEQUENTIAL_REGISTER_BLOCK)
        {
            const auto paddingToAdd = ALIGNMENT_SEQUENTIAL_REGISTER_BLOCK - (dest.size() % ALIGNMENT_SEQUENTIAL_REGISTER_BLOCK);
            pushBackPadded (dest, std::vector<uint8_t> {}, paddingToAdd);
        }
        header.flashConfigAddress = narrow_cast<uint32_t> (dest.size());
        for (auto value : block.values)
        {
            pushBack16 (dest, value);
        }
        header.flashConfigSize = narrow_cast<uint32_t> (dest.size() - header.flashConfigAddress);
        header.imagerAddress = block.imagerAddress;

        return header;
    }

    std::vector<uint8_t> getTableOfUseCases (const std::vector<PlacedExampleUseCase> &examples)
    {
        std::vector<uint8_t> table;

        pushBackIterable (table, toucMagic);
        pushBack32 (table, 0u); // space for the CRC

        // For every use case :
        for (const auto &example : examples)
        {
            auto ucd = example.ucd;
            std::vector<uint8_t> useCase;

            // Each block starts with the size of this block of data (16-bit), this is added at the
            // end of this iteration of the for loop.

            // seqRegBlock - registers that will be loaded by the imager (24p + 24s + 16a)
            pbSeqRegHeader (useCase, example.seqRegHeader, &ucd.seqRegBlock);

            // Image size (pixels)
            pushBack16 (useCase, ucd.width);
            pushBack16 (useCase, ucd.height);

            // UseCaseIdentifier GUID (128-bit, little-endian)
            pushBackPadded (useCase, ucd.guid.data(), 16);

            useCase.push_back (ucd.startFps);
            useCase.push_back (ucd.minFps);
            useCase.push_back (ucd.maxFps);

            // Processing parameter ID (128-bit, little-endian)
            pushBackPadded (useCase, ucd.procParamId.data(), 16);

            // On the next line, pushBack24 will already check that bits 31-24 are all zero
            pushBack24 (useCase, narrow_cast<uint32_t> (ucd.waitTime.count()));
            if (ucd.accessLevel == royale::CameraAccessLevel::L3)
            {
                useCase.push_back (1); // AccessLevel::L3_AND_RAW
            }
            else
            {
                useCase.push_back (0); // AccessLevel::NORMAL
            }

            // Length of use case name (8-bit) and counts of other things (16-bit)
            useCase.push_back (narrow_cast<uint8_t> (ucd.name.length()));
            pushBack16 (useCase, narrow_cast<uint16_t> (ucd.imageStreamBlockSizes.size()));
            pushBack16 (useCase, narrow_cast<uint16_t> (ucd.modulationFrequencies.size()));
            pushBack16 (useCase, narrow_cast<uint16_t> (ucd.timedRegList.size()));
            if (ucd.streamIds.empty())
            {
                LOG (WARN) << "Zwetschge test is creating a use case with no streams";
            }
            else if (ucd.streamIds.size() != 1)
            {
                throw NotImplemented ("mixed mode Zwetschge is not supported yet");
            }
            useCase.push_back (narrow_cast<uint8_t> (ucd.streamIds.size()));
            useCase.push_back (narrow_cast<uint8_t> (ucd.exposureGroups.size()));
            pushBack16 (useCase, narrow_cast<uint16_t> (ucd.rawFrameSets.size()));
            useCase.push_back (0u); // length of the reserved block

            // Use case name (ASCII string, without terminator)
            pushBackIterable (useCase, stringlikeMagicNumber (ucd.name));
            // Sizes of measurement blocks, corresponding to IImager::getMeasurementBlockSizes() (16-bit)
            for (auto x : ucd.imageStreamBlockSizes)
            {
                pushBack16 (useCase, x);
            }
            for (auto x : ucd.modulationFrequencies)
            {
                pushBack32 (useCase, x);
            }
            pbTimedRegisterList (useCase, ucd.timedRegList);
            for (const auto &streamId : ucd.streamIds)
            {
                pushBack16 (useCase, streamId);
            }
            for (const auto &exposureGroup : ucd.exposureGroups)
            {
                pushBack16 (useCase, narrow_cast<uint16_t> (exposureGroup.m_exposureTime));
                pushBack16 (useCase, narrow_cast<uint16_t> (exposureGroup.m_exposureLimits.first));
                pushBack16 (useCase, narrow_cast<uint16_t> (exposureGroup.m_exposureLimits.second));
            }
            for (const auto &rawFrameSet : ucd.rawFrameSets)
            {
                useCase.push_back (narrow_cast<uint8_t> (rawFrameSet.frameCount));
                pushBack32 (useCase, rawFrameSet.frequency);
                useCase.push_back (narrow_cast<uint8_t> (rawFrameSet.exposureGroupIdx));
            }
            // zero-byte reserved block

            // Prefix the use case's block with its size, the +2 is for the size counter itself
            pushBack16 (table, narrow_cast<uint16_t> (useCase.size() + 2));
            pushBackIterable (table, useCase);
        }

        insertCrc (table, toucMagic.size());

        return table;
    }

    std::vector<uint8_t> getTableOfRegisterMaps (const IImagerExternalConfig &extConfig,
            const std::vector<SequentialRegisterHeader> &placedFirmwarePages)
    {
        std::vector<uint8_t> table;
        pushBackIterable (table, tormMagic);
        pushBack32 (table, 0u); // space for the CRC
        pushBack24 (table, 0u); // version number

        assert (placedFirmwarePages.size() == 2);
        for (const auto &page : placedFirmwarePages)
        {
            pbSeqRegHeader (table, page);
        }

        pushBack16 (table, narrow_cast<uint16_t> (extConfig.getInitializationMap().size()));
        pushBack16 (table, narrow_cast<uint16_t> (extConfig.getFirmwarePage1().size()));
        pushBack16 (table, narrow_cast<uint16_t> (extConfig.getFirmwarePage2().size()));
        pushBack16 (table, narrow_cast<uint16_t> (extConfig.getFirmwareStartMap().size()));
        pushBack16 (table, narrow_cast<uint16_t> (extConfig.getStartMap().size()));
        pushBack16 (table, narrow_cast<uint16_t> (extConfig.getStopMap().size()));

        pbTimedRegisterList (table, extConfig.getInitializationMap());
        pbTimedRegisterList (table, extConfig.getFirmwarePage1());
        pbTimedRegisterList (table, extConfig.getFirmwarePage2());
        pbTimedRegisterList (table, extConfig.getFirmwareStartMap());
        pbTimedRegisterList (table, extConfig.getStartMap());
        pbTimedRegisterList (table, extConfig.getStopMap());

        insertCrc (table, tormMagic.size());
        return table;
    }

    /**
     * Common implementation of the getZwetschgeImage methods, relying on the assumptions that are ensured in
     * the getZwetschgeImage methods.
     *
     * The raw data that will be in the MiraBelle's storage. The image is quite large for a unit
     * test, but reasonable considering that Royale will normally be processing large buffers from
     * the imager.
     *
     * The use cases will be taken from the useCases argument, and the non-use-case register maps
     * from the extConfig.
     *
     * The only part of the extConfig that is used is the table of register maps.
     */
    std::vector<uint8_t> getImageAssumingArgumentsCorrect (const std::vector<uint8_t> &calibration, const ExampleUseCaseList &useCases = {}, const IImagerExternalConfig &extConfig = WrapperImagerExternalConfig {})
    {
        std::vector<uint8_t> image;

        std::vector<uint8_t> imagersWriteOnlyArea;
        pushBackIterable (imagersWriteOnlyArea, stringlikeMagicNumber ("Zwetschge file (first 0x1000 bytes are the write-only part for the imager)"));
        pushBackPadded (image, imagersWriteOnlyArea, 0x1000);

        std::vector<uint8_t> imagersBootRegisters;
        pushBackIterable (imagersBootRegisters, stringlikeMagicNumber ("this is the boot-register part"));
        pushBackPadded (image, imagersBootRegisters, 0x1000);

        // Reserve space for the table of contents. It will need a CRC calculation that includes
        // the sizes of the subblocks, so don't create it yet.
        const auto startOfToc = narrow_cast<uint32_t> (image.size());
        assert (0x2000 == startOfToc); // Code to create the StorageFormatZwetschge format has misplaced the ToC
        pushBackPadded (image, std::vector<uint8_t> {}, 0x1000);

        const auto startOfCalibration = narrow_cast<uint32_t> (image.size());
        pushBackIterable (image, calibration);

        // The registers for the imager to read, in a real image should be somewhere page-aligned,
        // but in this image are added after the calibration
        std::vector<PlacedExampleUseCase> placedUseCases;
        for (const auto &ucd : useCases)
        {
            auto header = pbFirmwarePage (image, ucd.seqRegBlock);
            placedUseCases.push_back ({ucd, header});
        }

        // And now the firmware pages, if they're given as SequentialRegisterBlocks
        // This currently isn't used by the tests, so two empty blocks are added instead;
        // images containing these blocks can be tested by creating images with the
        // ZwetschgeTool.
        std::vector<SequentialRegisterHeader> placedFirmwarePages;
        {
            const auto firmwareBlocks = std::vector<SequentialRegisterBlock> {{}, {}};

            for (const auto &page : firmwareBlocks)
            {
                auto header = pbFirmwarePage (image, page);
                placedFirmwarePages.push_back (header);
            }
        }

        const auto startOfUseCaseTable = narrow_cast<uint32_t> (image.size());
        const auto tableOfUseCases = getTableOfUseCases (placedUseCases);
        pushBackIterable (image, tableOfUseCases);

        const auto startOfRegisterMapTable = narrow_cast<uint32_t> (image.size());
        const auto tableOfRegisterMaps = getTableOfRegisterMaps (extConfig, placedFirmwarePages);
        pushBackIterable (image, tableOfRegisterMaps);

        // Construct the CRC32-checked part of the Table of Contents,
        std::vector<uint8_t> toc;
        pushBackIterable (toc, tocMagic);
        pushBack32 (toc, 0u); // space for the CRC
        pushBack24 (toc, 0x147); // Version of ToC (24-bit)
        pbTocEntry (toc, 0u, 0u); // Embedded-system specific data / USB specific data (24p + 24s, both zero if unused)
        pushBackPadded (toc, exampleProductIssuer, 4u);
        pushBackPadded (toc, exampleProductCode, 16u);
        pushBack32 (toc, 0u); // system frequency
        pbTocEntry (toc, startOfRegisterMapTable, tableOfRegisterMaps.size());
        pbTocEntry (toc, startOfCalibration, calibration.size());
        pushBack32 (toc, calculateCRC32 (calibration.data(), calibration.size()));
        toc.push_back (narrow_cast<uint8_t> (useCases.size())); // Number of use cases (8-bit)
        pbTocEntry (toc, startOfUseCaseTable, tableOfUseCases.size());
        pushBackPadded (toc, exampleModuleSerial, 19u);
        insertCrc (toc, tocMagic.size());
        std::copy (toc.begin(), toc.end(), image.begin() + startOfToc);

        return image;
    }
}

std::vector<uint8_t> royale::test::utils::zwetschge::getZwetschgeImage (const std::vector<uint8_t> &calibration, const ExampleUseCaseList &useCases)
{
    WrapperImagerExternalConfig extConfig;
    return getImageAssumingArgumentsCorrect (calibration, useCases, extConfig);
}

std::vector<uint8_t> royale::test::utils::zwetschge::idOfZwetschgeImage()
{
    return exampleProductCode;
}
