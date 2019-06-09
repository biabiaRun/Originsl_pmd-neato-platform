/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <Helper.hpp>

void CHECK_ROYALE_SUCCESS (CameraStatus status)
{
    INFO ("STATUS " << static_cast<int> (status) << ": " << getErrorString (status));
    REQUIRE (status == CameraStatus::SUCCESS);
}
