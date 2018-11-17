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

The Initial Developer of the Original Code is Sandro Furieri (a.furieri@lqt.it)
 
Portions created by the Initial Developer are Copyright (C) 2014-2015
the Initial Developer. All Rights Reserved.

Contributor(s):
Brad Hards (bradh@frogmouth.net)

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
#include "spatialite/gaiaaux.h"
#include "config.h"
#include "geopackage_internal.h"

#ifdef ENABLE_GEOPACKAGE
GEOPACKAGE_PRIVATE void
fnct_gpkgAddSpatialIndex (sqlite3_context * context, int argc
			  __attribute__ ((unused)), sqlite3_value ** argv)
{
/* SQL function:
/ gpkgAddSpatialIndex(table, column)
/
/ Adds Geopackage SpatialIndex triggers for the named table
/ and creates the "rtree_<T>_<C> Virtual Table
/ returns nothing on success, raises exception on error
/
*/
    const char *table;
    const char *column;
    char *xtable;
    char *xcolumn;
    char *sql_stmt = NULL;
    sqlite3 *sqlite = NULL;
    char *errMsg = NULL;
    int ret = 0;
    int i = 0;
    const char *trigger_stmts[] = {
	"CREATE TRIGGER \"rtree_%s_%s_insert\"\n"
	    "AFTER INSERT ON \"%s\"\n"
	    "WHEN (new.\"%s\" NOT NULL AND NOT ST_IsEmpty(NEW.\"%s\"))\n"
	    "BEGIN\n"
	    "INSERT OR REPLACE INTO \"rtree_%s_%s\" VALUES (NEW.ROWID, "
	    "ST_MinX(NEW.\"%s\"), ST_MaxX(NEW.\"%s\"), ST_MinY(NEW.\"%s\"), "
	    "ST_MaxY(NEW.\"%s\"));\nEND",

	"CREATE TRIGGER \"rtree_%s_%s_update1\"\n"
	    "AFTER UPDATE OF \"%s\" ON \"%s\"\n"
	    "WHEN OLD.ROWID = NEW.ROWID AND "
	    "(NEW.\"%s\" NOT NULL AND NOT ST_IsEmpty(NEW.\"%s\"))\n"
	    "BEGIN\n"
	    "INSERT OR REPLACE INTO \"rtree_%s_%s\" VALUES (NEW.ROWID, "
	    "ST_MinX(NEW.\"%s\"), ST_MaxX(NEW.\"%s\"), ST_MinY(NEW.\"%s\"), "
	    "ST_MaxY(NEW.\"%s\"));\nEND",

	"CREATE TRIGGER \"rtree_%s_%s_update2\"\n"
	    "AFTER UPDATE OF \"%s\" ON \"%s\"\n"
	    "WHEN OLD.ROWID = NEW.ROWID AND "
	    "(NEW.\"%s\" IS NULL OR ST_IsEmpty(NEW.\"%s\"))\n"
	    "BEGIN\n" "DELETE FROM \"rtree_%s_%s\" WHERE id = OLD.ROWID;\nEND",

	"CREATE TRIGGER \"rtree_%s_%s_update3\"\n"
	    "AFTER UPDATE OF \"%s\" ON \"%s\"\n"
	    "WHEN OLD.ROWID != NEW.ROWID AND "
	    "(NEW.\"%s\" NOT NULL AND NOT ST_IsEmpty(NEW.\"%s\"))\n"
	    "BEGIN\n"
	    "DELETE FROM \"rtree_%s_%s\" WHERE id = OLD.ROWID;\n"
	    "INSERT OR REPLACE INTO \"rtree_%s_%s\" VALUES (NEW.ROWID, "
	    "ST_MinX(NEW.\"%s\"), ST_MaxX(NEW.\"%s\"), ST_MinY(NEW.\"%s\"), "
	    "ST_MaxY(NEW.\"%s\"));\nEND",

	"CREATE TRIGGER \"rtree_%s_%s_update4\"\n"
	    "AFTER UPDATE ON \"%s\"\n"
	    "WHEN OLD.ROWID != NEW.ROWID AND "
	    "(NEW.\"%s\" IS NULL OR ST_IsEmpty(NEW.\"%s\"))\n"
	    "BEGIN\n"
	    "DELETE FROM \"rtree_%s_%s\" WHERE id IN (OLD.ROWID, NEW.ROWID);\n"
	    "END",

	"CREATE TRIGGER \"rtree_%s_%s_delete\"\n"
	    "AFTER DELETE ON \"%s\""
	    "WHEN old.\"%s\" NOT NULL\n"
	    "BEGIN\n" "DELETE FROM \"rtree_%s_%s\" WHERE id = OLD.ROWID;\nEND",

	NULL
    };

    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_error (context,
				"gpkgAddSpatialIndex() error: argument 1 [table] is not of the String type",
				-1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  sqlite3_result_error (context,
				"gpkgAddSpatialIndex() error: argument 2 [column] is not of the String type",
				-1);
	  return;
      }
    table = (const char *) sqlite3_value_text (argv[0]);
    column = (const char *) sqlite3_value_text (argv[1]);
    xtable = gaiaDoubleQuotedSql (table);
    xcolumn = gaiaDoubleQuotedSql (column);
    sqlite = sqlite3_context_db_handle (context);

    for (i = 0; i < 6; i++)
      {
	  /* inserting the Triggers */
	  if (i == 0)
	      sql_stmt =
		  sqlite3_mprintf (trigger_stmts[i], xtable, xcolumn, xtable,
				   xcolumn, xcolumn, xtable, xcolumn, xcolumn,
				   xcolumn, xcolumn, xcolumn);
	  else if (i == 1)
	      sql_stmt =
		  sqlite3_mprintf (trigger_stmts[i], xtable, xcolumn, xcolumn,
				   xtable, xcolumn, xcolumn, xtable, xcolumn,
				   xcolumn, xcolumn, xcolumn, xcolumn, xcolumn);
	  else if (i == 2)
	      sql_stmt =
		  sqlite3_mprintf (trigger_stmts[i], xtable, xcolumn, xcolumn,
				   xtable, xcolumn, xcolumn, xtable, xcolumn);
	  else if (i == 3)
	      sql_stmt =
		  sqlite3_mprintf (trigger_stmts[i], xtable, xcolumn, xcolumn,
				   xtable, xcolumn, xcolumn, xtable, xcolumn,
				   xtable, xcolumn, xcolumn, xcolumn, xcolumn,
				   xcolumn);
	  else if (i == 4)
	      sql_stmt =
		  sqlite3_mprintf (trigger_stmts[i], xtable, xcolumn, xtable,
				   xcolumn, xcolumn, xtable, xcolumn);
	  else
	      sql_stmt =
		  sqlite3_mprintf (trigger_stmts[i], xtable, xcolumn, xtable,
				   xcolumn, xtable, xcolumn);

	  ret = sqlite3_exec (sqlite, sql_stmt, NULL, NULL, &errMsg);
	  sqlite3_free (sql_stmt);
	  if (ret != SQLITE_OK)
	    {
		sqlite3_result_error (context, errMsg, -1);
		sqlite3_free (errMsg);
		free (xtable);
		free (xcolumn);
		return;
	    }
      }

/* creating the R*Tree Virtual Table */
    sql_stmt = sqlite3_mprintf ("CREATE VIRTUAL TABLE \"rtree_%s_%s\" "
				"USING rtree(id, minx, maxx, miny, maxy)",
				xtable, xcolumn);
    ret = sqlite3_exec (sqlite, sql_stmt, NULL, NULL, &errMsg);
    sqlite3_free (sql_stmt);
    if (ret != SQLITE_OK)
      {
	  sqlite3_result_error (context, errMsg, -1);
	  sqlite3_free (errMsg);
	  free (xtable);
	  free (xcolumn);
	  return;
      }
    free (xtable);
    free (xcolumn);

/* registering the GPKG extensions */
    sql_stmt = sqlite3_mprintf ("INSERT INTO gpkg_extensions "
				"(table_name, column_name, extension_name, definition, scope) "
				"VALUES (Lower(%Q), Lower(%Q), 'gpkg_rtree_index', "
				"'GeoPackage 1.0 Specification Annex L', 'write-only')",
				table, column);
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
