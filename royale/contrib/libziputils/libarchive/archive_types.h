/*-
 * Copyright (c) 2015 Infineon hack
 * All rights reserved.
 *
 */

#ifndef ARCHIVE_TYPES_H
#define	ARCHIVE_TYPES_H

/* Get appropriate definitions of standard POSIX-style types. */
/* These should match the types used in 'struct stat' */
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__WATCOMC__)
  typedef __int64 la_int64_t;
  
  # if defined(_SSIZE_T_DEFINED) || defined(_SSIZE_T_)
    typedef ssize_t la_ssize_t;
  # elif defined(_WIN64)
    typedef __int64 la_ssize_t;
  # else
    typedef long la_ssize_t;
  # endif
#else
  # include <unistd.h>  /* ssize_t */
  # if defined(_SCO_DS)
    typedef long long la_int64_t;
  # else
    typedef int64_t la_int64_t;
  # endif
    typedef ssize_t la_ssize_t;
#endif

#endif /* !ARCHIVE_TYPES_H */
