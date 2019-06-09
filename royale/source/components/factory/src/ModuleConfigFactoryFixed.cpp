/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <factory/ModuleConfigFactoryFixed.hpp>

using namespace royale::factory;

ModuleConfigFactoryFixed::ModuleConfigFactoryFixed (const royale::config::ModuleConfig &moduleConfig)
    : m_moduleConfig (std::make_shared<royale::config::ModuleConfig> (moduleConfig))
{
}

std::shared_ptr<const royale::config::ModuleConfig>
ModuleConfigFactoryFixed::probeAndCreate (royale::factory::IBridgeFactory &) const
{
    return m_moduleConfig;
}

royale::Vector<std::shared_ptr<const royale::config::ModuleConfig>> ModuleConfigFactoryFixed::enumerateConfigs() const
{
    return {m_moduleConfig};
}
