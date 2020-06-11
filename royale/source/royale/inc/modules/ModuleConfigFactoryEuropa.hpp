/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <factory/ModuleConfigFactoryZwetschge.hpp>

namespace royale
{
    namespace factory
    {
        /**
         * A factory that returns the ModuleConfigs for Europa
         * variants, in devices using the IRS277xC (M2455 A14) imager family.
         */
        class ModuleConfigFactoryEuropa : public ModuleConfigFactoryZwetschge
        {
        public:
            explicit ModuleConfigFactoryEuropa();
        };
    }
}
