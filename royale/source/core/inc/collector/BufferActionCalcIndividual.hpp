/****************************************************************************\
 * Copyright (C) 2019 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <collector/FrameCollectorBase.hpp>

#include <common/IPseudoDataInterpreter.hpp>

namespace royale
{
    namespace collector
    {
        /**
         * For a Bridge that receives each raw frame as a separate image,
         * calculate how the FrameCollector should handle the input.
         */
        class BufferActionCalcIndividual : public IBufferActionCalc
        {
        public:
            ROYALE_API explicit BufferActionCalcIndividual ();
            ROYALE_API virtual ~BufferActionCalcIndividual() override;

            // From IBufferActionCalc
            ROYALE_API IBufferActionCalc::Result calculateActions (
                const royale::usecase::UseCaseDefinition &useCase,
                const std::vector<CollectorFrameGroup> &frameGroupList,
                const royale::Vector<std::size_t> &blockSizes) const override;
        };
    }
}
