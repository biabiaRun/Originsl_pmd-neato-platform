/****************************************************************************\
 * Copyright (C) 2014 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <royale/ProcessingFlag.hpp>
#include <royale/Variant.hpp>

#include <spectre/Parameter.hpp>
#include <spectre/IConfiguration.hpp>

namespace royale
{
    namespace processing
    {

        /**
         * @brief Gets a Royale parameter map from a given Spectre configuration
         *
         * @param config config for conversion
         *
         * @return Royale parameter map based on the config
         */
        royale::ProcessingParameterMap convertConfiguration (const spectre::IExtendedConfiguration &config);

        /**
         * @brief Applies a configuration in Royale format to a Spectre configuration
         *
         * The conversion is *NOT* implemented in an atomic way. If the operation fails
         * the passed spectre::Configuration instance is in an undefined state.
         *
         * @param map Royale configuration
         * @param config Spectre configuration
         *
         * @return true if the operation was successful, false otherwise
         */
        bool applyRoyaleParameters (const royale::ProcessingParameterMap &map,
                                    spectre::IExtendedConfiguration &config);
    }
}
