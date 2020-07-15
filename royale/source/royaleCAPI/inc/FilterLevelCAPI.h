/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

/**
* \addtogroup royaleCAPI
* @{
*/

#pragma once

#include <DefinitionsCAPI.h>

ROYALE_CAPI_LINKAGE_TOP

typedef enum royale_filter_level
{
    ROYALE_FILTER_LEVEL_OFF = 0,        ///< Turn off all filtering of the data (validation will still be enabled)
    ROYALE_FILTER_LEVEL_LEGACY = 200,   ///< Standard settings for older cameras
    ROYALE_FILTER_LEVEL_FULL = 255,     ///< Enable all filters that are available for this camera
    ROYALE_FILTER_LEVEL_CUSTOM = 256    ///< Value returned by getFilterLevel if the processing parameters differ from all of the presets
} royale_filter_level;

ROYALE_CAPI_LINKAGE_BOTTOM
/** @}*/
