/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/


#ifndef __PARAMETERKEY_HPP__
#define __PARAMETERKEY_HPP__

#include "Parameter.def"

namespace spectre
{
    namespace common
    {

#define SPECTRE_ENUM_NAME(KEY,...) KEY,
        /**
         * @brief Parameters supported by Spectre
         *
         * This enumeration lists all parameters supported by Spectre.
         * However, it is not guaranteed that each parameter listed here is supported
         * by each Spectre instance. To query a list parameters supported by a specific
         * Spectre instance use ISpectre::configuration(), and then Configuration::getParameters(), if you
         * are using the libSpectre API. If you are using libProcessing directly check the relevant
         * IProcessingDescriptor.
         */
        enum class ParameterKey
        {
            /*!Used to marks invalid parameters*/UNDEFINED,
            /*!Modulation scheme*/MODULATION_SCHEME,
            /*!Allows customized control of the basic reconfiguration process*/CUSTOM_BASIC_CONTROL,
            SPECTRE_PARAMETERS (SPECTRE_ENUM_NAME)
            NUM_ENTRIES
        };
#undef SPECTRE_ENUM_NAME
    }  // common
}  // spectre

#endif /*__PARAMETERKEY_HPP__*/
