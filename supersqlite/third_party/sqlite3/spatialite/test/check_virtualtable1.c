/*

 check_virtualtable1.c -- SpatiaLite Test Case

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
		      "create VIRTUAL TABLE places USING VirtualText(\"testcase1.csv\", UTF-8, 0, POINT, DOUBLEQUOTE);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualText error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

    asprintf (&sql_statement,
	      "select col003, col005, col006, col008 from places WHERE col003 = \"Canal Creek\";");
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
    if ((rows != 2) || (columns != 4))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -11;
      }
    if (strcmp (results[0], "COL003") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -12;
      }
    if (strcmp (results[4], "Canal Creek") != 0)
      {
	  fprintf (stderr, "Unexpected error: name4() bad result: %s.\n",
		   results[4]);
	  return -13;
      }
    if (strncmp (results[5], "-27.86667", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: lat1() bad result: %s.\n",
		   results[5]);
	  return -14;
      }
    if (strncmp (results[6], "151.51667", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: lon2() bad result: %s.\n",
		   results[6]);
	  return -15;
      }
    if (strcmp (results[8], "Canal Creek") != 0)
      {
	  fprintf (stderr, "Unexpected error: name8() bad result: %s.\n",
		   results[8]);
	  return -16;
      }
    sqlite3_free_table (results);

    ret = sqlite3_exec (db_handle, "BEGIN;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "BEGIN error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }

    ret =
	sqlite3_exec (db_handle, "DELETE FROM places WHERE col015 > 100000;",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_READONLY)
      {
	  fprintf (stderr, "UPDATE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }
    sqlite3_free (err_msg);

    ret = sqlite3_exec (db_handle, "ROLLBACK;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "ROLLBACK error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT ROWNO, col003, col015 FROM places WHERE col015 > 100000",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -24;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col002 FROM places WHERE col001 > 2172517 AND col001 <= 2172519",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }
    if ((rows != 2) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -26;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col002 FROM places WHERE col001 >= 2172517 AND col001 < 2172519",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -27;
      }
    if ((rows != 2) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -28;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col002 FROM places WHERE col001 = 2172517",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -30;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col002 > 'Canberra' AND col002 <= 'Canbrae'",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -31;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -32;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col002 >= 'Canberra' AND col002 < 'Canbrae'",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -33;
      }
    if ((rows != 2) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -34;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col002 = 'Canbrae'",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -30;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE ROWNO = 5",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -31;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -32;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col006 > 149.0 AND col006 <= 149.1",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -33;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -34;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col006 >= 149.1 AND col006 < 149.2",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -35;
      }
    if ((rows != 2) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -36;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col006 = 149.1",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -37;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -38;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col006 >= 149 AND col006 < 150",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -39;
      }
    if ((rows != 4) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -40;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col006 > 149 AND col006 <= 150",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -41;
      }
    if ((rows != 4) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -42;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col012  = 23940.0",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -43;
      }
    if ((rows != 2) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -44;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col012  >= 20000.0 AND col012 < 24000.0",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -45;
      }
    if ((rows != 2) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -46;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT col001 FROM places WHERE col012  > 20000.0 AND col012 <= 24000.0",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -45;
      }
    if ((rows != 2) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result2: %i/%i.\n",
		   rows, columns);
	  return -46;
      }
    sqlite3_free_table (results);

    ret = sqlite3_exec (db_handle, "DROP TABLE places;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -47;
      }

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
#endif /* end ICONV conditional */

    spatialite_shutdown ();
    return 0;
}
