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

#include <config/ImagerConfig.hpp>

namespace royale
{
    namespace test
    {
        namespace utils
        {
            /**
             * There is a pregenerated Lena file created for an existing camera. This method returns
             * the filename, in the structure used to get NonVolatileStorageFactory to open that
             * file.
             *
             * This might not be the same device as the Zwetschge ExampleDevice.
             */
            ROYALE_API royale::config::ExternalConfigFileConfig getNvsfGetterForLenaFile();
        }
    }
}
