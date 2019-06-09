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

#include <royale/String.hpp>
#include <royale/Vector.hpp>
#include <stdint.h>

namespace royale
{
    namespace hal
    {
        /**
         * The memory that holds the serial number, calibration data, etc.
         */
        class INonVolatileStorage
        {
        public:
            virtual ~INonVolatileStorage() = default;

            /**
             * Returns a module subtype identifier, as a GUID-like data vector.
             *
             * This identifier can be used to distinguish different variants of a specific module
             * that are otherwise not distinguishable for Royale (e.g. different lenses but using
             * the same USB vendor and product identifier).
             *
             * Returns an empty string if the I/O succeeded, but the device does not have this data.
             * An all-zero identifier will be treated as the device not having an identifier, it
             * will cause an empty vector to be returned, not a vector filled with zeros.
             */
            virtual royale::Vector<uint8_t> getModuleIdentifier() = 0;

            /**
             * Returns a human-readable module subtype identifier.
             *
             * For some USB modules, this will appear in the device name shown by the operating
             * system.  For example a "pico maxx 105" would have "105" in this string.
             *
             * Returns an empty string if the I/O succeeded, but the device does not have this data.
             */
            virtual royale::String getModuleSuffix() = 0;

            /**
             * Returns an opaque (but printable, in human-readable form) unique ID for the module.
             *
             * Note that the imager has its own (possibly different) serial number.  For some
             * modules, the imager serial number is used as the module serial number, but this is a
             * design decision specific to those modules.
             *
             * \return if no proper PMD data blob was found, an empty string is returned
             */
            virtual royale::String getModuleSerialNumber() = 0;

            /**
             * Returns an opaque (interpretation known to the processing) set of data.  Throws an
             * exception on error, which may be caused by I/O errors or invalid or corrupted data.
             */
            virtual royale::Vector<uint8_t> getCalibrationData() = 0;

            /**
             * Returns the CRC of the calibration data which is included in the header. If the header
             * or the storage object that is used doesn't support the checksum it will return a zero.
             * Throws an exception on error, which may be caused by I/O errors or invalid or
             * corrupted data.
             */
            virtual uint32_t getCalibrationDataChecksum() = 0;

            /**
             * If supported, saves an opaque (interpretation known to the processing) set of data.
             * Throws an exception if this non-volatile storage is read-only.
             *
             * A system integrator may choose to implement a read-only INonVolatileStorage, even
             * if the underlying storage system would be read-write.
             */
            virtual void writeCalibrationData (const royale::Vector<uint8_t> &data) = 0;

            /**
             * If supported, saves an opaque (interpretation known to the processing) set of data.
             * Throws an exception if this non-volatile storage is read-only.
             *
             * A system integrator may choose to implement a read-only INonVolatileStorage, even
             * if the underlying storage system would be read-write.
             *
             * \param calibrationData the data that will be returned by getCalibrationData()
             * \param identifier the data that will be returned by getModuleIdentifier()
             * \param suffix the data that will be returned by getModuleSuffix()
             * \param serialNumber the data that will be returned by getModuleSerialNumber()
             */
            virtual void writeCalibrationData (const royale::Vector<uint8_t> &calibrationData,
                                               const royale::Vector<uint8_t> &identifier,
                                               const royale::String &suffix,
                                               const royale::String &serialNumber) = 0;
        };
    }
}
