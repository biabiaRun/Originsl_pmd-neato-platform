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
         * A factory that returns the ModuleConfigs for different module maker
         * variants, in devices using the IRS287xC (M2458) imager family.
         */
        class ModuleConfigFactoryPmdModule287x : public ModuleConfigFactoryZwetschge
        {
        public:
            explicit ModuleConfigFactoryPmdModule287x();
        };
    }
}
