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
    namespace usecase
    {
        class UseCaseFaceID : public UseCaseDefinition
        {
        public:
            ROYALE_API UseCaseFaceID (
                royale::usecase::UseCaseIdentifier identifier,
                uint16_t targetRate,
                uint32_t modulationFrequencyIR,
                uint32_t modulationFrequencyDepth,
                uint32_t modulationFrequencyGray,
                royale::Pair<uint32_t, uint32_t> exposureLimitsIR,
                royale::Pair<uint32_t, uint32_t> exposureLimitsDepth,
                royale::Pair<uint32_t, uint32_t> exposureLimitsGray,
                uint32_t exposureIR,
                uint32_t exposureDepth,
                uint32_t exposureGray
            );
        };
    }
}
