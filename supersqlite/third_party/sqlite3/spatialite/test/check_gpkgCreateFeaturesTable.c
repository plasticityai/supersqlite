/*

 check_gkpgCreateFeaturesTable.c - Test case for GeoPackage Extensions

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
 
Portions created by the Initial Developer are Copyright (C) 2012-2013
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
    void *cache = spatialite_alloc_connection ();

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    /* For debugging / testing if required */
    /*
       ret = sqlite3_open_v2 ("check_gkpgCreateFeaturesTable.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
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
		      "CREATE TABLE \"testfeats1\" (id INTEGER PRIMARY KEY AUTOINCREMENT, text_attribute TEXT, real_attribute REAL, boolean_attribute BOOLEAN, raster_or_photo BLOB)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected create testfeats1 table result: %i, (%s)\n", ret,
		   err_msg);
	  sqlite3_free (err_msg);
	  return -101;
      }

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(\"testfeats1\", \"thegeom\", \"POINT\", 0, 0, 4326)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgAddGeometryColumn() result: %i, (%s)\n", ret,
		   err_msg);
	  sqlite3_free (err_msg);
	  return -102;
      }

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(\"testfeats1\", \"thegeom\", \"POINT\", 0, 0, 4326)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add geometry column duplicate column name, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -103;
      }

#if 0
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
	  fprintf (stderr, "Unexpected last_change result - null)",
		   results[1 * columns + 3]);
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
#endif
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "CREATE TABLE \"ctd\" (id INTEGER PRIMARY KEY AUTOINCREMENT, text_attribute TEXT, real_attribute REAL, boolean_attribute BOOLEAN, raster_or_photo BLOB)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Unexpected create ctd table result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -200;
      }
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(\"ctd\", 2.4, \"POINT\", 0, 0, 4326)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add geometry column bad column type, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -201;
      }
    if (strcmp
	(err_msg,
	 "gpkgAddGeometryColumn() error: argument 2 [geometry_column_name] is not of the string type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 2: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -202;
      }
    sqlite3_free (err_msg);


    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(\"ctd\", \"the_geom\", 4, 0, 0, 4326)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add geometry column bad geom type, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -203;
      }
    if (strcmp
	(err_msg,
	 "gpkgAddGeometryColumn() error: argument 3 [geometry_type] is not of the string type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 3: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -204;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(8.3, \"the_geom\", \"POINT\", 0, 0, 4326)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add geometry column bad table type, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -205;
      }
    if (strcmp
	(err_msg,
	 "gpkgAddGeometryColumn() error: argument 1 [table] is not of the string type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 1: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -206;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(\"ctd\", \"the_geom\", \"POINT\", \"z\", 0, 4326)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add geometry column bad z type, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -207;
      }
    if (strcmp
	(err_msg,
	 "gpkgAddGeometryColumn() error: argument 4 [with_z] is not of the integer type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 4: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -208;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(\"ctd\", \"the_geom\", \"POINT\", 0, \"m\", 4326)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add geometry column bad m type, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -209;
      }
    if (strcmp
	(err_msg,
	 "gpkgAddGeometryColumn() error: argument 5 [with_m] is not of the integer type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 5: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -210;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(\"ctd\", \"the_geom\", \"POINT\", 3, 0, 4326)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add geometry column bad z value, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -211;
      }
    if (strcmp
	(err_msg,
	 "gpkgAddGeometryColumn() error: argument 4 [with_z] is not a known value (expected 0, 1 or 2)")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 4: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -212;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(\"ctd\", \"the_geom\", \"POINT\", 0, 3, 4326)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add geometry column bad m value, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -213;
      }
    if (strcmp
	(err_msg,
	 "gpkgAddGeometryColumn() error: argument 5 [with_m] is not a known value (expected 0, 1 or 2)")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 5: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -214;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(\"ctd\", \"the_geom\", \"POINT\", 0, 2, \"srid\")",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add geometry column bad srid type, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -215;
      }
    if (strcmp
	(err_msg,
	 "gpkgAddGeometryColumn() error: argument 6 [srid] is not of the integer type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 6: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -216;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgAddGeometryColumn(\"ctd\", \"the_geom\", \"blah\", 0, 1, 4326)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add geometry column bad geom type value, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -217;
      }
    if (strcmp
	(err_msg,
	 "gpkgAddGeometryColumn() error: argument 3 [geometry_type] not a recognised geometry type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 3: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -218;
      }
    sqlite3_free (err_msg);

    ret = sqlite3_close (db_handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (db_handle));
	  return -300;
      }

    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
