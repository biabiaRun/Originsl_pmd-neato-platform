/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Pair.hpp>
#include <usecase/UseCaseDefinition.hpp>

namespace royale
{
    namespace usecase
    {
        /**
         * The UseCaseSlave takes a UseCaseDefinition and adapts it to be used by
         * a slave camera.
         * For this we have to raise the target frame rate (to ensure that the synchronization
         * is working properly) and lower the maximum exposure time (to stay inside the
         * eye safety limits).
         */
        class UseCaseSlave : public UseCaseDefinition
        {
        public:
            ROYALE_API explicit UseCaseSlave (const UseCaseDefinition &masterDefinition);
        };
    }
}
