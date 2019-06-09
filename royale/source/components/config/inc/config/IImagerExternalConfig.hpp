/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <imager/ImagerCommon.hpp>
#include <imager/ImagerUseCaseIdentifier.hpp>

#include <chrono>
#include <string>
#include <vector>

namespace royale
{
    namespace imager
    {
        /**
        * One address/value pair, and the time to wait after sending this register before sending
        * the next one.
        */
        struct TimedRegisterListEntry
        {
            uint16_t address;
            uint16_t value;
            /**
            * Duration in microseconds to sleep before writing the following entry (or, for the
            * final entry, before returning from the writing function).
            *
            * Zero means no delay is required.
            */
            uint32_t sleepTime;
        };

        using TimedRegisterList = std::vector<TimedRegisterListEntry>;

        /**
        * A SequentialRegisterBlock is intended to be read directly by the imager.  If possible the
        * data won't be read by the host, the expectation is that only the size, address to read
        * from, and address to read to are stored in a structure that the host will read.
        *
        * A SequentialRegisterHeader contains the data that the host needs to read so that it could
        * tell the imager where to read from. If it is not used, all entries must be zero.
        */
        struct SequentialRegisterHeader
        {
            SequentialRegisterHeader (uint32_t fa, uint32_t fs, uint16_t ia) :
                flashConfigAddress (fa),
                flashConfigSize (fs),
                imagerAddress (ia)
            {
            }

            SequentialRegisterHeader() :
                SequentialRegisterHeader (0, 0, 0)
            {
            }

            /**
            * The address in the imager-attached storage to read from.
            */
            uint32_t flashConfigAddress;

            /**
            * The number of bytes to read.
            */
            uint32_t flashConfigSize;

            /**
            * Address of the first register in the imager to write this data to.
            */
            uint16_t imagerAddress;

            bool empty() const
            {
                return flashConfigSize == 0u;
            }
        };

        /**
        * This interface provides accessibility to a configuration that originates from
        * an external source, such as a SPI based flash or a file from the file system.
        * Only non-software-defined imagers may use this interface.
        *
        * The implementations of these functions must not perform any I/O.  In the expected usage
        * scenario data is being stored in flash accessible via the imager, which would suggest that
        * the data should be read before the imager starts capturing.  As all the existing and
        * expected implementations will satisfy this "should", the interface defines it as a "must".
        *
        * Implementations that check data should check it when creating the instance of this
        * interface, rather than postponing checks until the methods are called.
        *
        * Please note that this interface only contains definitions for known things,
        * as there is not yet a concept ready that defines the full software stack
        * for a flash-based imager this interface only includes everything that can
        * be guessed from what was used in the past by software-defined imagers.
        *
        * A utility class, WrapperImagerExternalConfig, exists for
        * implementations which just need a class to hold a set of vectors and
        * return them via this interface's getters.
        */
        class IImagerExternalConfig
        {
        public:
            virtual ~IImagerExternalConfig() = default;

            struct UseCaseData
            {
                UseCaseData (
                    ImagerUseCaseIdentifier guid,
                    std::string name,
                    std::vector<std::size_t> imageStreamBlockSizes,
                    std::vector<uint32_t> modulationFrequencies,
                    TimedRegisterList registerMap,
                    std::chrono::microseconds waitTime = std::chrono::microseconds::zero()) :
                    guid (std::move (guid)),
                    name (std::move (name)),
                    imageStreamBlockSizes (std::move (imageStreamBlockSizes)),
                    modulationFrequencies (std::move (modulationFrequencies)),
                    sequentialRegisterHeader (),
                    registerMap (std::move (registerMap)),
                    waitTime (waitTime)
                {
                }

                UseCaseData (
                    ImagerUseCaseIdentifier guid,
                    std::string name,
                    std::vector<std::size_t> imageStreamBlockSizes,
                    std::vector<uint32_t> modulationFrequencies,
                    SequentialRegisterHeader sequentialRegisterHeader,
                    std::chrono::microseconds waitTime = std::chrono::microseconds::zero()) :
                    guid (std::move (guid)),
                    name (std::move (name)),
                    imageStreamBlockSizes (std::move (imageStreamBlockSizes)),
                    modulationFrequencies (std::move (modulationFrequencies)),
                    sequentialRegisterHeader (std::move (sequentialRegisterHeader)),
                    registerMap (),
                    waitTime (waitTime)
                {
                }

                UseCaseData (
                    ImagerUseCaseIdentifier guid,
                    std::string name,
                    std::vector<std::size_t> imageStreamBlockSizes,
                    std::vector<uint32_t> modulationFrequencies,
                    SequentialRegisterHeader sequentialRegisterHeader,
                    TimedRegisterList registerMap,
                    std::chrono::microseconds waitTime = std::chrono::microseconds::zero()) :
                    guid (std::move (guid)),
                    name (std::move (name)),
                    imageStreamBlockSizes (std::move (imageStreamBlockSizes)),
                    modulationFrequencies (std::move (modulationFrequencies)),
                    sequentialRegisterHeader (std::move (sequentialRegisterHeader)),
                    registerMap (std::move (registerMap)),
                    waitTime (waitTime)
                {
                }

