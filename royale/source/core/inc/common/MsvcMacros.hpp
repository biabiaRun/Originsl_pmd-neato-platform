/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#if defined (ROYALE_TARGET_PLATFORM_WINDOWS)
#define fseek64_royale _fseeki64
#define ftell64_royale _ftelli64
#endif
#if defined (ROYALE_TARGET_PLATFORM_LINUX)
#define fseek64_royale fseeko64
#define ftell64_royale ftello64
#endif
#if defined (ROYALE_TARGET_PLATFORM_APPLE)
#define fseek64_royale fseeko
#define ftell64_royale ftello
#endif
#if defined (ROYALE_TARGET_PLATFORM_ANDROID)
#define fseek64_royale fseeko
#define ftell64_royale ftello
#endif


#ifdef WIN32

// sprintf
#define sprintf_royale( buf, str, len ) \
sprintf_s ( buf, len, str )

#define sprintf_royale_v1( buf, str, val1, len ) \
sprintf_s ( buf, len, str,  val1 )
#define sprintf_royale_v2( buf, str, val1, val2, len ) \
sprintf_s ( buf, len, str,  val1, val2 )
#define sprintf_royale_v3( buf, str, val1, val2, val3, len ) \
sprintf_s ( buf, len, str,  val1, val2, val3 )
#define sprintf_royale_v4( buf, str, val1, val2, val3, val4, len ) \
sprintf_s ( buf, len, str,  val1, val2, val3, val4 )
#define sprintf_royale_v5( buf, str, val1, val2, val3, val4, val5, len ) \
sprintf_s ( buf, len, str,  val1, val2, val3, val4, val5 )
#define sprintf_royale_v6( buf, str, val1, val2, val3, val4, val5, val6, len ) \
sprintf_s ( buf, len, str,  val1, val2, val3, val4, val5, val6 )

// fprintf
#define fprintf_royale( file, str, ... ) \
    fprintf_s ( file, str, __VA_ARGS__ )

// strncpy
#define strncpy_royale( dst, msg, max, len ) \
strncpy_s ( dst, len, msg, max )

// fopen
#define fopen_royale( file, filename, mode ) \
auto fopenerror = fopen_s( &file, filename, mode ); \
if (fopenerror) file = 0;

// fread
#define fread_royale( buf, bufsize, elsize, elcount, file ) \
fread_s( buf, bufsize, elsize, elcount, file );

// snprintf
#define snprintf_royale( dst, count, format, len ) \
_snprintf_s( dst, len, count, format )

#define snprintf_royale_v1( dst, count, format, val1, len ) \
_snprintf_s( dst, len, count, format, val1 )
#define snprintf_royale_v2( dst, count, format, val1, val2, len ) \
_snprintf_s( dst, len, count, format, val1, val2 )
#define snprintf_royale_v3( dst, count, format, val1, val2, val3, len ) \
_snprintf_s( dst, len, count, format, val1, val2, val3 )

// sscanf
#define sscanf_royale( src, format, target ) \
sscanf_s( src, format, target )

#define sscanf_royale_v2( src, format, target1, target2 ) \
sscanf_s( src, format, target1, target2 )

#define sscanf_royale_v3( src, format, target1, target2, target3 ) \
sscanf_s( src, format, target1, target2, target3 )

#define sscanf_royale_v4( src, format, target1, target2, target3, target4 ) \
sscanf_s( src, format, target1, target2, target3, target4 )

#define sscanf_royale_v5( src, format, target1, target2, target3, target4, target5 ) \
sscanf_s( src, format, target1, target2, target3, target4, target5 )

// fscanf
#define fscanf_royale( file, format, var ) \
fscanf_s ( file, format, var )

// strtok
#define strtok_royale( buf, delim ) \
strtok_s( buf, delim, 0 )

#define time_t_royale __time64_t
#define localtime_royale( tinfo, rtime ) \
_localtime64_s( tinfo, rtime )

#define strdup_royale _strdup

#define stat_royale _stat64

#else

// sprintf
#define sprintf_royale( buf, str, len ) \
sprintf ( buf, str )
#define sprintf_royale_v1( buf, str, val1, len ) \
sprintf ( buf, str, val1 )
#define sprintf_royale_v2( buf, str, val1, val2, len ) \
sprintf ( buf, str, val1, val2 )
#define sprintf_royale_v3( buf, str, val1, val2, val3, len ) \
sprintf ( buf, str, val1, val2, val3 )
#define sprintf_royale_v4( buf, str, val1, val2, val3, val4, len ) \
sprintf ( buf, str, val1, val2, val3, val4 )
#define sprintf_royale_v5( buf, str, val1, val2, val3, val4, val5, len ) \
sprintf ( buf, str, val1, val2, val3, val4, val5 )
#define sprintf_royale_v6( buf, str, val1, val2, val3, val4, val5, val6, len ) \
    sprintf ( buf, str, val1, val2, val3, val4, val5, val6 )

// fprintf
#define fprintf_royale( file, str, ... ) \
    fprintf ( file, str, __VA_ARGS__ )

// strncpy
#define strncpy_royale( dst, msg, max, len ) \
strncpy ( dst, msg, max )

// fopen
#define fopen_royale( file, filename, mode ) \
file = fopen ( filename, mode )

// fread
#define fread_royale( buf, bufsize, elsize, elcount, file ) \
fread( buf, elsize, elcount, file );

// snprintf
#define snprintf_royale( dst, count, format, len ) \
snprintf( dst, count, format )

#define snprintf_royale_v1( dst, count, format, val1, len ) \
snprintf( dst, count, format, val1 )
#define snprintf_royale_v2( dst, count, format, val1, val2, len ) \
snprintf( dst, count, format, val1, val2 )
#define snprintf_royale_v3( dst, count, format, val1, val2, val3, len ) \
snprintf( dst, count, format, val1, val2, val3 )

// sscanf
#define sscanf_royale( src, format, target ) \
sscanf ( src, format, target )

#define sscanf_royale_v2( src, format, target1, target2 ) \
sscanf ( src, format, target1, target2 )

#define sscanf_royale_v3( src, format, target1, target2, target3 ) \
sscanf ( src, format, target1, target2, target3 )

#define sscanf_royale_v4( src, format, target1, target2, target3, target4 ) \
sscanf ( src, format, target1, target2, target3, target4 )

#define sscanf_royale_v5( src, format, target1, target2, target3, target4, target5 ) \
sscanf ( src, format, target1, target2, target3, target4, target5 )

// fscanf
#define fscanf_royale( file, format, var ) \
fscanf ( file, format, var )

// strtok
#define strtok_royale( buf, delim ) \
strtok ( buf, delim )

#define time_t_royale time_t
#define localtime_royale( tinfo, rtime ) \
struct tm *tmpTimeInfo; \
tmpTimeInfo = localtime( rtime ); \
memcpy (tinfo, tmpTimeInfo, sizeof (struct tm))

#define strdup_royale strdup

#define stat_royale stat
#endif
