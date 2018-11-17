/*

 check_virtualtable2.c -- SpatiaLite Test Case

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
Ahmadou Dicko <dicko.ahmadou@gmail.com>

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
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

#ifdef _WIN32
#include "asprintf4win.h"
#endif

int
do_test (sqlite3 * db_handle)
{
#ifndef OMIT_ICONV		/* only if ICONV is supported */
    char *sql_statement;
    int ret;
    char *err_msg = NULL;
    char **results;
    int rows;
    int columns;

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE shapetest USING VirtualShape(\"shapetest1\", UTF-8, 4326);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualShape error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT RegisterVirtualGeometry('shapetest')",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "RegisterVirtualGeometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "RegisterVirtualGeometry Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -4;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "RegisterVirtualGeometry Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -5;
      }
    sqlite3_free_table (results);


    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where testcase2 < 20;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -7;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -8;
      }
    if (strcmp (results[3], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[3]);
	  return -9;
      }
    if (strcmp (results[4], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -10;
      }
    if (strcmp (results[5], "POINT(3480766.311245 4495355.740524)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -11;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where testcase2 <= 19;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -13;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -14;
      }
    if (strcmp (results[3], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[3]);
	  return -15;
      }
    if (strcmp (results[4], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -16;
      }
    if (strcmp (results[5], "POINT(3480766.311245 4495355.740524)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -17;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where testcase2 = 20;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -18;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -19;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -20;
      }
    if (strcmp (results[3], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees bad result: %s.\n",
		   results[3]);
	  return -21;
      }
    if (strcmp (results[4], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer2() bad result: %s.\n",
		   results[4]);
	  return -22;
      }
    if (strcmp (results[5], "POINT(3482470.825574 4495691.054818)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -23;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where testcase2 > 2;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -25;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -26;
      }
    if (strcmp (results[3], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees2 bad result: %s.\n",
		   results[3]);
	  return -27;
      }
    if (strcmp (results[4], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer4() bad result: %s.\n",
		   results[4]);
	  return -28;
      }
    if (strcmp (results[5], "POINT(3482470.825574 4495691.054818)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -29;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where testcase2 >= 20;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -31;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -32;
      }
    if (strcmp (results[3], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees3 bad result: %s.\n",
		   results[3]);
	  return -33;
      }
    if (strcmp (results[4], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer5() bad result: %s.\n",
		   results[4]);
	  return -33;
      }
    if (strcmp (results[5], "POINT(3482470.825574 4495691.054818)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -34;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where testcase1 < \"p\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -35;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -36;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -37;
      }
    if (strcmp (results[3], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees bad result: %s.\n",
		   results[3]);
	  return -38;
      }
    if (strcmp (results[4], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -39;
      }
    if (strcmp (results[5], "POINT(3482470.825574 4495691.054818)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -40;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where testcase1 <= \"p\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -41;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -42;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -43;
      }
    if (strcmp (results[3], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees bad result: %s.\n",
		   results[3]);
	  return -44;
      }
    if (strcmp (results[4], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -45;
      }
    if (strcmp (results[5], "POINT(3482470.825574 4495691.054818)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -46;
      }
    sqlite3_free_table (results);


    ret = sqlite3_exec (db_handle, "BEGIN;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "BEGIN error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -47;
      }

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where testcase1 > \"p\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -48;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -49;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -50;
      }
    if (strcmp (results[3], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[3]);
	  return -51;
      }
    if (strcmp (results[4], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -52;
      }
    if (strcmp (results[5], "POINT(3480766.311245 4495355.740524)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -53;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (db_handle, "DELETE FROM shapetest WHERE testcase2 = 2;",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_READONLY)
      {
	  fprintf (stderr, "UPDATE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -54;
      }
    sqlite3_free (err_msg);

    ret = sqlite3_exec (db_handle, "ROLLBACK;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "ROLLBACK error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -55;
      }

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where testcase1 >= \"p\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -56;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -57;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -58;
      }
    if (strcmp (results[3], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[3]);
	  return -59;
      }
    if (strcmp (results[4], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -60;
      }
    if (strcmp (results[5], "POINT(3480766.311245 4495355.740524)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -61;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where testcase1 = \"windward\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -62;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -63;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -64;
      }
    if (strcmp (results[3], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[3]);
	  return -65;
      }
    if (strcmp (results[4], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -66;
      }
    if (strcmp (results[5], "POINT(3480766.311245 4495355.740524)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -67;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where PKUID = 1;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -68;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -69;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -70;
      }
    if (strcmp (results[3], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[3]);
	  return -71;
      }
    if (strcmp (results[4], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -72;
      }
    if (strcmp (results[5], "POINT(3480766.311245 4495355.740524)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -73;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where PKUID < 2;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -74;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -75;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -76;
      }
    if (strcmp (results[3], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[3]);
	  return -77;
      }
    if (strcmp (results[4], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -78;
      }
    if (strcmp (results[5], "POINT(3480766.311245 4495355.740524)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -79;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where PKUID <= 1;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -80;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -81;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -82;
      }
    if (strcmp (results[3], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[3]);
	  return -83;
      }
    if (strcmp (results[4], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -84;
      }
    if (strcmp (results[5], "POINT(3480766.311245 4495355.740524)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -85;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where PKUID > 1;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -86;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -87;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -88;
      }
    if (strcmp (results[3], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees bad result: %s.\n",
		   results[3]);
	  return -89;
      }
    if (strcmp (results[4], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -90;
      }
    if (strcmp (results[5], "POINT(3482470.825574 4495691.054818)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -91;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2, AsText(Geometry) from shapetest where PKUID >= 2;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -92;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -93;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -94;
      }
    if (strcmp (results[3], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees bad result: %s.\n",
		   results[3]);
	  return -95;
      }
    if (strcmp (results[4], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -96;
      }
    if (strcmp (results[5], "POINT(3482470.825574 4495691.054818)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -97;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select PKUID, testcase1, testcase2, AsText(Geometry) from shapetest where testcase1 LIKE \"wind%%\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -98;
      }
    if ((rows != 1) || (columns != 4))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -99;
      }
    if (strcmp (results[0], "PKUID") != 0)
      {
	  fprintf (stderr, "Unexpected error: header uid bad result: %s.\n",
		   results[0]);
	  return -100;
      }
    if (strcmp (results[1], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header bad result: %s.\n",
		   results[1]);
	  return -101;
      }
    if (strcmp (results[4], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward PK bad result: %s.\n",
		   results[4]);
	  return -102;
      }
    if (strcmp (results[5], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[5]);
	  return -103;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle, "SELECT DropVirtualGeometry('shapetest')",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DropVirtualGeometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -104;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "DropVirtualGeometry Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -105;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "DropVirtualGeometry Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -106;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE shapetest2 USING VirtualShape(\"shp/merano-3d/roads\", CP1252, 25832);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualShape error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -107;
      }
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT RegisterVirtualGeometry('shapetest2')",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "RegisterVirtualGeometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -108;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "RegisterVirtualGeometry Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -109;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "RegisterVirtualGeometry Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -110;
      }
    sqlite3_free_table (results);
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT UpdateLayerStatistics('shapetest2')",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UpdateLayerStatistics error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -111;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "UpdateLayerStatistics Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -112;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "UpdateLayerStatistics Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -113;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT DropVirtualGeometry('shapetest2')", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DropVirtualGeometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -114;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "DropVirtualGeometry Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -115;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "DropVirtualGeometry Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -116;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE shapetest3 USING VirtualShape(\"shp/merano-3d/points\", CP1252, 25832);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualShape error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -117;
      }
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT RegisterVirtualGeometry('shapetest3')",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "RegisterVirtualGeometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -118;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "RegisterVirtualGeometry Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -119;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "RegisterVirtualGeometry Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -120;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT DropVirtualGeometry('shapetest3')", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DropVirtualGeometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -121;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "DropVirtualGeometry Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -122;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "DropVirtualGeometry Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -123;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE shapetest4 USING VirtualShape(\"shp/merano-3d/polygons\", CP1252, 25832);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualShape error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -124;
      }
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT RegisterVirtualGeometry('shapetest4')",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "RegisterVirtualGeometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -125;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "RegisterVirtualGeometry Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -126;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "RegisterVirtualGeometry Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -127;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT DropVirtualGeometry('shapetest4')", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DropVirtualGeometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -128;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "DropVirtualGeometry Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -129;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "DropVirtualGeometry Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -130;
      }
    sqlite3_free_table (results);

/* final DB cleanup */
    ret =
	sqlite3_exec (db_handle,
		      "DELETE FROM spatialite_history WHERE geometry_column IS NOT NULL",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DELETE FROM spatialite_history error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -131;
      }
    ret = sqlite3_exec (db_handle, "VACUUM", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VACUUM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -132;
      }
#endif /* end ICONV conditional */

    return 0;
}

int
main (int argc, char *argv[])
{
#ifndef OMIT_ICONV		/* only if ICONV is supported */
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

/* testing current style metadata layout >= v.4.0.0 */
    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  db_handle = NULL;
	  return -1;
      }

    spatialite_init_ex (db_handle, cache, 0);

    ret =
	sqlite3_exec (db_handle, "SELECT InitSpatialMetadata(1)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "InitSpatialMetadata() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -2;
      }

    ret = do_test (db_handle);
    if (ret != 0)
      {
	  fprintf (stderr,
		   "error while testing legacy style metadata layout\n");
	  return ret;
      }

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);

/* testing legacy style metadata layout <= v.3.1.0 */
    ret = system ("cp test-legacy-3.0.1.sqlite copy-virt2-legacy-3.0.1.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot copy legacy v.3.0.1 database\n");
	  return -1;
      }
    cache = spatialite_alloc_connection ();
    ret =
	sqlite3_open_v2 ("copy-virt2-legacy-3.0.1.sqlite", &db_handle,
			 SQLITE_OPEN_READWRITE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open legacy v.3.0.1 database: %s\n",
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  db_handle = NULL;
	  return -1;
      }

    spatialite_init_ex (db_handle, cache, 0);

    ret = do_test (db_handle);
    if (ret != 0)
      {
	  fprintf (stderr,
		   "error while testing legacy style metadata layout\n");
	  return ret;
      }

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
    ret = unlink ("copy-virt2-legacy-3.0.1.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot remove legacy v.3.0.1 database\n");
	  return -20;
      }

#endif /* end ICONV conditional */

    spatialite_shutdown ();
    return 0;
}