                UseCaseData () :
                    UseCaseData ( {}, "", {}, {}, SequentialRegisterHeader {})
                {
                }

                ImagerUseCaseIdentifier guid; //!< An identifier guaranteed to be unique for the scope of Royale related SW
                std::string name; //!< A human-readable name that shows the meaning of the use case

                /**
                * The receiver part of the Royale software (so called "bridge" component) expects information
                * about the structure of the incoming image data stream, especially when the exact distribution
                * of superframes is affected.
                * This method returns an ordered list of the number of associated raw frames for each block of
                * contiguous image data.
                */
                std::vector<std::size_t> imageStreamBlockSizes;

                /**
                * The imager needs to know the modulation frequencies to be able to compute exposure register
                * settings.
                * There should be one entry for each sequence entry with the frequency in Hz.
                */
                std::vector<uint32_t> modulationFrequencies;

                /**
                * The flash memory block that the imager can directly read this use case from. A
                * zero value indicates that the configuration for this use case should be done
                * solely by a TimedRegisterList object.
                */
                SequentialRegisterHeader sequentialRegisterHeader;

                /**
                * These registers are used by the imager to configure the imager silicon for a specific
                * use case. In case of a flash memory based use case this register map can be empty.
                * In case no flash memory based configuration is used this register map must contain
                * the full configuration of this use case.
                * In case of both is given (a flash memory based configuration and a register map),
                * the transfer of the flash based configuration will be done prior to the transferring
                * this register map (for example to set some initial exposure times defined by the host).
                */
                TimedRegisterList registerMap;

                /**
                 * When switching between use cases, there may have to be a pause
                 * to ensure eye safety, this pause is calculated from this wait time.
                 */
                std::chrono::microseconds waitTime;
            };

            /**
            * Returns the TimedRegisterList that should be used by an imager to bring it into
            * a well-known initial state directly after resetting it.
            */
            IMAGER_EXPORT virtual const TimedRegisterList &getInitializationMap() const = 0;

            /**
            * If the firmware can be loaded directly by the imager, and this has been configured in
            * a SequentialRegisterHeader, returns the SequentialRegisterHeader for loading it.
            *
            * If this returns a non-empty header then getFirmwarePage1() and getFirmwarePage2()
            * must return empty lists. If it returns an empty header then getFirmwareHeader2() must
            * also return an empty header.
            *
            * If the imager does not need firmware, or if it automatically loads it without host
            * interaction, then all of the getFirmware* methods should return empty headers and
            * empty lists.
            */
            IMAGER_EXPORT virtual SequentialRegisterHeader getFirmwareHeader1() const = 0;

            /**
            * The second page, see getFirmwareHeader1
            */
            IMAGER_EXPORT virtual SequentialRegisterHeader getFirmwareHeader2() const = 0;

            /**
            * Returns a TimedRegisterList that contains the first page of an imager firmware that needs
            * to be loaded by Royale, or an empty TimedRegisterList if the imager does not require Royale
            * to load any firmware (either because the imager does not need it, or because the
            * imager automatically loads it when reset).
            *
            * This must return an empty list if getFirmwareHeader1() returns a non-empty
            * SequentialRegisterHeader.
            *
            * The first register of the TimedRegisterList must perform a page switch.
            */
            IMAGER_EXPORT virtual const TimedRegisterList &getFirmwarePage1() const = 0;

            /**
            * Returns a TimedRegisterList that contains the second page of an imager firmware that needs
            * to be loaded by Royale, or an empty TimedRegisterList if the imager does not require Royale
            * to load any firmware (either because the imager does not need it, or because the
            * imager automatically loads it when reset).
            *
            * The first register of the TimedRegisterList must perform a page switch.
            */
            IMAGER_EXPORT virtual const TimedRegisterList &getFirmwarePage2() const = 0;

            /**
            * Returns the TimedRegisterList that should be used to configure and run
            * the firmware provided by the getFirmwarePage1() and getFirmwarePage2()
            * getters. This should not contain the configuration that transfers
            * the firmware from the host system to the imager.
            */
            IMAGER_EXPORT virtual const TimedRegisterList &getFirmwareStartMap() const = 0;

            /**
            * Returns the TimedRegisterList that should be used by an imager to start
            * a continuous measurement.
            */
            IMAGER_EXPORT virtual const TimedRegisterList &getStartMap() const = 0;

            /**
            * Returns the TimedRegisterList that should be used by an imager to stop
            * a continuous measurement.
            */
            IMAGER_EXPORT virtual const TimedRegisterList &getStopMap() const = 0;

            /**
            * Returns the list of use cases.
            * The initialized imager should use this information to configure the sensor so that it
            * is prepared to start a continuous measurement.
            */
            IMAGER_EXPORT virtual const std::vector<UseCaseData> &getUseCaseList() const = 0;
        };
    }
}
