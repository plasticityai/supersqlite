/*

    GeoPackage extensions for SpatiaLite / SQLite
 
Version: MPL 1.1/GPL 2.0/LGPL 2.1

The contents of this file are subject to the Mozilla Public License Version
1.1 (the "License"); you may not use this file except in compliance with
the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/
 
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is GeoPackage Extensions

The Initial Developer of the Original Code is Brad Hards (bradh@frogmouth.net)
 
Portions created by the Initial Developer are Copyright (C) 2012-2015
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
#include <math.h>

#include "spatialite/geopackage.h"
#include "config.h"
#include "geopackage_internal.h"

#ifdef ENABLE_GEOPACKAGE

static char *SUPPORTED_GEOMETRY_TYPES[] = {
    "GEOMETRY",
    "POINT",
    "LINESTRING",
    "POLYGON",
    "MULTIPOINT",
    "MULTILINESTRING",
    "MULTIPOLYGON",
    "GEOMCOLLECTION",
    NULL
};

GEOPACKAGE_PRIVATE void
fnct_gpkgAddGeometryColumn (sqlite3_context * context, int argc
			    __attribute__ ((unused)), sqlite3_value ** argv)
{
/* SQL function:
/ gpkgAddGeomtryColumn(table_name, geometry_column_name, geometry_type, with_z, with_m, srs_id)
/
/ Adds a geometry column to the specified table
/ geometry_type is a normal WKT name: "GEOMETRY", "POINT", "LINESTRING", "POLYGON",
/              "MULTIPOINT", "MULTILINESTRING", "MULTIPOLYGON", "GEOMCOLLECTION"
/ with_z is a flag (0 for no z values, 1 for mandatory z values, 2 for optional z values)
/ with_m is a flag (0 for no m values, 1 for mandatory m values, 2 for optional m values)
/
/ It also adds a matching entry into gpkg_contents (if not already present)
/
/ returns nothing on success, raises exception on error
/ 
/ This function assumes usual tile conventions, including that the tiles are power-of-two-zoom,
/ 256x256 pixels, 1 tile at the top level (zoom level 0).
/
*/
    const unsigned char *table;
    const unsigned char *geometry_column_name;
    const unsigned char *geometry_type_name = NULL;
    int with_z;
    int with_m;
    int srid;
    int i = 0;

    char *sql_stmt = NULL;
    sqlite3 *sqlite = NULL;
    char *errMsg = NULL;
    int ret = 0;

    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_error (context,
				"gpkgAddGeometryColumn() error: argument 1 [table] is not of the string type",
				-1);
	  return;
      }
    table = sqlite3_value_text (argv[0]);

    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  sqlite3_result_error (context,
				"gpkgAddGeometryColumn() error: argument 2 [geometry_column_name] is not of the string type",
				-1);
	  return;
      }
    geometry_column_name = sqlite3_value_text (argv[1]);

    if (sqlite3_value_type (argv[2]) != SQLITE_TEXT)
      {
	  sqlite3_result_error (context,
				"gpkgAddGeometryColumn() error: argument 3 [geometry_type] is not of the string type",
				-1);
	  return;
      }
    for (i = 0; SUPPORTED_GEOMETRY_TYPES[i] != NULL; ++i)
      {
	  if (strcasecmp
	      ((const char *) sqlite3_value_text (argv[2]),
	       SUPPORTED_GEOMETRY_TYPES[i]) == 0)
	    {
		geometry_type_name =
		    (const unsigned char *) SUPPORTED_GEOMETRY_TYPES[i];
		break;
	    }
      }
    if (geometry_type_name == NULL)
      {
	  sqlite3_result_error (context,
				"gpkgAddGeometryColumn() error: argument 3 [geometry_type] not a recognised geometry type",
				-1);
	  return;
      }

    if (sqlite3_value_type (argv[3]) != SQLITE_INTEGER)
      {
	  sqlite3_result_error (context,
				"gpkgAddGeometryColumn() error: argument 4 [with_z] is not of the integer type",
				-1);
	  return;
      }
    with_z = (double) sqlite3_value_int (argv[3]);
    if ((with_z != 0) && (with_z != 1) && (with_z != 2))
      {
	  sqlite3_result_error (context,
				"gpkgAddGeometryColumn() error: argument 4 [with_z] is not a known value (expected 0, 1 or 2)",
				-1);
	  return;
      }

    if (sqlite3_value_type (argv[4]) != SQLITE_INTEGER)
      {
	  sqlite3_result_error (context,
				"gpkgAddGeometryColumn() error: argument 5 [with_m] is not of the integer type",
				-1);
	  return;
      }
    with_m = (double) sqlite3_value_int (argv[4]);
    if ((with_m != 0) && (with_m != 1) && (with_m != 2))
      {
	  sqlite3_result_error (context,
				"gpkgAddGeometryColumn() error: argument 5 [with_m] is not a known value (expected 0, 1 or 2)",
				-1);
	  return;
      }

    if (sqlite3_value_type (argv[5]) != SQLITE_INTEGER)
      {
	  sqlite3_result_error (context,
				"gpkgAddGeometryColumn() error: argument 6 [srid] is not of the integer type",
				-1);
	  return;
      }
    srid = sqlite3_value_int (argv[5]);

    sqlite = sqlite3_context_db_handle (context);
    
    sql_stmt = sqlite3_mprintf("INSERT OR IGNORE INTO gpkg_contents "
                 "(table_name, data_type, srs_id, min_x, min_y, max_x, max_y) "
                 "VALUES (%Q, 'feature', %i, NULL, NULL, NULL, NULL)",
                 table, srid);

    /* Add column definition to metadata table */
    sql_stmt = sqlite3_mprintf ("INSERT INTO gpkg_geometry_columns "
				"(table_name, column_name, geometry_type_name, srs_id, z, m) "
				"VALUES (%Q, %Q, %Q, %i, %i, %i)",
				table, geometry_column_name, geometry_type_name,
				srid, with_z, with_m);

    ret = sqlite3_exec (sqlite, sql_stmt, NULL, NULL, &errMsg);
    sqlite3_free (sql_stmt);
    if (ret != SQLITE_OK)
      {
	  sqlite3_result_error (context, errMsg, -1);
	  sqlite3_free (errMsg);
	  return;
      }

    /* extend table_name to actually have a geometry column */
    sql_stmt = sqlite3_mprintf ("ALTER TABLE %s ADD COLUMN %s %s",
				table, geometry_column_name, geometry_type_name);
    ret = sqlite3_exec (sqlite, sql_stmt, NULL, NULL, &errMsg);
    sqlite3_free (sql_stmt);
    if (ret != SQLITE_OK)
      {
	  sqlite3_result_error (context, errMsg, -1);
	  sqlite3_free (errMsg);
	  return;
      }

    /* TODO: add triggers */
}

#endif
