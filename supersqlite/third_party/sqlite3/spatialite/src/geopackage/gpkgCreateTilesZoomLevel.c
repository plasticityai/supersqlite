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
GEOPACKAGE_PRIVATE void
fnct_gpkgCreateTilesZoomLevel (sqlite3_context * context, int argc
			       __attribute__ ((unused)), sqlite3_value ** argv)
{
/* SQL function:
/ gpkgCreateTilesZoomLevel(table_name, zoom_level, extent_width, extent_height)
/
/ Adds a zoom level for the specified table
/
/ returns nothing on success, raises exception on error
/ 
/ This function assumes usual tile conventions, including that the tiles are power-of-two-zoom,
/ 256x256 pixels, 1 tile at the top level (zoom level 0).
/
*/
    const unsigned char *table;
    int zoomlevel = 0;
    int columns = 1;
    int rows = 1;
    int tilesize = 256;
    double extent_height;
    double extent_width;
    char *sql_stmt = NULL;
    sqlite3 *sqlite = NULL;
    char *errMsg = NULL;
    int ret = 0;

    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_error (context,
				"gpkgCreateTilesZoomLevel() error: argument 1 [table] is not of the String type",
				-1);
	  return;
      }
    table = sqlite3_value_text (argv[0]);

    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_error (context,
				"gpkgCreateTilesZoomLevel() error: argument 2 [zoom level] is not of the integer type",
				-1);
	  return;
      }
    zoomlevel = sqlite3_value_int (argv[1]);
    if (zoomlevel < 0)
      {
	  sqlite3_result_error (context,
				"gpkgCreateTilesZoomLevel() error: argument 2 [zoom level] must be >= 0",
				-1);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
      {
	  extent_width = (double) sqlite3_value_int (argv[2]);
      }
    else if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
      {
	  extent_width = sqlite3_value_double (argv[2]);
      }
    else
      {
	  sqlite3_result_error (context,
				"gpkgCreateTilesZoomLevel() error: argument 3 [extent_width] is not of a numerical type",
				-1);
	  return;
      }

    if (sqlite3_value_type (argv[3]) == SQLITE_INTEGER)
      {
	  extent_height = (double) sqlite3_value_int (argv[3]);
      }
    else if (sqlite3_value_type (argv[3]) == SQLITE_FLOAT)
      {
	  extent_height = sqlite3_value_double (argv[3]);
      }
    else
      {
	  sqlite3_result_error (context,
				"gpkgCreateTilesZoomLevel() error: argument 4 [extent_height] is not of a numerical type",
				-1);
	  return;
      }

    sqlite = sqlite3_context_db_handle (context);
    columns = pow (2, zoomlevel);
    rows = columns;

    sql_stmt = sqlite3_mprintf ("INSERT INTO gpkg_tile_matrix"
				"(table_name, zoom_level, matrix_width, matrix_height, tile_width, tile_height, pixel_x_size, pixel_y_size)"
				"VALUES (%Q, %i, %i, %i, %i, %i, %g, %g)",
				table, zoomlevel, columns, rows, tilesize,
				tilesize, extent_width / (tilesize * columns),
				extent_height / (tilesize * rows));
    ret = sqlite3_exec (sqlite, sql_stmt, NULL, NULL, &errMsg);
    sqlite3_free (sql_stmt);
    if (ret != SQLITE_OK)
      {
	  sqlite3_result_error (context, errMsg, -1);
	  sqlite3_free (errMsg);
	  return;
      }
}
#endif
