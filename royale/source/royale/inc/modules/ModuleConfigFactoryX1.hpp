/****************************************************************************\
 * Copyright (C) 2020 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <factory/ModuleConfigFactoryByStorageId.hpp>

namespace royale
{
    namespace factory
    {
        /**
         * A factory that returns the ModuleConfigs for the X1 variants
         */
        class ModuleConfigFactoryX1 : public ModuleConfigFactoryByStorageId
        {
        public:
            explicit ModuleConfigFactoryX1();
        };
    }
}
