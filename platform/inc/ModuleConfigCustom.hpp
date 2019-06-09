/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <config/ModuleConfig.hpp>
#include <royale/Definitions.hpp>

namespace platform
{
    ROYALE_API std::shared_ptr<royale::config::ModuleConfig> getModuleConfigCustom();
}
