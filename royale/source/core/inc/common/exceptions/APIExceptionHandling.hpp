/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
* Unauthorized copying of this file, via any medium is strictly prohibited
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Status.hpp>

namespace royale
{
    namespace common
    {
        class APIExceptionHandler
        {
        public:
            static CameraStatus handleCurrentAPILevelException ();
        private:
            APIExceptionHandler();
        };
    }
}

// these macros are used to guard every exposed API call from exceptions.
// they are intended to be used as function try block, basically a try block for
// the entire function scope but getting rid of an extra pair of brackets.
// since we want to catch all uncatched exceptions on API level, this is perfect for us.
// detailed information: http://en.cppreference.com/w/cpp/language/function-try-block
#define ROYALE_API_EXCEPTION_SAFE_BEGIN try
#define ROYALE_API_EXCEPTION_SAFE_END catch (...) \
        { \
            return royale::common::APIExceptionHandler::handleCurrentAPILevelException(); \
        }
