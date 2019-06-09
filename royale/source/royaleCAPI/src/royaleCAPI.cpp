/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <royaleCAPI.h>
#include <StatusCAPI.h>
#include <royale.hpp>
#include <cstring>

#include <private/HelperFunctionsCAPI.hpp>


ROYALE_CAPI_LINKAGE_TOP

ROYALE_CAPI void royale_get_version_v220 (unsigned *major, unsigned *minor, unsigned *patch)
{
    royale::getVersion (*major, *minor, *patch);
}

ROYALE_CAPI void royale_get_version_with_build_v220 (unsigned *major, unsigned *minor, unsigned *patch, unsigned *build)
{
    royale::getVersion (*major, *minor, *patch, *build);
}

ROYALE_CAPI royale_camera_status royale_get_version_with_build_and_scm_revision_v320 (unsigned *major, unsigned *minor, unsigned *patch, unsigned *build, char **scm_rev)
{
    if ( (major == nullptr) || (minor == nullptr) || (patch == nullptr) || (build == nullptr) || (scm_rev == nullptr))
    {
        return ROYALE_STATUS_INVALID_VALUE;
    }

    royale::String tmp_scmrev;
    royale::getVersion (*major, *minor, *patch, *build, tmp_scmrev);
    // \fixme ROYAL-2064 unchecked malloc
    HelperFunctionsCAPI::copyRoyaleStringToCString (scm_rev, tmp_scmrev);

    return ROYALE_STATUS_SUCCESS;
}

ROYALE_CAPI void copyDataBlocked (void *dest, void *src, uint32_t length)
{
    memcpy (dest, src, length);
}

ROYALE_CAPI_LINKAGE_BOTTOM
