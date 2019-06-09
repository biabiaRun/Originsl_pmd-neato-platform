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

#include <royale/Definitions.hpp>
#include <royale/Pair.hpp>
#include <usecase/UseCaseDefinition.hpp>

namespace royale
{
    namespace usecase
    {
        /**
         * With the Flash Defined Imager, the processing and frame collector need a subset of the
         * use case definition data, but the data for a Software Defined Imager is not required.
         * This can only be used when another source is providing the information for the imager
         * itself, for example when there is additionally an IImagerExternalConfig.
         *
         * The container for this data is currently an instance of UseCaseDefinition, although it
         * may eventually become a separate class containing only that subset of the data.
         *
         * This factory is a helper class for creating such a UseCaseDefinition which can be used
         * for the Processing and Frame Collector, but does not have information for the imager.
         */
        class UseCaseDefFactoryProcessingOnly : public UseCaseDefinition
        {
        public:
            /**
             * The data used for each ExposureGroup. Group names will be generated automatically.
             */
            struct ProcOnlyExpo
            {
                /** Limits of the exposure, as a [min, max] pair, in microseconds */
                royale::Pair<uint32_t, uint32_t> exposureLimits;
                /** Default (starting) exposure time in microseconds */
                uint32_t                         exposureTime;
            };

            /**
             * The data used for each RawFrameSet.
             */
            struct ProcOnlyRFS
            {
                std::size_t frameCount;
                uint32_t modulationFrequency;
                ExposureGroupIdx exposureGroupIdx;
            };

            /**
             * Factory function used by callers that build UCDs from parts, for example the
             * Zwetschge reader. This uses shared_ptr instead of unique_ptr, as it's expected to be
             * passed to the constructor of UseCase.
             *
             * @param identifier the GUID
             * @param imageSize a pair [width, height]
             * @param frameRateLimits a pair [minRate, maxRate]
             * @param targetFrameRate the value to return from getTargetRate
             * @param expoGroups the exposure groups
             * @param rfs raw frame sets, must already have the correct exposureGroupIdx values
             */
            ROYALE_API static std::shared_ptr<UseCaseDefinition> createUcd (
                royale::usecase::UseCaseIdentifier identifier,
                royale::Pair<uint16_t, uint16_t> imageSize,
                royale::Pair<uint16_t, uint16_t> frameRateLimits,
                uint16_t targetFrameRate,
                const royale::Vector<ProcOnlyExpo> &expoGroups,
                const royale::Vector<ProcOnlyRFS> &rfs
            );

            /**
             * Constructor from another UseCaseDefinition, this copies only the subset of the data,
             * so it can take a normal UCD (as defined in a module config) and create the subset
             * matching a Zwetschge-using version of the same module.
             */
            ROYALE_API static std::shared_ptr<UseCaseDefinition> createUcd (const UseCaseDefinition &other);
        };
    }
}
