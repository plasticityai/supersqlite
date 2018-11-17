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

#include "spatialite/geopackage.h"
#include "config.h"
#include "geopackage_internal.h"

#ifdef ENABLE_GEOPACKAGE
GEOPACKAGE_PRIVATE void
fnct_gpkgGetNormalZoom (sqlite3_context * context, int argc UNUSED,
			sqlite3_value ** argv)
{
/* SQL function:
/ gpkgGetNormalZoom(tile_table_name, inverted_zoom_level)
/
/ Gets the normal integer zoom level for data stored in the specified table.
/ Raises a SQL exception if inverted zoom level is outside the range of
/ tile_matrix_metadata table normal zoom levels defined for the tile_table
/
/ Note that this function can also be used to convert from a normal zoom level
/ to an inverted zoom level - this conversion is symmetric.
*/
    const unsigned char *table;
    int inverted_zoom_level;
    long max_zoom_level;
    long normal_zoom_level;
    char *sql_stmt = NULL;
    sqlite3 *sqlite = NULL;
    char *errMsg = NULL;
    char **results;
    char *endptr = 0;
    int rows = 0;
    int columns = 0;
    int ret = 0;

    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_error (context,
				"gpkgGetNormalZoom() error: argument 1 [tile_table_name] is not of the String type",
				-1);
	  return;
      }
    table = sqlite3_value_text (argv[0]);

    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_error (context,
				"gpkgGetNormalZoom() error: argument 2 [inverted zoom level] is not of the integer type",
				-1);
	  return;
      }
    inverted_zoom_level = sqlite3_value_int (argv[1]);

    sql_stmt =
	sqlite3_mprintf
	("SELECT MAX(zoom_level) FROM gpkg_tile_matrix WHERE table_name=\"%q\"",
	 table);

    sqlite = sqlite3_context_db_handle (context);
    ret =
	sqlite3_get_table (sqlite, sql_stmt, &results, &rows, &columns,
			   &errMsg);
    sqlite3_free (sql_stmt);
    if (ret != SQLITE_OK)
      {
	  sqlite3_result_error (context, errMsg, -1);
	  sqlite3_free (errMsg);
	  return;
      }
    if ((rows != 1) || (results[1 * columns + 0] == NULL))
      {
	  sqlite3_free_table (results);
	  sqlite3_result_error (context,
				"gpkgGetNormalZoom: tile table not found in gpkg_tile_matrix",
				-1);
	  sqlite3_free (errMsg);
	  return;
      }
    errno = 0;
    max_zoom_level = strtol (results[1 * columns + 0], &endptr, 10);
    if ((endptr == results[1 * columns + 0])
	|| (max_zoom_level < 0)
	|| (errno == ERANGE && max_zoom_level == LONG_MAX)
	|| (errno != 0 && max_zoom_level == 0))
      {
	  sqlite3_free_table (results);
	  sqlite3_result_error (context,
				"gpkgGetNormalZoom: could not parse result (corrupt GeoPackage?)",
				-1);
	  return;
      }
    sqlite3_free_table (results);
    if ((inverted_zoom_level > max_zoom_level) || (inverted_zoom_level < 0))
      {
	  sqlite3_result_error (context,
				"gpkgGetNormalZoom: input zoom level number outside of valid zoom levels",
				-1);
	  return;
      }

    normal_zoom_level = max_zoom_level - inverted_zoom_level;
    sqlite3_result_int (context, normal_zoom_level);
}
#endif
