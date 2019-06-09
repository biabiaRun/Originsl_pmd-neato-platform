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

#include <common/StringFunctions.hpp>
#include <processing/ProcessingParameterId.hpp>
#include <royale/ProcessingFlag.hpp>
#include <royale/String.hpp>
#include <royale/Vector.hpp>

#include <common/MakeUnique.hpp>

namespace royale
{
    namespace factory
    {
        /**
         * Class for mapping ids to ProcessingParameterMaps, the main target devices for this are
         * the FlashDefinedImager ones (which will only provide an ID in their flash).
         *
         * Note that there should be one instance of this factory that will provide the parameters
         * for all supported devices.  The FlashDefinedImager concept means that we should not need
         * a specific module config for every device variant.
         */
        class IProcessingParameterMapFactory
        {
        public:
            virtual ~IProcessingParameterMapFactory() = default;

            /**
             * Return the parameters for a given processingId. This method is expected to return one
             * ProcessingParameterMap for each stream of the UseCase.
             *
             * Here the productId is intended to be used only for camera-specific corrections, most
             * cameras should get the correct map based on just the processingId.
             */
            virtual royale::Vector<royale::ProcessingParameterMap> getParameterMaps (
                const royale::Vector<uint8_t> &productId,
                const royale::processing::ProcessingParameterId &processingId) const = 0;
        };
    }
}
