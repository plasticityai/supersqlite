/*

 check_get_normal_zoom.c - Test case for GeoPackage Extensions

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
 
Portions created by the Initial Developer are Copyright (C) 2011
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

#include <sqlite3.h>
#include <spatialite.h>

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
       ret = sqlite3_open_v2 ("check_get_normal_zoom.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
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

    /* create tables to support future testing */
    ret =
	sqlite3_exec (db_handle, "SELECT gpkgCreateBaseTables()", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateBaseTables() result: %i, (%s)\n", ret,
		   err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"test1_matrix_tiles\", 4326, -180.0, -90.0, 180.0, 90.0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateTilesTable() float bounds result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }

    /* create matrix levels 0, 1 and 4 */
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(\"test1_matrix_tiles\", 0, 360, 180)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateTilesZoomLevel(0) result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(\"test1_matrix_tiles\", 1, 360, 180)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateTilesZoomLevel(1) result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(\"test1_matrix_tiles\", 4, 360, 180)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateTilesZoomLevel(4) result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT gpkgGetNormalZoom(\"test1_matrix_tiles\", 0)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error1: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected row / column count: %i x %i\n", rows,
		   columns);
	  sqlite3_free_table (results);
	  return -10;
      }
    if (strcmp (results[1 * columns + 0], "4") != 0)
      {
	  fprintf (stderr, "Unexpected result 0 (got %s, expected 4)",
		   results[rows * columns + 0]);
	  sqlite3_free_table (results);
	  return -11;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT gpkgGetNormalZoom(\"test1_matrix_tiles\", 1)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error 2: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected row / column count: %i x %i\n", rows,
		   columns);
	  return -13;
      }
    if (strcmp (results[1 * columns + 0], "3") != 0)
      {
	  fprintf (stderr, "Unexpected result 1 (got %s, expected 3)",
		   results[rows * columns + 0]);
	  return -14;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT gpkgGetNormalZoom(\"test1_matrix_tiles\", 4)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error 3: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -15;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected row / column count: %i x %i\n", rows,
		   columns);
	  sqlite3_free_table (results);
	  return -16;
      }
    if (strcmp (results[1 * columns + 0], "0") != 0)
      {
	  fprintf (stderr, "Unexpected result 4(got %s, expected 0)",
		   results[rows * columns + 0]);
	  sqlite3_free_table (results);
	  return -17;
      }
    sqlite3_free_table (results);

    /* test an out-of-range zoom number - expect exception */
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT gpkgGetNormalZoom(\"test1_matrix_tiles\", 5)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "Expected error for overrange zoom level, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -18;
      }
    if (strcmp
	(err_msg,
	 "gpkgGetNormalZoom: input zoom level number outside of valid zoom levels")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for overrange zoom level: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -19;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT gpkgGetNormalZoom(\"test1_matrix_tiles\", -1)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "Expected error for underrange zoom level, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -20;
      }
    if (strcmp
	(err_msg,
	 "gpkgGetNormalZoom: input zoom level number outside of valid zoom levels")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for underrange zoom level: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }
    sqlite3_free (err_msg);

    /* test bad table name */
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT gpkgGetNormalZoom(\"no_such_table\", 0)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "Expected error for bad table name level, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -22;
      }
    if (strcmp
	(err_msg,
	 "gpkgGetNormalZoom: tile table not found in gpkg_tile_matrix") != 0)
      {
	  fprintf (stderr, "Unexpected error message for bad table name: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }
    sqlite3_free (err_msg);

    /* test bad argument types */
    ret =
	sqlite3_get_table (db_handle, "SELECT gpkgGetNormalZoom(3, 0)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "Expected error for bad arg1 type, got %i\n", ret);
	  sqlite3_free (err_msg);
	  return -26;
      }
    if (strcmp
	(err_msg,
	 "gpkgGetNormalZoom() error: argument 1 [tile_table_name] is not of the String type")
	!= 0)
      {
	  fprintf (stderr, "Unexpected error message for bad arg1 type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -27;
      }
    sqlite3_free (err_msg);

    /* test bad argument types */
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT gpkgGetNormalZoom(\"test1_matrix_tiles\", 0.2)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "Expected error for bad arg2 type, got %i\n", ret);
	  sqlite3_free (err_msg);
	  return -28;
      }
    if (strcmp
	(err_msg,
	 "gpkgGetNormalZoom() error: argument 2 [inverted zoom level] is not of the integer type")
	!= 0)
      {
	  fprintf (stderr, "Unexpected error message for bad arg2 type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }
    sqlite3_free (err_msg);

    ret = sqlite3_close (db_handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (db_handle));
	  return -100;
      }

    spatialite_cleanup_ex (cache);

    return 0;
}
