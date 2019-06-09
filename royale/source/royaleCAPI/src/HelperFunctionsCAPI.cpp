/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <private/HelperFunctionsCAPI.hpp>

void HelperFunctionsCAPI::copyRoyaleStringToCString (char **dst, const royale::String src)
{
    uint32_t sz = (uint32_t) src.size() + 1;
    *dst = (char *) malloc (sizeof (char) * sz);
    memcpy ( (void *) *dst, src.c_str(), sz);
}
