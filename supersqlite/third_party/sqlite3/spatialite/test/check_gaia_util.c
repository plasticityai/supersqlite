/*

 check_gaia_util.c -- SpatiaLite Test Case

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

#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gaiaaux.h"

#ifdef _WIN32
#include "asprintf4win.h"
#endif

int
main (int argc, char *argv[])
{
    int result;
    char *test_str1;
    char *quoted_str;

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    result = gaiaIsReservedSqliteName ("AUTOINCREMENT");
    if (!result)
      {
	  fprintf (stderr,
		   "gaiaIsReservedSqliteName() fail for AUTOINCREMENT: %i\n",
		   result);
	  return -1;
      }

    result = gaiaIsReservedSqliteName ("AUTOINCREMEN");
    if (result)
      {
	  fprintf (stderr,
		   "gaiaIsReservedSqliteName() fail for AUTOINCREMEN: %i\n",
		   result);
	  return -2;
      }

    result = gaiaIsReservedSqliteName ("AUTOINCREMENTED");
    if (result)
      {
	  fprintf (stderr,
		   "gaiaIsReservedSqliteName() fail for AUTOINCREMENT: %i\n",
		   result);
	  return -3;
      }

    result = gaiaIsReservedSqliteName ("foo");
    if (result)
      {
	  fprintf (stderr, "gaiaIsReservedSqliteName() fail for foo: %i\n",
		   result);
	  return -4;
      }

    result = gaiaIsReservedSqliteName ("ALL");	/* first item */
    if (!result)
      {
	  fprintf (stderr, "gaiaIsReservedSqliteName() fail for ALL: %i\n",
		   result);
	  return -5;
      }

    result = gaiaIsReservedSqliteName ("WHERE");	/* last item */
    if (!result)
      {
	  fprintf (stderr, "gaiaIsReservedSqliteName() fail for WHERE: %i\n",
		   result);
	  return -6;
      }

    result = gaiaIsReservedSqliteName ("autoincrement");
    if (!result)
      {
	  fprintf (stderr,
		   "gaiaIsReservedSqliteName() fail for autoincrement: %i\n",
		   result);
	  return -7;
      }

    result = gaiaIsReservedSqlName ("SELECT");
    if (!result)
      {
	  fprintf (stderr, "gaiaIsReservedSqlName() fail for SELECT: %i\n",
		   result);
	  return -8;
      }

    result = gaiaIsReservedSqlName ("select");
    if (!result)
      {
	  fprintf (stderr, "gaiaIsReservedSqlName() fail for select: %i\n",
		   result);
	  return -9;
      }

    result = gaiaIsReservedSqlName ("foo");
    if (result)
      {
	  fprintf (stderr, "gaiaIsReservedSqlName() fail for foo: %i\n",
		   result);
	  return -10;
      }

    result = gaiaIsReservedSqlName ("ABSOLUTE");
    if (!result)
      {
	  fprintf (stderr, "gaiaIsReservedSqlName() fail for ABSOLUTE: %i\n",
		   result);
	  return -11;
      }

    result = gaiaIsReservedSqlName ("ZONE");
    if (!result)
      {
	  fprintf (stderr, "gaiaIsReservedSqlName() fail for ZONE: %i\n",
		   result);
	  return -12;
      }

    result = gaiaIsReservedSqlName ("SELECTED");
    if (result)
      {
	  fprintf (stderr, "gaiaIsReservedSqlName() fail for SELECTED: %i\n",
		   result);
	  return -13;
      }

    result = gaiaIsReservedSqlName ("SELEC");
    if (result)
      {
	  fprintf (stderr, "gaiaIsReservedSqlName() fail for SELEC: %i\n",
		   result);
	  return -14;
      }

    result = gaiaIllegalSqlName (NULL);
    if (!result)
      {
	  fprintf (stderr, "gaiaIllegalSqlName() fail for (NULL): %i\n",
		   result);
	  return -15;
      }

    result = gaiaIllegalSqlName ("a");
    if (result)
      {
	  fprintf (stderr, "gaiaIllegalSqlName() fail for a: %i\n", result);
	  return -16;
      }
    result = gaiaIllegalSqlName ("-");
    if (!result)
      {
	  fprintf (stderr, "gaiaIllegalSqlName() fail for -: %i\n", result);
	  return -17;
      }
    result = gaiaIllegalSqlName ("AbbcdE0187r");
    if (result)
      {
	  fprintf (stderr, "gaiaIllegalSqlName() fail for AbbcdE0187r: %i\n",
		   result);
	  return -18;
      }
    result = gaiaIllegalSqlName ("AbbcdE0187+");
    if (!result)
      {
	  fprintf (stderr, "gaiaIllegalSqlName() fail for AbbcdE0187r+: %i\n",
		   result);
	  return -19;
      }
    result = gaiaIllegalSqlName ("");
    if (!result)
      {
	  fprintf (stderr, "gaiaIllegalSqlName() fail for (empty): %i\n",
		   result);
	  return -20;
      }
    result = gaiaIllegalSqlName ("_ABCedFg");
    if (!result)
      {
	  fprintf (stderr, "gaiaIllegalSqlName() fail for (empty): %i\n",
		   result);
	  return -21;
      }

    asprintf (&test_str1, "SELECT %s from %s;", "Foo", "Bar");
    gaiaCleanSqlString (test_str1);
    if (strcmp (test_str1, "SELECT Foo from Bar;") != 0)
      {
	  fprintf (stderr, "gaiaCleanSqlString failure: %s\n", test_str1);
	  free (test_str1);
	  return -22;
      }
    free (test_str1);

    asprintf (&test_str1, "SELECT %s from %s;           ", "Foo", "Bar");
    gaiaCleanSqlString (test_str1);
    if (strcmp (test_str1, "SELECT Foo from Bar;") != 0)
      {
	  fprintf (stderr, "gaiaCleanSqlString failure: %s\n", test_str1);
	  free (test_str1);
	  return -23;
      }
    free (test_str1);

    asprintf (&test_str1, "SELECT %s from %s;           ", "Foo", "'");
    gaiaCleanSqlString (test_str1);
    if (strcmp (test_str1, "SELECT Foo from '';") != 0)
      {
	  fprintf (stderr, "gaiaCleanSqlString failure: %s\n", test_str1);
	  free (test_str1);
	  return -24;
      }
    free (test_str1);

