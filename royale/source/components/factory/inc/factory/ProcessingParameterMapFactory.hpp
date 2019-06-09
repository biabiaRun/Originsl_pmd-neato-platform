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

#include <factory/IProcessingParameterMapFactory.hpp>
#include <royale/Pair.hpp>

#include <map>
#include <vector>

namespace royale
{
    namespace factory
    {
        /**
         * An implementation of IProcessingParameterMapFactory which looks for parameters with the
         * following keys:
         *
         * 1. (productId, processingId)
         * 2. (EMPTY_PRODUCT, processingId)
         * 3. (EMPTY_PRODUCT, defaultId)
         *
         * Where EMPTY_PRODUCT is a empty Vector.
         *
         * The parameters are expected to mainly be found by the second search, using the
         * processingId with the EMPTY_PRODUCT key. The option to override it with a
         * productId-specific value is intended only for corrections for devices that are already
         * in-the-field.
         *
         * \todo ROYAL-3354 although the (productId, processingId) pair is supported by this class,
         * it is currently not supported by CoreConfigFactory. When CoreConfigFactory calls
         * getParameterMaps(), it will pass EMPTY_PRODUCT as the productId.
         */
        class ProcessingParameterMapFactory : public IProcessingParameterMapFactory
        {
        public:
            /**
             * If you're having trouble constructing this, note that the productId is taken as a
             * std::vector, not a royale::Vector.
             */
            using key_type = std::pair<std::vector<uint8_t>, royale::processing::ProcessingParameterId>;
            using value_type = royale::Pair<key_type, const royale::Vector<royale::ProcessingParameterMap>>;

            ROYALE_API ProcessingParameterMapFactory (const royale::Vector<value_type> &mapOfMaps,
                    const royale::processing::ProcessingParameterId &defaultId = {});

            royale::Vector<royale::ProcessingParameterMap> getParameterMaps (
                const royale::Vector<uint8_t> &productId,
                const royale::processing::ProcessingParameterId &processingId) const override;

        private:
            const std::map<key_type, const royale::Vector<royale::ProcessingParameterMap>> m_maps;
            const royale::processing::ProcessingParameterId m_defaultId;
        };
    }
}
