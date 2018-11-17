/*

 check_gaia_utf8.c -- SpatiaLite Test Case

 Author: Brad Hards <bradh@frogmouth.net>

 ------------------------------------------------------------------------------
 
 Version: MPL 1.1/GPL 2.0/LGPL 2.1
 
 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/
 
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is the SpatiaLite library

The Initial Developer of the Original Code is Alessandro Furieri
 
Portions created by the Initial Developer are Copyright (C) 2011
the Initial Developer. All Rights Reserved.

Contributor(s):
Brad Hards <bradh@frogmouth.net>

Alternatively, the contents of this file may be used under the terms of
either the GNU General Public License Version 2 or later (the "GPL"), or
the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
in which case the provisions of the GPL or the LGPL are applicable instead
of those above. If you wish to allow use of your version of this file only
under the terms of either the GPL or the LGPL, and not to allow others to
use your version of this file under the terms of the MPL, indicate your
decision by deleting the provisions above and replace them with the notice
and other provisions required by the GPL or the LGPL. If you do not delete
the provisions above, a recipient may use your version of this file under
the terms of any one of the MPL, the GPL or the LGPL.
 
*/
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gaiaaux.h"

#ifdef _WIN32
#include "asprintf4win.h"
#endif

int
main (int argc, char *argv[])
{
#ifndef OMIT_ICONV		/* only if ICONV is supported */
    void *converter;
    char *test_str1;
    int err;

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    asprintf (&test_str1, "Hello World");
    gaiaConvertCharset (&test_str1, "ASCII", "UTF-8");
    if (strcmp (test_str1, "Hello World") != 0)
      {
	  fprintf (stderr, "bad ASCII to UTF-8 conversion: %s\n", test_str1);
	  free (test_str1);
	  return -1;
      }
    free (test_str1);

#if 0
    /* TODO: this will cause a buffer overflow */
    asprintf (&test_str1, "Hello World");
    gaiaConvertCharset (&test_str1, "ASCII", "UTF-16LE");
    if (memcmp (test_str1, "H\0e\0l\0l\0o\0 \0W\0o\0r\0l\0d\0\0\0", 24) != 0)
      {
	  fprintf (stderr, "bad ASCII to UTF-16LE conversion\n");
	  free (test_str1);
	  return -2;
      }
    free (test_str1);
#endif

    converter = gaiaCreateUTF8Converter ("CP1252");
    if (!converter)
      {
	  fprintf (stderr, "null UTF8 converter\n");
	  return -3;
      }

    test_str1 =
	gaiaConvertToUTF8 (converter, "Hello world", strlen ("Hello world"),
			   &err);
    if (memcmp ("Hello world", test_str1, strlen ("Hello world") + 1) != 0)
      {
	  fprintf (stderr, "bad conversion to UTF8: %s\n", test_str1);
	  free (test_str1);
	  return -4;
      }
    free (test_str1);

    gaiaFreeUTF8Converter (converter);
    converter = NULL;
    /* test null converter */
    gaiaFreeUTF8Converter (converter);

    test_str1 =
	gaiaConvertToUTF8 (converter, "Hello world", strlen ("Hello world"),
			   &err);
    if ((test_str1 != NULL) || (err != 1))
      {
	  fprintf (stderr, "unexpected null converter result: %s, %i\n",
		   test_str1, err);
	  return -5;
      }

    /* there is no sane way to test this automatically */
    printf ("Local codeset: %s\n", gaiaGetLocaleCharset ());
#endif /* end ICONV conditional */

    spatialite_shutdown ();
    return 0;
}