#if 0
    /* TODO: This will cause a buffer overflow */
    asprintf (&test_str1, "SELECT %s from %s;", "Foo", "'");
    gaiaCleanSqlString (test_str1);
    if (strcmp (test_str1, "SELECT Foo from '';") != 0)
      {
	  fprintf (stderr, "gaiaCleanSqlString failure: %s\n", test_str1);
	  free (test_str1);
	  return -25;
      }
    free (test_str1);
#endif

    asprintf (&test_str1, "           ");
    gaiaCleanSqlString (test_str1);
    if (strcmp (test_str1, "") != 0)
      {
	  fprintf (stderr, "gaiaCleanSqlString failure: %s\n", test_str1);
	  free (test_str1);
	  return -26;
      }
    free (test_str1);

    asprintf (&test_str1, "SELECT %s from %s;", "Foo", "Bar");
    quoted_str = gaiaSingleQuotedSql (test_str1);
    if (strcmp (quoted_str, "SELECT Foo from Bar;") != 0)
      {
	  fprintf (stderr, "gaiaSingleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -27;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "SELECT %s from %s;           ", "Foo", "Bar");
    quoted_str = gaiaSingleQuotedSql (test_str1);
    if (strcmp (quoted_str, "SELECT Foo from Bar;") != 0)
      {
	  fprintf (stderr, "gaiaSingleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -28;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "SELECT %s from %s;", "Foo", "'");
    quoted_str = gaiaSingleQuotedSql (test_str1);
    if (strcmp (quoted_str, "SELECT Foo from '';") != 0)
      {
	  fprintf (stderr, "gaiaSingleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -29;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "SELECT %s from %s   ;    ", "Foo", "Bar");
    quoted_str = gaiaSingleQuotedSql (test_str1);
    if (strcmp (quoted_str, "SELECT Foo from Bar   ;") != 0)
      {
	  fprintf (stderr, "gaiaSingleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -30;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "SELECT %s from %s;", "'", "Bar");
    quoted_str = gaiaSingleQuotedSql (test_str1);
    if (strcmp (quoted_str, "SELECT '' from Bar;") != 0)
      {
	  fprintf (stderr, "gaiaSingleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -31;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "My Name");
    quoted_str = gaiaDoubleQuotedSql (test_str1);
    if (strcmp (quoted_str, "My Name") != 0)
      {
	  fprintf (stderr, "gaiaDoubleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -32;
      }
    free (test_str1);
    free (quoted_str);

    quoted_str = gaiaDoubleQuotedSql (NULL);
    if (quoted_str != NULL)
      {
	  fprintf (stderr, "gaiaDoubleQuotedSql NULL failure: %s\n",
		   quoted_str);
	  free (quoted_str);
	  return -33;
      }

    quoted_str = gaiaSingleQuotedSql (NULL);
    if (quoted_str != NULL)
      {
	  fprintf (stderr, "gaiaSingleQuotedSql NULL failure: %s\n",
		   quoted_str);
	  free (quoted_str);
	  return -34;
      }

    quoted_str = gaiaQuotedSql ("foo", GAIA_SQL_SINGLE_QUOTE);
    if (strcmp (quoted_str, "foo") != 0)
      {
	  fprintf (stderr, "gaiaQuotedSql failure SINGLE_QUOTE: %s\n",
		   quoted_str);
	  free (quoted_str);
	  return -35;
      }
    free (quoted_str);

    quoted_str = gaiaQuotedSql ("foo", GAIA_SQL_DOUBLE_QUOTE);
    if (strcmp (quoted_str, "foo") != 0)
      {
	  fprintf (stderr, "gaiaQuotedSql failure DOUBLE_QUOTE: %s\n",
		   quoted_str);
	  free (quoted_str);
	  return -36;
      }
    free (quoted_str);

    quoted_str = gaiaQuotedSql ("foo", 1000);
    if (quoted_str != NULL)
      {
	  fprintf (stderr, "gaiaQuotedSql failure 1000: %s\n", quoted_str);
	  free (quoted_str);
	  return -37;
      }

    asprintf (&test_str1, "My \"Name");
    quoted_str = gaiaDoubleQuotedSql (test_str1);
    if (strcmp (quoted_str, "My \"\"Name") != 0)
      {
	  fprintf (stderr, "gaiaDoubleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -38;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "My \"Name                   ");
    quoted_str = gaiaDoubleQuotedSql (test_str1);
    if (strcmp (quoted_str, "My \"\"Name") != 0)
      {
	  fprintf (stderr, "gaiaDoubleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -39;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "%s", "");
    quoted_str = gaiaDoubleQuotedSql (test_str1);
    if (strcmp (quoted_str, "") != 0)
      {
	  fprintf (stderr, "gaiaDoubleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -40;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "          ");
    quoted_str = gaiaDoubleQuotedSql (test_str1);
    if (strcmp (quoted_str, "") != 0)
      {
	  fprintf (stderr, "gaiaDoubleQuotedSql failure: |%s|\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -41;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "'          ");
    quoted_str = gaiaDoubleQuotedSql (test_str1);
    if (strcmp (quoted_str, "'") != 0)
      {
	  fprintf (stderr, "gaiaDoubleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -42;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "'");
    quoted_str = gaiaDoubleQuotedSql (test_str1);
    if (strcmp (quoted_str, "'") != 0)
      {
	  fprintf (stderr, "gaiaDoubleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -43;
      }
    free (test_str1);
    free (quoted_str);

    asprintf (&test_str1, "\"");
    quoted_str = gaiaSingleQuotedSql (test_str1);
    if (strcmp (quoted_str, "\"") != 0)
      {
	  fprintf (stderr, "gaiaSingleQuotedSql failure: %s\n", quoted_str);
	  free (test_str1);
	  free (quoted_str);
	  return -44;
      }
    free (test_str1);
    free (quoted_str);

    spatialite_shutdown ();
    return 0;
}
