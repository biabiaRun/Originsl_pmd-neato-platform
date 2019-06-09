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

#include <usecase/UseCaseIdentifier.hpp>
#include <string>

namespace royale
{
    namespace imager
    {
        /**
         * This holds a use case identifier (probably a UUID / GUID).
         */
        using ImagerUseCaseIdentifier = royale::usecase::UseCaseIdentifier;

        inline ImagerUseCaseIdentifier toImagerUseCaseIdentifier (const royale::usecase::UseCaseIdentifier &uci)
        {
            return uci;
        }

        inline ImagerUseCaseIdentifier toImagerUseCaseIdentifier (const std::array<uint8_t, 16> &a)
        {
            return toImagerUseCaseIdentifier (royale::usecase::UseCaseIdentifier (a));
        }

        inline ImagerUseCaseIdentifier parseImagerUseCaseIdentifierGuidString (const std::string &s)
        {
            return ImagerUseCaseIdentifier::parseRfc4122 (s);
        }
    }
}
