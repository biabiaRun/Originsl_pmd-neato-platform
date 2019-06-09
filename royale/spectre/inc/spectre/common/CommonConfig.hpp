/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

/* This file contains information about the configuration used to compile Spectre.
 * The file is created by CMake, and should not be changed manually.
 */

/* #undef SPECTRE_SHARED */


// Export Definition
#ifdef SPECTRE_COMMON_SHARED
#if defined(_WIN32) && !defined(SPECTRE_COMMON_EXPORT) && !defined(SPECTRE_FORCE_STATIC)
#define SPECTRE_COMMON_API __declspec( dllimport  )
#elif defined(_WIN32)
#define SPECTRE_COMMON_API __declspec( dllexport )
#elif defined(SPECTRE_COMMON_EXPORT)
#define SPECTRE_COMMON_API __attribute__ ((visibility ("default")))
#else
#define SPECTRE_COMMON_API
#endif
#else
#define SPECTRE_COMMON_API
#endif


