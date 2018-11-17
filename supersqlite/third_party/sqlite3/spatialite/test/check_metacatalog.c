/*

 check_metacatalog.c -- SpatiaLite Test Case

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
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    char **results;
    int rows;
    int columns;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

/* testing current style metadata layout >= v.4.0.0 */
    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory database: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }

    spatialite_init_ex (handle, cache, 0);

    ret =
	sqlite3_exec (handle, "SELECT InitSpatialMetadata()", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "InitSpatialMetadata() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -2;
      }

    ret =
	sqlite3_exec (handle,
		      "CREATE TABLE abc (def VARCHAR(10), ghi TEXT NOT NULL)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -3;
      }
    ret =
	sqlite3_exec (handle,
		      "INSERT INTO abc (def, ghi) VALUES (NULL, 'alpha')", NULL,
		      NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT-1 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -4;
      }
    ret =
	sqlite3_exec (handle,
		      "INSERT INTO abc (def, ghi) VALUES (NULL, 'gamma')", NULL,
		      NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT-2 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -4;
      }
    ret =
	sqlite3_exec (handle,
		      "INSERT INTO abc (def, ghi) VALUES ('one', 'beta')", NULL,
		      NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT-3 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -5;
      }
    ret =
	sqlite3_exec (handle,
		      "INSERT INTO abc (def, ghi) VALUES ('two', 'gamma')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT-4 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -6;
      }
    ret =
	sqlite3_exec (handle,
		      "INSERT INTO abc (def, ghi) VALUES ('one', 'alpha')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT-5 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -7;
      }

    ret =
	sqlite3_exec (handle, "SELECT CreateMetaCatalogTables(1)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CreateMetaCatalogTables error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -8;
      }

    rows = 0;
    columns = 0;
    ret =
	sqlite3_get_table (handle,
			   "SELECT table_name, column_name, type, not_null "
			   "FROM splite_metacatalog "
			   "WHERE table_name = 'abc'", &results, &rows,
			   &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SELECT splite_metacatalog: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }
    if ((rows != 2) || (columns != 4))
      {
	  fprintf (stderr,
		   "Unexpected error: SELECT splite_metacatalog result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -10;
      }
    if (strcmp (results[4], "abc") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[4]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -11;
      }
    if (strcmp (results[5], "def") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[5]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -12;
      }
    if (strcmp (results[6], "VARCHAR(10)") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[6]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -13;
      }
    if (strcmp (results[7], "0") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[7]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -14;
      }
    if (strcmp (results[8], "abc") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[8]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -15;
      }
    if (strcmp (results[9], "ghi") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[9]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -14;
      }
    if (strcmp (results[10], "TEXT") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[10]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -13;
      }
    if (strcmp (results[11], "1") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[11]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -14;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (handle,
		      "SELECT UpdateMetaCatalogStatistics(1, 'abc', 'def')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UpdateMetaCatalogStatistics error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -15;
      }

    rows = 0;
    columns = 0;
    ret =
	sqlite3_get_table (handle,
			   "SELECT value, count FROM splite_metacatalog_statistics",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Error SELECT splite_metacatalog_statistics(1): %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -16;
      }
    if ((rows != 3) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: SELECT splite_metacatalog_statistics result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -17;
      }
    if (results[2] != NULL)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[2]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -18;
      }
    if (strcmp (results[3], "2") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[3]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -19;
      }
    if (strcmp (results[4], "one") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[4]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -20;
      }
    if (strcmp (results[5], "2") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[5]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -21;
      }
    if (strcmp (results[6], "two") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[6]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -22;
      }
    if (strcmp (results[7], "1") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[7]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -23;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (handle,
		      "SELECT UpdateMetaCatalogStatistics(1, 'abc_bad_table', 'def')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "UpdateMetaCatalogStatistics (bad table) error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -24;
      }
    ret =
	sqlite3_exec (handle,
		      "SELECT UpdateMetaCatalogStatistics(1, 'abc', 'def_bad_column')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "UpdateMetaCatalogStatistics (bad column) error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -25;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO abc (def, ghi) VALUES ('two', 'alpha')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT-6 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -26;
      }
    ret =
	sqlite3_exec (handle, "DELETE FROM abc WHERE def IS NULL", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DELETE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -27;
      }

    ret =
	sqlite3_exec (handle,
		      "CREATE TABLE my_master (tbl TEXT NOT NULL, col TEXT NOT NULL)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -28;
      }
    ret =
	sqlite3_exec (handle,
		      "INSERT INTO my_master (tbl, col) VALUES ('abc', 'def')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT-master-1 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -29;
      }
    ret =
	sqlite3_exec (handle,
		      "INSERT INTO my_master (tbl, col) VALUES ('abc', 'ghi')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT-master-2 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -30;
      }
    ret =
	sqlite3_exec (handle,
		      "INSERT INTO my_master (tbl, col) VALUES ('abc_bad', 'def')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT-master-3 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -31;
      }
    ret =
	sqlite3_exec (handle,
		      "INSERT INTO my_master (tbl, col) VALUES ('abc', 'ghi_bad')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT-master-4 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -32;
      }

    ret =
	sqlite3_exec (handle,
		      "SELECT UpdateMetaCatalogStatistics(1, 'my_master', 'tbl', 'col')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UpdateMetaCatalogStatistics error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -33;
      }

    rows = 0;
    columns = 0;
    ret =
	sqlite3_get_table (handle,
			   "SELECT column_name, value, count FROM splite_metacatalog_statistics",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Error SELECT splite_metacatalog_statistics(2): %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -34;
      }
    if ((rows != 5) || (columns != 3))
      {
	  fprintf (stderr,
		   "Unexpected error: SELECT splite_metacatalog_statistics result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -35;
      }
    if (strcmp (results[3], "def") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[3]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -36;
      }
    if (strcmp (results[4], "one") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[4]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -37;
      }
    if (strcmp (results[5], "2") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[5]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -38;
      }
    if (strcmp (results[6], "def") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[6]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -39;
      }
    if (strcmp (results[7], "two") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[7]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -40;
      }
    if (strcmp (results[8], "2") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[8]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -41;
      }
    if (strcmp (results[9], "ghi") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[9]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -42;
      }
    if (strcmp (results[10], "alpha") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[10]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -43;
      }
    if (strcmp (results[11], "2") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[11]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -44;
      }
    if (strcmp (results[12], "ghi") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[12]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -45;
      }
    if (strcmp (results[13], "beta") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[13]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -46;
      }
    if (strcmp (results[14], "1") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[14]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -47;
      }
    if (strcmp (results[15], "ghi") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[15]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -48;
      }
    if (strcmp (results[16], "gamma") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[16]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -49;
      }
    if (strcmp (results[17], "1") != 0)
      {
	  fprintf (stderr, "unexpected splite_metacatalog: %s\n", results[17]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -50;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (handle,
		      "SELECT UpdateMetaCatalogStatistics(1, 'my_master_bad', 'tbl', 'col')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UpdateMetaCatalogStatistics error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -51;
      }
    ret =
	sqlite3_exec (handle,
		      "SELECT UpdateMetaCatalogStatistics(1, 'my_master', 'tbl_bad', 'col')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UpdateMetaCatalogStatistics error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -52;
      }
    ret =
	sqlite3_exec (handle,
		      "SELECT UpdateMetaCatalogStatistics(1, 'my_master', 'tbl', 'col_bad')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UpdateMetaCatalogStatistics error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -53;
      }

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -101;
      }

    spatialite_cleanup_ex (cache);

    return 0;
}
