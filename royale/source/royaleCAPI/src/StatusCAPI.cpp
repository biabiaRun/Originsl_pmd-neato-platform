/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <StatusCAPI.h>
#include <royale/Status.hpp>
#include <private/HelperFunctionsCAPI.hpp>

using namespace royale;

ROYALE_CAPI_LINKAGE_TOP

ROYALE_CAPI char *royale_status_get_error_string (royale_camera_status status)
{
    char *errorStr = NULL;

    if (ROYALE_STATUS_INVALID_HANDLE == status)
    {
        HelperFunctionsCAPI::copyRoyaleStringToCString (&errorStr, royale::String ("Invalid handle, no instance found."));
    }
    else
    {
        HelperFunctionsCAPI::copyRoyaleStringToCString (&errorStr, getStatusString ( (CameraStatus) status));
    }

    return errorStr;
}

ROYALE_CAPI_LINKAGE_BOTTOM
