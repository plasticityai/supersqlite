/*

 check_add_tile_triggers.c - Test case for GeoPackage Extensions

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
    void *cache = spatialite_alloc_connection ();
    char *old_SPATIALITE_SECURITY_ENV = NULL;
#ifdef _WIN32
    char *env;
#endif /* not WIN32 */

    old_SPATIALITE_SECURITY_ENV = getenv ("SPATIALITE_SECURITY");
#ifdef _WIN32
    putenv ("SPATIALITE_SECURITY=relaxed");
#else /* not WIN32 */
    setenv ("SPATIALITE_SECURITY", "relaxed", 1);
#endif

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    /* For debugging / testing if required */
    /*
       ret = sqlite3_open_v2 ("check_add_tile_triggers.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
     */
    spatialite_init_ex (db_handle, cache, 0);

    if (old_SPATIALITE_SECURITY_ENV)
      {
#ifdef _WIN32
	  env =
	      sqlite3_mprintf ("SPATIALITE_SECURITY=%s",
			       old_SPATIALITE_SECURITY_ENV);
	  putenv (env);
	  sqlite3_free (env);
#else /* not WIN32 */
	  setenv ("SPATIALITE_SECURITY", old_SPATIALITE_SECURITY_ENV, 1);
#endif
      }
    else
      {
#ifdef _WIN32
	  putenv ("SPATIALITE_SECURITY=");
#else /* not WIN32 */
	  unsetenv ("SPATIALITE_SECURITY");
#endif
      }
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
	  return -2;
      }

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"test1_matrix_tiles\", 4326, -180.0, -90.0, 180.0, 90.0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateTilesTable() result: %i, (%s)\n", ret,
		   err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }

    /* test the trigger works - several different cases here */
    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO test1_matrix_tiles VALUES ( 0, 0, 0, 0, BlobFromFile('empty.png'))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected INSERT INTO 4 result: %i (%s)\n", ret,
		   err_msg);
	  return -4;
      }
    if (strcmp
	(err_msg,
	 "insert on table 'test1_matrix_tiles' violates constraint: zoom_level not specified for table in gpkg_tile_matrix")
	!= 0)
      {
	  fprintf (stderr, "unexpected INSERT INTO error message 5: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }
    sqlite3_free (err_msg);

    /* create matrix level 0 and 4 */
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
	  return -6;
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
	  return -7;
      }

    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO test1_matrix_tiles VALUES ( 0, 0, 1, 0, BlobFromFile('empty.png'))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected INSERT INTO 8 result: %i\n", ret);
	  return -8;
      }
    if (strcmp
	(err_msg,
	 "insert on table 'test1_matrix_tiles' violates constraint: tile_column must be < matrix_width specified for table and zoom level in gpkg_tile_matrix")
	!= 0)
      {
	  fprintf (stderr, "unexpected INSERT INTO error message 9: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO test1_matrix_tiles (zoom_level, tile_column, tile_row, tile_data) VALUES ( 0, 0, 1, BlobFromFile('empty.png'))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected INSERT INTO 10 result: %i\n", ret);
	  return -10;
      }
    if (strcmp
	(err_msg,
	 "insert on table 'test1_matrix_tiles' violates constraint: tile_row must be < matrix_height specified for table and zoom level in gpkg_tile_matrix")
	!= 0)
      {
	  fprintf (stderr, "unexpected INSERT INTO error message 11: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO test1_matrix_tiles (zoom_level, tile_column, tile_row, tile_data) VALUES ( 4, 16, 3, BlobFromFile('empty.png'))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected INSERT INTO 12 result: %i\n", ret);
	  return -12;
      }
    if (strcmp
	(err_msg,
	 "insert on table 'test1_matrix_tiles' violates constraint: tile_column must be < matrix_width specified for table and zoom level in gpkg_tile_matrix")
	!= 0)
      {
	  fprintf (stderr, "unexpected INSERT INTO error message 13: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO test1_matrix_tiles (zoom_level, tile_column, tile_row, tile_data) VALUES ( 4, 6, 16, BlobFromFile('empty.png'))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected INSERT INTO 14 result: %i\n", ret);
	  return -14;
      }
    if (strcmp
	(err_msg,
	 "insert on table 'test1_matrix_tiles' violates constraint: tile_row must be < matrix_height specified for table and zoom level in gpkg_tile_matrix")
	!= 0)
      {
	  fprintf (stderr, "unexpected INSERT INTO error message 15: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -15;
      }
    sqlite3_free (err_msg);

    /* Check a proper INSERT */
    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO test1_matrix_tiles (zoom_level, tile_column, tile_row, tile_data) VALUES (0, 0 ,0, BlobFromFile('empty.png'))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT error 21: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }
    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO test1_matrix_tiles (zoom_level, tile_column, tile_row, tile_data) VALUES (4, 15 , 15, BlobFromFile('empty.png'))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT error 22: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }

    /* check negative row / column numbers */
    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO test1_matrix_tiles (zoom_level, tile_column, tile_row, tile_data) VALUES ( 4, -1, 15, BlobFromFile('empty.png'))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected INSERT INTO 23 result: %i\n", ret);
	  return -23;
      }
    if (strcmp
	(err_msg,
	 "insert on table 'test1_matrix_tiles' violates constraint: tile_column cannot be < 0")
	!= 0)
      {
	  fprintf (stderr, "unexpected INSERT INTO error message 24: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }
    sqlite3_free (err_msg);
    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO test1_matrix_tiles (zoom_level, tile_column, tile_row, tile_data) VALUES ( 4, 15, -1, BlobFromFile('empty.png'))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected INSERT INTO 25 result: %i\n", ret);
	  return -25;
      }
    if (strcmp
	(err_msg,
	 "insert on table 'test1_matrix_tiles' violates constraint: tile_row cannot be < 0")
	!= 0)
      {
	  fprintf (stderr, "unexpected INSERT INTO error message 26: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -26;
      }
    sqlite3_free (err_msg);

    /* try some update checks */
    ret =
	sqlite3_exec (db_handle,
		      "UPDATE test1_matrix_tiles SET zoom_level = 1 WHERE id = 1",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected UPDATE 27 result: %i\n", ret);
	  return -27;
      }
    if (strcmp
	(err_msg,
	 "update on table 'test1_matrix_tiles' violates constraint: zoom_level not specified for table in gpkg_tile_matrix")
	!= 0)
      {
	  fprintf (stderr, "unexpected UPDATE error message 28: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -28;
      }
    sqlite3_free (err_msg);


    ret =
	sqlite3_exec (db_handle,
		      "UPDATE test1_matrix_tiles SET zoom_level = -1 WHERE id = 1",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected UPDATE 29 result: %i\n", ret);
	  return -29;
      }
    if (strcmp
	(err_msg,
	 "update on table 'test1_matrix_tiles' violates constraint: zoom_level not specified for table in gpkg_tile_matrix")
	!= 0)
      {
	  fprintf (stderr, "unexpected UPDATE error message 30: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "UPDATE test1_matrix_tiles SET tile_column = -1 WHERE id = 1",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected UPDATE 31 result: %i\n", ret);
	  return -31;
      }
    if (strcmp
	(err_msg,
	 "update on table 'test1_matrix_tiles' violates constraint: tile_column cannot be < 0")
	!= 0)
      {
	  fprintf (stderr, "unexpected UPDATE error message 32: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -32;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "UPDATE test1_matrix_tiles SET tile_row = -1 WHERE id = 1",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected UPDATE 33 result: %i\n", ret);
	  return -33;
      }
    if (strcmp
	(err_msg,
	 "update on table 'test1_matrix_tiles' violates constraint: tile_row cannot be < 0")
	!= 0)
      {
	  fprintf (stderr, "unexpected UPDATE error message 34: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -34;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "UPDATE test1_matrix_tiles SET tile_row = 1 WHERE id = 1",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected UPDATE 35 result: %i\n", ret);
	  return -35;
      }
    if (strcmp
	(err_msg,
	 "update on table 'test1_matrix_tiles' violates constraint: tile_row must be < matrix_height specified for table and zoom level in gpkg_tile_matrix")
	!= 0)
      {
	  fprintf (stderr, "unexpected UPDATE error message 36: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -36;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "UPDATE test1_matrix_tiles SET tile_column = 1 WHERE id = 1",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_CONSTRAINT)
      {
	  fprintf (stderr, "unexpected UPDATE 37 result: %i\n", ret);
	  return -37;
      }
    if (strcmp
	(err_msg,
	 "update on table 'test1_matrix_tiles' violates constraint: tile_column must be < matrix_width specified for table and zoom level in gpkg_tile_matrix")
	!= 0)
      {
	  fprintf (stderr, "unexpected UPDATE error message 38: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -38;
      }
    sqlite3_free (err_msg);

    /* test trigger setup, bad table type */
    ret =
	sqlite3_exec (db_handle, "SELECT gpkgAddTileTriggers(0)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "unexpected SELECT gpkgAddTileTriggers(0) result: %i\n",
		   ret);
	  return -100;
      }
    if (strcmp
	(err_msg,
	 "gpkgAddTileTriggers() error: argument 1 [table] is not of the String type")
	!= 0)
      {
	  fprintf (stderr,
		   "unexpected SELECT gpkgAddTileTriggers(0) error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -101;
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
