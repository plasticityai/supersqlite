/*

 check_gpkg_base_core_spatial_ref_sys_data_values_default - Test case for GeoPackage Extensions

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
 
Portions created by the Initial Developer are Copyright (C) 2013
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

#include <stdbool.h>
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
    void *cache = spatialite_alloc_connection ();
    int rows;
    int columns;

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    /* For debugging / testing if required */
    /*
       ret = sqlite3_open_v2 ("check_gpkg_base_core_spatial_ref_sys_data_values_default.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
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
	  return -2;
      }

    /*
       / Test Case ID: /base/core/spatial_ref_sys/data_values_default
       / Test Purpose: Verify that the spatial_ref_sys table contains the required default contents.
       / Test Method:
       /    1) SELECT srid, auth_name, auth_srid, srtext FROM spatial_ref_sys WHERE srid = -1 returns -1 “NONE” -1 “Undefined”, AND
       /    2) SELECT srid, auth_name, auth_srid, srtext FROM spatial_ref_sys WHERE srid = 0 returns 0 “NONE” 0 “Undefined”, AND
       /    3) SELECT srid, auth_name, auth_srid, srtext FROM spatial_ref_sys WHERE srid = 4326 returns 
       /        4326 epsg 4326 GEOGCS["WGS 84", DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563, AUTHORITY["EPSG","7030"]],
       /        AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]], AUTHORITY["EPSG","4326"]]
       /     (whitespace irrelevant)
       /  Pass if tests 1-3 are met
       /  Fail otherwise
       / Reference: Clause 1.1.2.1.2 Req 11:
       / Test Type: Capability
     */

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT srs_id, organization, organization_coordsys_id, definition FROM gpkg_spatial_ref_sys WHERE srs_id = -1",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "get table error, srs_id = -1: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }
    if ((rows != 1) || (columns != 4))
      {
	  fprintf (stderr, "Unexpected row / column count: %i x %i\n", rows,
		   columns);
	  sqlite3_free_table (results);
	  return -10;
      }
    if (strcmp (results[1 * columns + 0], "-1") != 0)
      {
	  fprintf (stderr, "Unexpected srs_id result (got %s, expected -1)",
		   results[1 * columns + 0]);
	  sqlite3_free_table (results);
	  return -11;
      }
    if (strcmp (results[1 * columns + 1], "NONE") != 0)
      {
	  fprintf (stderr,
		   "Unexpected organization result (got %s, expected NONE)",
		   results[1 * columns + 1]);
	  sqlite3_free_table (results);
	  return -12;
      }
    if (strcmp (results[1 * columns + 2], "-1") != 0)
      {
	  fprintf (stderr,
		   "Unexpected organization_coordsys_id result (got %s, expected -1)",
		   results[1 * columns + 2]);
	  sqlite3_free_table (results);
	  return -13;
      }
    if (strcmp (results[1 * columns + 3], "Undefined") != 0)
      {
	  fprintf (stderr,
		   "Unexpected definition result (got %s, expected Undefined)",
		   results[1 * columns + 3]);
	  sqlite3_free_table (results);
	  return -14;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT srs_id, organization, organization_coordsys_id, definition FROM gpkg_spatial_ref_sys WHERE srs_id = 0",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "get table error, srs_id = 0: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -20;
      }
    if ((rows != 1) || (columns != 4))
      {
	  fprintf (stderr, "Unexpected row / column count (srs_id0): %i x %i\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  return -21;
      }
    if (strcmp (results[1 * columns + 0], "0") != 0)
      {
	  fprintf (stderr, "Unexpected srs_id result (got %s, expected 0)",
		   results[1 * columns + 0]);
	  sqlite3_free_table (results);
	  return -22;
      }
    if (strcmp (results[1 * columns + 1], "NONE") != 0)
      {
	  fprintf (stderr,
		   "Unexpected organization result (got %s, expected NONE)",
		   results[1 * columns + 1]);
	  sqlite3_free_table (results);
	  return -23;
      }
    if (strcmp (results[1 * columns + 2], "0") != 0)
      {
	  fprintf (stderr,
		   "Unexpected organization_coordsys_id result (got %s, expected 0)",
		   results[1 * columns + 2]);
	  sqlite3_free_table (results);
	  return -24;
      }
    if (strcmp (results[1 * columns + 3], "Undefined") != 0)
      {
	  fprintf (stderr,
		   "Unexpected definition result (got %s, expected Undefined)",
		   results[1 * columns + 3]);
	  sqlite3_free_table (results);
	  return -25;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (db_handle,
			   "SELECT srs_id, organization, organization_coordsys_id, definition FROM gpkg_spatial_ref_sys WHERE srs_id = 4326",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "get table error, srs_id = 4326: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }
    if ((rows != 1) || (columns != 4))
      {
	  fprintf (stderr,
		   "Unexpected row / column count (srs_id4326): %i x %i\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  return -31;
      }
    if (strcmp (results[1 * columns + 0], "4326") != 0)
      {
	  fprintf (stderr, "Unexpected srs_id result (got %s, expected 4326)",
		   results[1 * columns + 0]);
	  sqlite3_free_table (results);
	  return -32;
      }
    if (strcasecmp (results[1 * columns + 1], "epsg") != 0)
      {
	  fprintf (stderr,
		   "Unexpected organization result (got %s, expected epsg)",
		   results[1 * columns + 1]);
	  sqlite3_free_table (results);
	  return -33;
      }
    if (strcmp (results[1 * columns + 2], "4326") != 0)
      {
	  fprintf (stderr,
		   "Unexpected organization_coordsys_id result (got %s, expected 4326)",
		   results[1 * columns + 2]);
	  sqlite3_free_table (results);
	  return -34;
      }
    if (strcmp
	(results[1 * columns + 3],
	 "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected definition result (got %s, expected GEOGCS[...])",
		   results[1 * columns + 3]);
	  sqlite3_free_table (results);
	  return -35;
      }
    sqlite3_free_table (results);

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
