/*

 check_virtualtable3.c -- SpatiaLite Test Case

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
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

#ifdef _WIN32
#include "asprintf4win.h"
#endif

int
main (int argc, char *argv[])
{
#ifndef OMIT_ICONV		/* only if ICONV is supported */
    sqlite3 *db_handle = NULL;
    char *sql_statement;
    int ret;
    char *err_msg = NULL;
    char **results;
    int rows;
    int columns;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

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
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE dbftest USING VirtualDBF(shapetest1.dbf, UTF-8);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualDBF error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where testcase2 < 20;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -4;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -5;
      }
    if (strcmp (results[2], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[2]);
	  return -6;
      }
    if (strcmp (results[3], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -7;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where testcase2 <= 19;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -11;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -12;
      }
    if (strcmp (results[2], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[2]);
	  return -13;
      }
    if (strcmp (results[3], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -14;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where testcase2 = 20;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -16;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -17;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -18;
      }
    if (strcmp (results[2], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees bad result: %s.\n",
		   results[2]);
	  return -19;
      }
    if (strcmp (results[3], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer2() bad result: %s.\n",
		   results[3]);
	  return -20;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where testcase2 > 2;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -23;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -24;
      }
    if (strcmp (results[2], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees2 bad result: %s.\n",
		   results[2]);
	  return -25;
      }
    if (strcmp (results[3], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer4() bad result: %s.\n",
		   results[3]);
	  return -26;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where testcase2 >= 20;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -28;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -29;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -30;
      }
    if (strcmp (results[2], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees3 bad result: %s.\n",
		   results[2]);
	  return -31;
      }
    if (strcmp (results[3], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer5() bad result: %s.\n",
		   results[3]);
	  return -32;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where testcase1 < \"p\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -34;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -35;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -36;
      }
    if (strcmp (results[2], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees bad result: %s.\n",
		   results[2]);
	  return -37;
      }
    if (strcmp (results[3], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -38;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where testcase1 <= \"p\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -40;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -41;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -42;
      }
    if (strcmp (results[2], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees bad result: %s.\n",
		   results[2]);
	  return -43;
      }
    if (strcmp (results[3], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -44;
      }
    sqlite3_free_table (results);

    ret = sqlite3_exec (db_handle, "BEGIN;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "BEGIN error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -46;
      }

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where testcase1 > \"p\";");
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
    if ((rows != 1) || (columns != 2))
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
    if (strcmp (results[2], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[2]);
	  return -51;
      }
    if (strcmp (results[3], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -52;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (db_handle, "DELETE FROM dbftest WHERE testcase2 = 2;",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_READONLY)
      {
	  fprintf (stderr, "UPDATE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -46;
      }
    sqlite3_free (err_msg);

    ret = sqlite3_exec (db_handle, "ROLLBACK;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "ROLLBACK error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -47;
      }

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where testcase1 >= \"p\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -54;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -55;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -56;
      }
    if (strcmp (results[2], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[2]);
	  return -57;
      }
    if (strcmp (results[3], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -58;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where testcase1 = \"windward\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -54;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -55;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -56;
      }
    if (strcmp (results[2], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[2]);
	  return -57;
      }
    if (strcmp (results[3], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -58;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where PKUID = 1;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -60;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -61;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -62;
      }
    if (strcmp (results[2], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[2]);
	  return -63;
      }
    if (strcmp (results[3], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -64;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where PKUID < 2;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -66;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -67;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -68;
      }
    if (strcmp (results[2], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[2]);
	  return -69;
      }
    if (strcmp (results[3], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -70;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where PKUID <= 1;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -72;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -73;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -74;
      }
    if (strcmp (results[2], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[2]);
	  return -75;
      }
    if (strcmp (results[3], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -76;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where PKUID > 1;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -78;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -79;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -80;
      }
    if (strcmp (results[2], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees bad result: %s.\n",
		   results[2]);
	  return -81;
      }
    if (strcmp (results[3], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -82;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select testcase1, testcase2 from dbftest where PKUID >= 2;");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -84;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -85;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -86;
      }
    if (strcmp (results[2], "orde lees") != 0)
      {
	  fprintf (stderr, "Unexpected error: orde lees bad result: %s.\n",
		   results[2]);
	  return -87;
      }
    if (strcmp (results[3], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -88;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select PKUID, testcase1, testcase2 from dbftest where testcase1 LIKE \"wind%%\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -90;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -91;
      }
    if (strcmp (results[0], "PKUID") != 0)
      {
	  fprintf (stderr, "Unexpected error: header uid bad result: %s.\n",
		   results[0]);
	  return -92;
      }
    if (strcmp (results[1], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header bad result: %s.\n",
		   results[1]);
	  return -93;
      }
    if (strcmp (results[3], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward PK bad result: %s.\n",
		   results[3]);
	  return -93;
      }
    if (strcmp (results[4], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[4]);
	  return -94;
      }
    sqlite3_free_table (results);

    ret = sqlite3_exec (db_handle, "DROP TABLE dbftest;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -49;
      }

    /* error cases */
    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE toofewargs USING VirtualDBF(\"shapetest1.dbf\");",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "VirtualDBF unexpected result: %i\n", ret);
	  return -95;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE toomanyargs USING VirtualDBF(\"shapetest1.dbf\", UTF-8, 1, 1);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "VirtualDBF unexpected result: %i\n", ret);
	  return -96;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE nosuchfile USING VirtualDBF(\"not_a_file.dbf\", UTF-8);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualDBF error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -97;
      }
    ret =
	sqlite3_get_table (db_handle, "SELECT * from nosuchfile;", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -98;
      }
    if ((rows != 0) || (columns != 0))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns no suchfile: %i/%i.\n",
		   rows, columns);
	  return -99;
      }
    sqlite3_free_table (results);
    ret =
	sqlite3_exec (db_handle, "DROP TABLE nosuchfile;", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -100;
      }

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE onesidedquote USING VirtualDBF('shapetest1.dbf, UTF-8);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "VirtualDBF unexpected result: %i\n", ret);
	  return -101;
      }
    sqlite3_free (err_msg);

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
#endif /* end ICONV conditional */

    spatialite_shutdown ();
    return 0;
}
