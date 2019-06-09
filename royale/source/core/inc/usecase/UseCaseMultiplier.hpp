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

#include <royale/Definitions.hpp>
#include <royale/Pair.hpp>
#include <usecase/UseCaseDefinition.hpp>

namespace royale
{
    namespace usecase
    {
        /**
         * This test-only class takes any other UCD as a template, including any of the mixed mode
         * use cases.  It has extra copies of the streams, for example a UseCaseMultiplier (3,
         * UseCaseMixedXHt) will have 3 HT streams, and 3 ES streams.  Each copy of the stream
         * receives the same data.
         */
        class UseCaseMultiplier : public UseCaseDefinition
        {
        public:
            ROYALE_API explicit UseCaseMultiplier (std::size_t multiplier, const UseCaseDefinition &base);
        };

        /**
         * The UseCaseMultiplier is expected to be used in the ModuleConfig of a camera,
         * as a quick modification one of the normal use cases.
         *
         * A few lines later in the ModuleConfig, the processing parameters can be:
         * royale::usecase::paramsUCMultiplier<royale::ProcessingParameterMap> (MULTIPLIER, { NORMAL_PARAMS })
         *
         * This is a template to avoid a dependency on the processing component.
         */
        template <typename T>
        royale::Vector<T> paramsUCMultiplier (std::size_t multiplier, const royale::Vector<T> &base)
        {
            royale::Vector<T> result;
            for (std::size_t i = 0; i < multiplier; i++)
            {
                for (T t : base)
                {
                    result.emplace_back (t);
                }
            }
            return result;
        }

    }
}
