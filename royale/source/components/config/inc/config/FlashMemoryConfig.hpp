/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>

#include <chrono>

#include <cstddef>
#include <cstdint>

#include <hal/INonVolatileStorage.hpp>

namespace royale
{
    namespace config
    {

        /**
         * Data container for parameters of non-volatile storage in the module.
         *
         * The "FlashMemory" in this class' name is a historical legacy with the original
         * FlashMemoryPico (now called StorageFormatPicoLegacy) being a single class containing all
         * of the functionality for reading calibration from the SPI flash in PicoS devices. The
         * config was generalised to handle more types of storage, but the name remains.
         */
        struct FlashMemoryConfig
        {
            enum class FlashMemoryType
            {
                /**
                 * The module does not include non-volatile storage.  The other parameters
                 * in this FlashMemoryConfig struct are not meaningful.
                 */
                NONE,
                /**
                 * The memory is accessed by reading complete memory pages, and the data
                 * layout is handled by the StorageFormatPicoLegacy class.
                 *
                 * This type of memory needs the imageSize and pageSize to be configured.
                 */
                PICO_PAGED,
                /**
                 * The memory can be accessed starting from any offset, and the data
                 * layout is handled by the StorageFormatPolar class.  This type has a header
                 * at the start of the data area, and does not require the imageSize.
                 */
                POLAR_RANDOM,
                /**
                 * This can be used as a place holder if the calibration data was already
                 * read out before and the storage can or should not be accessed again.
                 */
                FIXED,
                /**
                 * The storage area contains exactly the bytes that should be returned by
                 * INonVolatileStorage::getCalibrationData(), with no headers or metadata.
                 */
                JUST_CALIBRATION,
                /**
                 * The memory can be accessed starting from any offset, and the data
                 * layout is handled by the StorageFormatZwetschge class.  This type has a header
                 * near the start of the data area, and does not require the imageSize.
                 *
                 * This format will be represented as an ExternalConfig, which includes but
                 * has more data than an INonVolatileStorage.
                 */
                ZWETSCHGE
            };

            /**
             * Construct a FlashMemoryConfig indicating there is no flash memory.
             */
            FlashMemoryConfig()
                : type (FlashMemoryType::NONE)
            {}

            /**
             * Constructor where the many size options can be set with the Named Parameter Idiom.
             */
            explicit FlashMemoryConfig (FlashMemoryType type)
                : type (type)
            {
            }

            /**
             * Type of the flash memory.
             */
            FlashMemoryType type;

            /**
             * The first address beyond the end of the storage area.  This the size in bytes, if
             * accessOffset is zero.
             *
             * For some storage implementations, configuring this is optional and a value of zero
             * will simply not check for out-of-bounds access.  Storage implementations may (but
             * also might not) check for out-of-bounds access.
             *
             * It may be less than the physical storage size; but if it is not aligned to the page
             * and sector sizes then the results are implementation-defined.  For example, storage
             * implementations that use sector-based erase are likely to expect the address to be
             * aligned to the sector size.
             */
            std::size_t imageSize = 0;

            /** Accessor for the Named Parameter Idiom */
            FlashMemoryConfig &setImageSize (std::size_t size)
            {
                imageSize = size;
                return *this;
            }

            /**
             * A write can only happen within and in multiples of this number.
             * If set to zero, the write interface will try a worst-case implementation.
             *
             * The page size of the memory according to the data sheet.
             */
            std::size_t pageSize = 0;

            /** Accessor for the Named Parameter Idiom */
            FlashMemoryConfig &setPageSize (std::size_t size)
            {
                pageSize = size;
                return *this;
            }

            /**
             * The data may start at a non-zero location in the device.  For example, this allows
             * the boot EEPROM to also be used as calibration storage.
             *
             * This must be a multiple of the pageSize and the sectorSize (unless it or they are
             * zero).
             */
            std::size_t accessOffset = 0;

            /** Accessor for the Named Parameter Idiom */
            FlashMemoryConfig &setAccessOffset (std::size_t size)
            {
                accessOffset = size;
                return *this;
            }

            /**
             * An erase can only happen as an aligned multiple and in an entirety of this number.
             * This is only applicable to deletable memories, in other cases set to zero.
             *
             * The sector size of the memory according to the data sheet.
             */
            std::size_t sectorSize = 0;

            /** Accessor for the Named Parameter Idiom */
            FlashMemoryConfig &setSectorSize (std::size_t size)
            {
                sectorSize = size;
                return *this;
            }

            /**
             * On devices that have to be polled to find out if an erase operation has completed,
             * Royale waits at least this time between triggering the erase and first polling the
             * device.
             *
             * Most devices and platforms will not use this setting.
             */
            std::chrono::microseconds eraseTime = std::chrono::microseconds {0};

            /** Accessor for the Named Parameter Idiom */
            FlashMemoryConfig &setEraseTime (std::chrono::microseconds time)
            {
                eraseTime = time;
                return *this;
            }

            /**
             * On devices that have to be polled to find out if a read operation has completed,
             * Royale waits at least this time between triggering the read and first polling the
             * device.
             *
             * Most devices and platforms will not use this setting.
             */
            std::chrono::microseconds readTime = std::chrono::microseconds {0};

            /** Accessor for the Named Parameter Idiom */
            FlashMemoryConfig &setReadTime (std::chrono::microseconds time)
            {
                readTime = time;
                return *this;
            }

            /**
             * During write operations that are split in to multiple writes, Royale waits at least
             * this time between successive writes.  On some hardware, it's possible to determine
             * when each write operation has finished, and on these devices setting may be ignored.
             *
             * The write time of the memory according to the data sheet, in microseconds (note:
             * the datasheet will probably state this time in milliseconds).  If the data sheet
             * gives separate times for writing a page and for writing a single byte, use the time
             * for writing a page.
             */
            std::chrono::microseconds writeTime = std::chrono::microseconds {0};

            /** Accessor for the Named Parameter Idiom */
            FlashMemoryConfig &setWriteTime (std::chrono::microseconds time)
            {
                writeTime = time;
                return *this;
            }

            /**
             * This will be initialized if type == FIXED is used
             */
            std::shared_ptr<royale::hal::INonVolatileStorage> nonVolatileStorageFixed;

            /**
             * If useCaching is set to true and the type is set to FIXED
             * Royale will try to load already existing calibration data from the working path
             * (external storage on Android). If it doesn't find calibration data it is loaded
             * as usual and placed onto the file system for the next start.
             */
            bool useCaching = false;

            /** Accessor for the Named Parameter Idiom */
            FlashMemoryConfig &setUseCaching (bool b)
            {
                useCaching = b;
                return *this;
            }

            /**
             * If true, this type of module requires the data corresponding to INonVolatileStorage's
             * getModuleIdentifier(), getModuleSuffix() and getModuleSerialNumber() methods.  The
             * single-argument form of INonVolatileStorage::writeCalibrationData should throw a
             * LogicError.
             */
            bool identifierIsMandatory = false;

            /** Accessor for the Named Parameter Idiom */
            FlashMemoryConfig &setIdentifierIsMandatory (bool b)
            {
                identifierIsMandatory = b;
                return *this;
            }
        };
    }
}
