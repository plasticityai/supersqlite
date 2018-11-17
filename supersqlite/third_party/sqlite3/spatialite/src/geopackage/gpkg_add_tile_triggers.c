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
fnct_gpkgAddTileTriggers (sqlite3_context * context, int argc
			  __attribute__ ((unused)), sqlite3_value ** argv)
{
/* SQL function:
/ gpkgAddTileTriggers(table)
/
/ Adds Geopackage tile table triggers for the named table
/ returns nothing on success, raises exception on error
/
*/
    const unsigned char *table;
    char *sql_stmt = NULL;
    sqlite3 *sqlite = NULL;
    char *errMsg = NULL;
    int ret = 0;
    int i = 0;
    /* Note: the code below relies on there being twelve (or less) varargs, all of which are the table name */
    const char *trigger_stmts[] = {
	"CREATE TRIGGER \"%s_zoom_insert\"\n"
	    "BEFORE INSERT ON \"%s\"\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table ''%s'' violates constraint: zoom_level not specified for table in gpkg_tile_matrix')\n"
	    "WHERE NOT (NEW.zoom_level IN (SELECT zoom_level FROM gpkg_tile_matrix WHERE table_name = \"%s\"));\n"
	    "END",

	"CREATE TRIGGER \"%s_zoom_update\"\n"
	    "BEFORE UPDATE OF zoom_level ON \"%s\"\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table ''%s'' violates constraint: zoom_level not specified for table in gpkg_tile_matrix')\n"
	    "WHERE NOT (NEW.zoom_level IN (SELECT zoom_level FROM gpkg_tile_matrix WHERE table_name = \"%s\"));\n"
	    "END",

	"CREATE TRIGGER \"%s_tile_column_insert\"\n"
	    "BEFORE INSERT ON \"%s\"\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table ''%s'' violates constraint: tile_column cannot be < 0')\n"
	    "WHERE (NEW.tile_column < 0) ;\n"
	    "SELECT RAISE(ABORT, 'insert on table ''%s'' violates constraint: tile_column must be < matrix_width specified for table and zoom level in gpkg_tile_matrix')\n"
	    "WHERE NOT (NEW.tile_column < (SELECT matrix_width FROM gpkg_tile_matrix WHERE table_name = '%s' AND zoom_level = NEW.zoom_level));\n"
	    "END",

	"CREATE TRIGGER \"%s_tile_column_update\"\n"
	    "BEFORE UPDATE OF tile_column ON \"%s\"\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table ''%s'' violates constraint: tile_column cannot be < 0')\n"
	    "WHERE (NEW.tile_column < 0) ;\n"
	    "SELECT RAISE(ABORT, 'update on table ''%s'' violates constraint: tile_column must be < matrix_width specified for table and zoom level in gpkg_tile_matrix')\n"
	    "WHERE NOT (NEW.tile_column < (SELECT matrix_width FROM gpkg_tile_matrix WHERE table_name = '%s' AND zoom_level = NEW.zoom_level));\n"
	    "END",

	"CREATE TRIGGER \"%s_tile_row_insert\"\n"
	    "BEFORE INSERT ON \"%s\"\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table ''%s'' violates constraint: tile_row cannot be < 0')\n"
	    "WHERE (NEW.tile_row < 0) ;\n"
	    "SELECT RAISE(ABORT, 'insert on table ''%s'' violates constraint: tile_row must be < matrix_height specified for table and zoom level in gpkg_tile_matrix')\n"
	    "WHERE NOT (NEW.tile_row < (SELECT matrix_height FROM gpkg_tile_matrix WHERE table_name = '%s' AND zoom_level = NEW.zoom_level));\n"
	    "END",

	"CREATE TRIGGER \"%s_tile_row_update\"\n"
	    "BEFORE UPDATE OF tile_row ON \"%s\"\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table ''%s'' violates constraint: tile_row cannot be < 0')\n"
	    "WHERE (NEW.tile_row < 0) ;\n"
	    "SELECT RAISE(ABORT, 'update on table ''%s'' violates constraint: tile_row must be < matrix_height specified for table and zoom level in gpkg_tile_matrix')\n"
	    "WHERE NOT (NEW.tile_row < (SELECT matrix_height FROM gpkg_tile_matrix WHERE table_name = '%s' AND zoom_level = NEW.zoom_level));\n"
	    "END",

	NULL
    };

    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_error (context,
				"gpkgAddTileTriggers() error: argument 1 [table] is not of the String type",
				-1);
	  return;
      }
    table = sqlite3_value_text (argv[0]);

    for (i = 0; trigger_stmts[i] != NULL; ++i)
      {
	  sql_stmt =
	      sqlite3_mprintf (trigger_stmts[i], table, table, table, table,
			       table, table, table, table, table, table, table,
			       table);
	  sqlite = sqlite3_context_db_handle (context);
	  ret = sqlite3_exec (sqlite, sql_stmt, NULL, NULL, &errMsg);
	  sqlite3_free (sql_stmt);
	  if (ret != SQLITE_OK)
	    {
		sqlite3_result_error (context, errMsg, -1);
		sqlite3_free (errMsg);
		return;
	    }
      }
}
#endif
