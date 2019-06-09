/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
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
         * A factory that returns the ModuleConfigs for different Orpheus
         * variants.
         *
         */
        class ModuleConfigFactoryOrpheus : public ModuleConfigFactoryZwetschge
        {
        public:
            explicit ModuleConfigFactoryOrpheus ();
        };
    }
}
