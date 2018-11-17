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
fnct_gpkgInsertEpsgSRID (sqlite3_context * context, int argc UNUSED,
			 sqlite3_value ** argv)
{
/* SQL function:
/ gpkgInsertEpsgSRID(srid)
/
/ Adds a spatial reference system entry for the specified EPSG identifier
/
/ It is an error to try to add the entry if it already exists (this is often indicated by
/ the "PRIMARY KEY must be unique" error message)
/
/ returns nothing on success, raises exception on error
*/
    char *sqlcmd = NULL;
    sqlite3 *sqlite = NULL;
    sqlite3_stmt *sql_stmt;
    int ret = 0;
    int srid;
    struct epsg_defs *first = NULL;
    struct epsg_defs *last = NULL;

    if (sqlite3_value_type (argv[0]) != SQLITE_INTEGER)
      {
	  sqlite3_result_error (context,
				"gpkgInsertEpsgSRID() error: argument 1 [srid] is not of the integer type",
				-1);
	  return;
      }
    srid = sqlite3_value_int (argv[0]);

    /* get the EPSG definition for this SRID from our master list */
    initialize_epsg (srid, &first, &last);
    if (first == NULL)
      {
	  sqlite3_result_error (context,
				"gpkgInsertEpsgSRID() error: srid is not defined in the EPSG inlined dataset",
				-1);
	  return;
      }

    /* get a context handle */
    sqlite = sqlite3_context_db_handle (context);

    /* add the definition for the SRID */
    sqlcmd =
	"INSERT INTO gpkg_spatial_ref_sys (srs_name, srs_id, organization, "
	"organization_coordsys_id, definition) VALUES (?, ?, ?, ?, ?)";
    ret = sqlite3_prepare_v2 (sqlite, sqlcmd, strlen (sqlcmd), &sql_stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  sqlite3_result_error (context, sqlite3_errmsg (sqlite), -1);
	  goto stop;
      }
    sqlite3_bind_text (sql_stmt, 1, first->ref_sys_name,
		       strlen (first->ref_sys_name), SQLITE_STATIC);
    sqlite3_bind_int (sql_stmt, 2, first->srid);
    sqlite3_bind_text (sql_stmt, 3, first->auth_name, strlen (first->auth_name),
		       SQLITE_STATIC);
    sqlite3_bind_int (sql_stmt, 4, first->auth_srid);
    if (strlen (first->srs_wkt) == 0)
      {
	  sqlite3_bind_text (sql_stmt, 5, "Undefined", 9, SQLITE_STATIC);
      }
    else
      {
	  sqlite3_bind_text (sql_stmt, 5, first->srs_wkt,
			     strlen (first->srs_wkt), SQLITE_STATIC);
      }
    ret = sqlite3_step (sql_stmt);
    if (ret != SQLITE_DONE && ret != SQLITE_ROW)
      {
	  sqlite3_result_error (context, sqlite3_errmsg (sqlite), -1);
	  goto stop;
      }
  stop:
    if (sql_stmt != NULL)
      {
	  sqlite3_finalize (sql_stmt);
      }

/* freeing the EPSG defs list */
    free_epsg (first);
}
#endif
