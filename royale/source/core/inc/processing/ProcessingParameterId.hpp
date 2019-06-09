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
    namespace processing
    {
        /**
         * An identifier which the ProcessingParameterMapFactory can use to locate the
         * ProcessingParameterMaps for a given use case.
         *
         * This is a subclass merely so that it's not interchangable with other subclasses of
         * UuidlikeIdentifier.
         *
         * @inheritdoc
         */
        class ProcessingParameterId : public royale::common::UuidlikeIdentifier<ProcessingParameterId>
        {
        public:
            // \todo ROYAL-3355 all of the constructors could be added with just
            // using UuidlikeIdentifier<ProcessingParameterId>::UuidlikeIdentifier;
            // but only when we drop support for older compilers

            ROYALE_API ProcessingParameterId () :
                UuidlikeIdentifier<ProcessingParameterId> ()
            {
            }

            ROYALE_API ProcessingParameterId (const datatype &data) :
                UuidlikeIdentifier<ProcessingParameterId> (data)
            {
            }

            ROYALE_API ProcessingParameterId (const royale::String &s) :
                UuidlikeIdentifier<ProcessingParameterId> (s)
            {
            }
        };
    }
}
