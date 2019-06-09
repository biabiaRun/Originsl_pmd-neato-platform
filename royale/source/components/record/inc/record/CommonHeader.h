/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
#  if defined(RRFACCESSAPI_EXPORTS)
#    define RRFACCESSAPI __declspec(dllexport)
#  else
#    define RRFACCESSAPI __declspec(dllimport)
#  endif
#else
#  define RRFACCESSAPI
#endif

// Current version of the recording
#define ROYALE_FILEHEADER_VERSION 3

// "ROYALE_RECORD"
#define ROYALE_FILEHEADER_MAGIC "ROYALE_RECORD"
#define ROYALE_FILEHEADER_MAGIC_LENGTH 13

typedef size_t royale_rrf_handle;

// Error codes
typedef enum royale_rrf_api_error
{
    RRF_NO_ERROR = 0,
    RRF_RUNTIME_ERROR,
    RRF_COULD_NOT_OPEN,
    RRF_HANDLE_IN_USE,
    RRF_HANDLE_INVALID,
    RRF_NO_VALID_FRAME,
    RRF_WRONG_FILE_FORMAT,
    RRF_WRONG_VERSION,
    RRF_FILE_EMPTY,
    RRF_FRAME_NOT_FOUND
} royale_rrf_api_error;

typedef enum royale_rrf_platformtype
{
    RRF_ROYALE_WINDOWS,
    RRF_ROYALE_LINUX,
    RRF_ROYALE_MAC,
    RRF_ROYALE_ANDROID,
    RRF_ROYALE_UNDEFINED
} royale_rrf_platformtype;

typedef enum royale_rrf_compressiontype
{
    NONE
} royale_rrf_compressiontype;

#define CHECK_RRFHANDLE(rwc,handle,retval) auto instanceit = g_instances.find(handle); \
    if (instanceit == g_instances.end ()) \
    {\
        return retval; \
    } \
    rwc *rw = (rwc *) instanceit->second.get ()

#ifdef _WIN32
#define fseek64_royale_rrf _fseeki64
#define ftell64_royale_rrf _ftelli64
#elif __APPLE__
#define fseek64_royale_rrf fseeko
#define ftell64_royale_rrf ftello
#elif __ANDROID_API__
#define fseek64_royale_rrf fseeko
#define ftell64_royale_rrf ftello
#else
#define fseek64_royale_rrf fseeko64
#define ftell64_royale_rrf ftello64
#endif


#ifdef WIN32

// fopen
#define fopen_royale_rrf( file, filename, mode ) \
auto fopenerror = fopen_s( &file, filename, mode ); \
if (fopenerror) file = 0;

// fread
#define fread_royale_rrf( buf, bufsize, elsize, elcount, file ) \
fread_s( buf, bufsize, elsize, elcount, file );

#define stat_royale_rrf _stat64

#else

// fopen
#define fopen_royale_rrf( file, filename, mode ) \
file = fopen ( filename, mode )

// fread
#define fread_royale_rrf( buf, bufsize, elsize, elcount, file ) \
fread( buf, elsize, elcount, file );

#define stat_royale_rrf stat64

#endif
