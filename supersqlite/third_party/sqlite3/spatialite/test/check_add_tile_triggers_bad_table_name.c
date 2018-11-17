/*

 check_add_tile_triggers_bad_table_name.c - Test case for GeoPackage Extensions

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

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    /* For debugging / testing if required */
    /*
       ret = sqlite3_open_v2 ("check_add_tile_triggers_bad_table_name.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
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

    /* create a minimal raster_format_metadata table (not spec compliant) */
    ret =
	sqlite3_exec (db_handle, "DROP TABLE IF EXISTS raster_format_metadata",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP raster_format_metadata error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }
    ret =
	sqlite3_exec (db_handle,
		      "CREATE TABLE raster_format_metadata (r_table_name TEXT NOT NULL, r_raster_column TEXT NOT NULL, mime_type TEXT NOT NULL DEFAULT 'image/jpeg', bit_depth INTEGER NOT NULL DEFAULT 24)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE raster_format_metadata error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }

    /* add in a test entry */
    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO raster_format_metadata VALUES (\"mytable_tiles\", \"tile_data\", \"image/png\", 24)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT raster_format_metadata error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }

    /* create a minimal tile_matrix_metadata table (not spec compliant) */
    ret =
	sqlite3_exec (db_handle, "DROP TABLE IF EXISTS tile_matrix_metadata",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP tile_matrix_metadata error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }
    ret =
	sqlite3_exec (db_handle,
		      "CREATE TABLE tile_matrix_metadata (t_table_name TEXT NOT NULL, zoom_level INTEGER NOT NULL, matrix_width INTEGER NOT NULL, matrix_height INTEGER NOT NULL)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE tile_matrix_metadata error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -8;
      }
    /* add in a test entry */
    ret =
	sqlite3_exec (db_handle,
		      "INSERT INTO tile_matrix_metadata VALUES (\"mytable_tiles\", 0, 1, 1)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT tile_matrix_metadata error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }

    /* create a target table */
    ret =
	sqlite3_exec (db_handle, "DROP TABLE IF EXISTS mytable_tiles", NULL,
		      NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP mytable_tiles error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }
    ret =
	sqlite3_exec (db_handle,
		      "CREATE TABLE mytable_tiles (id INTEGER PRIMARY KEY AUTOINCREMENT, zoom_level INTEGER NOT NULL DEFAULT 0, tile_column INTEGER NOT NULL DEFAULT 0, tile_row INTEGER NOT NULL DEFAULT 0, tile_data BLOB NOT NULL DEFAULT (zeroblob(4)))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE mytable_tiles error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

    /* test trigger setup */
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddTileTriggers(\"no_such_table\")", NULL,
		      NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "Unexpected gpkgAddTileTriggers result: %i\n", ret);
	  return -12;
      }
    if (strcmp (err_msg, "no such table: main.no_such_table") != 0)
      {
	  fprintf (stderr,
		   "SELECT gpkgAddTileTriggers unexpected error message: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -13;
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
