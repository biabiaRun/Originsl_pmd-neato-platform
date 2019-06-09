/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __SPECTRECONFIG_HPP__
#define __SPECTRECONFIG_HPP__

/* This file contains information about the configuration used to compile Spectre.
 * The file is created by CMake, and shoudl not be changed manually.
 */

#define SPECTRE_SHARED

// Export Definition
#if defined(SPECTRE_SHARED) && !defined(SPECTRE_FORCE_STATIC)
#if defined(_WIN32) && !defined(SPECTRE_EXPORT)
#define SPECTRE_API __declspec( dllimport  )
#elif defined(_WIN32)
#define SPECTRE_API __declspec( dllexport )
#elif defined(SPECTRE_EXPORT)
#define SPECTRE_API __attribute__ ((visibility ("default")))
#else
#define SPECTRE_API
#endif
#else
#define SPECTRE_API
#endif

/* #undef SPECTRE_ENABLE_ADDITIONAL_CHECKS */
/* #undef SPECTRE_USE_LOCK_FREE_BARRIER */

/// Version of Spectre
/**
 * On a binary build this file is generated on the master jenkins.
 * Therefore, the build number does match with the build numbers
 * of the individual binary libraries necessarily.
 *
 */
#define SPECTRE_BUILD_VERSION "4.1.0.183"

#endif /*__SPECTRECONFIG_HPP__*/
