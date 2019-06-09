/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>

#include <royale/Definitions.hpp>

namespace royale
{
    namespace common
    {
        /** Display a warning about not being an official build (Windows only).
         *
         * Behavior, depending on JENKINS_BUILD_TYPE in CMake:
         *
         * DEVELOPER (default) or DIRTY build type --> Dirty Warning on Windows
         * RELEASE build type --> No Warning
         * NIGHTLY build type --> version 0.0.0, Dirty Warning on Windows
         * EXCEPT_DIRTY_DEV == True --> No Warning if the current user has the username that compiled this build, otherwise behave according to build type
         */
        ROYALE_API void showDirtyVersionWarning ();
    }
}
