/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

/****************************************************************************\
* These functions will only be used internally for the .NET wrapper.
* They are hidden from the publicly available CAPI.
\****************************************************************************/

#pragma once

#include <VariantCAPI.h>
#include <DefinitionsCAPI.h>
#include <stdint.h>

ROYALE_CAPI_LINKAGE_TOP

#define ROYALE_NO_VARIANT_INSTANCE_CREATED 0

typedef uint64_t royale_variant_managed_hnd;

ROYALE_CAPI royale_variant_managed_hnd royale_variant_managed_create();

ROYALE_CAPI royale_variant_managed_hnd royale_variant_managed_create_float (float f, float min, float max);

ROYALE_CAPI royale_variant_managed_hnd royale_variant_managed_create_int (int i, int min, int max);

ROYALE_CAPI royale_variant_managed_hnd royale_variant_managed_create_bool (bool b);

ROYALE_CAPI royale_variant_managed_hnd royale_variant_managed_create_type (royale_variant_type type, uint32_t value);

ROYALE_CAPI void royale_variant_managed_delete (royale_variant_managed_hnd handle);

ROYALE_CAPI void royale_variant_managed_set_float (royale_variant_managed_hnd handle, float f);
ROYALE_CAPI float royale_variant_managed_get_float (royale_variant_managed_hnd handle);
ROYALE_CAPI float royale_variant_managed_get_float_min (royale_variant_managed_hnd handle);
ROYALE_CAPI float royale_variant_managed_get_float_max (royale_variant_managed_hnd handle);

ROYALE_CAPI void royale_variant_managed_set_int (royale_variant_managed_hnd handle, int i);
ROYALE_CAPI int royale_variant_managed_get_int (royale_variant_managed_hnd handle);
ROYALE_CAPI int royale_variant_managed_get_int_min (royale_variant_managed_hnd handle);
ROYALE_CAPI int royale_variant_managed_get_int_max (royale_variant_managed_hnd handle);

ROYALE_CAPI void royale_variant_managed_set_bool (royale_variant_managed_hnd handle, bool b);
ROYALE_CAPI bool royale_variant_managed_get_bool (royale_variant_managed_hnd handle);

ROYALE_CAPI void royale_variant_managed_set_data (royale_variant_managed_hnd handle, royale_variant_type type, uint32_t value);
ROYALE_CAPI const uint32_t royale_variant_managed_get_data (royale_variant_managed_hnd handle);

ROYALE_CAPI royale_variant_type royale_variant_managed_get_type (royale_variant_managed_hnd handle);

ROYALE_CAPI bool royale_variant_managed_is_equal (royale_variant_managed_hnd handle1, royale_variant_managed_hnd handle2);
ROYALE_CAPI bool royale_variant_managed_is_not_equal (royale_variant_managed_hnd handle1, royale_variant_managed_hnd handle2);
ROYALE_CAPI bool royale_variant_managed_is_greater_than (royale_variant_managed_hnd handle1, royale_variant_managed_hnd handle2);
ROYALE_CAPI bool royale_variant_managed_is_less_than (royale_variant_managed_hnd handle1, royale_variant_managed_hnd handle2);

ROYALE_CAPI_LINKAGE_BOTTOM
