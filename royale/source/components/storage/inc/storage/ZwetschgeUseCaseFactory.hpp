/****************************************************************************\
* Copyright (C) 2020 pmdtechnologies ag
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
    namespace storage
    {
        class ZwetschgeUseCaseFactory
        {
        public:
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
            ROYALE_API static std::shared_ptr<royale::usecase::UseCaseDefinition> createUcd (
                royale::usecase::UseCaseIdentifier identifier,
                royale::Pair<uint16_t, uint16_t> imageSize,
                royale::Pair<uint16_t, uint16_t> frameRateLimits,
                uint16_t targetFrameRate,
                const royale::Vector<royale::usecase::ExposureGroup> &expoGroups,
                const royale::Vector<royale::usecase::RawFrameSet> &rfs
            );

        };
    }
}
