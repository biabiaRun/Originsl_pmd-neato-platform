/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <config/IImagerExternalConfig.hpp>
#include <config/ImagerConfig.hpp>
#include <pal/IStorageReadRandom.hpp>
#include <processing/ProcessingParameterId.hpp>
#include <royale/CameraAccessLevel.hpp>
#include <royale/StreamId.hpp>
#include <usecase/ExposureGroup.hpp>
#include <usecase/UseCaseIdentifier.hpp>

#include <chrono>
#include <memory>

/*
 * This file contains functions for loading existing Zwetschge images for use in tests, both of the
 * Zwetschge reader itself and also for tests of module config factories, etc.
 */
namespace royale
{
    namespace test
    {
        namespace utils
        {
            /**
             * There is a pregenerated file created from the ExampleDevice.py input file. This
             * method returns the filename, in the structure used to get NonVolatileStorageFactory
             * to open that file.
             */
            ROYALE_API royale::config::ExternalConfigFileConfig getNvsfGetterForZwetschgeExampleDevice();

            /**
             * There is a pregenerated file created from the ExampleDevice.py input file. This
             * method opens that file and returns the corresponding reader (or throws on failure).
             *
             * This device uses TimedRegisterLists, and does not use SequentialRegisterBlocks.
             *
             * It returns the IStorageReadRandom, because the test may wish to access the raw data
             * in addition to parsing it with StorageFormatZwetschge.
             */
            ROYALE_API std::shared_ptr<royale::pal::IStorageReadRandom> getZwetschgeExampleDevice();

            /**
             * Returns a (hardcoded) copy of the product ID of the ExampleDevice.
             */
            ROYALE_API std::vector<uint8_t> idOfZwetschgeExampleDevice();

            /**
             * There is a pregenerated file created from the ExampleFlashImage.py input file. This
             * method opens that file and returns the corresponding reader (or throws on failure).
             *
             * This device uses SequentialRegisterBlocks, and may also use TimedRegisterLists.
             *
             * It returns the IStorageReadRandom, because the test may wish to access the raw data
             * in addition to parsing it with StorageFormatZwetschge.
             */
            ROYALE_API std::shared_ptr<royale::pal::IStorageReadRandom> getZwetschgeExampleFlashImage();

            /**
             * Returns a (hardcoded) copy of the product ID of the ExampleFlashImage; this may be
             * the same as the product ID of ExampleDevice.
             */
            ROYALE_API std::vector<uint8_t> idOfZwetschgeExampleFlashImage();

            /**
             * This sub-namespace has functions for creating a Zwetschge image via
             * getZwetschgeImage(), containing data specified by the caller.
             */
            namespace zwetschge
            {
                /**
                 * In the TimedRegisterList, the delays are stored at a lower resolution than the
                 * IImagerExternalConfig's data type (32 microsecond units vs 1 microsecond units).
                 */
                const auto TIMED_REG_LIST_TIME_UNIT = uint32_t
                                                      {
                                                          32
                                                      };

                /**
                 * A block of register values that the imager should read directly from the storage.
                 *
                 * This doesn't include the address in the storage that the data will be stored at.
                 * The utility functions will choose an address, and return a corresponding
                 * SequentialRegisterHeader pointing to this block of data.
                 */
                struct SequentialRegisterBlock
                {
                    SequentialRegisterBlock (std::vector<uint16_t> values, uint16_t imagerAddress) :
                        values (std::move (values)),
                        imagerAddress (imagerAddress)
                    {
                        if (imagerAddress)
                        {
                            assert (!this->values.empty());
                        }
                        else
                        {
                            assert (this->values.empty());
                        }
                    }

                    SequentialRegisterBlock ()
                        : SequentialRegisterBlock ( {}, 0)
                    {
                    }

                    std::vector<uint16_t> values;
                    uint16_t imagerAddress;
                };

                /**
                 * The core RawFrameSet includes many items for the software imager, which are not
                 * relevant to testing Zwetschge. While StorageFormatZwetschge itself creates
                 * UseCaseDefinitions containing instances of the core RawFrameSet, setting up tests
                 * requires only this subset.
                 */
                struct ExampleRawFrameSet
                {
                    std::size_t frameCount;
                    uint32_t frequency;
                    royale::usecase::ExposureGroupIdx exposureGroupIdx;
                };

                /**
                 * Structure for the tests to specify the data which will be read as an
                 * IImagerExternalConfig::UseCaseData and usecase::UseCase. Because it contains
                 * enough data to generate both of those classes, it has a separate data class
                 * instead of reusing one of the existing ones.
                 *
                 * If a SequentialRegisterBlock is given, this expects the data to be stored in the
                 * flash, but doesn't specify the location, instead it gives the content of that
                 * seqRegBlock. The getZwetschgeImage function will generate an image including that
                 * data, and add to the tables in the image the address that Zwetschge should tell
                 * the software imager that the hardware imager can read it from.
                 *
                 * If both a non-empty SequentialRegisterBlock and a non-empty TimedRegisterList are
                 * given, both will be put in to the image. The spec says this is invalid and
                 * StorageFormatZwetschge is expected to throw when given such an image.
                 */
                struct ExampleUseCase
                {
                    std::string name;
                    uint16_t width;
                    uint16_t height;
                    royale::usecase::UseCaseIdentifier guid;
                    std::vector<uint16_t> imageStreamBlockSizes;
                    std::vector<uint32_t> modulationFrequencies;
                    SequentialRegisterBlock seqRegBlock;
                    royale::imager::TimedRegisterList timedRegList;
                    uint8_t startFps;
                    uint8_t minFps;
                    uint8_t maxFps;
                    royale::processing::ProcessingParameterId procParamId;
                    std::chrono::microseconds waitTime;
                    std::vector<royale::StreamId> streamIds;
                    std::vector<royale::usecase::ExposureGroup> exposureGroups;
                    std::vector<ExampleRawFrameSet> rawFrameSets;
                    royale::CameraAccessLevel accessLevel;
                };
                using ExampleUseCaseList = std::vector<ExampleUseCase>;

                /**
                 * Create the raw data that will be in the MiraBelle's storage. The image is quite
                 * large for a unit test, but reasonable considering that Royale will normally be
                 * processing large buffers from the imager.
                 *
                 * The use cases will be taken from the useCases argument, if that argument is not
                 * given then it will use an empty map.
                 *
                 * The non-use-case Table of Register Maps will contain only empty lists.
                 */
                std::vector<uint8_t> getZwetschgeImage (const std::vector<uint8_t> &calibration, const ExampleUseCaseList &useCases = {});

                /**
                 * The product id used by getZwetschgeImage().
                 */
                std::vector<uint8_t> idOfZwetschgeImage();
            }
        }
    }
}
