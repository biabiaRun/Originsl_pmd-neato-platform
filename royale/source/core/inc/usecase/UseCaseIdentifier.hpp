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

#include <common/UuidlikeIdentifier.hpp>

namespace royale
{
    namespace usecase
    {
        /**
         * This is a subclass merely so that it's not interchangable with other subclasses of
         * UuidlikeIdentifier.
         *
         * @inheritdoc
         */
        class UseCaseIdentifier : public royale::common::UuidlikeIdentifier<UseCaseIdentifier>
        {
        public:
            // \todo ROYAL-3355 all of the constructors could be added with just
            // using UuidlikeIdentifier<UseCaseIdentifier>::UuidlikeIdentifier;
            // but only when we drop support for older compilers

            ROYALE_API UseCaseIdentifier () :
                UuidlikeIdentifier<UseCaseIdentifier> ()
            {
            }

            ROYALE_API UseCaseIdentifier (const datatype &data) :
                UuidlikeIdentifier<UseCaseIdentifier> (data)
            {
            }

            ROYALE_API UseCaseIdentifier (const royale::String &s) :
                UuidlikeIdentifier<UseCaseIdentifier> (s)
            {
            }
        };
    }
}
