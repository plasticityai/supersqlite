/*

 check_virtual_ovflw.c -- SpatiaLite Test Case

 Author: Sandro Furieri <a.furieri@lqt.it>

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
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "sqlite3.h"
#include "spatialite.h"

int
main (int argc, char *argv[])
{
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
    char **results;
    int rows;
    int columns;
    int suffix_len = 16 * 1024;	/* 16 KB suffix */
    char *suffix;
    char *table;
    char *sql;
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
	sqlite3_exec (db_handle, "SELECT InitSpatialMetadata(1)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "InitSpatialMetadata() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -2;
      }

/* setting up very long table and column names */
    suffix = malloc (suffix_len);
    memset (suffix, 'z', suffix_len);
    suffix[suffix_len - 1] = '\0';

#ifndef OMIT_FREEXL		/* only if FreeXL is supported */
    table = sqlite3_mprintf ("xltest_%s", suffix);

    sql =
	sqlite3_mprintf
	("create VIRTUAL TABLE %s USING VirtualXL(\"testcase1.xls\");", table);
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualXL error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }

    sql = sqlite3_mprintf ("select col_2, col_4, col_5, col_7, rowid "
			   "from %s WHERE col_2 = \"Canal Creek\";", table);
    ret =
	sqlite3_get_table (db_handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }
    if ((rows != 2) || (columns != 5))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -5;
      }
    if (strcmp (results[0], "col_2") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -6;
      }
    if (strcmp (results[5], "Canal Creek") != 0)
      {
	  fprintf (stderr, "Unexpected error: name5() bad result: %s.\n",
		   results[5]);
	  return -7;
      }
    if (strncmp (results[6], "-27.86667", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: lat1() bad result: %s.\n",
		   results[6]);
	  return -8;
      }
    if (strncmp (results[7], "151.51667", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: lon2() bad result: %s.\n",
		   results[7]);
	  return -9;
      }
    if (strcmp (results[10], "Canal Creek") != 0)
      {
	  fprintf (stderr, "Unexpected error: name10() bad result: %s.\n",
		   results[10]);
	  return -10;
      }
    sqlite3_free_table (results);

    sqlite3_free (table);
#endif /* end FreeXL conditional */

#ifndef OMIT_ICONV		/* only if ICONV is supported */
    table = sqlite3_mprintf ("shapetest_%s", suffix);

    sql =
	sqlite3_mprintf
	("create VIRTUAL TABLE %s USING VirtualShape(\"shapetest1\", UTF-8, 4326);",
	 table);
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualShape error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }
    sql = sqlite3_mprintf ("SELECT RegisterVirtualGeometry(%Q)", table);
    ret =
	sqlite3_get_table (db_handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "RegisterVirtualGeometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "RegisterVirtualGeometry Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -13;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "RegisterVirtualGeometry Unexpected error: header() bad result: %s.\n",
		   results[1]);
	  return -14;
      }
    sqlite3_free_table (results);

    sql =
	sqlite3_mprintf
	("select testcase1, testcase2, AsText(Geometry) from %s where testcase2 < 20;",
	 table);
    ret =
	sqlite3_get_table (db_handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -15;
      }
    if ((rows != 1) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -16;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -17;
      }
    if (strcmp (results[3], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[3]);
	  return -18;
      }
    if (strcmp (results[4], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[4]);
	  return -19;
      }
    if (strcmp (results[5], "POINT(3480766.311245 4495355.740524)") != 0)
      {
	  fprintf (stderr, "Unexpected error: geometry() bad result: %s.\n",
		   results[5]);
	  return -20;
      }
    sqlite3_free_table (results);

    sql = sqlite3_mprintf ("SELECT DropVirtualGeometry(%Q)", table);
    ret =
	sqlite3_get_table (db_handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DropVirtualGeometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "DropVirtualGeometry Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -22;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "DropVirtualGeometry Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -23;
      }
    sqlite3_free_table (results);

    sql =
	sqlite3_mprintf
	("create VIRTUAL TABLE %s USING VirtualDBF(shapetest1.dbf, UTF-8);",
	 table);
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualDBF error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }

    sql =
	sqlite3_mprintf
	("select testcase1, testcase2 from %s where testcase2 < 20;", table);
    ret =
	sqlite3_get_table (db_handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -26;
      }
    if (strcmp (results[0], "testcase1") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -27;
      }
    if (strcmp (results[2], "windward") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[2]);
	  return -28;
      }
    if (strcmp (results[3], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -29;
      }
    sqlite3_free_table (results);

    sql = sqlite3_mprintf ("DROP TABLE %s;", table);
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }

    sqlite3_free (table);
#endif /* end ICONV conditional */

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);

    ret = system ("cp sql_stmt_tests/testdb1.sqlite testdb1.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot copy testdb1.sqlite database\n");
	  return -131;
      }
    cache = spatialite_alloc_connection ();
    ret =
	sqlite3_open_v2 ("testdb1.sqlite", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open testdb1.sqlite db: %s\n",
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  db_handle = NULL;
	  return -31;
      }

    spatialite_init_ex (db_handle, cache, 0);

    table = sqlite3_mprintf ("roads_net_%s", suffix);

    sql =
	sqlite3_mprintf
	("create VIRTUAL TABLE %s USING VirtualNetwork(\"roads_net_data\");",
	 table);
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualNetwork error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -32;
      }

    sql =
	sqlite3_mprintf
	("SELECT NodeFrom, NodeTo FROM %s WHERE NodeTo = 29 AND NodeFrom = 32;",
	 table);
    ret =
	sqlite3_get_table (db_handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -33;
      }
    if ((rows != 4) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -34;
      }
    if (strcmp (results[0], "NodeFrom") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -35;
      }
    if (strcmp (results[2], "32") != 0)
      {
	  fprintf (stderr, "Unexpected error: windward bad result: %s.\n",
		   results[2]);
	  return -36;
      }
    if (strcmp (results[3], "29") != 0)
      {
	  fprintf (stderr, "Unexpected error: integer() bad result: %s.\n",
		   results[3]);
	  return -37;
      }
    sqlite3_free_table (results);

    sql = sqlite3_mprintf ("DROP TABLE %s;", table);
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -38;
      }

    sqlite3_free (table);

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
    ret = unlink ("testdb1.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot remove testdb1 database\n");
	  return -39;
      }

    free (suffix);

    spatialite_shutdown ();
    return 0;
}
