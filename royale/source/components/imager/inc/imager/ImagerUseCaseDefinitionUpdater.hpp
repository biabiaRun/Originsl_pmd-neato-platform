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

#include <imager/ImagerUseCaseDefinition.hpp>

#include <vector>

namespace royale
{
    namespace imager
    {
        /**
        * Provides setter methods for ImagerUseCaseDefinition.
        *
        */
        class ImagerUseCaseDefinitionUpdater : public royale::imager::ImagerUseCaseDefinition
        {
        public:
            /**
            * Default ctor. The only users of this are the ones that will overwrite the contents by
            * assigning a complete UseCaseDefinition.
            *
            * \todo is this documentation wrong?
            */
            explicit ImagerUseCaseDefinitionUpdater (const ImagerUseCaseDefinition &iucd);

            /**
            * Set the target frame rates for the usecase [Hz].
            */
            void setTargetFrameRate (uint16_t targetFrameRate);

            /**
            * Set exposure times for all raw frames.
            *
            * In case the size of the exposureTimes vector doesn't match the IUCD configuration,
            * an exception is thrown; the IUCD may be partially updated after this.
            *
            */
            void setExposureTimes (const std::vector<uint32_t> &exposureTimes);
        };
    }
}
