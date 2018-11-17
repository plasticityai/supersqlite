/*

 check_gpkgMode.c - Test case for GeoPackage Extensions

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

The Original Code is GeoPackage extensions

The Initial Developer of the Original Code is Sandro Furieri
 
Portions created by the Initial Developer are Copyright (C) 2014
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
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#ifdef ENABLE_GEOPACKAGE	/* only if GEOPACKAGE is enabled */

#include "sqlite3.h"
#include "spatialite.h"

static int
test_sql_query (sqlite3 * db_handle, const char *sql)
{
/* testing an SQL query in GeoPackage mode */
    int ret;
    char *err_msg = NULL;
    char **results;
    int rows;
    int columns;
    ret =
	sqlite3_get_table (db_handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Test: %s\nError: %s\n", sql, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Test: %s\nUnexpected error: select columns bad result: %i/%i.\n",
		   sql, rows, columns);
	  return 0;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "Test: %s\nUnexpected error: bad result: %s.\n",
		   sql, results[1]);
	  return 0;
      }
    sqlite3_free_table (results);
    return 1;
}

int
main (int argc, char *argv[])
{
    sqlite3 *db_handle;
    const char *sql;
    void *cache = NULL;
    int ret = 0;
    char *err_msg = NULL;
    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

/* directly testing GPKG */
    cache = spatialite_alloc_connection ();
    ret =
	sqlite3_open_v2 ("./gpkg_test.gpkg", &db_handle,
			 SQLITE_OPEN_READWRITE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open '%s': %s\n", "copy-gpkg_test.gpkg",
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }
    spatialite_init_ex (db_handle, cache, 0);

/* enabling GeoPackage Mode */
    ret =
	sqlite3_exec (db_handle, "SELECT EnableGpkgMode()", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "EnableGpkgMode error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  ret = -2;
	  goto end;
      }
    sql = "SELECT GetGpkgMode()";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -3;
	  goto end;
      }
    ret = 0;

/* testing POINTs */
    sql =
	"SELECT IsValidGPB( ShiftCoords(geom, 1.5, 2.5) ) FROM pt2d WHERE id = 3";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -3;
	  goto end;
      }
    sql =
	"SELECT IsValidGPB( ST_Translate(geom, 1.5, 2.5, 3.5) ) FROM pt3dz WHERE id = 3";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -4;
	  goto end;
      }
    sql = "SELECT IsValidGPB( RotateCoords(geom, 30) ) FROM pt2dm WHERE id = 3";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -5;
	  goto end;
      }
    sql = "SELECT IsValidGPB( SwapCoords(geom) ) FROM pt3dzm WHERE id = 3";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -6;
	  goto end;
      }

/* testing LINESTRINGs */
    sql = "SELECT IsValidGPB( ST_Collect(geom) ) FROM ln2d";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -7;
	  goto end;
      }
    sql = "SELECT IsValidGPB( ST_Union(geom) ) FROM ln2dm";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -8;
	  goto end;
      }
    sql = "SELECT IsValidGPB( ST_Buffer(geom, 0.01) ) FROM ln3dz WHERE id = 2";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -9;
	  goto end;
      }
    sql =
	"SELECT IsValidGPB( ST_OffsetCurve(geom, -0.1) ) FROM ln3dzm WHERE id = 1";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -10;
	  goto end;
      }

/* testing POLYGONs */
    sql = "SELECT IsValidGPB( ST_Collect(geom) ) FROM pg2d";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -11;
	  goto end;
      }
    sql = "SELECT IsValidGPB( ST_Union(geom) ) FROM pg2dm";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -12;
	  goto end;
      }
    sql = "SELECT IsValidGPB( ST_Buffer(geom, 0.01) ) FROM pg3dz WHERE id = 2";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -13;
	  goto end;
      }
    sql = "SELECT IsValidGPB( ST_Envelope(geom) ) FROM pg3dzm WHERE id = 1";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -14;
	  goto end;
      }

/* testing MULTIPOINTs */
    sql = "SELECT IsValidGPB( ST_ConvexHull(geom) ) FROM mpt2d WHERE id = 2";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -15;
	  goto end;
      }
    sql =
	"SELECT IsValidGPB( ST_UnaryUnion( ST_Buffer(geom, 1.5) ) ) FROM mpt3dz WHERE id = 2";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -16;
	  goto end;
      }
    sql = "SELECT IsValidGPB( ST_Difference( "
	"ST_GeomFromText('POLYGON((0 0, 0 30, 30 30, 30 0, 0 0))'), "
	"ST_Buffer(geom, 15) ) ) FROM mpt2dm WHERE id = 2";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -17;
	  goto end;
      }
    sql = "SELECT IsValidGPB( ST_SymDifference( "
	"ST_GeomFromText('POLYGON((0 0, 0 30, 30 30, 30 0, 0 0))'), "
	"ST_Buffer(geom, 15) ) ) FROM mpt3dzm WHERE id = 1";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -18;
	  goto end;
      }

/* testing MULTILINESTRINGs */
    sql =
	"SELECT IsValidGPB( ST_StartPoint( ST_GeometryN(geom, 1) ) ) FROM mln2d WHERE id = 1";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -19;
	  goto end;
      }
    sql =
	"SELECT IsValidGPB( ST_EndPoint( ST_GeometryN(geom, 1) ) ) FROM mln2dm WHERE id = 2";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -20;
	  goto end;
      }
    sql =
	"SELECT IsValidGPB( ST_Line_Interpolate_Point( ST_GeometryN(geom, 1), 0.66) ) FROM mln3dz WHERE id = 2";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -21;
	  goto end;
      }
    sql =
	"SELECT IsValidGPB(ST_Line_Substring( ST_GeometryN(geom, 1), 0.33, 0.66) ) FROM mln3dzm WHERE id = 1";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -22;
	  goto end;
      }

/* testing MULTIPOLYGONs */
    sql =
	"SELECT IsValidGPB( ST_EndPoint( ST_ExteriorRing( ST_GeometryN(geom, 1) )  ) ) FROM mpg2d WHERE id = 2";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -23;
	  goto end;
      }
    sql =
	"SELECT IsValidGPB( ST_DissolvePoints( geom ) ) FROM mpg2dm WHERE id = 2";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -24;
	  goto end;
      }
    sql =
	"SELECT IsValidGPB( ST_DissolveSegments( geom ) ) FROM mpg3dz WHERE id = 2";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -25;
	  goto end;
      }
    sql =
	"SELECT IsValidGPB( ST_LinesFromRings(geom) ) FROM mpg3dzm WHERE id = 1";
    if (!test_sql_query (db_handle, sql))
      {
	  ret = -26;
	  goto end;
      }

  end:
    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();
    return ret;
}

#endif /* endif GEOPACKAGE enabled */
