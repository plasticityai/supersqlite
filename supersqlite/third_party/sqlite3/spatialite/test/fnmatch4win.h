/* Copyright (C) 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.  */

/*
 * Modified for WinCvs/MacCVS : Alexandre Parenteau <aubonbeurre@hotmail.com> --- June 1998
 */

#ifndef	_FNMATCH_H

#define	_FNMATCH_H	1

#ifdef __cplusplus
extern "C"
{
#endif

#define FNM_NOMATCH     1	/* Match failed. */
#define FNM_NOESCAPE    0x01	/* Disable backslash escaping. */
#define FNM_PATHNAME    0x02	/* Slash must be matched by slash. */
#define FNM_PERIOD      0x04	/* Period must be matched by period. */
#define FNM_LEADING_DIR 0x08	/* Ignore /<tail> after Imatch. */
#define FNM_CASEFOLD    0x10	/* Case insensitive search. */
#define FNM_PREFIX_DIRS 0x20	/* Directory prefixes of pattern match too. */

/* Value returned by `fnmatch' if STRING does not match PATTERN.  */
#undef FNM_NOMATCH
#define	FNM_NOMATCH	1

#if !defined(__STDC__) && (defined(WIN32)|| defined(TARGET_OS_MAC))
#	define __STDC__ 1
#endif

/* Match STRING against the filename pattern PATTERN,
   returning zero if it matches, FNM_NOMATCH if not.  */
#if __STDC__
    extern int fnmatch (const char *pattern, const char *string, int flags);
#else
    extern int fnmatch ();
#endif

#if defined (__CYGWIN32__) || defined (WIN32)
    /* Under Windows NT, filenames are case-insensitive, and both / and \
       are path component separators.  */

#	define FOLD_FN_CHAR(c) (WNT_filename_classes[(unsigned char) (c)])
    extern unsigned char WNT_filename_classes[];
#endif				/* defined (__CYGWIN32__) || defined (WIN32) */

#ifdef __cplusplus
}
#endif

#endif				/* fnmatch.h */
