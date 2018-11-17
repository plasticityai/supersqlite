/*

 check_virtualtable4.c -- SpatiaLite Test Case

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

#ifdef _WIN32
#include "asprintf4win.h"
#endif

struct test_step
{
    const char *sql;
    const int num_rows;
};

#define NUMSTEPS 36

struct test_step steps[NUMSTEPS] = {
    {"select col_2, col_4, col_5, col_7 from xltest WHERE col_2 > \"Canary Creek\";", 9},
    {"select col_2, col_4, col_5, col_7 from xltest WHERE col_2 < \"Canary Creek\";", 7},
    {"select col_2, col_4, col_5, col_7 from xltest WHERE col_2 >= \"Canary Creek\";", 10},
    {"select col_2, col_4, col_5, col_7 from xltest WHERE col_2 <= \"Canary Creek\";", 8},
    {"select col_2, col_4, col_5, col_7 from xltest WHERE col_2 = 3;", 0},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 > 100000;", 1},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 < 100000;", 16},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 < 100000 AND col_14 > 0;",
     1},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 = 895;", 1},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 = 895.0;", 1},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 <= 895;", 16},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 >= 895;", 2},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 > 100000.0;", 1},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 < 100000.0;", 16},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 = 895.0;", 1},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 <= 895.0;", 16},
    {"SELECT col_2, col_14 FROM xltest WHERE col_14 >= 895.0;", 2},
    {"SELECT col_2, col_14 FROM xltest WHERE row_no = 4", 1},
    {"SELECT col_2, col_14 FROM xltest WHERE row_no < 4", 3},
    {"SELECT col_2, col_14 FROM xltest WHERE row_no <= 4", 4},
    {"SELECT col_2, col_14 FROM xltest WHERE row_no >= 4", 14},
    {"SELECT col_2, col_14 FROM xltest WHERE row_no > 4", 13},
    {"SELECT col_2, col_14 FROM xltest WHERE row_no = 4.00", 0},
    {"select col_2, col_4, col_5 from xltest where col_4 < -30.0;", 8},
    {"select col_2, col_4, col_5 from xltest where col_4 > -30.0;", 9},
    {"select col_2, col_4, col_5 from xltest where col_4 <= -30.0;", 8},
    {"select col_2, col_4, col_5 from xltest where col_4 >= -30.0;", 9},
    {"select col_2, col_4, col_5 from xltest where col_5 = 149.1;", 1},
    {"select col_2, col_4, col_5 from xltest where col_4 < -30;", 8},
    {"select col_2, col_4, col_5 from xltest where col_4 > -30;", 9},
    {"select col_2, col_4, col_5 from xltest where col_4 <= -30;", 8},
    {"select col_2, col_4, col_5 from xltest where col_4 >= -30;", 9},
    {"select col_2, col_4, col_5 from xltest where col_4 = -30;", 0},
    {"SELECT row_no, col_4, col_5 from xltest", 17},
    {"select rowid, col_4, col_5 from xltest where col_4 = -26.0;", 1},
    {"select rowid, col_4, col_5 from xltest where col_4 = -26;", 1},
};

int
main (int argc, char *argv[])
{
#ifndef OMIT_FREEXL		/* only if FreeXL is supported */
    sqlite3 *db_handle = NULL;
    char *sql_statement;
    int ret;
    char *err_msg = NULL;
    int i;
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
		      "create VIRTUAL TABLE xltest USING VirtualXL(\"testcase1.xls\");",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualXL error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

    asprintf (&sql_statement,
	      "select col_2, col_4, col_5, col_7, rowid from xltest WHERE col_2 = \"Canal Creek\";");
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
    if ((rows != 2) || (columns != 5))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -11;
      }
    if (strcmp (results[0], "col_2") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -12;
      }
    if (strcmp (results[5], "Canal Creek") != 0)
      {
	  fprintf (stderr, "Unexpected error: name5() bad result: %s.\n",
		   results[5]);
	  return -13;
      }
    if (strncmp (results[6], "-27.86667", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: lat1() bad result: %s.\n",
		   results[6]);
	  return -14;
      }
    if (strncmp (results[7], "151.51667", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: lon2() bad result: %s.\n",
		   results[7]);
	  return -15;
      }
    if (strcmp (results[10], "Canal Creek") != 0)
      {
	  fprintf (stderr, "Unexpected error: name10() bad result: %s.\n",
		   results[10]);
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
	sqlite3_exec (db_handle, "DELETE FROM xltest WHERE col_14 > 100000;",
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

    for (i = 0; i < NUMSTEPS; ++i)
      {
	  ret =
	      sqlite3_get_table (db_handle, steps[i].sql, &results, &rows,
				 &columns, &err_msg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return -23;
	    }
	  if (rows != steps[i].num_rows)
	    {
		fprintf (stderr, "Unexpected num of rows for test %i: %i.\n", i,
			 rows);
		return -24;
	    }
	  sqlite3_free_table (results);
      }

    ret = sqlite3_exec (db_handle, "DROP TABLE xltest;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE nosuchworksheet USING VirtualXL(\"testcase1.xls\", 3);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualXL error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -26;
      }
    ret =
	sqlite3_exec (db_handle, "DROP TABLE nosuchworksheet;", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -27;
      }

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE nosuchfile USING VirtualXL(\"not_a_file.xls\", 3);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualXL error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -28;
      }
    ret =
	sqlite3_exec (db_handle, "DROP TABLE nosuchfile;", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE sheet2 USING VirtualXL(\"testcase1.xls\", 1, 1);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualXL error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }
    asprintf (&sql_statement,
	      "select row_no, place, lat, lon, rowid from sheet2 WHERE place = \"Canal Creek\";");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -31;
      }
    if ((rows != 4) || (columns != 5))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -32;
      }
    if (strcmp (results[0], "row_no") != 0)
      {
	  fprintf (stderr, "Unexpected error: sheet2() bad result: %s.\n",
		   results[0]);
	  return -33;
      }
    if (strcmp (results[6], "Canal Creek") != 0)
      {
	  fprintf (stderr, "Unexpected error: sheet2() bad result place: %s.\n",
		   results[6]);
	  return -34;
      }
    if (strncmp (results[7], "-27.86667", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: sheet2() bad result lat: %s.\n",
		   results[7]);
	  return -35;
      }
    if (strncmp (results[8], "151.51667", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: sheet2() bad result lon: %s.\n",
		   results[8]);
	  return -36;
      }
    if (strcmp (results[11], "Canal Creek") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: sheet2() bad result place2: %s.\n",
		   results[11]);
	  return -37;
      }
    sqlite3_free_table (results);

    asprintf (&sql_statement,
	      "select row_no, place, lat, lon, rowid from sheet2 WHERE row_no = 16");
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -38;
      }
    if ((rows != 1) || (columns != 5))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -39;
      }
    if (strcmp (results[0], "row_no") != 0)
      {
	  fprintf (stderr, "Unexpected error: sheet2() bad result: %s.\n",
		   results[0]);
	  return -40;
      }
    if (strcmp (results[6], "Canal Creek") != 0)
      {
	  fprintf (stderr, "Unexpected error: sheet2() bad result place: %s.\n",
		   results[6]);
	  return -41;
      }
    if (strncmp (results[7], "-27.86667", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: sheet2() bad result lat: %s.\n",
		   results[7]);
	  return -42;
      }
    if (strncmp (results[8], "151.51667", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: sheet2() bad result lon: %s.\n",
		   results[8]);
	  return -43;
      }
    sqlite3_free_table (results);

    ret = sqlite3_exec (db_handle, "DROP TABLE sheet2;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -44;
      }

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE noquote USING VirtualXL(testcase1.xls);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualXL error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -45;
      }
    ret = sqlite3_exec (db_handle, "DROP TABLE noquote;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -46;
      }
    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE noheader USING VirtualXL(\"testcase1.xls\", 0, 0);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualXL error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -47;
      }
    ret =
	sqlite3_exec (db_handle, "DROP TABLE noheader;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -48;
      }
    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE nofile USING VirtualXL();", NULL,
		      NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "VirtualXL unexpected result: %i\n", ret);
	  return -49;
      }
    sqlite3_free (err_msg);

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
#endif /* end FreeXL conditional */

    spatialite_shutdown ();
    return 0;
}
