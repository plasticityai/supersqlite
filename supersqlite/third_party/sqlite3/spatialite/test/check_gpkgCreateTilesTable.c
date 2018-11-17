/*

 check_gpkgCreateTilesTable.c - Test case for GeoPackage Extensions

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

The Original Code is GeoPackage extensions

The Initial Developer of the Original Code is Brad Hards
 
Portions created by the Initial Developer are Copyright (C) 2012
the Initial Developer. All Rights Reserved.

Contributor(s):


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
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"
#include "spatialite.h"

#include "test_helpers.h"

int
main (int argc UNUSED, char *argv[]UNUSED)
{
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
    char **results;
    int rows;
    int columns;
    void *cache = spatialite_alloc_connection ();

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    /* For debugging / testing if required */
    /*
       ret = sqlite3_open_v2 ("check_gpkgCreateTilesTable.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
     */
    spatialite_init_ex (db_handle, cache, 0);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  db_handle = NULL;
	  return -1;
      }

    ret =
	sqlite3_exec (db_handle, "SELECT gpkgCreateBaseTables()", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateBaseTables() result: %i, (%s)\n", ret,
		   err_msg);
	  sqlite3_free (err_msg);
	  return -100;
      }

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"testtiles1\", 4326, -180.0, -90.0, 180.0, 90.0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateTilesTable() float bounds result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -101;
      }

    /* same, but using integer bounds */
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"testtiles2\", 4326, -180, -90, 180, 90)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateTilesTable() integer bounds result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -102;
      }

    /* check gpkg_contents table entry is OK */
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT data_type, identifier, description, last_change, min_x, min_y, max_x, max_y, srs_id FROM gpkg_contents WHERE table_name = 'testtiles1'",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error1: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -104;
      }
    if ((rows != 1) || (columns != 9))
      {
	  sqlite3_free_table (results);
	  fprintf (stderr, "Unexpected row / column count: %i x %i\n", rows,
		   columns);
	  return -105;
      }
    if (strcmp (results[1 * columns + 0], "tiles") != 0)
      {
	  fprintf (stderr,
		   "Unexpected data_type result (got %s, expected tiles)",
		   results[1 * columns + 0]);
	  sqlite3_free_table (results);
	  return -106;
      }
    if (results[1 * columns + 1] != NULL)
      {
	  fprintf (stderr,
		   "Unexpected identifier result (got %s, expected NULL)",
		   results[1 * columns + 1]);
	  sqlite3_free_table (results);
	  return -107;
      }
    if (strcmp (results[1 * columns + 2], "") != 0)
      {
	  fprintf (stderr,
		   "Unexpected description result (got %s, expected an empty string)",
		   results[1 * columns + 2]);
	  sqlite3_free_table (results);
	  return -108;
      }
    /* TODO: this test should be more rigorous */
    if (results[1 * columns + 3] == NULL)
      {
	  fprintf (stderr, "Unexpected last_change result - null)");
	  sqlite3_free_table (results);
	  return -109;
      }
    if ((results[1 * columns + 4] == NULL)
	|| (strcmp (results[1 * columns + 4], "-180.0") != 0))
      {
	  fprintf (stderr, "Unexpected min_x result (got %s, expected -180.0)",
		   results[1 * columns + 4]);
	  sqlite3_free_table (results);
	  return -110;
      }
    if ((results[1 * columns + 5] == NULL)
	|| (strcmp (results[1 * columns + 5], "-90.0") != 0))
      {
	  fprintf (stderr, "Unexpected min_y result (got %s, expected -90.0)",
		   results[1 * columns + 5]);
	  sqlite3_free_table (results);
	  return -111;
      }
    if ((results[1 * columns + 6] == NULL)
	|| (strcmp (results[1 * columns + 6], "180.0") != 0))
      {
	  fprintf (stderr, "Unexpected max_x result (got %s, expected 180.0)",
		   results[1 * columns + 6]);
	  sqlite3_free_table (results);
	  return -112;
      }
    if ((results[1 * columns + 7] == NULL)
	|| (strcmp (results[1 * columns + 7], "90.0") != 0))
      {
	  fprintf (stderr, "Unexpected max_y result (got %s, expected 90.0)",
		   results[1 * columns + 7]);
	  sqlite3_free_table (results);
	  return -113;
      }
    if ((results[1 * columns + 8] == NULL)
	|| (strcmp (results[1 * columns + 8], "4326") != 0))
      {
	  fprintf (stderr, "Unexpected srid result (got %s, expected 4326)",
		   results[1 * columns + 8]);
	  sqlite3_free_table (results);
	  return -114;
      }
    sqlite3_free_table (results);

    /* check gpkg_tile_matrix_set table entry is OK */
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT min_x, min_y, max_x, max_y, srs_id FROM gpkg_tile_matrix_set WHERE table_name = 'testtiles1'",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error1: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -140;
      }
    if ((rows != 1) || (columns != 5))
      {
	  sqlite3_free_table (results);
	  fprintf (stderr, "Unexpected row / column count: %i x %i\n", rows,
		   columns);
	  return -141;
      }
    if ((results[1 * columns + 0] == NULL)
	|| (strcmp (results[1 * columns + 0], "-180.0") != 0))
      {
	  fprintf (stderr, "Unexpected min_x result (got %s, expected -180.0)",
		   results[1 * columns + 0]);
	  sqlite3_free_table (results);
	  return -142;
      }
    if ((results[1 * columns + 1] == NULL)
	|| (strcmp (results[1 * columns + 1], "-90.0") != 0))
      {
	  fprintf (stderr, "Unexpected min_y result (got %s, expected -90.0)",
		   results[1 * columns + 1]);
	  sqlite3_free_table (results);
	  return -143;
      }
    if ((results[1 * columns + 2] == NULL)
	|| (strcmp (results[1 * columns + 2], "180.0") != 0))
      {
	  fprintf (stderr, "Unexpected max_x result (got %s, expected 180.0)",
		   results[1 * columns + 2]);
	  sqlite3_free_table (results);
	  return -144;
      }
    if ((results[1 * columns + 3] == NULL)
	|| (strcmp (results[1 * columns + 3], "90.0") != 0))
      {
	  fprintf (stderr, "Unexpected max_y result (got %s, expected 90.0)",
		   results[1 * columns + 3]);
	  sqlite3_free_table (results);
	  return -145;
      }
    if ((results[1 * columns + 4] == NULL)
	|| (strcmp (results[1 * columns + 4], "4326") != 0))
      {
	  fprintf (stderr, "Unexpected srid result (got %s, expected 4326)",
		   results[1 * columns + 4]);
	  sqlite3_free_table (results);
	  return -146;
      }
    sqlite3_free_table (results);

    sqlite3_free (err_msg);

    /*  try some bad arguments */
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(3, 4326, -180, -90, 180, 90)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add tiles table, non-string table name, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -150;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesTable() error: argument 1 [table] is not of the String type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgCreateTilesTable arg 1 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -151;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"test2\", \"srid\", -180, -90, 180, 90)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add tiles table, non-integer SRID value, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -152;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesTable() error: argument 2 [srid] is not of the integer type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgCreateTilesTable arg 2 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -153;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"test2\", 0, \"minx\", -90, 180, 90)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add tiles table, non-numeric min_x, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -154;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesTable() error: argument 3 [min_x] is not a numeric type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgCreateTilesTable arg 3 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -155;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"test2\", 0, -180, \"min_y\", 180, 90)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add tiles table, non-numeric min_y, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -156;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesTable() error: argument 4 [min_y] is not a numeric type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgCreateTilesTable arg 4 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -157;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"test2\", 0, -180, -90, \"max_x\", 90)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add tiles table, non-numeric max_x, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -158;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesTable() error: argument 5 [max_x] is not a numeric type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgCreateTilesTable arg 5 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -159;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"test2\", 0, -180, -90, 180, \"max_y\")",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add tiles table, non-numeric max_y, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -160;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesTable() error: argument 6 [max_y] is not a numeric type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgCreateTilesTable arg 6 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -161;
      }
    sqlite3_free (err_msg);

    /* try duplicate entry */
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"testtiles2\", 0, -180, -85, 180, 85)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add tiles table, duplicate table, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -162;
      }
    sqlite3_free (err_msg);

    /* try duplicate entry */
    ret =
	sqlite3_exec (db_handle,
		      "CREATE TABLE alreadythere (id INTEGER PRIMARY KEY AUTOINCREMENT)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected error for add tiles table, existing table setup %i (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -164;
      }
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"alreadythere\", 0, -180, -85, 180, 85)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add tiles table, duplicate table manual, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -165;
      }
    if (strcmp (err_msg, "table alreadythere already exists") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgCreateTilesTable dupe manual table: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -163;
      }
    sqlite3_free (err_msg);

    ret = sqlite3_close (db_handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (db_handle));
	  return -200;
      }

    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
