/*

 metatables.c -- creating the metadata tables and related triggers

 version 4.3, 2015 June 29

 Author: Sandro Furieri a.furieri@lqt.it

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

The Original Code is the SpatiaLite library

The Initial Developer of the Original Code is Alessandro Furieri
 
Portions created by the Initial Developer are Copyright (C) 2008-2015
the Initial Developer. All Rights Reserved.

Contributor(s):
Pepijn Van Eeckhoudt <pepijnvaneeckhoudt@luciad.com>
(implementing Android support)

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

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>
#include <spatialite/debug.h>

#include <spatialite.h>
#include <spatialite/spatialite.h>
#include <spatialite_private.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/gaiageo.h>

#ifdef _WIN32
#define strcasecmp	_stricmp
#endif /* not WIN32 */

struct spatial_index_str
{
/* a struct to implement a linked list of spatial-indexes */
    char ValidRtree;
    char ValidCache;
    char *TableName;
    char *ColumnName;
    struct spatial_index_str *Next;
};

static int
testSpatiaLiteHistory (sqlite3 * sqlite)
{
/* internal utility function:
/
/ checks if the SPATIALITE_HISTORY table already exists
/
*/
    int event_id = 0;
    int table_name = 0;
    int geometry_column = 0;
    int event = 0;
    int timestamp = 0;
    int ver_sqlite = 0;
    int ver_splite = 0;
    char sql[1024];
    int ret;
    const char *name;
    int i;
    char **results;
    int rows;
    int columns;
/* checking the SPATIALITE_HISTORY table */
    strcpy (sql, "PRAGMA table_info(spatialite_history)");
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "event_id") == 0)
		    event_id = 1;
		if (strcasecmp (name, "table_name") == 0)
		    table_name = 1;
		if (strcasecmp (name, "geometry_column") == 0)
		    geometry_column = 1;
		if (strcasecmp (name, "event") == 0)
		    event = 1;
		if (strcasecmp (name, "timestamp") == 0)
		    timestamp = 1;
		if (strcasecmp (name, "ver_sqlite") == 0)
		    ver_sqlite = 1;
		if (strcasecmp (name, "ver_splite") == 0)
		    ver_splite = 1;
	    }
      }
    sqlite3_free_table (results);
    if (event_id && table_name && geometry_column && event && timestamp
	&& ver_sqlite && ver_splite)
	return 1;
    return 0;
}

static int
checkSpatiaLiteHistory (sqlite3 * sqlite)
{
/* internal utility function:
/
/ checks if the SPATIALITE_HISTORY table already exists
/ if not, such table will then be created
/
*/
    char sql[1024];
    char *errMsg = NULL;
    int ret;

    if (testSpatiaLiteHistory (sqlite))
	return 1;

/* creating the SPATIALITE_HISTORY table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "spatialite_history (\n");
    strcat (sql, "event_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,\n");
    strcat (sql, "table_name TEXT NOT NULL,\n");
    strcat (sql, "geometry_column TEXT,\n");
    strcat (sql, "event TEXT NOT NULL,\n");
    strcat (sql, "timestamp TEXT NOT NULL,\n");
    strcat (sql, "ver_sqlite TEXT NOT NULL,\n");
    strcat (sql, "ver_splite TEXT NOT NULL)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	return 0;

    if (testSpatiaLiteHistory (sqlite))
	return 1;
    return 0;
}

SPATIALITE_PRIVATE void
updateSpatiaLiteHistory (void *p_sqlite, const char *table,
			 const char *geom, const char *operation)
{
/* inserting a row in SPATIALITE_HISTORY */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    char sql[2048];
    sqlite3_stmt *stmt = NULL;
    int ret;

    if (checkSpatiaLiteHistory (sqlite) == 0)
	return;

    strcpy (sql, "INSERT INTO spatialite_history ");
    strcat (sql, "(event_id, table_name, geometry_column, event, timestamp, ");
    strcat (sql, "ver_sqlite, ver_splite) VALUES (NULL, ?, ?, ?, ");
    strcat (sql, "strftime('%Y-%m-%dT%H:%M:%fZ', 'now'), ");
    strcat (sql, "sqlite_version(), spatialite_version())");
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s\n%s\n", sql, sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, table, strlen (table), SQLITE_STATIC);
    if (!geom)
	sqlite3_bind_null (stmt, 2);
    else
	sqlite3_bind_text (stmt, 2, geom, strlen (geom), SQLITE_STATIC);
    sqlite3_bind_text (stmt, 3, operation, strlen (operation), SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	goto stop;
    spatialite_e ("SQL error: %s\n", sqlite3_errmsg (sqlite));

  stop:
    if (stmt)
	sqlite3_finalize (stmt);
}

static int
create_views_geometry_columns (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the VIEWS_GEOMETRY_COLUMNS table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "views_geometry_columns (\n");
    strcat (sql, "view_name TEXT NOT NULL,\n");
    strcat (sql, "view_geometry TEXT NOT NULL,\n");
    strcat (sql, "view_rowid TEXT NOT NULL,\n");
    strcat (sql, "f_table_name TEXT NOT NULL,\n");
    strcat (sql, "f_geometry_column TEXT NOT NULL,\n");
    strcat (sql, "read_only INTEGER NOT NULL,\n");
    strcat (sql, "CONSTRAINT pk_geom_cols_views PRIMARY KEY ");
    strcat (sql, "(view_name, view_geometry),\n");
    strcat (sql, "CONSTRAINT fk_views_geom_cols FOREIGN KEY ");
    strcat (sql, "(f_table_name, f_geometry_column) ");
    strcat (sql, "REFERENCES geometry_columns ");
    strcat (sql, "(f_table_name, f_geometry_column) ");
    strcat (sql, "ON DELETE CASCADE,\n");
    strcat (sql, "CONSTRAINT ck_vw_rdonly CHECK (read_only IN ");
    strcat (sql, "(0,1)))");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating an INDEX supporting the GEOMETRY_COLUMNS FK */
    strcpy (sql, "CREATE INDEX IF NOT EXISTS ");
    strcat (sql, "idx_viewsjoin ON views_geometry_columns\n");
    strcat (sql, "(f_table_name, f_geometry_column)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VIEWS_GEOMETRY_COLUMNS triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgc_view_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: ");
    strcat (sql, "view_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: ");
    strcat (sql, "view_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: \n");
    strcat (sql, "view_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_name <> lower(NEW.view_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgc_view_name_update\n");
    strcat (sql, "BEFORE UPDATE OF 'view_name' ON 'views_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "view_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "view_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "view_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_name <> lower(NEW.view_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgc_view_geometry_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: ");
    strcat (sql, "view_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: \n");
    strcat (sql, "view_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: ");
    strcat (sql, "view_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_geometry <> lower(NEW.view_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgc_view_geometry_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'view_geometry' ON 'views_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "view_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: \n");
    strcat (sql, "view_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "view_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_geometry <> lower(NEW.view_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgc_view_rowid_update\n");
    strcat (sql, "BEFORE UPDATE OF 'view_rowid' ON 'views_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "view_rowid value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "view_rowid value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_rowid LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "view_rowid value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_rowid <> lower(NEW.view_rowid);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgc_view_rowid_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: ");
    strcat (sql, "view_rowid value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_rowid LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: \n");
    strcat (sql, "view_rowid value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_rowid LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: ");
    strcat (sql, "view_rowid value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_rowid <> lower(NEW.view_rowid);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgc_f_table_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: \n");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgc_f_table_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_table_name' ON 'views_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql,
	    "CREATE TRIGGER IF NOT EXISTS vwgc_f_geometry_column_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: \n");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql,
	    "CREATE TRIGGER IF NOT EXISTS vwgc_f_geometry_column_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_geometry_column' ON 'views_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_virts_geometry_columns (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the VIRTS_GEOMETRY_COLUMNS table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "virts_geometry_columns (\n");
    strcat (sql, "virt_name TEXT NOT NULL,\n");
    strcat (sql, "virt_geometry TEXT NOT NULL,\n");
    strcat (sql, "geometry_type INTEGER NOT NULL,\n");
    strcat (sql, "coord_dimension INTEGER NOT NULL,\n");
    strcat (sql, "srid INTEGER NOT NULL,\n");
    strcat (sql, "CONSTRAINT pk_geom_cols_virts PRIMARY KEY ");
    strcat (sql, "(virt_name, virt_geometry),\n");
    strcat (sql, "CONSTRAINT fk_vgc_srid FOREIGN KEY ");
    strcat (sql, "(srid) REFERENCES spatial_ref_sys (srid))");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating an INDEX supporting the SPATIAL_REF_SYS FK */
    strcpy (sql, "CREATE INDEX IF NOT EXISTS ");
    strcat (sql, "idx_virtssrid ON virts_geometry_columns\n");
    strcat (sql, "(srid)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VIRTS_GEOMETRY_COLUMNS triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgc_virt_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'virts_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns violates constraint: ");
    strcat (sql, "virt_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns violates constraint: ");
    strcat (sql, "virt_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns violates constraint: \n");
    strcat (sql, "virt_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_name <> lower(NEW.virt_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgc_virt_name_update\n");
    strcat (sql, "BEFORE UPDATE OF 'virt_name' ON 'virts_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns violates constraint: ");
    strcat (sql, "virt_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns violates constraint: ");
    strcat (sql, "virt_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns violates constraint: ");
    strcat (sql, "virt_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_name <> lower(NEW.virt_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgc_virt_geometry_insert\n");
    strcat (sql, "BEFORE INSERT ON 'virts_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns violates constraint: ");
    strcat (sql, "virt_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns violates constraint: \n");
    strcat (sql, "virt_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns violates constraint: ");
    strcat (sql, "virt_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_geometry <> lower(NEW.virt_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgc_virt_geometry_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'virt_geometry' ON 'virts_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns violates constraint: ");
    strcat (sql, "virt_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns violates constraint: \n");
    strcat (sql, "virt_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns violates constraint: ");
    strcat (sql, "virt_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_geometry <> lower(NEW.virt_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgc_geometry_type_insert\n");
    strcat (sql, "BEFORE INSERT ON 'virts_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql, "SELECT RAISE(ABORT,'geometry_type must be one of ");
    strcat (sql, "0,1,2,3,4,5,6,7,");
    strcat (sql, "1000,1001,1002,1003,1004,1005,1006,1007,");
    strcat (sql, "2000,2001,2002,2003,2004,2005,2006,2007,");
    strcat (sql, "3000,3001,3002,3003,3004,3005,3006,3007')\n");
    strcat (sql, "WHERE NOT(NEW.geometry_type IN (0,1,2,3,4,5,6,7,");
    strcat (sql, "1000,1001,1002,1003,1004,1005,1006,1007,");
    strcat (sql, "2000,2001,2002,2003,2004,2005,2006,2007,");
    strcat (sql, "3000,3001,3002,3003,3004,3005,3006,3007));\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgc_geometry_type_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'geometry_type' ON 'virts_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql, "SELECT RAISE(ABORT,'geometry_type must be one of ");
    strcat (sql, "0,1,2,3,4,5,6,7,");
    strcat (sql, "1000,1001,1002,1003,1004,1005,1006,1007,");
    strcat (sql, "2000,2001,2002,2003,2004,2005,2006,2007,");
    strcat (sql, "3000,3001,3002,3003,3004,3005,3006,3007')\n");
    strcat (sql, "WHERE NOT(NEW.geometry_type IN (0,1,2,3,4,5,6,7,");
    strcat (sql, "1000,1001,1002,1003,1004,1005,1006,1007,");
    strcat (sql, "2000,2001,2002,2003,2004,2005,2006,2007,");
    strcat (sql, "3000,3001,3002,3003,3004,3005,3006,3007));\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgc_coord_dimension_insert\n");
    strcat (sql, "BEFORE INSERT ON 'virts_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'coord_dimension must be one of 2,3,4')\n");
    strcat (sql, "WHERE NOT(NEW.coord_dimension IN (2,3,4));\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgc_coord_dimension_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'coord_dimension' ON 'virts_geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'coord_dimension must be one of 2,3,4')\n");
    strcat (sql, "WHERE NOT(NEW.coord_dimension IN (2,3,4));\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_geometry_columns_statistics (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the GEOMETRY_COLUMNS_STATISTICS table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "geometry_columns_statistics (\n");
    strcat (sql, "f_table_name TEXT NOT NULL,\n");
    strcat (sql, "f_geometry_column TEXT NOT NULL,\n");
    strcat (sql, "last_verified TIMESTAMP,\n");
    strcat (sql, "row_count INTEGER,\n");
    strcat (sql, "extent_min_x DOUBLE,\n");
    strcat (sql, "extent_min_y DOUBLE,\n");
    strcat (sql, "extent_max_x DOUBLE,\n");
    strcat (sql, "extent_max_y DOUBLE,\n");
    strcat (sql, "CONSTRAINT pk_gc_statistics PRIMARY KEY ");
    strcat (sql, "(f_table_name, f_geometry_column),\n");
    strcat (sql, "CONSTRAINT fk_gc_statistics FOREIGN KEY ");
    strcat (sql, "(f_table_name, f_geometry_column) REFERENCES ");
    strcat (sql, "geometry_columns (f_table_name, f_geometry_column) ");
    strcat (sql, "ON DELETE CASCADE)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the GEOMETRY_COLUMNS_STATISTICS triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS gcs_f_table_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_statistics violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_statistics violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_statistics violates constraint: \n");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS gcs_f_table_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_table_name' ON 'geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_statistics violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_statistics violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_statistics violates constraint: ");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS gcs_f_geometry_column_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_statistics violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_statistics violates constraint: \n");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_statistics violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS gcs_f_geometry_column_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_geometry_column' ON 'geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_statistics violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_statistics violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_statistics violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_views_geometry_columns_statistics (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the VIEWS_GEOMETRY_COLUMNS_STATISTICS table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "views_geometry_columns_statistics (\n");
    strcat (sql, "view_name TEXT NOT NULL,\n");
    strcat (sql, "view_geometry TEXT NOT NULL,\n");
    strcat (sql, "last_verified TIMESTAMP,\n");
    strcat (sql, "row_count INTEGER,\n");
    strcat (sql, "extent_min_x DOUBLE,\n");
    strcat (sql, "extent_min_y DOUBLE,\n");
    strcat (sql, "extent_max_x DOUBLE,\n");
    strcat (sql, "extent_max_y DOUBLE,\n");
    strcat (sql, "CONSTRAINT pk_vwgc_statistics PRIMARY KEY ");
    strcat (sql, "(view_name, view_geometry),\n");
    strcat (sql, "CONSTRAINT fk_vwgc_statistics FOREIGN KEY ");
    strcat (sql, "(view_name, view_geometry) REFERENCES ");
    strcat (sql, "views_geometry_columns (view_name, view_geometry) ");
    strcat (sql, "ON DELETE CASCADE)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VIEWS_GEOMETRY_COLUMNS_STATISTICS triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcs_view_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_statistics violates constraint: ");
    strcat (sql, "view_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_statistics violates constraint: ");
    strcat (sql, "view_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_statistics violates constraint: \n");
    strcat (sql, "view_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_name <> lower(NEW.view_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcs_view_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'view_name' ON 'views_geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_statistics violates constraint: ");
    strcat (sql, "view_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_statistics violates constraint: ");
    strcat (sql, "view_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_statistics violates constraint: ");
    strcat (sql, "view_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_name <> lower(NEW.view_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcs_view_geometry_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_statistics violates constraint: ");
    strcat (sql, "view_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_statistics violates constraint: \n");
    strcat (sql, "view_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_statistics violates constraint: ");
    strcat (sql, "view_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_geometry <> lower(NEW.view_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcs_view_geometry_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'view_geometry' ON 'views_geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_statistics violates constraint: ");
    strcat (sql, "view_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_statistics violates constraint: \n");
    strcat (sql, "view_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_statistics violates constraint: ");
    strcat (sql, "view_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_geometry <> lower(NEW.view_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_virts_geometry_columns_statistics (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the VIRTS_GEOMETRY_COLUMNS_STATISTICS table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "virts_geometry_columns_statistics (\n");
    strcat (sql, "virt_name TEXT NOT NULL,\n");
    strcat (sql, "virt_geometry TEXT NOT NULL,\n");
    strcat (sql, "last_verified TIMESTAMP,\n");
    strcat (sql, "row_count INTEGER,\n");
    strcat (sql, "extent_min_x DOUBLE,\n");
    strcat (sql, "extent_min_y DOUBLE,\n");
    strcat (sql, "extent_max_x DOUBLE,\n");
    strcat (sql, "extent_max_y DOUBLE,\n");
    strcat (sql, "CONSTRAINT pk_vrtgc_statistics PRIMARY KEY ");
    strcat (sql, "(virt_name, virt_geometry),\n");
    strcat (sql, "CONSTRAINT fk_vrtgc_statistics FOREIGN KEY ");
    strcat (sql, "(virt_name, virt_geometry) REFERENCES ");
    strcat (sql, "virts_geometry_columns (virt_name, virt_geometry) ");
    strcat (sql, "ON DELETE CASCADE)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VIRTS_GEOMETRY_COLUMNS_STATISTICS triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcs_virt_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'virts_geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_statistics violates constraint: ");
    strcat (sql, "virt_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_statistics violates constraint: ");
    strcat (sql, "virt_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_statistics violates constraint: \n");
    strcat (sql, "virt_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_name <> lower(NEW.virt_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcs_virt_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'virt_name' ON 'virts_geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_statistics violates constraint: ");
    strcat (sql, "virt_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_statistics violates constraint: ");
    strcat (sql, "virt_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_statistics violates constraint: ");
    strcat (sql, "virt_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_name <> lower(NEW.virt_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcs_virt_geometry_insert\n");
    strcat (sql, "BEFORE INSERT ON 'virts_geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_statistics violates constraint: ");
    strcat (sql, "virt_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_statistics violates constraint: \n");
    strcat (sql, "virt_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_statistics violates constraint: ");
    strcat (sql, "virt_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_geometry <> lower(NEW.virt_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcs_virt_geometry_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'virt_geometry' ON 'virts_geometry_columns_statistics'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_statistics violates constraint: ");
    strcat (sql, "virt_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_statistics violates constraint: \n");
    strcat (sql, "virt_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_statistics violates constraint: ");
    strcat (sql, "virt_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_geometry <> lower(NEW.virt_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_geometry_columns_field_infos (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the GEOMETRY_COLUMNS_FIELD_INFOS table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "geometry_columns_field_infos (\n");
    strcat (sql, "f_table_name TEXT NOT NULL,\n");
    strcat (sql, "f_geometry_column TEXT NOT NULL,\n");
    strcat (sql, "ordinal INTEGER NOT NULL,\n");
    strcat (sql, "column_name TEXT NOT NULL,\n");
    strcat (sql, "null_values INTEGER NOT NULL,\n");
    strcat (sql, "integer_values INTEGER NOT NULL,\n");
    strcat (sql, "double_values INTEGER NOT NULL,\n");
    strcat (sql, "text_values INTEGER NOT NULL,\n");
    strcat (sql, "blob_values INTEGER NOT NULL,\n");
    strcat (sql, "max_size INTEGER,\n");
    strcat (sql, "integer_min INTEGER,\n");
    strcat (sql, "integer_max INTEGER,\n");
    strcat (sql, "double_min DOUBLE,\n");
    strcat (sql, "double_max DOUBLE,\n");
    strcat (sql, "CONSTRAINT pk_gcfld_infos PRIMARY KEY ");
    strcat (sql, "(f_table_name, f_geometry_column, ordinal, column_name),\n");
    strcat (sql, "CONSTRAINT fk_gcfld_infos FOREIGN KEY ");
    strcat (sql, "(f_table_name, f_geometry_column) REFERENCES ");
    strcat (sql, "geometry_columns (f_table_name, f_geometry_column) ");
    strcat (sql, "ON DELETE CASCADE)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the GEOMETRY_COLUMNS_FIELD_INFOS triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS gcfi_f_table_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_field_infos violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_field_infos violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_field_infos violates constraint: \n");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS gcfi_f_table_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_table_name' ON 'geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_field_infos violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_field_infos violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_field_infos violates constraint: ");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql,
	    "CREATE TRIGGER IF NOT EXISTS gcfi_f_geometry_column_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_field_infos violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_field_infos violates constraint: \n");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_field_infos violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql,
	    "CREATE TRIGGER IF NOT EXISTS gcfi_f_geometry_column_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_geometry_column' ON 'geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_field_infos violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_field_infos violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_field_infos violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_views_geometry_columns_field_infos (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the VIEWS_COLUMNS_FIELD_INFOS table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "views_geometry_columns_field_infos (\n");
    strcat (sql, "view_name TEXT NOT NULL,\n");
    strcat (sql, "view_geometry TEXT NOT NULL,\n");
    strcat (sql, "ordinal INTEGER NOT NULL,\n");
    strcat (sql, "column_name TEXT NOT NULL,\n");
    strcat (sql, "null_values INTEGER NOT NULL,\n");
    strcat (sql, "integer_values INTEGER NOT NULL,\n");
    strcat (sql, "double_values INTEGER NOT NULL,\n");
    strcat (sql, "text_values INTEGER NOT NULL,\n");
    strcat (sql, "blob_values INTEGER NOT NULL,\n");
    strcat (sql, "max_size INTEGER,\n");
    strcat (sql, "integer_min INTEGER,\n");
    strcat (sql, "integer_max INTEGER,\n");
    strcat (sql, "double_min DOUBLE,\n");
    strcat (sql, "double_max DOUBLE,\n");
    strcat (sql, "CONSTRAINT pk_vwgcfld_infos PRIMARY KEY ");
    strcat (sql, "(view_name, view_geometry, ordinal, column_name),\n");
    strcat (sql, "CONSTRAINT fk_vwgcfld_infos FOREIGN KEY ");
    strcat (sql, "(view_name, view_geometry) REFERENCES ");
    strcat (sql, "views_geometry_columns (view_name, view_geometry) ");
    strcat (sql, "ON DELETE CASCADE)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VIEWS_COLUMNS_FIELD_INFOS triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcfi_view_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "view_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "view_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_field_infos violates constraint: \n");
    strcat (sql, "view_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_name <> lower(NEW.view_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcfi_view_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'view_name' ON 'views_geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "view_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "view_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "view_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_name <> lower(NEW.view_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcfi_view_geometry_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "view_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_field_infos violates constraint: \n");
    strcat (sql, "view_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "view_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_geometry <> lower(NEW.view_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcfi_view_geometry_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'view_geometry' ON 'views_geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "view_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_field_infos violates constraint: \n");
    strcat (sql, "view_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "view_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_geometry <> lower(NEW.view_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_virts_geometry_columns_field_infos (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the VIRTS_GEOMETRY_COLUMNS_FIELD_INFOS table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "virts_geometry_columns_field_infos (\n");
    strcat (sql, "virt_name TEXT NOT NULL,\n");
    strcat (sql, "virt_geometry TEXT NOT NULL,\n");
    strcat (sql, "ordinal INTEGER NOT NULL,\n");
    strcat (sql, "column_name TEXT NOT NULL,\n");
    strcat (sql, "null_values INTEGER NOT NULL,\n");
    strcat (sql, "integer_values INTEGER NOT NULL,\n");
    strcat (sql, "double_values INTEGER NOT NULL,\n");
    strcat (sql, "text_values INTEGER NOT NULL,\n");
    strcat (sql, "blob_values INTEGER NOT NULL,\n");
    strcat (sql, "max_size INTEGER,\n");
    strcat (sql, "integer_min INTEGER,\n");
    strcat (sql, "integer_max INTEGER,\n");
    strcat (sql, "double_min DOUBLE,\n");
    strcat (sql, "double_max DOUBLE,\n");
    strcat (sql, "CONSTRAINT pk_vrtgcfld_infos PRIMARY KEY ");
    strcat (sql, "(virt_name, virt_geometry, ordinal, column_name),\n");
    strcat (sql, "CONSTRAINT fk_vrtgcfld_infos FOREIGN KEY ");
    strcat (sql, "(virt_name, virt_geometry) REFERENCES ");
    strcat (sql, "virts_geometry_columns (virt_name, virt_geometry) ");
    strcat (sql, "ON DELETE CASCADE)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VIRTS_GEOMETRY_COLUMNS_FIELD_INFOS triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcfi_virt_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'virts_geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "virt_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "virt_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_field_infos violates constraint: \n");
    strcat (sql, "virt_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_name <> lower(NEW.virt_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcfi_virt_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'virt_name' ON 'virts_geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "virt_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "virt_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "virt_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_name <> lower(NEW.virt_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcfi_virt_geometry_insert\n");
    strcat (sql, "BEFORE INSERT ON 'virts_geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "virt_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_field_infos violates constraint: \n");
    strcat (sql, "virt_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "virt_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_geometry <> lower(NEW.virt_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcfi_virt_geometry_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'virt_geometry' ON 'virts_geometry_columns_field_infos'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "virt_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_field_infos violates constraint: \n");
    strcat (sql, "virt_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_field_infos violates constraint: ");
    strcat (sql, "virt_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_geometry <> lower(NEW.virt_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_geometry_columns_times (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the GEOMETRY_COLUMNS_TIME table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "geometry_columns_time (\n");
    strcat (sql, "f_table_name TEXT NOT NULL,\n");
    strcat (sql, "f_geometry_column TEXT NOT NULL,\n");
    strcat (sql,
	    "last_insert TIMESTAMP NOT NULL DEFAULT '0000-01-01T00:00:00.000Z',\n");
    strcat (sql,
	    "last_update TIMESTAMP NOT NULL DEFAULT '0000-01-01T00:00:00.000Z',\n");
    strcat (sql,
	    "last_delete TIMESTAMP NOT NULL DEFAULT '0000-01-01T00:00:00.000Z',\n");
    strcat (sql, "CONSTRAINT pk_gc_time PRIMARY KEY ");
    strcat (sql, "(f_table_name, f_geometry_column),\n");
    strcat (sql, "CONSTRAINT fk_gc_time FOREIGN KEY ");
    strcat (sql, "(f_table_name, f_geometry_column) ");
    strcat (sql, "REFERENCES geometry_columns ");
    strcat (sql, "(f_table_name, f_geometry_column) ");
    strcat (sql, "ON DELETE CASCADE)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the GEOMETRY_COLUMNS_TIME triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS gctm_f_table_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns_time'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_time violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_time violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_time violates constraint: \n");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS gctm_f_table_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_table_name' ON 'geometry_columns_time'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_time violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_time violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_time violates constraint: ");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql,
	    "CREATE TRIGGER IF NOT EXISTS gctm_f_geometry_column_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns_time'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_time violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_time violates constraint: \n");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_time violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql,
	    "CREATE TRIGGER IF NOT EXISTS gctm_f_geometry_column_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_geometry_column' ON 'geometry_columns_time'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_time violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_time violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_time violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_geometry_columns_auth (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the GEOMETRY_COLUMNS_AUTH table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "geometry_columns_auth (\n");
    strcat (sql, "f_table_name TEXT NOT NULL,\n");
    strcat (sql, "f_geometry_column TEXT NOT NULL,\n");
    strcat (sql, "read_only INTEGER NOT NULL,\n");
    strcat (sql, "hidden INTEGER NOT NULL,\n");
    strcat (sql, "CONSTRAINT pk_gc_auth PRIMARY KEY ");
    strcat (sql, "(f_table_name, f_geometry_column),\n");
    strcat (sql, "CONSTRAINT fk_gc_auth FOREIGN KEY ");
    strcat (sql, "(f_table_name, f_geometry_column) ");
    strcat (sql, "REFERENCES geometry_columns ");
    strcat (sql, "(f_table_name, f_geometry_column) ");
    strcat (sql, "ON DELETE CASCADE,\n");
    strcat (sql, "CONSTRAINT ck_gc_ronly CHECK (read_only IN ");
    strcat (sql, "(0,1)),\n");
    strcat (sql, "CONSTRAINT ck_gc_hidden CHECK (hidden IN ");
    strcat (sql, "(0,1)))");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the GEOMETRY_COLUMNS_AUTH triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS gcau_f_table_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_auth violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_auth violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_auth violates constraint: \n");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS gcau_f_table_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_table_name' ON 'geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_auth violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_auth violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_auth violates constraint: ");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql,
	    "CREATE TRIGGER IF NOT EXISTS gcau_f_geometry_column_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_auth violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_auth violates constraint: \n");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns_auth violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql,
	    "CREATE TRIGGER IF NOT EXISTS gcau_f_geometry_column_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_geometry_column' ON 'geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_auth violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_auth violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns_auth violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_views_geometry_columns_auth (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the VIEWS_GEOMETRY_COLUMNS_AUTH table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "views_geometry_columns_auth (\n");
    strcat (sql, "view_name TEXT NOT NULL,\n");
    strcat (sql, "view_geometry TEXT NOT NULL,\n");
    strcat (sql, "hidden INTEGER NOT NULL,\n");
    strcat (sql, "CONSTRAINT pk_vwgc_auth PRIMARY KEY ");
    strcat (sql, "(view_name, view_geometry),\n");
    strcat (sql, "CONSTRAINT fk_vwgc_auth FOREIGN KEY ");
    strcat (sql, "(view_name, view_geometry) ");
    strcat (sql, "REFERENCES views_geometry_columns ");
    strcat (sql, "(view_name, view_geometry) ");
    strcat (sql, "ON DELETE CASCADE,\n");
    strcat (sql, "CONSTRAINT ck_vwgc_hidden CHECK (hidden IN ");
    strcat (sql, "(0,1)))");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VIEWS_GEOMETRY_COLUMNS_AUTH triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcau_view_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_auth violates constraint: ");
    strcat (sql, "view_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_auth violates constraint: ");
    strcat (sql, "view_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_auth violates constraint: \n");
    strcat (sql, "view_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_name <> lower(NEW.view_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcau_view_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'view_name' ON 'views_geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_auth violates constraint: ");
    strcat (sql, "view_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_auth violates constraint: ");
    strcat (sql, "view_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_auth violates constraint: ");
    strcat (sql, "view_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_name <> lower(NEW.view_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcau_view_geometry_insert\n");
    strcat (sql, "BEFORE INSERT ON 'views_geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_auth violates constraint: ");
    strcat (sql, "view_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_auth violates constraint: \n");
    strcat (sql, "view_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on views_geometry_columns_auth violates constraint: ");
    strcat (sql, "view_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_geometry <> lower(NEW.view_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vwgcau_view_geometry_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'view_geometry'  ON 'views_geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_auth violates constraint: ");
    strcat (sql, "view_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_auth violates constraint: \n");
    strcat (sql, "view_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.view_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on views_geometry_columns_auth violates constraint: ");
    strcat (sql, "view_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.view_geometry <> lower(NEW.view_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_virts_geometry_columns_auth (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the VIRTS_GEOMETRY_COLUMNS_AUTH table */
    strcpy (sql, "CREATE TABLE IF NOT EXISTS ");
    strcat (sql, "virts_geometry_columns_auth (\n");
    strcat (sql, "virt_name TEXT NOT NULL,\n");
    strcat (sql, "virt_geometry TEXT NOT NULL,\n");
    strcat (sql, "hidden INTEGER NOT NULL,\n");
    strcat (sql, "CONSTRAINT pk_vrtgc_auth PRIMARY KEY ");
    strcat (sql, "(virt_name, virt_geometry),\n");
    strcat (sql, "CONSTRAINT fk_vrtgc_auth FOREIGN KEY ");
    strcat (sql, "(virt_name, virt_geometry) ");
    strcat (sql, "REFERENCES virts_geometry_columns ");
    strcat (sql, "(virt_name, virt_geometry) ");
    strcat (sql, "ON DELETE CASCADE,\n");
    strcat (sql, "CONSTRAINT ck_vrtgc_hidden CHECK (hidden IN ");
    strcat (sql, "(0,1)))");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VIRTS_GEOMETRY_COLUMNS_AUTH triggers */
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcau_virt_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'virts_geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_auth violates constraint: ");
    strcat (sql, "virt_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_auth violates constraint: ");
    strcat (sql, "virt_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_auth violates constraint: \n");
    strcat (sql, "virt_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_name <> lower(NEW.virt_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcau_virt_name_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'virt_name' ON 'virts_geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_auth violates constraint: ");
    strcat (sql, "virt_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_auth violates constraint: ");
    strcat (sql, "virt_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_auth violates constraint: ");
    strcat (sql, "virt_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_name <> lower(NEW.virt_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcau_virt_geometry_insert\n");
    strcat (sql, "BEFORE INSERT ON 'virts_geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_auth violates constraint: ");
    strcat (sql, "virt_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_auth violates constraint: \n");
    strcat (sql, "virt_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on virts_geometry_columns_auth violates constraint: ");
    strcat (sql, "virt_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_geometry <> lower(NEW.virt_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    strcpy (sql, "CREATE TRIGGER IF NOT EXISTS vtgcau_virt_geometry_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'virt_geometry' ON 'virts_geometry_columns_auth'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_auth violates constraint: ");
    strcat (sql, "virt_geometry value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_auth violates constraint: \n");
    strcat (sql, "virt_geometry value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.virt_geometry LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on virts_geometry_columns_auth violates constraint: ");
    strcat (sql, "virt_geometry value must be lower case')\n");
    strcat (sql, "WHERE NEW.virt_geometry <> lower(NEW.virt_geometry);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

static int
create_geometry_columns_views (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the VECTOR_LAYERS view */
    strcpy (sql, "CREATE VIEW  IF NOT EXISTS ");
    strcat (sql, "vector_layers AS\n");
    strcat (sql, "SELECT 'SpatialTable' AS layer_type, ");
    strcat (sql, "f_table_name AS table_name, ");
    strcat (sql, "f_geometry_column AS geometry_column, ");
    strcat (sql, "geometry_type AS geometry_type, ");
    strcat (sql, "coord_dimension AS coord_dimension, ");
    strcat (sql, "srid AS srid, ");
    strcat (sql, "spatial_index_enabled AS spatial_index_enabled\n");
    strcat (sql, "FROM geometry_columns\n");
    strcat (sql, "UNION\n");
    strcat (sql, "SELECT 'SpatialView' AS layer_type, ");
    strcat (sql, "a.view_name AS table_name, ");
    strcat (sql, "a.view_geometry AS geometry_column, ");
    strcat (sql, "b.geometry_type AS geometry_type, ");
    strcat (sql, "b.coord_dimension AS coord_dimension, ");
    strcat (sql, "b.srid AS srid, ");
    strcat (sql, "b.spatial_index_enabled AS spatial_index_enabled\n");
    strcat (sql, "FROM views_geometry_columns AS a\n");
    strcat (sql, "LEFT JOIN geometry_columns AS b ON (");
    strcat (sql, "Upper(a.f_table_name) = Upper(b.f_table_name) AND ");
    strcat (sql, "Upper(a.f_geometry_column) = Upper(b.f_geometry_column))\n");
    strcat (sql, "UNION\n");
    strcat (sql, "SELECT 'VirtualShape' AS layer_type, ");
    strcat (sql, "virt_name AS table_name, ");
    strcat (sql, "virt_geometry AS geometry_column, ");
    strcat (sql, "geometry_type AS geometry_type, ");
    strcat (sql, "coord_dimension AS coord_dimension, ");
    strcat (sql, "srid AS srid, ");
    strcat (sql, "0 AS spatial_index_enabled\n");
    strcat (sql, "FROM virts_geometry_columns");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VECTOR_LAYERS_AUTH view */
    strcpy (sql, "CREATE VIEW  IF NOT EXISTS ");
    strcat (sql, "vector_layers_auth AS\n");
    strcat (sql, "SELECT 'SpatialTable' AS layer_type, ");
    strcat (sql, "f_table_name AS table_name, ");
    strcat (sql, "f_geometry_column AS geometry_column, ");
    strcat (sql, "read_only AS read_only, ");
    strcat (sql, "hidden AS hidden\n");
    strcat (sql, "FROM geometry_columns_auth\n");
    strcat (sql, "UNION\n");
    strcat (sql, "SELECT 'SpatialView' AS layer_type, ");
    strcat (sql, "a.view_name AS table_name, ");
    strcat (sql, "a.view_geometry AS geometry_column, ");
    strcat (sql, "b.read_only AS read_only, ");
    strcat (sql, "a.hidden AS hidden\n");
    strcat (sql, "FROM views_geometry_columns_auth AS a\n");
    strcat (sql, "JOIN views_geometry_columns AS b ON (");
    strcat (sql, "Upper(a.view_name) = Upper(b.view_name) AND ");
    strcat (sql, "Upper(a.view_geometry) = Upper(b.view_geometry))\n");
    strcat (sql, "UNION\n");
    strcat (sql, "SELECT 'VirtualShape' AS layer_type, ");
    strcat (sql, "virt_name AS table_name, ");
    strcat (sql, "virt_geometry AS geometry_column, ");
    strcat (sql, "1 AS read_only, ");
    strcat (sql, "hidden AS hidden\n");
    strcat (sql, "FROM virts_geometry_columns_auth");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VECTOR_LAYERS_STATISTICS view */
    strcpy (sql, "CREATE VIEW IF NOT EXISTS ");
    strcat (sql, "vector_layers_statistics AS\n");
    strcat (sql, "SELECT 'SpatialTable' AS layer_type, ");
    strcat (sql, "f_table_name AS table_name, ");
    strcat (sql, "f_geometry_column AS geometry_column, ");
    strcat (sql, "last_verified AS last_verified, ");
    strcat (sql, "row_count AS row_count, ");
    strcat (sql, "extent_min_x AS extent_min_x, ");
    strcat (sql, "extent_min_y AS extent_min_y, ");
    strcat (sql, "extent_max_x AS extent_max_x, ");
    strcat (sql, "extent_max_y AS extent_max_y\n");
    strcat (sql, "FROM geometry_columns_statistics\n");
    strcat (sql, "UNION\n");
    strcat (sql, "SELECT 'SpatialView' AS layer_type, ");
    strcat (sql, "view_name AS table_name, ");
    strcat (sql, "view_geometry AS geometry_column, ");
    strcat (sql, "last_verified AS last_verified, ");
    strcat (sql, "row_count AS row_count, ");
    strcat (sql, "extent_min_x AS extent_min_x, ");
    strcat (sql, "extent_min_y AS extent_min_y, ");
    strcat (sql, "extent_max_x AS extent_max_x, ");
    strcat (sql, "extent_max_y AS extent_max_y\n");
    strcat (sql, "FROM views_geometry_columns_statistics\n");
    strcat (sql, "UNION\n");
    strcat (sql, "SELECT 'VirtualShape' AS layer_type, ");
    strcat (sql, "virt_name AS table_name, ");
    strcat (sql, "virt_geometry AS geometry_column, ");
    strcat (sql, "last_verified AS last_verified, ");
    strcat (sql, "row_count AS row_count, ");
    strcat (sql, "extent_min_x AS extent_min_x, ");
    strcat (sql, "extent_min_y AS extent_min_y, ");
    strcat (sql, "extent_max_x AS extent_max_x, ");
    strcat (sql, "extent_max_y AS extent_max_y\n");
    strcat (sql, "FROM virts_geometry_columns_statistics");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the VECTOR_LAYERS_FIELD_INFOS view */
    strcpy (sql, "CREATE VIEW IF NOT EXISTS ");
    strcat (sql, "vector_layers_field_infos AS\n");
    strcat (sql, "SELECT 'SpatialTable' AS layer_type, ");
    strcat (sql, "f_table_name AS table_name, ");
    strcat (sql, "f_geometry_column AS geometry_column, ");
    strcat (sql, "ordinal AS ordinal, ");
    strcat (sql, "column_name AS column_name, ");
    strcat (sql, "null_values AS null_values, ");
    strcat (sql, "integer_values AS integer_values, ");
    strcat (sql, "double_values AS double_values, ");
    strcat (sql, "text_values AS text_values, ");
    strcat (sql, "blob_values AS blob_values, ");
    strcat (sql, "max_size AS max_size, ");
    strcat (sql, "integer_min AS integer_min, ");
    strcat (sql, "integer_max AS integer_max, ");
    strcat (sql, "double_min AS double_min, ");
    strcat (sql, "double_max double_max\n");
    strcat (sql, "FROM geometry_columns_field_infos\n");
    strcat (sql, "UNION\n");
    strcat (sql, "SELECT 'SpatialView' AS layer_type, ");
    strcat (sql, "view_name AS table_name, ");
    strcat (sql, "view_geometry AS geometry_column, ");
    strcat (sql, "ordinal AS ordinal, ");
    strcat (sql, "column_name AS column_name, ");
    strcat (sql, "null_values AS null_values, ");
    strcat (sql, "integer_values AS integer_values, ");
    strcat (sql, "double_values AS double_values, ");
    strcat (sql, "text_values AS text_values, ");
    strcat (sql, "blob_values AS blob_values, ");
    strcat (sql, "max_size AS max_size, ");
    strcat (sql, "integer_min AS integer_min, ");
    strcat (sql, "integer_max AS integer_max, ");
    strcat (sql, "double_min AS double_min, ");
    strcat (sql, "double_max double_max\n");
    strcat (sql, "FROM views_geometry_columns_field_infos\n");
    strcat (sql, "UNION\n");
    strcat (sql, "SELECT 'VirtualShape' AS layer_type, ");
    strcat (sql, "virt_name AS table_name, ");
    strcat (sql, "virt_geometry AS geometry_column, ");
    strcat (sql, "ordinal AS ordinal, ");
    strcat (sql, "column_name AS column_name, ");
    strcat (sql, "null_values AS null_values, ");
    strcat (sql, "integer_values AS integer_values, ");
    strcat (sql, "double_values AS double_values, ");
    strcat (sql, "text_values AS text_values, ");
    strcat (sql, "blob_values AS blob_values, ");
    strcat (sql, "max_size AS max_size, ");
    strcat (sql, "integer_min AS integer_min, ");
    strcat (sql, "integer_max AS integer_max, ");
    strcat (sql, "double_min AS double_min, ");
    strcat (sql, "double_max double_max\n");
    strcat (sql, "FROM virts_geometry_columns_field_infos");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

SPATIALITE_PRIVATE int
check_layer_statistics (void *p_sqlite)
{
/*
/ checks the LAYER_STATISTICS table for validity;
/ if the table doesn't exist, attempts to create
*/
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    char sql[8192];
    char **results;
    int rows;
    int columns;
    int ret;
    int raster_layer = 0;
    int table_name = 0;
    int geometry_column = 0;
    int row_count = 0;
    int extent_min_x = 0;
    int extent_min_y = 0;
    int extent_max_x = 0;
    int extent_max_y = 0;
    int i;
    const char *name;
    int has_pk = 0;

/* checking the LAYER_STATISTICS table */
    ret =
	sqlite3_get_table (sqlite, "PRAGMA table_info(layer_statistics)",
			   &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "raster_layer") == 0)
		    raster_layer = 1;
		if (strcasecmp (name, "table_name") == 0)
		    table_name = 1;
		if (strcasecmp (name, "geometry_column") == 0)
		    geometry_column = 1;
		if (strcasecmp (name, "row_count") == 0)
		    row_count = 1;
		if (strcasecmp (name, "extent_min_x") == 0)
		    extent_min_x = 1;
		if (strcasecmp (name, "extent_min_y") == 0)
		    extent_min_y = 1;
		if (strcasecmp (name, "extent_max_x") == 0)
		    extent_max_x = 1;
		if (strcasecmp (name, "extent_max_y") == 0)
		    extent_max_y = 1;
	    }
      }
    sqlite3_free_table (results);

/* LAYER_STATISTICS already exists and has a valid layout */
    if (raster_layer && table_name && geometry_column && row_count
	&& extent_min_x && extent_max_x && extent_min_y && extent_max_y)
	return 1;
/* LAYER_STATISTICS already exists, but has an invalid layout */
    if (raster_layer || table_name || geometry_column || row_count
	|| extent_min_x || extent_max_x || extent_min_y || extent_max_y)
	return 0;

/* checking if GEOMETRY_COLUMNS has a Primary Key */
    ret =
	sqlite3_get_table (sqlite, "PRAGMA table_info(geometry_columns)",
			   &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 5];
		if (atoi (name) != 0)
		    has_pk = 1;
	    }
      }
    sqlite3_free_table (results);

/* attempting to create LAYER_STATISTICS */
    if (has_pk)
      {
	  /* GEOMETRY_COLUMNS has a Primary Key */
	  strcpy (sql, "CREATE TABLE layer_statistics (\n");
	  strcat (sql, "raster_layer INTEGER NOT NULL,\n");
	  strcat (sql, "table_name TEXT NOT NULL,\n");
	  strcat (sql, "geometry_column TEXT NOT NULL,\n");
	  strcat (sql, "row_count INTEGER,\n");
	  strcat (sql, "extent_min_x DOUBLE,\n");
	  strcat (sql, "extent_min_y DOUBLE,\n");
	  strcat (sql, "extent_max_x DOUBLE,\n");
	  strcat (sql, "extent_max_y DOUBLE,\n");
	  strcat (sql, "CONSTRAINT pk_layer_statistics PRIMARY KEY ");
	  strcat (sql, "(raster_layer, table_name, geometry_column),\n");
	  strcat (sql, "CONSTRAINT fk_layer_statistics FOREIGN KEY ");
	  strcat (sql, "(table_name, geometry_column) REFERENCES ");
	  strcat (sql, "geometry_columns (f_table_name, f_geometry_column) ");
	  strcat (sql, "ON DELETE CASCADE)");
      }
    else
      {
	  /* there is no Primary Key on GEOMETRY_COLUMNS */
	  strcpy (sql, "CREATE TABLE layer_statistics (\n");
	  strcat (sql, "raster_layer INTEGER NOT NULL,\n");
	  strcat (sql, "table_name TEXT NOT NULL,\n");
	  strcat (sql, "geometry_column TEXT NOT NULL,\n");
	  strcat (sql, "row_count INTEGER,\n");
	  strcat (sql, "extent_min_x DOUBLE,\n");
	  strcat (sql, "extent_min_y DOUBLE,\n");
	  strcat (sql, "extent_max_x DOUBLE,\n");
	  strcat (sql, "extent_max_y DOUBLE,\n");
	  strcat (sql, "CONSTRAINT pk_layer_statistics PRIMARY KEY ");
	  strcat (sql, "(raster_layer, table_name, geometry_column))");
      }
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, NULL);
    if (ret != SQLITE_OK)
	return 0;
    return 1;
}

SPATIALITE_PRIVATE int
check_views_layer_statistics (void *p_sqlite)
{
/*
/ checks the VIEWS_LAYER_STATISTICS table for validity;
/ if the table doesn't exist, attempts to create
*/
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    char sql[8192];
    char **results;
    int rows;
    int columns;
    int ret;
    int view_name = 0;
    int view_geometry = 0;
    int row_count = 0;
    int extent_min_x = 0;
    int extent_min_y = 0;
    int extent_max_x = 0;
    int extent_max_y = 0;
    int i;
    const char *name;

/* checking the VIEWS_LAYER_STATISTICS table */
    ret =
	sqlite3_get_table (sqlite, "PRAGMA table_info(views_layer_statistics)",
			   &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "view_name") == 0)
		    view_name = 1;
		if (strcasecmp (name, "view_geometry") == 0)
		    view_geometry = 1;
		if (strcasecmp (name, "row_count") == 0)
		    row_count = 1;
		if (strcasecmp (name, "extent_min_x") == 0)
		    extent_min_x = 1;
		if (strcasecmp (name, "extent_min_y") == 0)
		    extent_min_y = 1;
		if (strcasecmp (name, "extent_max_x") == 0)
		    extent_max_x = 1;
		if (strcasecmp (name, "extent_max_y") == 0)
		    extent_max_y = 1;
	    }
      }
    sqlite3_free_table (results);

/* VIEWS_LAYER_STATISTICS already exists and has a valid layout */
    if (view_name && view_geometry && row_count && extent_min_x && extent_max_x
	&& extent_min_y && extent_max_y)
	return 1;
/* VIEWS_LAYER_STATISTICS already exists, but has an invalid layout */
    if (view_name || view_geometry || row_count || extent_min_x || extent_max_x
	|| extent_min_y || extent_max_y)
	return 0;

/* attempting to create VIEWS_LAYER_STATISTICS */
    strcpy (sql, "CREATE TABLE views_layer_statistics (\n");
    strcat (sql, "view_name TEXT NOT NULL,\n");
    strcat (sql, "view_geometry TEXT NOT NULL,\n");
    strcat (sql, "row_count INTEGER,\n");
    strcat (sql, "extent_min_x DOUBLE,\n");
    strcat (sql, "extent_min_y DOUBLE,\n");
    strcat (sql, "extent_max_x DOUBLE,\n");
    strcat (sql, "extent_max_y DOUBLE,\n");
    strcat (sql, "CONSTRAINT pk_views_layer_statistics PRIMARY KEY ");
    strcat (sql, "(view_name, view_geometry),\n");
    strcat (sql, "CONSTRAINT fk_views_layer_statistics FOREIGN KEY ");
    strcat (sql, "(view_name, view_geometry) REFERENCES ");
    strcat (sql, "views_geometry_columns (view_name, view_geometry) ");
    strcat (sql, "ON DELETE CASCADE)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, NULL);
    if (ret != SQLITE_OK)
	return 0;
    return 1;
}

SPATIALITE_PRIVATE int
check_virts_layer_statistics (void *p_sqlite)
{
/*
/ checks the VIRTS_LAYER_STATISTICS table for validity;
/ if the table doesn't exist, attempts to create
*/
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    char sql[8192];
    char **results;
    int rows;
    int columns;
    int ret;
    int virt_name = 0;
    int virt_geometry = 0;
    int row_count = 0;
    int extent_min_x = 0;
    int extent_min_y = 0;
    int extent_max_x = 0;
    int extent_max_y = 0;
    int i;
    const char *name;

/* checking the VIRTS_LAYER_STATISTICS table */
    ret =
	sqlite3_get_table (sqlite, "PRAGMA table_info(virts_layer_statistics)",
			   &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "virt_name") == 0)
		    virt_name = 1;
		if (strcasecmp (name, "virt_geometry") == 0)
		    virt_geometry = 1;
		if (strcasecmp (name, "row_count") == 0)
		    row_count = 1;
		if (strcasecmp (name, "extent_min_x") == 0)
		    extent_min_x = 1;
		if (strcasecmp (name, "extent_min_y") == 0)
		    extent_min_y = 1;
		if (strcasecmp (name, "extent_max_x") == 0)
		    extent_max_x = 1;
		if (strcasecmp (name, "extent_max_y") == 0)
		    extent_max_y = 1;
	    }
      }
    sqlite3_free_table (results);

/* VIRTS_LAYER_STATISTICS already exists and has a valid layout */
    if (virt_name && virt_geometry && row_count && extent_min_x && extent_max_x
	&& extent_min_y && extent_max_y)
	return 1;
/* VIRTS_LAYER_STATISTICS already exists, but has an invalid layout */
    if (virt_name || virt_geometry || row_count || extent_min_x || extent_max_x
	|| extent_min_y || extent_max_y)
	return 0;

/* attempting to create VIRTS_LAYER_STATISTICS */
    strcpy (sql, "CREATE TABLE virts_layer_statistics (\n");
    strcat (sql, "virt_name TEXT NOT NULL,\n");
    strcat (sql, "virt_geometry TEXT NOT NULL,\n");
    strcat (sql, "row_count INTEGER,\n");
    strcat (sql, "extent_min_x DOUBLE,\n");
    strcat (sql, "extent_min_y DOUBLE,\n");
    strcat (sql, "extent_max_x DOUBLE,\n");
    strcat (sql, "extent_max_y DOUBLE,\n");
    strcat (sql, "CONSTRAINT pk_virts_layer_statistics PRIMARY KEY ");
    strcat (sql, "(virt_name, virt_geometry),\n");
    strcat (sql, "CONSTRAINT fk_virts_layer_statistics FOREIGN KEY ");
    strcat (sql, "(virt_name, virt_geometry) REFERENCES ");
    strcat (sql, "virts_geometry_columns (virt_name, virt_geometry) ");
    strcat (sql, "ON DELETE CASCADE)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, NULL);
    if (ret != SQLITE_OK)
	return 0;
    return 1;
}

static int
create_sql_statements_log (sqlite3 * sqlite)
{
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the SQL_STATEMENTS_LOG table */
    strcpy (sql, "CREATE TABLE  IF NOT EXISTS ");
    strcat (sql, "sql_statements_log (\n");
    strcat (sql, "id INTEGER PRIMARY KEY AUTOINCREMENT,\n");
    strcat (sql,
	    "time_start TIMESTAMP NOT NULL DEFAULT '0000-01-01T00:00:00.000Z',\n");
    strcat (sql,
	    "time_end TIMESTAMP NOT NULL DEFAULT '0000-01-01T00:00:00.000Z',\n");
    strcat (sql, "user_agent TEXT NOT NULL,\n");
    strcat (sql, "sql_statement TEXT NOT NULL,\n");
    strcat (sql, "success INTEGER NOT NULL DEFAULT 0,\n");
    strcat (sql, "error_cause TEXT NOT NULL DEFAULT 'ABORTED',\n");
    strcat (sql, "CONSTRAINT sqllog_success CHECK ");
    strcat (sql, "(success IN (0,1)))");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    return 1;
}

SPATIALITE_PRIVATE int
createAdvancedMetaData (void *p_sqlite)
{
/* creating the advanced MetaData tables */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    if (!create_views_geometry_columns (sqlite))
	return 0;
    if (!create_virts_geometry_columns (sqlite))
	return 0;
    if (!create_geometry_columns_statistics (sqlite))
	return 0;
    if (!create_views_geometry_columns_statistics (sqlite))
	return 0;
    if (!create_virts_geometry_columns_statistics (sqlite))
	return 0;
    if (!create_geometry_columns_field_infos (sqlite))
	return 0;
    if (!create_views_geometry_columns_field_infos (sqlite))
	return 0;
    if (!create_virts_geometry_columns_field_infos (sqlite))
	return 0;
    if (!create_geometry_columns_times (sqlite))
	return 0;
    if (!create_geometry_columns_auth (sqlite))
	return 0;
    if (!create_views_geometry_columns_auth (sqlite))
	return 0;
    if (!create_virts_geometry_columns_auth (sqlite))
	return 0;
    if (!create_geometry_columns_views (sqlite))
	return 0;
    if (!create_sql_statements_log (sqlite))
	return 0;

    return 1;
}

SPATIALITE_PRIVATE int
createGeometryColumns (void *p_sqlite)
{
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    char sql[4186];
    char *errMsg = NULL;
    int ret;
/* creating the GEOMETRY_COLUMNS table */
    strcpy (sql, "CREATE TABLE geometry_columns (\n");
    strcat (sql, "f_table_name TEXT NOT NULL,\n");
    strcat (sql, "f_geometry_column TEXT NOT NULL,\n");
    strcat (sql, "geometry_type INTEGER NOT NULL,\n");
    strcat (sql, "coord_dimension INTEGER NOT NULL,\n");
    strcat (sql, "srid INTEGER NOT NULL,\n");
    strcat (sql, "spatial_index_enabled INTEGER NOT NULL,\n");
    strcat (sql, "CONSTRAINT pk_geom_cols PRIMARY KEY ");
    strcat (sql, "(f_table_name, f_geometry_column),\n");
    strcat (sql, "CONSTRAINT fk_gc_srs FOREIGN KEY ");
    strcat (sql, "(srid) REFERENCES spatial_ref_sys (srid),\n");
    strcat (sql, "CONSTRAINT ck_gc_rtree CHECK ");
    strcat (sql, "(spatial_index_enabled IN (0,1,2)))");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    updateSpatiaLiteHistory (sqlite, "geometry_columns", NULL,
			     "table successfully created");
/* creating an INDEX corresponding to the SRID FK */
    strcpy (sql, "CREATE INDEX idx_srid_geocols ON geometry_columns\n");
    strcat (sql, "(srid) ");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
/* creating the GEOMETRY_COLUMNS triggers */
    strcpy (sql, "CREATE TRIGGER geometry_columns_f_table_name_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns violates constraint: \n");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    updateSpatiaLiteHistory (sqlite, "geometry_columns", NULL,
			     "trigger 'geometry_columns_f_table_name_insert' successfully created");
    strcpy (sql, "CREATE TRIGGER geometry_columns_f_table_name_update\n");
    strcat (sql, "BEFORE UPDATE OF 'f_table_name' ON 'geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns violates constraint: ");
    strcat (sql, "f_table_name value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns violates constraint: ");
    strcat (sql, "f_table_name value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_table_name LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns violates constraint: ");
    strcat (sql, "f_table_name value must be lower case')\n");
    strcat (sql, "WHERE NEW.f_table_name <> lower(NEW.f_table_name);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    updateSpatiaLiteHistory (sqlite, "geometry_columns", NULL,
			     "trigger 'geometry_columns_f_table_name_update' successfully created");
    strcpy (sql, "CREATE TRIGGER geometry_columns_f_geometry_column_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns violates constraint: \n");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'insert on geometry_columns violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    updateSpatiaLiteHistory (sqlite, "geometry_columns", NULL,
			     "trigger 'geometry_columns_f_geometry_column_insert' successfully created");
    strcpy (sql, "CREATE TRIGGER geometry_columns_f_geometry_column_update\n");
    strcat (sql,
	    "BEFORE UPDATE OF 'f_geometry_column' ON 'geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a single quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%''%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns violates constraint: ");
    strcat (sql, "f_geometry_column value must not contain a double quote')\n");
    strcat (sql, "WHERE NEW.f_geometry_column LIKE ('%\"%');\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'update on geometry_columns violates constraint: ");
    strcat (sql, "f_geometry_column value must be lower case')\n");
    strcat (sql,
	    "WHERE NEW.f_geometry_column <> lower(NEW.f_geometry_column);\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    updateSpatiaLiteHistory (sqlite, "geometry_columns", NULL,
			     "trigger 'geometry_columns_f_geometry_column_update' successfully created");
    strcpy (sql, "CREATE TRIGGER geometry_columns_geometry_type_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql, "SELECT RAISE(ABORT,'geometry_type must be one of ");
    strcat (sql, "0,1,2,3,4,5,6,7,");
    strcat (sql, "1000,1001,1002,1003,1004,1005,1006,1007,");
    strcat (sql, "2000,2001,2002,2003,2004,2005,2006,2007,");
    strcat (sql, "3000,3001,3002,3003,3004,3005,3006,3007')\n");
    strcat (sql, "WHERE NOT(NEW.geometry_type IN (0,1,2,3,4,5,6,7,");
    strcat (sql, "1000,1001,1002,1003,1004,1005,1006,1007,");
    strcat (sql, "2000,2001,2002,2003,2004,2005,2006,2007,");
    strcat (sql, "3000,3001,3002,3003,3004,3005,3006,3007));\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    updateSpatiaLiteHistory (sqlite, "geometry_columns", NULL,
			     "trigger 'geometry_columns_geometry_type_insert' successfully created");
    strcpy (sql, "CREATE TRIGGER geometry_columns_geometry_type_update\n");
    strcat (sql, "BEFORE UPDATE OF 'geometry_type' ON 'geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql, "SELECT RAISE(ABORT,'geometry_type must be one of ");
    strcat (sql, "0,1,2,3,4,5,6,7,");
    strcat (sql, "1000,1001,1002,1003,1004,1005,1006,1007,");
    strcat (sql, "2000,2001,2002,2003,2004,2005,2006,2007,");
    strcat (sql, "3000,3001,3002,3003,3004,3005,3006,3007')\n");
    strcat (sql, "WHERE NOT(NEW.geometry_type IN (0,1,2,3,4,5,6,7,");
    strcat (sql, "1000,1001,1002,1003,1004,1005,1006,1007,");
    strcat (sql, "2000,2001,2002,2003,2004,2005,2006,2007,");
    strcat (sql, "3000,3001,3002,3003,3004,3005,3006,3007));\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    updateSpatiaLiteHistory (sqlite, "geometry_columns", NULL,
			     "trigger 'geometry_columns_geometry_type_update' successfully created");
    strcpy (sql, "CREATE TRIGGER geometry_columns_coord_dimension_insert\n");
    strcat (sql, "BEFORE INSERT ON 'geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'coord_dimension must be one of 2,3,4')\n");
    strcat (sql, "WHERE NOT(NEW.coord_dimension IN (2,3,4));\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    updateSpatiaLiteHistory (sqlite, "geometry_columns", NULL,
			     "trigger 'geometry_columns_coord_dimension_insert' successfully created");
    strcpy (sql, "CREATE TRIGGER geometry_columns_coord_dimension_update\n");
    strcat (sql, "BEFORE UPDATE OF 'coord_dimension' ON 'geometry_columns'\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ABORT,'coord_dimension must be one of 2,3,4')\n");
    strcat (sql, "WHERE NOT(NEW.coord_dimension IN (2,3,4));\n");
    strcat (sql, "END");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SQL error: %s: %s\n", sql, errMsg);
	  sqlite3_free (errMsg);
	  return 0;
      }
    updateSpatiaLiteHistory (sqlite, "geometry_columns", NULL,
			     "trigger 'geometry_columns_coord_dimension_update' successfully created");
    return 1;
}

SPATIALITE_PRIVATE int
upgradeGeometryTriggers (void *p_sqlite)
{
/* upgrading all triggers for any Spatial Column */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    sqlite3_stmt *stmt = NULL;
    char *sql_statement;
    int retcode;
    int metadata_version = checkSpatialMetaData (sqlite);
    if (metadata_version < 3)
	return 0;

    sql_statement =
	sqlite3_mprintf ("SELECT f_table_name, f_geometry_column "
			 "FROM geometry_columns");
/* compiling SQL prepared statement */
    ret =
	sqlite3_prepare_v2 (sqlite, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("upgradeGeometryTriggers: error %d \"%s\"\n",
			sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return 0;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		const char *table =
		    (const char *) sqlite3_column_text (stmt, 0);
		const char *column =
		    (const char *) sqlite3_column_text (stmt, 1);
		updateGeometryTriggers (sqlite, table, column);
		retcode = 1;
	    }
	  else
	    {
		retcode = 0;
		break;
	    }
      }
    ret = sqlite3_finalize (stmt);
    return retcode;
}

SPATIALITE_PRIVATE void
updateGeometryTriggers (void *p_sqlite, const char *table, const char *column)
{
/* updates triggers for some Spatial Column */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    int col_index;
    const char *col_dims;
    int index;
    int cached;
    int dims;
    char *txt_dims = NULL;
    int len;
    char *errMsg = NULL;
    char *sql_statement;
    char *raw;
    char *quoted_trigger;
    char *quoted_rtree;
    char *quoted_table;
    char *quoted_column;
    char *p_table = NULL;
    char *p_column = NULL;
    sqlite3_stmt *stmt;
    struct spatial_index_str *first_idx = NULL;
    struct spatial_index_str *last_idx = NULL;
    struct spatial_index_str *curr_idx;
    struct spatial_index_str *next_idx;
    int metadata_version = checkSpatialMetaData (sqlite);

    if (!getRealSQLnames (sqlite, table, column, &p_table, &p_column))
      {
	  spatialite_e
	      ("updateTableTriggers() error: not existing Table or Column\n");
	  return;
      }
    if (metadata_version == 3)
      {
	  /* current metadata style >= v.4.0.0 */
	  sql_statement = sqlite3_mprintf ("SELECT spatial_index_enabled "
					   "FROM geometry_columns WHERE Lower(f_table_name) = Lower(?) "
					   "AND Lower(f_geometry_column) = Lower(?)");
      }
    else
      {
	  /* legacy metadata style <= v.3.1.0 */
	  sql_statement =
	      sqlite3_mprintf ("SELECT spatial_index_enabled, coord_dimension "
			       "FROM geometry_columns WHERE Lower(f_table_name) = Lower(?) "
			       "AND Lower(f_geometry_column) = Lower(?)");
      }
/* compiling SQL prepared statement */
    ret =
	sqlite3_prepare_v2 (sqlite, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateTableTriggers: error %d \"%s\"\n",
			sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, table, strlen (table), SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, column, strlen (column), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		col_index = sqlite3_column_int (stmt, 0);
		if (metadata_version == 1)
		  {
		      /* legacy metadata style <= v.3.1.0 */
		      col_dims = (const char *) sqlite3_column_text (stmt, 1);
		      dims = GAIA_XY;
		      if (strcasecmp (col_dims, "XYZ") == 0)
			  dims = GAIA_XY_Z;
		      if (strcasecmp (col_dims, "XYM") == 0)
			  dims = GAIA_XY_M;
		      if (strcasecmp (col_dims, "XYZM") == 0)
			  dims = GAIA_XY_Z_M;
		      switch (dims)
			{
			case GAIA_XY_Z:
			    txt_dims = "XYZ";
			    break;
			case GAIA_XY_M:
			    txt_dims = "XYM";
			    break;
			case GAIA_XY_Z_M:
			    txt_dims = "XYZM";
			    break;
			default:
			    txt_dims = "XY";
			    break;
			};
		  }
		index = 0;
		cached = 0;
		if (col_index == 1)
		    index = 1;
		if (col_index == 2)
		    cached = 1;

		/* trying to delete old versions [v2.0, v2.2] triggers[if any] */
		raw = sqlite3_mprintf ("gti_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;
		raw = sqlite3_mprintf ("gtu_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;
		raw = sqlite3_mprintf ("gsi_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;
		raw = sqlite3_mprintf ("gsu_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;
		/* end deletion old versions [v2.0, v2.2] triggers[if any] */

		/* deleting the old INSERT trigger TYPE [if any] */
		raw = sqlite3_mprintf ("ggi_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;

		/* inserting the new INSERT trigger TYPE */
		raw = sqlite3_mprintf ("ggi_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		quoted_table = gaiaDoubleQuotedSql (p_table);
		quoted_column = gaiaDoubleQuotedSql (p_column);
		if (metadata_version == 3)
		  {
		      /* current metadata style >= v.4.0.0 */
		      sql_statement =
			  sqlite3_mprintf
			  ("CREATE TRIGGER \"%s\" BEFORE INSERT ON \"%s\"\n"
			   "FOR EACH ROW BEGIN\n"
			   "SELECT RAISE(ROLLBACK, '%q.%q violates Geometry constraint [geom-type or SRID not allowed]')\n"
			   "WHERE (SELECT geometry_type FROM geometry_columns\n"
			   "WHERE Lower(f_table_name) = Lower(%Q) AND "
			   "Lower(f_geometry_column) = Lower(%Q)\n"
			   "AND GeometryConstraints(NEW.\"%s\", geometry_type, srid) = 1) IS NULL;\nEND",
			   quoted_trigger, quoted_table, p_table, p_column,
			   p_table, p_column, quoted_column);
		      free (quoted_trigger);
		      free (quoted_table);
		      free (quoted_column);
		  }
		else
		  {
		      /* legacy metadata style <= v.3.1.0 */
		      sql_statement =
			  sqlite3_mprintf
			  ("CREATE TRIGGER \"%s\" BEFORE INSERT ON \"%s\"\n"
			   "FOR EACH ROW BEGIN\n"
			   "SELECT RAISE(ROLLBACK, '%q.%q violates Geometry constraint [geom-type or SRID not allowed]')\n"
			   "WHERE (SELECT type FROM geometry_columns\n"
			   "WHERE f_table_name = %Q AND f_geometry_column = %Q\n"
			   "AND GeometryConstraints(NEW.\"%s\", type, srid, %Q) = 1) IS NULL;\nEND",
			   quoted_trigger, quoted_table, p_table, p_column,
			   p_table, p_column, quoted_column, txt_dims);
		      free (quoted_trigger);
		      free (quoted_table);
		      free (quoted_column);
		  }
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;
		/* deleting the old UPDATE trigger TYPE [if any] */
		raw = sqlite3_mprintf ("ggu_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;

		/* inserting the new UPDATE trigger TYPE */
		raw = sqlite3_mprintf ("ggu_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		quoted_table = gaiaDoubleQuotedSql (p_table);
		quoted_column = gaiaDoubleQuotedSql (p_column);
		if (metadata_version == 3)
		  {
		      /* current metadata style >= v.4.0.0 */
		      sql_statement =
			  sqlite3_mprintf
			  ("CREATE TRIGGER \"%s\" BEFORE UPDATE OF \"%s\" ON \"%s\"\n"
			   "FOR EACH ROW BEGIN\n"
			   "SELECT RAISE(ROLLBACK, '%q.%q violates Geometry constraint [geom-type or SRID not allowed]')\n"
			   "WHERE (SELECT geometry_type FROM geometry_columns\n"
			   "WHERE Lower(f_table_name) = Lower(%Q) AND "
			   "Lower(f_geometry_column) = Lower(%Q)\n"
			   "AND GeometryConstraints(NEW.\"%s\", geometry_type, srid) = 1) IS NULL;\nEND",
			   quoted_trigger, quoted_column, quoted_table, p_table,
			   p_column, p_table, p_column, quoted_column);
		      free (quoted_trigger);
		      free (quoted_table);
		      free (quoted_column);
		  }
		else
		  {
		      /* legacy metadata style <= v.3.1.0 */
		      sql_statement =
			  sqlite3_mprintf
			  ("CREATE TRIGGER \"%s\" BEFORE UPDATE ON \"%s\"\n"
			   "FOR EACH ROW BEGIN\n"
			   "SELECT RAISE(ROLLBACK, '%q.%q violates Geometry constraint [geom-type or SRID not allowed]')\n"
			   "WHERE (SELECT type FROM geometry_columns\n"
			   "WHERE f_table_name = %Q AND f_geometry_column = %Q\n"
			   "AND GeometryConstraints(NEW.\"%s\", type, srid, %Q) = 1) IS NULL;\nEND",
			   quoted_trigger, quoted_table, p_table, p_column,
			   p_table, p_column, quoted_column, txt_dims);
		      free (quoted_trigger);
		      free (quoted_table);
		      free (quoted_column);
		  }
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;

		/* inserting SpatialIndex information into the linked list */
		curr_idx = malloc (sizeof (struct spatial_index_str));
		len = strlen (p_table);
		curr_idx->TableName = malloc (len + 1);
		strcpy (curr_idx->TableName, p_table);
		len = strlen (p_column);
		curr_idx->ColumnName = malloc (len + 1);
		strcpy (curr_idx->ColumnName, p_column);
		curr_idx->ValidRtree = (char) index;
		curr_idx->ValidCache = (char) cached;
		curr_idx->Next = NULL;
		if (!first_idx)
		    first_idx = curr_idx;
		if (last_idx)
		    last_idx->Next = curr_idx;
		last_idx = curr_idx;

		/* deleting the old INSERT trigger SPATIAL_INDEX [if any] */
		raw = sqlite3_mprintf ("gii_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;

		if (metadata_version == 3)
		  {
		      /* deleting the old UPDATE (timestamp) trigger [if any] */
		      raw = sqlite3_mprintf ("tmu_%s_%s", p_table, p_column);
		      quoted_trigger = gaiaDoubleQuotedSql (raw);
		      sqlite3_free (raw);
		      sql_statement =
			  sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
					   quoted_trigger);
		      free (quoted_trigger);
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;
		      /* deleting the old INSERT (timestamp) trigger [if any] */
		      raw = sqlite3_mprintf ("tmi_%s_%s", p_table, p_column);
		      quoted_trigger = gaiaDoubleQuotedSql (raw);
		      sqlite3_free (raw);
		      sql_statement =
			  sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
					   quoted_trigger);
		      free (quoted_trigger);
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;
		      /* deleting the old DELETE (timestamp) trigger [if any] */
		      raw = sqlite3_mprintf ("tmd_%s_%s", p_table, p_column);
		      quoted_trigger = gaiaDoubleQuotedSql (raw);
		      sqlite3_free (raw);
		      sql_statement =
			  sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
					   quoted_trigger);
		      free (quoted_trigger);
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;
		  }

		if (metadata_version == 3)
		  {
		      /* current metadata style >= v.4.0.0 */

		      /* inserting the new UPDATE (timestamp) trigger */
		      raw = sqlite3_mprintf ("tmu_%s_%s", p_table, p_column);
		      quoted_trigger = gaiaDoubleQuotedSql (raw);
		      sqlite3_free (raw);
		      quoted_table = gaiaDoubleQuotedSql (p_table);
		      sql_statement =
			  sqlite3_mprintf
			  ("CREATE TRIGGER \"%s\" AFTER UPDATE ON \"%s\"\n"
			   "FOR EACH ROW BEGIN\n"
			   "UPDATE geometry_columns_time SET last_update = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now')\n"
			   "WHERE Lower(f_table_name) = Lower(%Q) AND "
			   "Lower(f_geometry_column) = Lower(%Q);\nEND",
			   quoted_trigger, quoted_table, p_table, p_column);
		      free (quoted_trigger);
		      free (quoted_table);
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;

		      /* inserting the new INSERT (timestamp) trigger */
		      raw = sqlite3_mprintf ("tmi_%s_%s", p_table, p_column);
		      quoted_trigger = gaiaDoubleQuotedSql (raw);
		      sqlite3_free (raw);
		      quoted_table = gaiaDoubleQuotedSql (p_table);
		      sql_statement =
			  sqlite3_mprintf
			  ("CREATE TRIGGER \"%s\" AFTER INSERT ON \"%s\"\n"
			   "FOR EACH ROW BEGIN\n"
			   "UPDATE geometry_columns_time SET last_insert = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now')\n"
			   "WHERE Lower(f_table_name) = Lower(%Q) AND "
			   "Lower(f_geometry_column) = Lower(%Q);\nEND",
			   quoted_trigger, quoted_table, p_table, p_column);
		      free (quoted_trigger);
		      free (quoted_table);
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;

		      /* inserting the new DELETE (timestamp) trigger */
		      raw = sqlite3_mprintf ("tmd_%s_%s", p_table, p_column);
		      quoted_trigger = gaiaDoubleQuotedSql (raw);
		      sqlite3_free (raw);
		      quoted_table = gaiaDoubleQuotedSql (p_table);
		      sql_statement =
			  sqlite3_mprintf
			  ("CREATE TRIGGER \"%s\" AFTER DELETE ON \"%s\"\n"
			   "FOR EACH ROW BEGIN\n"
			   "UPDATE geometry_columns_time SET last_delete = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now')\n"
			   "WHERE Lower(f_table_name) = Lower(%Q) AND "
			   "Lower(f_geometry_column) = Lower(%Q);\nEND",
			   quoted_trigger, quoted_table, p_table, p_column);
		      free (quoted_trigger);
		      free (quoted_table);
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;
		  }

		/* deleting the old INSERT trigger SPATIAL_INDEX [if any] */
		raw = sqlite3_mprintf ("gid_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;

		/* deleting the old UPDATE trigger SPATIAL_INDEX [if any] */
		raw = sqlite3_mprintf ("giu_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;

		/* deleting the old DELETE trigger SPATIAL_INDEX [if any] */
		raw = sqlite3_mprintf ("gid_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;

		/* deleting the old INSERT trigger MBR_CACHE [if any] */
		raw = sqlite3_mprintf ("gci_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;

		/* deleting the old UPDATE trigger MBR_CACHE [if any] */
		raw = sqlite3_mprintf ("gcu_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;

		/* deleting the old UPDATE trigger MBR_CACHE [if any] */
		raw = sqlite3_mprintf ("gcd_%s_%s", p_table, p_column);
		quoted_trigger = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement =
		    sqlite3_mprintf ("DROP TRIGGER IF EXISTS main.\"%s\"",
				     quoted_trigger);
		free (quoted_trigger);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;

		if (index)
		  {
		      /* inserting the new INSERT trigger RTree */
		      if (metadata_version == 3)
			{
			    /* current metadata style >= v.4.0.0 */
			    raw =
				sqlite3_mprintf ("gii_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("idx_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER INSERT ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "DELETE FROM \"%s\" WHERE pkid=NEW.ROWID;\n"
				 "SELECT RTreeAlign(%Q, NEW.ROWID, NEW.\"%s\");\nEND",
				 quoted_trigger, quoted_table, quoted_rtree,
				 raw, quoted_column);
			    sqlite3_free (raw);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			    free (quoted_column);
			}
		      else
			{
			    /* legacy metadata style <= v.3.1.0 */
			    raw =
				sqlite3_mprintf ("gii_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("idx_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER INSERT ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "DELETE FROM \"%s\" WHERE pkid=NEW.ROWID;\n"
				 "SELECT RTreeAlign(%Q, NEW.ROWID, NEW.\"%s\");\nEND",
				 quoted_trigger, quoted_table, quoted_rtree,
				 raw, quoted_column);
			    sqlite3_free (raw);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			    free (quoted_column);
			}
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;

		      /* inserting the new UPDATE trigger RTree */
		      if (metadata_version == 3)
			{
			    /* current metadata style >= v.4.0.0 */
			    raw =
				sqlite3_mprintf ("giu_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("idx_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER UPDATE OF \"%s\" ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "DELETE FROM \"%s\" WHERE pkid=NEW.ROWID;\n"
				 "SELECT RTreeAlign(%Q, NEW.ROWID, NEW.\"%s\");\nEND",
				 quoted_trigger, quoted_column, quoted_table,
				 quoted_rtree, raw, quoted_column);
			    sqlite3_free (raw);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			    free (quoted_column);
			}
		      else
			{
			    /* legacy metadata style <= v.3.1.0 */
			    raw =
				sqlite3_mprintf ("giu_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("idx_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER UPDATE ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "DELETE FROM \"%s\" WHERE pkid=NEW.ROWID;\n"
				 "SELECT RTreeAlign(%Q, NEW.ROWID, NEW.\"%s\");\nEND",
				 quoted_trigger, quoted_table, quoted_rtree,
				 raw, quoted_column);
			    sqlite3_free (raw);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			    free (quoted_column);
			}
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;

		      /* inserting the new DELETE trigger RTree */
		      if (metadata_version == 3)
			{
			    /* current metadata style >= v.4.0.0 */
			    raw =
				sqlite3_mprintf ("gid_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("idx_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER DELETE ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "DELETE FROM \"%s\" WHERE pkid=OLD.ROWID;\nEND",
				 quoted_trigger, quoted_table, quoted_rtree);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			    free (quoted_column);
			}
		      else
			{
			    /* legacy metadata style <= v.3.1.0 */
			    raw =
				sqlite3_mprintf ("gid_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("idx_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER DELETE ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "DELETE FROM \"%s\" WHERE pkid=OLD.ROWID;\nEND",
				 quoted_trigger, quoted_table, quoted_rtree);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			    free (quoted_column);
			}
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;
		  }

		if (cached)
		  {
		      /* inserting the new INSERT trigger MBRcache */
		      if (metadata_version == 3)
			{
			    /* current metadata style >= v.4.0.0 */
			    raw =
				sqlite3_mprintf ("gci_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("cache_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER INSERT ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "INSERT INTO \"%s\" (rowid, mbr) VALUES (NEW.ROWID,\nBuildMbrFilter("
				 "MbrMinX(NEW.\"%s\"), MbrMinY(NEW.\"%s\"), MbrMaxX(NEW.\"%s\"), MbrMaxY(NEW.\"%s\")));\nEND",
				 quoted_trigger, quoted_table, quoted_rtree,
				 quoted_column, quoted_column, quoted_column,
				 quoted_column);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			    free (quoted_column);
			}
		      else
			{
			    /* legacy metadata style <= v.3.1.0 */
			    raw =
				sqlite3_mprintf ("gci_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("cache_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER INSERT ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "INSERT INTO \"%s\" (rowid, mbr) VALUES (NEW.ROWID,\nBuildMbrFilter("
				 "MbrMinX(NEW.\"%s\"), MbrMinY(NEW.\"%s\"), MbrMaxX(NEW.\"%s\"), MbrMaxY(NEW.\"%s\")));\nEND",
				 quoted_trigger, quoted_table, quoted_rtree,
				 quoted_column, quoted_column, quoted_column,
				 quoted_column);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			    free (quoted_column);
			}
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;

		      /* inserting the new UPDATE trigger MBRcache */
		      if (metadata_version == 3)
			{
			    /* current metadata style >= v.4.0.0 */
			    raw =
				sqlite3_mprintf ("gcu_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("cache_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER UPDATE OF \"%s\" ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "UPDATE \"%s\" SET mbr = BuildMbrFilter("
				 "MbrMinX(NEW.\"%s\"), MbrMinY(NEW.\"%s\"), MbrMaxX(NEW.\"%s\"), MbrMaxY(NEW.\"%s\"))\n"
				 "WHERE rowid = NEW.ROWID;\nEND",
				 quoted_trigger, quoted_column, quoted_table,
				 quoted_rtree, quoted_column, quoted_column,
				 quoted_column, quoted_column);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			    free (quoted_column);
			}
		      else
			{
			    /* legacy metadata style <= v.3.1.0 */
			    raw =
				sqlite3_mprintf ("gcu_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("cache_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER UPDATE ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "UPDATE \"%s\" SET mbr = BuildMbrFilter("
				 "MbrMinX(NEW.\"%s\"), MbrMinY(NEW.\"%s\"), MbrMaxX(NEW.\"%s\"), MbrMaxY(NEW.\"%s\"))\n"
				 "WHERE rowid = NEW.ROWID;\nEND",
				 quoted_trigger, quoted_table, quoted_rtree,
				 quoted_column, quoted_column, quoted_column,
				 quoted_column);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			    free (quoted_column);
			}
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;

		      /* inserting the new DELETE trigger MBRcache */
		      if (metadata_version == 3)
			{
			    /* current metadata style >= v.4.0.0 */
			    raw =
				sqlite3_mprintf ("gcd_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("cache_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER DELETE ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "DELETE FROM \"%s\" WHERE rowid = OLD.ROWID;\nEND",
				 quoted_trigger, quoted_table, quoted_rtree);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			}
		      else
			{
			    /* legacy metadata style <= v.3.1.0 */
			    raw =
				sqlite3_mprintf ("gcd_%s_%s", p_table,
						 p_column);
			    quoted_trigger = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    raw =
				sqlite3_mprintf ("cache_%s_%s", p_table,
						 p_column);
			    quoted_rtree = gaiaDoubleQuotedSql (raw);
			    sqlite3_free (raw);
			    quoted_table = gaiaDoubleQuotedSql (p_table);
			    quoted_column = gaiaDoubleQuotedSql (p_column);
			    sql_statement =
				sqlite3_mprintf
				("CREATE TRIGGER \"%s\" AFTER DELETE ON \"%s\"\n"
				 "FOR EACH ROW BEGIN\n"
				 "DELETE FROM \"%s\" WHERE rowid = OLD.ROWID;\nEND",
				 quoted_trigger, quoted_table, quoted_rtree);
			    free (quoted_trigger);
			    free (quoted_rtree);
			    free (quoted_table);
			}
		      ret =
			  sqlite3_exec (sqlite, sql_statement, NULL, NULL,
					&errMsg);
		      sqlite3_free (sql_statement);
		      if (ret != SQLITE_OK)
			  goto error;
		  }

	    }
      }
    ret = sqlite3_finalize (stmt);
/* now we'll adjust any related SpatialIndex as required */
    curr_idx = first_idx;
    while (curr_idx)
      {
	  if (curr_idx->ValidRtree)
	    {
		/* building RTree SpatialIndex */
		int status;
		raw = sqlite3_mprintf ("idx_%s_%s", curr_idx->TableName,
				       curr_idx->ColumnName);
		quoted_rtree = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		sql_statement = sqlite3_mprintf ("CREATE VIRTUAL TABLE \"%s\" "
						 "USING rtree(pkid, xmin, xmax, ymin, ymax)",
						 quoted_rtree);
		free (quoted_rtree);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;
		status = buildSpatialIndexEx (sqlite,
					      (unsigned char
					       *) (curr_idx->TableName),
					      curr_idx->ColumnName);
		if (status == 0)
		    ;
		else
		  {
		      if (status == -2)
			  errMsg =
			      sqlite3_mprintf
			      ("SpatialIndex error: a physical column named ROWID shadows the real ROWID");
		      else
			  errMsg =
			      sqlite3_mprintf
			      ("SpatialIndex error: unable to rebuild the T*Tree");
		      goto error;
		  }
	    }
	  if (curr_idx->ValidCache)
	    {
		/* building MbrCache SpatialIndex */
		raw = sqlite3_mprintf ("cache_%s_%s", curr_idx->TableName,
				       curr_idx->ColumnName);
		quoted_rtree = gaiaDoubleQuotedSql (raw);
		sqlite3_free (raw);
		quoted_table = gaiaDoubleQuotedSql (curr_idx->TableName);
		quoted_column = gaiaDoubleQuotedSql (curr_idx->ColumnName);
		sql_statement = sqlite3_mprintf ("CREATE VIRTUAL TABLE \"%s\" "
						 "USING MbrCache(\"%s\", \"%s\")",
						 quoted_rtree, quoted_table,
						 quoted_column);
		free (quoted_rtree);
		free (quoted_table);
		free (quoted_column);
		ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
		sqlite3_free (sql_statement);
		if (ret != SQLITE_OK)
		    goto error;
	    }
	  curr_idx = curr_idx->Next;
      }
    goto index_cleanup;
  error:
    spatialite_e ("updateTableTriggers: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
  index_cleanup:
    curr_idx = first_idx;
    while (curr_idx)
      {
	  next_idx = curr_idx->Next;
	  if (curr_idx->TableName)
	      free (curr_idx->TableName);
	  if (curr_idx->ColumnName)
	      free (curr_idx->ColumnName);
	  free (curr_idx);
	  curr_idx = next_idx;
      }
    if (p_table)
	free (p_table);
    if (p_column)
	free (p_column);
}

SPATIALITE_PRIVATE void
buildSpatialIndex (void *p_sqlite, const unsigned char *table,
		   const char *column)
{
/* DEPRECATED - always use buildSpatialIndexEx as a safer replacement */
    buildSpatialIndexEx (p_sqlite, table, column);
}

SPATIALITE_PRIVATE int
validateRowid (void *p_sqlite, const char *table)
{
/* check for tables containing a physical column named ROWID */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int rowid = 0;
    char *sql;
    int ret;
    const char *name;
    int i;
    char **results;
    int rows;
    int columns;
    char *quoted_table = gaiaDoubleQuotedSql (table);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", quoted_table);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
    sqlite3_free (sql);
    free (quoted_table);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "rowid") == 0)
		    rowid = 1;
	    }
      }
    sqlite3_free_table (results);
    if (rowid == 0)
	return 1;
    return 0;
}

SPATIALITE_PRIVATE int
buildSpatialIndexEx (void *p_sqlite, const unsigned char *table,
		     const char *column)
{
/* loading a SpatialIndex [RTree] */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    char *raw;
    char *quoted_rtree;
    char *quoted_table;
    char *quoted_column;
    char *sql_statement;
    char *errMsg = NULL;
    int ret;

    if (!validateRowid (sqlite, (const char *) table))
      {
	  /* a physical column named "rowid" shadows the real ROWID */
	  spatialite_e
	      ("buildSpatialIndex error: a physical column named ROWID shadows the real ROWID\n");
	  return -2;
      }

    raw = sqlite3_mprintf ("idx_%s_%s", table, column);
    quoted_rtree = gaiaDoubleQuotedSql (raw);
    sqlite3_free (raw);
    quoted_table = gaiaDoubleQuotedSql ((const char *) table);
    quoted_column = gaiaDoubleQuotedSql (column);
    sql_statement = sqlite3_mprintf ("INSERT INTO \"%s\" "
				     "(pkid, xmin, xmax, ymin, ymax) "
				     "SELECT ROWID, MbrMinX(\"%s\"), MbrMaxX(\"%s\"), MbrMinY(\"%s\"), MbrMaxY(\"%s\") "
				     "FROM \"%s\" WHERE MbrMinX(\"%s\") IS NOT NULL",
				     quoted_rtree, quoted_column, quoted_column,
				     quoted_column, quoted_column, quoted_table,
				     quoted_column);
    free (quoted_rtree);
    free (quoted_table);
    free (quoted_column);
    ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("buildSpatialIndex error: \"%s\"\n", errMsg);
	  sqlite3_free (errMsg);
	  return -1;
      }
    return 0;
}

SPATIALITE_PRIVATE int
getRealSQLnames (void *p_sqlite, const char *table, const char *column,
		 char **real_table, char **real_column)
{
/* attempting to retrieve the "real" table and column names (upper/lowercase) */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    char *p_table = NULL;
    char *p_column = NULL;
    char *sql_statement;
    char *quoted;
    const char *name;
    int len;
    sqlite3_stmt *stmt;
    int ret;

    sql_statement = sqlite3_mprintf ("SELECT name "
				     "FROM sqlite_master WHERE type = 'table' "
				     "AND Lower(name) = Lower(?)");
/* compiling SQL prepared statement */
    ret =
	sqlite3_prepare_v2 (sqlite, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("real_names: error %d \"%s\"\n",
			sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, table, strlen (table), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		name = (const char *) sqlite3_column_text (stmt, 0);
		len = sqlite3_column_bytes (stmt, 0);
		if (p_table)
		    free (p_table);
		p_table = malloc (len + 1);
		strcpy (p_table, name);
	    }
      }
    sqlite3_finalize (stmt);

    if (p_table == NULL)
	return 0;

    quoted = gaiaDoubleQuotedSql (p_table);
    sql_statement = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", quoted);
    free (quoted);
/* compiling SQL prepared statement */
    ret =
	sqlite3_prepare_v2 (sqlite, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("real_names: error %d \"%s\"\n",
			sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  free (p_table);
	  return 0;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		name = (const char *) sqlite3_column_text (stmt, 1);
		len = sqlite3_column_bytes (stmt, 1);
		if (strcasecmp (name, column) == 0)
		  {
		      if (p_column)
			  free (p_column);
		      p_column = malloc (len + 1);
		      strcpy (p_column, name);
		  }
	    }
      }
    sqlite3_finalize (stmt);

    if (p_column == NULL)
      {
	  free (p_table);
	  return 0;
      }

    *real_table = p_table;
    *real_column = p_column;
    return 1;
}

SPATIALITE_PRIVATE void
addLayerAttributeField (void *x_list, const char *table_name,
			const char *geometry_column, int ordinal,
			const char *column_name, int null_values,
			int integer_values, int double_values, int text_values,
			int blob_values, int null_max_size, int max_size,
			int null_int_range, void *x_integer_min,
			void *x_integer_max, int null_double_range,
			double double_min, double double_max)
{
/* adding some AttributeFiled to a VectorLayer */
    gaiaVectorLayersListPtr list = (gaiaVectorLayersListPtr) x_list;
    sqlite3_int64 integer_min = *((sqlite3_int64 *) x_integer_min);
    sqlite3_int64 integer_max = *((sqlite3_int64 *) x_integer_max);
    gaiaLayerAttributeFieldPtr fld;
    int len;
    gaiaVectorLayerPtr lyr = list->Current;
    if (lyr)
      {
	  if (strcasecmp (lyr->TableName, table_name) == 0
	      && strcasecmp (lyr->GeometryName, geometry_column) == 0)
	      goto valid_pointer;
      }
    list->Current = NULL;
    lyr = list->First;
    while (lyr)
      {
	  if (strcasecmp (lyr->TableName, table_name) == 0
	      && strcasecmp (lyr->GeometryName, geometry_column) == 0)
	    {
		list->Current = lyr;
		goto valid_pointer;
	    }
	  lyr = lyr->Next;
      }
  valid_pointer:
    lyr = list->Current;
    if (lyr == NULL)
	return;
    fld = malloc (sizeof (gaiaLayerAttributeField));
    fld->Ordinal = ordinal;
    len = strlen (column_name);
    fld->AttributeFieldName = malloc (len + 1);
    strcpy (fld->AttributeFieldName, column_name);
    fld->NullValuesCount = null_values;
    fld->IntegerValuesCount = integer_values;
    fld->DoubleValuesCount = double_values;
    fld->TextValuesCount = text_values;
    fld->BlobValuesCount = blob_values;
    fld->MaxSize = NULL;
    fld->IntRange = NULL;
    fld->DoubleRange = NULL;
    if (!null_max_size)
      {
	  fld->MaxSize = malloc (sizeof (gaiaAttributeFieldMaxSize));
	  fld->MaxSize->MaxSize = max_size;
      }
    if (!null_int_range)
      {
	  fld->IntRange = malloc (sizeof (gaiaAttributeFieldIntRange));
	  fld->IntRange->MinValue = integer_min;
	  fld->IntRange->MaxValue = integer_max;
      }
    if (!null_double_range)
      {
	  fld->DoubleRange = malloc (sizeof (gaiaAttributeFieldDoubleRange));
	  fld->DoubleRange->MinValue = double_min;
	  fld->DoubleRange->MaxValue = double_max;
      }
    fld->Next = NULL;
    if (lyr->First == NULL)
	lyr->First = fld;
    if (lyr->Last != NULL)
	lyr->Last->Next = fld;
    lyr->Last = fld;
}

SPATIALITE_PRIVATE void
addVectorLayer (void *x_list, const char *layer_type,
		const char *table_name, const char *geometry_column,
		int geometry_type, int srid, int spatial_index)
{
/* adding a Layer to a VectorLayersList */
    gaiaVectorLayersListPtr list = (gaiaVectorLayersListPtr) x_list;
    int len;
    gaiaVectorLayerPtr lyr = malloc (sizeof (gaiaVectorLayer));
    lyr->LayerType = GAIA_VECTOR_UNKNOWN;
    if (strcasecmp (layer_type, "SpatialTable") == 0)
	lyr->LayerType = GAIA_VECTOR_TABLE;
    if (strcasecmp (layer_type, "SpatialView") == 0)
	lyr->LayerType = GAIA_VECTOR_VIEW;
    if (strcasecmp (layer_type, "VirtualShape") == 0)
	lyr->LayerType = GAIA_VECTOR_VIRTUAL;
    len = strlen (table_name);
    lyr->TableName = malloc (len + 1);
    strcpy (lyr->TableName, table_name);
    len = strlen (geometry_column);
    lyr->GeometryName = malloc (len + 1);
    strcpy (lyr->GeometryName, geometry_column);
    lyr->Srid = srid;
    switch (geometry_type)
      {
      case 0:
	  lyr->GeometryType = GAIA_VECTOR_GEOMETRY;
	  lyr->Dimensions = GAIA_XY;
	  break;
      case 1000:
	  lyr->GeometryType = GAIA_VECTOR_GEOMETRY;
	  lyr->Dimensions = GAIA_XY_Z;
	  break;
      case 2000:
	  lyr->GeometryType = GAIA_VECTOR_GEOMETRY;
	  lyr->Dimensions = GAIA_XY_M;
	  break;
      case 3000:
	  lyr->GeometryType = GAIA_VECTOR_GEOMETRY;
	  lyr->Dimensions = GAIA_XY_Z_M;
	  break;
      case 1:
	  lyr->GeometryType = GAIA_VECTOR_POINT;
	  lyr->Dimensions = GAIA_XY;
	  break;
      case 1001:
	  lyr->GeometryType = GAIA_VECTOR_POINT;
	  lyr->Dimensions = GAIA_XY_Z;
	  break;
      case 2001:
	  lyr->GeometryType = GAIA_VECTOR_POINT;
	  lyr->Dimensions = GAIA_XY_M;
	  break;
      case 3001:
	  lyr->GeometryType = GAIA_VECTOR_POINT;
	  lyr->Dimensions = GAIA_XY_Z_M;
	  break;
      case 2:
	  lyr->GeometryType = GAIA_VECTOR_LINESTRING;
	  lyr->Dimensions = GAIA_XY;
	  break;
      case 1002:
	  lyr->GeometryType = GAIA_VECTOR_LINESTRING;
	  lyr->Dimensions = GAIA_XY_Z;
	  break;
      case 2002:
	  lyr->GeometryType = GAIA_VECTOR_LINESTRING;
	  lyr->Dimensions = GAIA_XY_M;
	  break;
      case 3002:
	  lyr->GeometryType = GAIA_VECTOR_LINESTRING;
	  lyr->Dimensions = GAIA_XY_Z_M;
	  break;
      case 3:
	  lyr->GeometryType = GAIA_VECTOR_POLYGON;
	  lyr->Dimensions = GAIA_XY;
	  break;
      case 1003:
	  lyr->GeometryType = GAIA_VECTOR_POLYGON;
	  lyr->Dimensions = GAIA_XY_Z;
	  break;
      case 2003:
	  lyr->GeometryType = GAIA_VECTOR_POLYGON;
	  lyr->Dimensions = GAIA_XY_M;
	  break;
      case 3003:
	  lyr->GeometryType = GAIA_VECTOR_POLYGON;
	  lyr->Dimensions = GAIA_XY_Z_M;
	  break;
      case 4:
	  lyr->GeometryType = GAIA_VECTOR_MULTIPOINT;
	  lyr->Dimensions = GAIA_XY;
	  break;
      case 1004:
	  lyr->GeometryType = GAIA_VECTOR_MULTIPOINT;
	  lyr->Dimensions = GAIA_XY_Z;
	  break;
      case 2004:
	  lyr->GeometryType = GAIA_VECTOR_MULTIPOINT;
	  lyr->Dimensions = GAIA_XY_M;
	  break;
      case 3004:
	  lyr->GeometryType = GAIA_VECTOR_MULTIPOINT;
	  lyr->Dimensions = GAIA_XY_Z_M;
	  break;
      case 5:
	  lyr->GeometryType = GAIA_VECTOR_MULTILINESTRING;
	  lyr->Dimensions = GAIA_XY;
	  break;
      case 1005:
	  lyr->GeometryType = GAIA_VECTOR_MULTILINESTRING;
	  lyr->Dimensions = GAIA_XY_Z;
	  break;
      case 2005:
	  lyr->GeometryType = GAIA_VECTOR_MULTILINESTRING;
	  lyr->Dimensions = GAIA_XY_M;
	  break;
      case 3005:
	  lyr->GeometryType = GAIA_VECTOR_MULTILINESTRING;
	  lyr->Dimensions = GAIA_XY_Z_M;
	  break;
      case 6:
	  lyr->GeometryType = GAIA_VECTOR_MULTIPOLYGON;
	  lyr->Dimensions = GAIA_XY;
	  break;
      case 1006:
	  lyr->GeometryType = GAIA_VECTOR_MULTIPOLYGON;
	  lyr->Dimensions = GAIA_XY_Z;
	  break;
      case 2006:
	  lyr->GeometryType = GAIA_VECTOR_MULTIPOLYGON;
	  lyr->Dimensions = GAIA_XY_M;
	  break;
      case 3006:
	  lyr->GeometryType = GAIA_VECTOR_MULTIPOLYGON;
	  lyr->Dimensions = GAIA_XY_Z_M;
	  break;
      case 7:
	  lyr->GeometryType = GAIA_VECTOR_GEOMETRYCOLLECTION;
	  lyr->Dimensions = GAIA_XY;
	  break;
      case 1007:
	  lyr->GeometryType = GAIA_VECTOR_GEOMETRYCOLLECTION;
	  lyr->Dimensions = GAIA_XY_Z;
	  break;
      case 2007:
	  lyr->GeometryType = GAIA_VECTOR_GEOMETRYCOLLECTION;
	  lyr->Dimensions = GAIA_XY_M;
	  break;
      case 3007:
	  lyr->GeometryType = GAIA_VECTOR_GEOMETRYCOLLECTION;
	  lyr->Dimensions = GAIA_XY_Z_M;
	  break;
      default:
	  lyr->GeometryType = GAIA_VECTOR_UNKNOWN;
	  lyr->Dimensions = GAIA_VECTOR_UNKNOWN;
	  break;
      };
    switch (spatial_index)
      {
      case 0:
	  lyr->SpatialIndex = GAIA_SPATIAL_INDEX_NONE;
	  break;
      case 1:
	  lyr->SpatialIndex = GAIA_SPATIAL_INDEX_RTREE;
	  break;
      case 2:
	  lyr->SpatialIndex = GAIA_SPATIAL_INDEX_MBRCACHE;
	  break;
      default:
	  lyr->SpatialIndex = GAIA_VECTOR_UNKNOWN;
	  break;
      };
    lyr->ExtentInfos = NULL;
    lyr->AuthInfos = NULL;
    lyr->First = NULL;
    lyr->Last = NULL;
    lyr->Next = NULL;
    list->Current = NULL;
    if (list->First == NULL)
	list->First = lyr;
    if (list->Last != NULL)
	list->Last->Next = lyr;
    list->Last = lyr;
}

SPATIALITE_PRIVATE void
addVectorLayerExtent (void *x_list, const char *table_name,
		      const char *geometry_column, int count, double min_x,
		      double min_y, double max_x, double max_y)
{
/* appending a LayerExtent object to the corresponding VectorLayer */
    gaiaVectorLayersListPtr list = (gaiaVectorLayersListPtr) x_list;
    gaiaVectorLayerPtr lyr = list->First;
    while (lyr)
      {
	  if (strcasecmp (lyr->TableName, table_name) == 0
	      && strcasecmp (lyr->GeometryName, geometry_column) == 0)
	    {
		lyr->ExtentInfos = malloc (sizeof (gaiaLayerExtent));
		lyr->ExtentInfos->Count = count;
		lyr->ExtentInfos->MinX = min_x;
		lyr->ExtentInfos->MinY = min_y;
		lyr->ExtentInfos->MaxX = max_x;
		lyr->ExtentInfos->MaxY = max_y;
		return;
	    }
	  lyr = lyr->Next;
      }
}

static void
addVectorLayerAuth (gaiaVectorLayersListPtr list, const char *table_name,
		    const char *geometry_column, int read_only, int hidden)
{
/* appending a LayerAuth object to the corresponding VectorLayer */
    gaiaVectorLayerPtr lyr = list->First;
    while (lyr)
      {
	  if (strcasecmp (lyr->TableName, table_name) == 0
	      && strcasecmp (lyr->GeometryName, geometry_column) == 0)
	    {
		gaiaLayerAuthPtr auth = malloc (sizeof (gaiaLayerAuth));
		lyr->AuthInfos = auth;
		auth->IsReadOnly = read_only;
		auth->IsHidden = hidden;
		return;
	    }
	  lyr = lyr->Next;
      }
}

static void
freeLayerAttributeField (gaiaLayerAttributeFieldPtr fld)
{
/* destroying an AttributeField */
    if (fld->AttributeFieldName)
	free (fld->AttributeFieldName);
    if (fld->MaxSize)
	free (fld->MaxSize);
    if (fld->IntRange)
	free (fld->IntRange);
    if (fld->DoubleRange)
	free (fld->DoubleRange);
    free (fld);
}

static void
freeVectorLayer (gaiaVectorLayerPtr lyr)
{
/* destroyng a VectorLayer object */
    gaiaLayerAttributeFieldPtr fld;
    gaiaLayerAttributeFieldPtr fldn;
    if (!lyr)
	return;
    if (lyr->TableName)
	free (lyr->TableName);
    if (lyr->GeometryName)
	free (lyr->GeometryName);
    if (lyr->ExtentInfos)
	free (lyr->ExtentInfos);
    if (lyr->AuthInfos)
	free (lyr->AuthInfos);
    fld = lyr->First;
    while (fld)
      {
	  fldn = fld->Next;
	  freeLayerAttributeField (fld);
	  fld = fldn;
      }
    free (lyr);
}

SPATIALITE_DECLARE void
gaiaFreeVectorLayersList (gaiaVectorLayersListPtr list)
{
/* destroying a VectorLayersList object */
    gaiaVectorLayerPtr lyr;
    gaiaVectorLayerPtr lyr_n;
    if (!list)
	return;
    lyr = list->First;
    while (lyr)
      {
	  /* destroying each layer */
	  lyr_n = lyr->Next;
	  freeVectorLayer (lyr);
	  lyr = lyr_n;
      }
    free (list);
}

static int
gaiaGetVectorLayersList_v4 (sqlite3 * handle, const char *table,
			    const char *geometry, gaiaVectorLayersListPtr list)
{
/* creating a VectorLayersList object - v.4.0.0 DB layout */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    int error = 0;

/* querying the vector_layers view */
    if (table == NULL)
	sql =
	    sqlite3_mprintf
	    ("SELECT layer_type, table_name, geometry_column, geometry_type, "
	     "srid, spatial_index_enabled FROM vector_layers");
    else
      {
	  if (geometry == NULL)
	      sql =
		  sqlite3_mprintf
		  ("SELECT layer_type, table_name, geometry_column, geometry_type, "
		   "srid, spatial_index_enabled FROM vector_layers "
		   "WHERE Lower(table_name) = Lower(%Q)", table);
	  else
	      sql =
		  sqlite3_mprintf
		  ("SELECT layer_type, table_name, geometry_column, geometry_type, "
		   "srid, spatial_index_enabled FROM vector_layers "
		   "WHERE Lower(table_name) = Lower(%Q) AND "
		   "Lower(geometry_column) = Lower(%Q)", table, geometry);
      }
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  error = 1;
	  goto stop;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a VectorLayer row */
		const char *layer_type =
		    (const char *) sqlite3_column_text (stmt, 0);
		const char *table_name =
		    (const char *) sqlite3_column_text (stmt, 1);
		const char *geometry_column =
		    (const char *) sqlite3_column_text (stmt, 2);
		int geometry_type = sqlite3_column_int (stmt, 3);
		int srid = sqlite3_column_int (stmt, 4);
		int spatial_index = sqlite3_column_int (stmt, 5);
		addVectorLayer (list, layer_type, table_name, geometry_column,
				geometry_type, srid, spatial_index);
	    }
	  else
	      error = 1;
      }
    ret = sqlite3_finalize (stmt);

  stop:
    if (error)
	return 0;

/* querying the vector_layers_statistics view */
    if (table == NULL)
	sql =
	    sqlite3_mprintf
	    ("SELECT table_name, geometry_column, row_count, extent_min_x, "
	     "extent_min_y, extent_max_x, extent_max_y FROM vector_layers_statistics");
    else
      {
	  if (geometry == NULL)
	      sql =
		  sqlite3_mprintf
		  ("SELECT table_name, geometry_column, row_count, extent_min_x, "
		   "extent_min_y, extent_max_x, extent_max_y FROM vector_layers_statistics "
		   "WHERE Lower(table_name) = Lower(%Q)", table);
	  else
	      sql =
		  sqlite3_mprintf
		  ("SELECT table_name, geometry_column, row_count, extent_min_x, "
		   "extent_min_y, extent_max_x, extent_max_y FROM vector_layers_statistics "
		   "WHERE Lower(table_name) = Lower(%Q) AND "
		   "Lower(geometry_column) = Lower(%Q)", table, geometry);
      }
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  error = 1;
	  goto stop2;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a VectorLayer row */
		int is_null = 0;
		const char *table_name =
		    (const char *) sqlite3_column_text (stmt, 0);
		const char *geometry_column =
		    (const char *) sqlite3_column_text (stmt, 1);
		int count = 0;
		double min_x = DBL_MAX;
		double min_y = DBL_MAX;
		double max_x = 0.0 - DBL_MAX;
		double max_y = 0.0 - DBL_MAX;
		if (sqlite3_column_type (stmt, 2) == SQLITE_NULL)
		    is_null = 1;
		else
		    count = sqlite3_column_int (stmt, 2);
		if (sqlite3_column_type (stmt, 3) == SQLITE_NULL)
		    is_null = 1;
		else
		    min_x = sqlite3_column_double (stmt, 3);
		if (sqlite3_column_type (stmt, 4) == SQLITE_NULL)
		    is_null = 1;
		else
		    min_y = sqlite3_column_double (stmt, 4);
		if (sqlite3_column_type (stmt, 5) == SQLITE_NULL)
		    is_null = 1;
		else
		    max_x = sqlite3_column_double (stmt, 5);
		if (sqlite3_column_type (stmt, 6) == SQLITE_NULL)
		    is_null = 1;
		else
		    max_y = sqlite3_column_double (stmt, 6);
		if (!is_null)
		    addVectorLayerExtent (list, table_name, geometry_column,
					  count, min_x, min_y, max_x, max_y);
	    }
      }
    ret = sqlite3_finalize (stmt);

  stop2:
/* querying the vector_layers_auth view */
    if (table == NULL)
	sql =
	    sqlite3_mprintf
	    ("SELECT table_name, geometry_column, read_only, hidden "
	     "FROM vector_layers_auth");
    else
      {
	  if (geometry == NULL)
	      sql =
		  sqlite3_mprintf
		  ("SELECT table_name, geometry_column, read_only, hidden "
		   "FROM vector_layers_auth WHERE Lower(table_name) = Lower(%Q)",
		   table, geometry);
	  else
	      sql =
		  sqlite3_mprintf
		  ("SELECT table_name, geometry_column, read_only, hidden "
		   "FROM vector_layers_auth "
		   "WHERE Lower(table_name) = Lower(%Q) AND "
		   "Lower(geometry_column) = Lower(%Q)", table, geometry);
      }
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  error = 1;
	  goto stop3;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a VectorLayer row */
		int is_null = 0;
		const char *table_name =
		    (const char *) sqlite3_column_text (stmt, 0);
		const char *geometry_column =
		    (const char *) sqlite3_column_text (stmt, 1);
		int read_only;
		int hidden;
		if (sqlite3_column_type (stmt, 2) == SQLITE_NULL)
		    is_null = 1;
		else
		    read_only = sqlite3_column_int (stmt, 2);
		if (sqlite3_column_type (stmt, 3) == SQLITE_NULL)
		    is_null = 1;
		else
		    hidden = sqlite3_column_int (stmt, 3);
		if (!is_null)
		    addVectorLayerAuth (list, table_name, geometry_column,
					read_only, hidden);
	    }
      }
    ret = sqlite3_finalize (stmt);

  stop3:
/* querying the vector_layers_field_infos view */
    if (table == NULL)
      {
	  /* if no "table" is set, we'll never return AttributeField Infos */
	  goto stop4;
      }
    else
      {
	  if (geometry == NULL)
	      sql =
		  sqlite3_mprintf
		  ("SELECT table_name, geometry_column, ordinal, column_name, "
		   "null_values, integer_values, double_values, text_values, blob_values,"
		   "max_size, integer_min, integer_max, double_min, double_max "
		   "FROM vector_layers_field_infos WHERE Lower(table_name) = Lower(%Q)",
		   table, geometry);
	  else
	      sql =
		  sqlite3_mprintf
		  ("SELECT table_name, geometry_column, ordinal, column_name, "
		   "null_values, integer_values, double_values, text_values, blob_values,"
		   "max_size, integer_min, integer_max, double_min, double_max "
		   "FROM vector_layers_field_infos "
		   "WHERE Lower(table_name) = Lower(%Q) AND "
		   "Lower(geometry_column) = Lower(%Q)", table, geometry);
      }
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  error = 1;
	  goto stop4;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a VectorLayer row */
		int null_max_size = 0;
		int null_int_range = 0;
		int null_double_range = 0;
		int max_size = 0;
		sqlite3_int64 integer_min;
		sqlite3_int64 integer_max;
		double double_min = DBL_MAX;
		double double_max = 0.0 - DBL_MAX;
		const char *table_name =
		    (const char *) sqlite3_column_text (stmt, 0);
		const char *geometry_column =
		    (const char *) sqlite3_column_text (stmt, 1);
		int ordinal = sqlite3_column_int (stmt, 2);
		const char *column_name =
		    (const char *) sqlite3_column_text (stmt, 3);
		int null_values = sqlite3_column_int (stmt, 4);
		int integer_values = sqlite3_column_int (stmt, 5);
		int double_values = sqlite3_column_int (stmt, 6);
		int text_values = sqlite3_column_int (stmt, 7);
		int blob_values = sqlite3_column_int (stmt, 8);
		if (sqlite3_column_type (stmt, 9) == SQLITE_NULL)
		    null_max_size = 1;
		else
		    max_size = sqlite3_column_int (stmt, 9);
		if (sqlite3_column_type (stmt, 10) == SQLITE_NULL
		    || sqlite3_column_type (stmt, 11) == SQLITE_NULL)
		    null_int_range = 1;
		else
		  {
		      integer_min = sqlite3_column_int64 (stmt, 10);
		      integer_max = sqlite3_column_int64 (stmt, 11);
		  }
		if (sqlite3_column_type (stmt, 12) == SQLITE_NULL
		    || sqlite3_column_type (stmt, 13) == SQLITE_NULL)
		    null_double_range = 1;
		else
		  {
		      double_min = sqlite3_column_double (stmt, 12);
		      double_max = sqlite3_column_double (stmt, 13);
		  }
		addLayerAttributeField (list, table_name, geometry_column,
					ordinal, column_name, null_values,
					integer_values, double_values,
					text_values, blob_values, null_max_size,
					max_size, null_int_range, &integer_min,
					&integer_max, null_double_range,
					double_min, double_max);
	    }
      }
    ret = sqlite3_finalize (stmt);

  stop4:
    if (error)
	return 0;
    return 1;
}

static int
get_table_layers_legacy (sqlite3 * handle, const char *table,
			 const char *geometry, gaiaVectorLayersListPtr list)
{
/* fetching Table-based geometries */
    int ret;
    char *sql;
    const char *name;
    int i;
    char **results;
    int rows;
    int columns;
    int f_table = 0;
    int f_geometry = 0;
    int type = 0;
    int dims = 0;
    int srid = 0;
    int spatial_index = 0;
    sqlite3_stmt *stmt;
    int error = 0;

/* checking the GEOMETRY_COLUMN table */
    sql = "PRAGMA table_info(geometry_columns)";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "f_table_name") == 0)
		    f_table = 1;
		if (strcasecmp (name, "f_geometry_column") == 0)
		    f_geometry = 1;
		if (strcasecmp (name, "type") == 0)
		    type = 1;
		if (strcasecmp (name, "coord_dimension") == 0)
		    dims = 1;
		if (strcasecmp (name, "srid") == 0)
		    srid = 1;
		if (strcasecmp (name, "spatial_index_enabled") == 0)
		    spatial_index = 1;
	    }
      }
    sqlite3_free_table (results);
    if (f_table && f_geometry && type && dims && srid && spatial_index)
	;
    else
	return 1;

    if (table == NULL)
	sql =
	    sqlite3_mprintf
	    ("SELECT f_table_name, f_geometry_column, type, coord_dimension, "
	     "srid, spatial_index_enabled FROM geometry_columns");
    else
      {
	  if (geometry == NULL)
	      sql =
		  sqlite3_mprintf
		  ("SELECT f_table_name, f_geometry_column, type, coord_dimension, "
		   "srid, spatial_index_enabled FROM geometry_columns "
		   "WHERE Lower(f_table_name) = Lower(%Q)", table);
	  else
	      sql =
		  sqlite3_mprintf
		  ("SELECT f_table_name, f_geometry_column, type, coord_dimension, "
		   "srid, spatial_index_enabled FROM geometry_columns "
		   "WHERE Lower(f_table_name) = Lower(%Q) AND "
		   "Lower(f_geometry_column) = Lower(%Q)", table, geometry);
      }
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  error = 1;
	  goto stop;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a VectorLayer row */
		const char *table_name =
		    (const char *) sqlite3_column_text (stmt, 0);
		const char *geometry_column =
		    (const char *) sqlite3_column_text (stmt, 1);
		const char *type = (const char *) sqlite3_column_text (stmt, 2);
		const char *dims = (const char *) sqlite3_column_text (stmt, 3);
		int srid = sqlite3_column_int (stmt, 4);
		int spatial_index = sqlite3_column_int (stmt, 5);
		int geometry_type = -1;
		if (strcasecmp (type, "POINT") == 0)
		    geometry_type = 1;
		if (strcasecmp (type, "LINESTRING") == 0)
		    geometry_type = 2;
		if (strcasecmp (type, "POLYGON") == 0)
		    geometry_type = 3;
		if (strcasecmp (type, "MULTIPOINT") == 0)
		    geometry_type = 4;
		if (strcasecmp (type, "MULTILINESTRING") == 0)
		    geometry_type = 5;
		if (strcasecmp (type, "MULTIPOLYGON") == 0)
		    geometry_type = 6;
		if (strcasecmp (type, "GEOMETRYCOLLECTION") == 0)
		    geometry_type = 7;
		if (strcasecmp (type, "GEOMETRY") == 0)
		    geometry_type = 0;
		if (strcasecmp (dims, "XYZ") == 0
		    || strcasecmp (dims, "3") == 0)
		    geometry_type += 1000;
		if (strcasecmp (dims, "XYM") == 0)
		    geometry_type += 2000;
		if (strcasecmp (dims, "XYZM") == 0
		    || strcasecmp (dims, "4") == 0)
		    geometry_type += 3000;
		addVectorLayer (list, "SpatialTable", table_name,
				geometry_column, geometry_type, srid,
				spatial_index);
	    }
	  else
	      error = 1;
      }
    ret = sqlite3_finalize (stmt);

  stop:
    if (error)
	return 0;
    return 1;
}

static int
get_view_layers_legacy (sqlite3 * handle, const char *table,
			const char *geometry, gaiaVectorLayersListPtr list)
{
/* fetching View-based geometries */
    int ret;
    char *sql;
    const char *name;
    int i;
    char **results;
    int rows;
    int columns;
    int f_table = 0;
    int f_geometry = 0;
    int type = 0;
    int dims = 0;
    int srid = 0;
    int spatial_index = 0;
    int view_name = 0;
    int view_geometry = 0;
    int fvw_table = 0;
    int fvw_geometry = 0;
    sqlite3_stmt *stmt;
    int error = 0;

/* checking the GEOMETRY_COLUMN table */
    sql = "PRAGMA table_info(geometry_columns)";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "f_table_name") == 0)
		    f_table = 1;
		if (strcasecmp (name, "f_geometry_column") == 0)
		    f_geometry = 1;
		if (strcasecmp (name, "type") == 0)
		    type = 1;
		if (strcasecmp (name, "coord_dimension") == 0)
		    dims = 1;
		if (strcasecmp (name, "srid") == 0)
		    srid = 1;
		if (strcasecmp (name, "spatial_index_enabled") == 0)
		    spatial_index = 1;
	    }
      }
    sqlite3_free_table (results);
    if (f_table && f_geometry && type && dims && srid && spatial_index)
	;
    else
	return 1;

/* checking the VIEWS_GEOMETRY_COLUMN table */
    sql = "PRAGMA table_info(views_geometry_columns)";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "view_name") == 0)
		    view_name = 1;
		if (strcasecmp (name, "view_geometry") == 0)
		    view_geometry = 1;
		if (strcasecmp (name, "f_table_name") == 0)
		    fvw_table = 1;
		if (strcasecmp (name, "f_geometry_column") == 0)
		    fvw_geometry = 1;
	    }
      }
    sqlite3_free_table (results);
    if (view_name && view_geometry && fvw_table && fvw_geometry)
	;
    else
	return 1;

    if (table == NULL)
	sql =
	    sqlite3_mprintf
	    ("SELECT a.view_name, a.view_geometry, b.type, b.coord_dimension, "
	     "b.srid, b.spatial_index_enabled FROM views_geometry_columns AS a "
	     "JOIN geometry_columns AS b ON (Lower(a.f_table_name) = Lower(b.f_table_name) "
	     "AND Lower(a.f_geometry_column) = Lower(b.f_geometry_column))");
    else
      {
	  if (geometry == NULL)
	      sql =
		  sqlite3_mprintf
		  ("SELECT a.view_name, a.view_geometry, b.type, b.coord_dimension, "
		   "b.srid, b.spatial_index_enabled FROM views_geometry_columns AS a "
		   "JOIN geometry_columns AS b ON (Lower(a.f_table_name) = Lower(b.f_table_name) "
		   "AND Lower(a.f_geometry_column) = Lower(b.f_geometry_column)) "
		   "WHERE Lower(a.view_name) = Lower(%Q)", table);
	  else
	      sql =
		  sqlite3_mprintf
		  ("SELECT a.view_name, a.view_geometry, b.type, b.coord_dimension, "
		   "b.srid, b.spatial_index_enabled FROM views_geometry_columns AS a "
		   "JOIN geometry_columns AS b ON (Lower(a.f_table_name) = Lower(b.f_table_name) "
		   "AND Lower(a.f_geometry_column) = Lower(b.f_geometry_column)) "
		   "WHERE Lower(a.view_name) = Lower(%Q) AND "
		   "Lower(a.view_geometry) = Lower(%Q)", table, geometry);
      }
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  error = 1;
	  goto stop;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a VectorLayer row */
		const char *table_name =
		    (const char *) sqlite3_column_text (stmt, 0);
		const char *geometry_column =
		    (const char *) sqlite3_column_text (stmt, 1);
		const char *type = (const char *) sqlite3_column_text (stmt, 2);
		const char *dims = (const char *) sqlite3_column_text (stmt, 3);
		int srid = sqlite3_column_int (stmt, 4);
		int spatial_index = sqlite3_column_int (stmt, 5);
		int geometry_type = -1;
		if (strcasecmp (type, "POINT") == 0)
		    geometry_type = 1;
		if (strcasecmp (type, "LINESTRING") == 0)
		    geometry_type = 2;
		if (strcasecmp (type, "POLYGON") == 0)
		    geometry_type = 3;
		if (strcasecmp (type, "MULTIPOINT") == 0)
		    geometry_type = 4;
		if (strcasecmp (type, "MULTILINESTRING") == 0)
		    geometry_type = 5;
		if (strcasecmp (type, "MULTIPOLYGON") == 0)
		    geometry_type = 6;
		if (strcasecmp (type, "GEOMETRYCOLLECTION") == 0)
		    geometry_type = 7;
		if (strcasecmp (type, "GEOMETRY") == 0)
		    geometry_type = 0;
		if (strcasecmp (dims, "XYZ") == 0
		    || strcasecmp (dims, "3") == 0)
		    geometry_type += 1000;
		if (strcasecmp (dims, "XYM") == 0)
		    geometry_type += 2000;
		if (strcasecmp (dims, "XYZM") == 0
		    || strcasecmp (dims, "4") == 0)
		    geometry_type += 3000;
		addVectorLayer (list, "SpatialView", table_name,
				geometry_column, geometry_type, srid,
				spatial_index);
	    }
	  else
	      error = 1;
      }
    ret = sqlite3_finalize (stmt);

  stop:
    if (error)
	return 0;
    return 1;
}

static int
get_table_extent_legacy (sqlite3 * handle, const char *table,
			 const char *geometry, gaiaVectorLayersListPtr list)
{
/* fetching Table-based extents */
    int ret;
    char *sql;
    const char *name;
    int i;
    char **results;
    int rows;
    int columns;
    int f_table = 0;
    int f_geometry = 0;
    int count = 0;
    int minx = 0;
    int miny = 0;
    int maxx = 0;
    int maxy = 0;
    sqlite3_stmt *stmt;

/* checking the LAYER_STATISTICS table */
    sql = "PRAGMA table_info(layer_statistics)";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "table_name") == 0)
		    f_table = 1;
		if (strcasecmp (name, "geometry_column") == 0)
		    f_geometry = 1;
		if (strcasecmp (name, "row_count") == 0)
		    count = 1;
		if (strcasecmp (name, "extent_min_x") == 0)
		    minx = 1;
		if (strcasecmp (name, "extent_min_y") == 0)
		    miny = 1;
		if (strcasecmp (name, "extent_max_x") == 0)
		    maxx = 1;
		if (strcasecmp (name, "extent_max_y") == 0)
		    maxy = 1;
	    }
      }
    sqlite3_free_table (results);
    if (f_table && f_geometry && count && minx && miny && maxx && maxy)
	;
    else
	return 1;

/* querying the layer_statistics table */
    if (table == NULL)
	sql =
	    sqlite3_mprintf
	    ("SELECT table_name, geometry_column, row_count, extent_min_x, "
	     "extent_min_y, extent_max_x, extent_max_y FROM layer_statistics");
    else
      {
	  if (geometry == NULL)
	      sql =
		  sqlite3_mprintf
		  ("SELECT table_name, geometry_column, row_count, extent_min_x, "
		   "extent_min_y, extent_max_x, extent_max_y FROM layer_statistics "
		   "WHERE Lower(table_name) = Lower(%Q)", table);
	  else
	      sql =
		  sqlite3_mprintf
		  ("SELECT table_name, geometry_column, row_count, extent_min_x, "
		   "extent_min_y, extent_max_x, extent_max_y FROM layer_statistics "
		   "WHERE Lower(table_name) = Lower(%Q) AND "
		   "Lower(geometry_column) = Lower(%Q)", table, geometry);
      }
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto stop;
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a VectorLayer row */
		int is_null = 0;
		const char *table_name =
		    (const char *) sqlite3_column_text (stmt, 0);
		const char *geometry_column =
		    (const char *) sqlite3_column_text (stmt, 1);
		int count = 0;
		double min_x = DBL_MAX;
		double min_y = DBL_MAX;
		double max_x = 0.0 - DBL_MAX;
		double max_y = 0.0 - DBL_MAX;
		if (sqlite3_column_type (stmt, 2) == SQLITE_NULL)
		    is_null = 1;
		else
		    count = sqlite3_column_int (stmt, 2);
		if (sqlite3_column_type (stmt, 3) == SQLITE_NULL)
		    is_null = 1;
		else
		    min_x = sqlite3_column_double (stmt, 3);
		if (sqlite3_column_type (stmt, 4) == SQLITE_NULL)
		    is_null = 1;
		else
		    min_y = sqlite3_column_double (stmt, 4);
		if (sqlite3_column_type (stmt, 5) == SQLITE_NULL)
		    is_null = 1;
		else
		    max_x = sqlite3_column_double (stmt, 5);
		if (sqlite3_column_type (stmt, 6) == SQLITE_NULL)
		    is_null = 1;
		else
		    max_y = sqlite3_column_double (stmt, 6);
		if (!is_null)
		    addVectorLayerExtent (list, table_name, geometry_column,
					  count, min_x, min_y, max_x, max_y);
	    }
      }
    ret = sqlite3_finalize (stmt);
  stop:
    return 1;
}

static int
get_view_extent_legacy (sqlite3 * handle, const char *table,
			const char *geometry, gaiaVectorLayersListPtr list)
{
/* fetching View-based extents */
    int ret;
    char *sql;
    const char *name;
    int i;
    char **results;
    int rows;
    int columns;
    int f_table = 0;
    int f_geometry = 0;
    int count = 0;
    int minx = 0;
    int miny = 0;
    int maxx = 0;
    int maxy = 0;
    sqlite3_stmt *stmt;

/* checking the VIEWS_LAYER_STATISTICS table */
    sql = "PRAGMA table_info(views_layer_statistics)";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "view_name") == 0)
		    f_table = 1;
		if (strcasecmp (name, "view_geometry") == 0)
		    f_geometry = 1;
		if (strcasecmp (name, "row_count") == 0)
		    count = 1;
		if (strcasecmp (name, "extent_min_x") == 0)
		    minx = 1;
		if (strcasecmp (name, "extent_min_y") == 0)
		    miny = 1;
		if (strcasecmp (name, "extent_max_x") == 0)
		    maxx = 1;
		if (strcasecmp (name, "extent_max_y") == 0)
		    maxy = 1;
	    }
      }
    sqlite3_free_table (results);
    if (f_table && f_geometry && count && minx && miny && maxx && maxy)
	;
    else
	return 1;

/* querying the views_layer_statistics table */
    if (table == NULL)
	sql =
	    sqlite3_mprintf
	    ("SELECT view_name, view_geometry, row_count, extent_min_x, "
	     "extent_min_y, extent_max_x, extent_max_y FROM views_layer_statistics");
    else
      {
	  if (geometry == NULL)
	      sql =
		  sqlite3_mprintf
		  ("SELECT view_name, view_geometry, row_count, extent_min_x, "
		   "extent_min_y, extent_max_x, extent_max_y FROM views_layer_statistics "
		   "WHERE Lower(view_name) = Lower(%Q)", table);
	  else
	      sql =
		  sqlite3_mprintf
		  ("SELECT view_name, view_geometry, row_count, extent_min_x, "
		   "extent_min_y, extent_max_x, extent_max_y FROM views_layer_statistics "
		   "WHERE Lower(view_name) = Lower(%Q) AND "
		   "Lower(view_geometry) = Lower(%Q)", table, geometry);
      }
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto stop;
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a VectorLayer row */
		int is_null = 0;
		const char *table_name =
		    (const char *) sqlite3_column_text (stmt, 0);
		const char *geometry_column =
		    (const char *) sqlite3_column_text (stmt, 1);
		int count = 0;
		double min_x = DBL_MAX;
		double min_y = DBL_MAX;
		double max_x = 0.0 - DBL_MAX;
		double max_y = 0.0 - DBL_MAX;
		if (sqlite3_column_type (stmt, 2) == SQLITE_NULL)
		    is_null = 1;
		else
		    count = sqlite3_column_int (stmt, 2);
		if (sqlite3_column_type (stmt, 3) == SQLITE_NULL)
		    is_null = 1;
		else
		    min_x = sqlite3_column_double (stmt, 3);
		if (sqlite3_column_type (stmt, 4) == SQLITE_NULL)
		    is_null = 1;
		else
		    min_y = sqlite3_column_double (stmt, 4);
		if (sqlite3_column_type (stmt, 5) == SQLITE_NULL)
		    is_null = 1;
		else
		    max_x = sqlite3_column_double (stmt, 5);
		if (sqlite3_column_type (stmt, 6) == SQLITE_NULL)
		    is_null = 1;
		else
		    max_y = sqlite3_column_double (stmt, 6);
		if (!is_null)
		    addVectorLayerExtent (list, table_name, geometry_column,
					  count, min_x, min_y, max_x, max_y);
	    }
      }
    ret = sqlite3_finalize (stmt);
  stop:
    return 1;
}

static int
get_table_auth_legacy (sqlite3 * handle, const char *table,
		       const char *geometry, gaiaVectorLayersListPtr list)
{
/* fetching Table-based Auth */
    int ret;
    char *sql;
    const char *name;
    int i;
    char **results;
    int rows;
    int columns;
    int f_table = 0;
    int f_geometry = 0;
    int read_only = 0;
    int hidden = 0;
    sqlite3_stmt *stmt;

/* checking the GEOMETRY_COLUMNS_AUTH table */
    sql = "PRAGMA table_info(geometry_columns_auth)";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "f_table_name") == 0)
		    f_table = 1;
		if (strcasecmp (name, "f_geometry_column") == 0)
		    f_geometry = 1;
		if (strcasecmp (name, "read_only") == 0)
		    read_only = 1;
		if (strcasecmp (name, "hidden") == 0)
		    hidden = 1;
	    }
      }
    sqlite3_free_table (results);
    if (f_table && f_geometry && read_only && hidden)
	;
    else
	return 1;

/* querying the geometry_columns_auth table */
    if (table == NULL)
	sql =
	    sqlite3_mprintf
	    ("SELECT f_table_name, f_geometry_column, read_only, hidden "
	     "FROM geometry_columns_auth");
    else
      {
	  if (geometry == NULL)
	      sql =
		  sqlite3_mprintf
		  ("SELECT f_table_name, f_geometry_column, read_only, hidden "
		   "FROM geometry_columns_auth "
		   "WHERE Lower(f_table_name) = Lower(%Q)", table);
	  else
	      sql =
		  sqlite3_mprintf
		  ("SELECT f_table_name, f_geometry_column, read_only, hidden "
		   "FROM geometry_columns_auth "
		   "WHERE Lower(f_table_name) = Lower(%Q) AND "
		   "Lower(f_geometry_column) = Lower(%Q)", table, geometry);
      }
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto stop;
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a VectorLayer row */
		int is_null = 0;
		const char *table_name =
		    (const char *) sqlite3_column_text (stmt, 0);
		const char *geometry_column =
		    (const char *) sqlite3_column_text (stmt, 1);
		int read_only;
		int hidden;
		if (sqlite3_column_type (stmt, 2) == SQLITE_NULL)
		    is_null = 1;
		else
		    read_only = sqlite3_column_int (stmt, 2);
		if (sqlite3_column_type (stmt, 3) == SQLITE_NULL)
		    is_null = 1;
		else
		    hidden = sqlite3_column_int (stmt, 3);
		if (!is_null)
		    addVectorLayerAuth (list, table_name, geometry_column,
					read_only, hidden);
	    }
      }
    ret = sqlite3_finalize (stmt);
  stop:
    return 1;
}

static int
compute_table_fields_statistics (sqlite3 * handle, const char *table,
				 const char *geometry,
				 gaiaVectorLayersListPtr list)
{
/* computing field statistics - legacy */
    gaiaVectorLayerPtr lyr;
    lyr = list->First;
    while (lyr)
      {
	  /* testing if the Table/Geometry is already defined */
	  if (geometry == NULL)
	    {
		if (strcasecmp (lyr->TableName, table) == 0)
		  {
		      if (!doComputeFieldInfos
			  (handle, lyr->TableName, lyr->GeometryName,
			   SPATIALITE_STATISTICS_LEGACY, lyr))
			  return 0;
		  }
	    }
	  else
	    {
		if (strcasecmp (lyr->TableName, table) == 0
		    && strcasecmp (lyr->GeometryName, geometry) == 0)
		  {
		      if (!doComputeFieldInfos
			  (handle, lyr->TableName, lyr->GeometryName,
			   SPATIALITE_STATISTICS_LEGACY, lyr))
			  return 0;
		  }
	    }
	  lyr = lyr->Next;
      }
    return 1;
}

static int
check_v4_statistics (sqlite3 * handle)
{
/* checking if v.4.0.0 statistics tables are supported */
    char *sql_statement;
    int ret;
    char **results;
    int rows;
    int columns;

/* testing the SQL Query */
    sql_statement =
	sqlite3_mprintf ("SELECT g.table_name, g.geometry_column, "
			 "s.row_count, s.extent_min_x, s.extent_min_y, "
			 "s.extent_max_x, s.extent_max_y "
			 "FROM vector_layers AS g "
			 "LEFT JOIN vector_layers_statistics AS s ON "
			 "(g.table_name = s.table_name AND "
			 "g.geometry_column = s.geometry_column) LIMIT 1");
    ret =
	sqlite3_get_table (handle, sql_statement, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;
    sqlite3_free_table (results);

    return 1;
}

static int
check_v3_statistics (sqlite3 * handle)
{
/* checking if v.3.0.0 statistics tables are supported */
    char *sql_statement;
    int ret;
    char **results;
    int rows;
    int columns;

    if (!check_layer_statistics (handle))
	return 0;
/* testing the SQL Query - Table-based Geometries */
    sql_statement =
	sqlite3_mprintf ("SELECT g.f_table_name, g.f_geometry_column, "
			 "s.row_count, s.extent_min_x, s.extent_min_y, "
			 "s.extent_max_x, s.extent_max_y "
			 "FROM geometry_columns AS g "
			 "LEFT JOIN layer_statistics AS s ON "
			 "(g.f_table_name = s.table_name AND "
			 "g.f_geometry_column = s.geometry_column) LIMIT 1");
    ret =
	sqlite3_get_table (handle, sql_statement, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;
    sqlite3_free_table (results);

    if (!check_views_layer_statistics (handle))
	return 0;
/* testing the SQL Query - View-based Geometries */
    sql_statement =
	sqlite3_mprintf ("SELECT g.view_name, g.view_geometry, "
			 "s.row_count, s.extent_min_x, s.extent_min_y, "
			 "s.extent_max_x, s.extent_max_y "
			 "FROM views_geometry_columns AS g "
			 "LEFT JOIN views_layer_statistics AS s ON "
			 "(g.view_name = s.view_name AND "
			 "g.view_geometry = s.view_geometry) LIMIT 1");
    ret =
	sqlite3_get_table (handle, sql_statement, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;
    sqlite3_free_table (results);

    if (!check_virts_layer_statistics (handle))
	return 0;
/* testing the SQL Query - VirtualShape-based Geometries */
    sql_statement =
	sqlite3_mprintf ("SELECT g.virt_name, g.virt_geometry, "
			 "s.row_count, s.extent_min_x, s.extent_min_y, "
			 "s.extent_max_x, s.extent_max_y "
			 "FROM virts_geometry_columns AS g "
			 "LEFT JOIN virts_layer_statistics AS s ON "
			 "(g.virt_name = s.virt_name AND "
			 "g.virt_geometry = s.virt_geometry) LIMIT 1");
    ret =
	sqlite3_get_table (handle, sql_statement, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;
    sqlite3_free_table (results);

    return 1;
}

static int
check_v2_statistics (sqlite3 * handle)
{
/* checking if v.2.0.0 statistics tables are supported */
    char *sql_statement;
    int ret;
    char **results;
    int rows;
    int columns;

    if (!check_layer_statistics (handle))
	return 0;
/* testing the SQL Query - Table-based Geometries */
    sql_statement =
	sqlite3_mprintf ("SELECT g.f_table_name, g.f_geometry_column, "
			 "s.row_count, s.extent_min_x, s.extent_min_y, "
			 "s.extent_max_x, s.extent_max_y "
			 "FROM geometry_columns AS g "
			 "LEFT JOIN layer_statistics AS s ON "
			 "(g.f_table_name = s.table_name AND "
			 "g.f_geometry_column = s.geometry_column) LIMIT 1");
    ret =
	sqlite3_get_table (handle, sql_statement, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;
    sqlite3_free_table (results);

    return 1;
}

static int
optimistic_layer_statistics_v4 (sqlite3 * handle, const char *table,
				const char *geometry)
{
/* selective statistics update - v.4.0.0 layout */
    char *sql_statement;
    int ret;
    const char *f_table_name;
    const char *f_geometry_column;
    int i;
    char **results;
    int rows;
    int columns;
    int error = 0;

    if (table == NULL && geometry == NULL)
      {
	  /* processing any table/geometry found in GEOMETRY_COLUMNS */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.table_name, g.geometry_column "
			       "FROM vector_layers AS g "
			       "LEFT JOIN vector_layers_statistics AS s ON "
			       "(g.table_name = s.table_name AND "
			       "g.geometry_column = s.geometry_column) "
			       "WHERE s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL");
      }
    else if (geometry == NULL)
      {
	  /* processing any geometry belonging to this table */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.table_name, g.geometry_column "
			       "FROM vector_layers AS g "
			       "LEFT JOIN vector_layers_statistics AS s ON "
			       "(g.table_name = s.table_name AND "
			       "g.geometry_column = s.geometry_column) "
			       "WHERE Lower(g.table_name) = Lower(%Q) AND "
			       "(s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL)", table);
      }
    else
      {
	  /* processing a single table/geometry entry */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.table_name, g.geometry_column "
			       "FROM vector_layers AS g "
			       "LEFT JOIN vector_layers_statistics AS s ON "
			       "(g.table_name = s.table_name AND "
			       "g.geometry_column = s.geometry_column) "
			       "WHERE Lower(g.table_name) = Lower(%Q) AND "
			       "Lower(g.geometry_column) = Lower(%Q) AND "
			       "(s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL)", table, geometry);
      }
    ret =
	sqlite3_get_table (handle, sql_statement, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;

    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		f_table_name = results[(i * columns) + 0];
		f_geometry_column = results[(i * columns) + 1];
		if (!update_layer_statistics
		    (handle, f_table_name, f_geometry_column))
		  {
		      error = 1;
		      break;
		  }
	    }
      }
    sqlite3_free_table (results);
    if (error)
	return 0;
    return 1;
}

static int
optimistic_layer_statistics_v3 (sqlite3 * handle, const char *table,
				const char *geometry)
{
/* selective statistics update - v.3.0.0 layout */
    char *sql_statement;
    int ret;
    const char *f_table_name;
    const char *f_geometry_column;
    int i;
    char **results;
    int rows;
    int columns;
    int error = 0;

/* genuine Table-based Geometries */
    if (table == NULL && geometry == NULL)
      {
	  /* processing any table/geometry found in GEOMETRY_COLUMNS */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.f_table_name, g.f_geometry_column "
			       "FROM geometry_columns AS g "
			       "LEFT JOIN layer_statistics AS s ON "
			       "(g.f_table_name = s.table_name AND "
			       "g.f_geometry_column = s.geometry_column) "
			       "WHERE s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL");
      }
    else if (geometry == NULL)
      {
	  /* processing any geometry belonging to this table */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.f_table_name, g.f_geometry_column "
			       "FROM geometry_columns AS g "
			       "LEFT JOIN layer_statistics AS s ON "
			       "(g.f_table_name = s.table_name AND "
			       "g.f_geometry_column = s.geometry_column) "
			       "WHERE Lower(g.f_table_name) = Lower(%Q) AND "
			       "(s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL)", table);
      }
    else
      {
	  /* processing a single table/geometry entry */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.f_table_name, g.f_geometry_column "
			       "FROM geometry_columns AS g "
			       "LEFT JOIN layer_statistics AS s ON "
			       "(g.f_table_name = s.table_name AND "
			       "g.f_geometry_column = s.geometry_column) "
			       "WHERE Lower(g.f_table_name) = Lower(%Q) AND "
			       "Lower(g.f_geometry_column) = Lower(%Q) AND "
			       "(s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL)", table, geometry);
      }
    ret =
	sqlite3_get_table (handle, sql_statement, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;

    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		f_table_name = results[(i * columns) + 0];
		f_geometry_column = results[(i * columns) + 1];
		if (!update_layer_statistics
		    (handle, f_table_name, f_geometry_column))
		  {
		      error = 1;
		      break;
		  }
	    }
      }
    sqlite3_free_table (results);
    if (error)
	return 0;

/* View Based Geometries */
    if (table == NULL && geometry == NULL)
      {
	  /* processing any table/geometry found in GEOMETRY_COLUMNS */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.view_name, g.view_geometry "
			       "FROM views_geometry_columns AS g "
			       "LEFT JOIN views_layer_statistics AS s ON "
			       "(g.view_name = s.view_name AND "
			       "g.view_geometry = s.view_geometry) "
			       "WHERE s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL");
      }
    else if (geometry == NULL)
      {
	  /* processing any geometry belonging to this table */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.view_name, g.view_geometry "
			       "FROM views_geometry_columns AS g "
			       "LEFT JOIN views_layer_statistics AS s ON "
			       "(g.view_name = s.view_name AND "
			       "g.view_geometry = s.view_geometry) "
			       "WHERE Lower(g.view_name) = Lower(%Q) AND "
			       "(s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL)", table);
      }
    else
      {
	  /* processing a single table/geometry entry */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.view_name, g.view_geometry "
			       "FROM views_geometry_columns AS g "
			       "LEFT JOIN views_layer_statistics AS s ON "
			       "(g.view_name = s.view_name AND "
			       "g.view_geometry = s.view_geometry) "
			       "WHERE Lower(g.view_name) = Lower(%Q) AND "
			       "Lower(g.view_geometry) = Lower(%Q) AND "
			       "(s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL)", table, geometry);
      }
    ret =
	sqlite3_get_table (handle, sql_statement, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;

    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		f_table_name = results[(i * columns) + 0];
		f_geometry_column = results[(i * columns) + 1];
		if (!update_layer_statistics
		    (handle, f_table_name, f_geometry_column))
		  {
		      error = 1;
		      break;
		  }
	    }
      }
    sqlite3_free_table (results);
    if (error)
	return 0;

/* VirtualShape Based Geometries */
    if (table == NULL && geometry == NULL)
      {
	  /* processing any table/geometry found in GEOMETRY_COLUMNS */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.virt_name, g.virt_geometry "
			       "FROM virts_geometry_columns AS g "
			       "LEFT JOIN virts_layer_statistics AS s ON "
			       "(g.virt_name = s.virt_name AND "
			       "g.virt_geometry = s.virt_geometry) "
			       "WHERE s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL");
      }
    else if (geometry == NULL)
      {
	  /* processing any geometry belonging to this table */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.virt_name, g.virt_geometry "
			       "FROM virts_geometry_columns AS g "
			       "LEFT JOIN virts_layer_statistics AS s ON "
			       "(g.virt_name = s.virt_name AND "
			       "g.virt_geometry = s.virt_geometry) "
			       "WHERE Lower(g.virt_name) = Lower(%Q) AND "
			       "(s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL)", table);
      }
    else
      {
	  /* processing a single table/geometry entry */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.virt_name, g.virt_geometry "
			       "FROM virts_geometry_columns AS g "
			       "LEFT JOIN virts_layer_statistics AS s ON "
			       "(g.virt_name = s.virt_name AND "
			       "g.virt_geometry = s.virt_geometry) "
			       "WHERE Lower(g.virt_name) = Lower(%Q) AND "
			       "Lower(g.virt_geometry) = Lower(%Q) AND "
			       "(s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL)", table, geometry);
      }
    ret =
	sqlite3_get_table (handle, sql_statement, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;

    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		f_table_name = results[(i * columns) + 0];
		f_geometry_column = results[(i * columns) + 1];
		if (!update_layer_statistics
		    (handle, f_table_name, f_geometry_column))
		  {
		      error = 1;
		      break;
		  }
	    }
      }
    sqlite3_free_table (results);
    if (error)
	return 0;
    return 1;
}

static int
optimistic_layer_statistics_v2 (sqlite3 * handle, const char *table,
				const char *geometry)
{
/* selective statistics update - v.2.0.0 layout */
    char *sql_statement;
    int ret;
    const char *f_table_name;
    const char *f_geometry_column;
    int i;
    char **results;
    int rows;
    int columns;
    int error = 0;

/* genuine Table-based Geometries */
    if (table == NULL && geometry == NULL)
      {
	  /* processing any table/geometry found in GEOMETRY_COLUMNS */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.f_table_name, g.f_geometry_column "
			       "FROM geometry_columns AS g "
			       "LEFT JOIN layer_statistics AS s ON "
			       "(g.f_table_name = s.table_name AND "
			       "g.f_geometry_column = s.geometry_column) "
			       "WHERE s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL");
      }
    else if (geometry == NULL)
      {
	  /* processing any geometry belonging to this table */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.f_table_name, g.f_geometry_column "
			       "FROM geometry_columns AS g "
			       "LEFT JOIN layer_statistics AS s ON "
			       "(g.f_table_name = s.table_name AND "
			       "g.f_geometry_column = s.geometry_column) "
			       "WHERE Lower(g.f_table_name) = Lower(%Q) AND "
			       "(s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL)", table);
      }
    else
      {
	  /* processing a single table/geometry entry */
	  sql_statement =
	      sqlite3_mprintf ("SELECT g.f_table_name, g.f_geometry_column "
			       "FROM geometry_columns AS g "
			       "LEFT JOIN layer_statistics AS s ON "
			       "(g.f_table_name = s.table_name AND "
			       "g.f_geometry_column = s.geometry_column) "
			       "WHERE Lower(g.f_table_name) = Lower(%Q) AND "
			       "Lower(g.f_geometry_column) = Lower(%Q) AND "
			       "(s.row_count IS NULL OR s.extent_min_x IS NULL "
			       "OR s.extent_min_y IS NULL OR s.extent_max_y IS NULL "
			       "OR s.extent_max_y IS NULL)", table, geometry);
      }
    ret =
	sqlite3_get_table (handle, sql_statement, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;

    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		f_table_name = results[(i * columns) + 0];
		f_geometry_column = results[(i * columns) + 1];
		if (!update_layer_statistics
		    (handle, f_table_name, f_geometry_column))
		  {
		      error = 1;
		      break;
		  }
	    }
      }
    sqlite3_free_table (results);
    if (error)
	return 0;
    return 1;
}

static int
optimistic_layer_statistics (sqlite3 * handle, const char *table,
			     const char *geometry)
{
/* selective statistics update */
    if (check_v4_statistics (handle))
	return optimistic_layer_statistics_v4 (handle, table, geometry);
    else if (check_v3_statistics (handle))
	return optimistic_layer_statistics_v3 (handle, table, geometry);
    else if (check_v2_statistics (handle))
	return optimistic_layer_statistics_v2 (handle, table, geometry);
    else
	return 0;
}

SPATIALITE_DECLARE gaiaVectorLayersListPtr
gaiaGetVectorLayersList (sqlite3 * handle, const char *table,
			 const char *geometry, int mode)
{
/* creating a VectorLayersList object */
    gaiaVectorLayersListPtr list;
    int metadata_version;

    if (mode == GAIA_VECTORS_LIST_PESSIMISTIC)
      {
	  /* updating anyway the statistics before querying */
	  if (!update_layer_statistics (handle, table, geometry))
	      return NULL;
      }
    if (mode == GAIA_VECTORS_LIST_OPTIMISTIC)
      {
	  /* selectively updating the statistics before querying */
	  if (!optimistic_layer_statistics (handle, table, geometry))
	    {
		/* failure: defaulting to Pessimistic */
		if (!update_layer_statistics (handle, table, geometry))
		    return NULL;
	    }
      }

/* allocating an empty VectorLayersList */
    list = malloc (sizeof (gaiaVectorLayersList));
    list->First = NULL;
    list->Last = NULL;
    list->Current = NULL;

    metadata_version = checkSpatialMetaData (handle);
    if (metadata_version == 3)
      {
	  /* current metadata style >= v.4.0.0 */
	  if (!gaiaGetVectorLayersList_v4 (handle, table, geometry, list))
	    {
		gaiaFreeVectorLayersList (list);
		return NULL;
	    }
	  if (list->First == NULL)
	    {
		gaiaFreeVectorLayersList (list);
		return NULL;
	    }
	  return list;
      }

/* legacy metadata style <= v.3.x.x */
    if (!get_table_layers_legacy (handle, table, geometry, list))
	goto error;
    if (!get_view_layers_legacy (handle, table, geometry, list))
	goto error;
    if (!get_table_extent_legacy (handle, table, geometry, list))
	goto error;
    if (!get_view_extent_legacy (handle, table, geometry, list))
	goto error;
    if (!get_table_auth_legacy (handle, table, geometry, list))
	goto error;
    if (table != NULL && mode == GAIA_VECTORS_LIST_PESSIMISTIC)
      {
	  if (!compute_table_fields_statistics (handle, table, geometry, list))
	      goto error;
      }

    if (list->First == NULL)
      {
	  gaiaFreeVectorLayersList (list);
	  return NULL;
      }
    return list;

  error:
    gaiaFreeVectorLayersList (list);
    return NULL;
}

SPATIALITE_DECLARE gaiaGeomCollPtr
gaiaGetLayerExtent (sqlite3 * handle, const char *table,
		    const char *geometry, int mode)
{
/* attempting to get a Layer Full Extent (Envelope) */
    gaiaVectorLayersListPtr list;
    gaiaVectorLayerPtr lyr;
    double minx = -DBL_MAX;
    double miny = -DBL_MAX;
    double maxx = DBL_MAX;
    double maxy = DBL_MAX;
    int srid;
    gaiaGeomCollPtr bbox;
    gaiaPolygonPtr polyg;
    gaiaRingPtr rect;
    int md = GAIA_VECTORS_LIST_OPTIMISTIC;

    if (table == NULL)
	return NULL;
    if (mode)
	md = GAIA_VECTORS_LIST_PESSIMISTIC;

    list = gaiaGetVectorLayersList (handle, table, geometry, md);
    if (list == NULL)
	return NULL;
    lyr = list->First;
    if (lyr == list->Last && lyr != NULL)
      {
	  srid = lyr->Srid;
	  if (lyr->ExtentInfos)
	    {
		minx = lyr->ExtentInfos->MinX;
		miny = lyr->ExtentInfos->MinY;
		maxx = lyr->ExtentInfos->MaxX;
		maxy = lyr->ExtentInfos->MaxY;
	    }
      }
    gaiaFreeVectorLayersList (list);

    if (minx == -DBL_MIN || miny == -DBL_MAX || maxx == DBL_MAX
	|| maxy == DBL_MAX)
	return NULL;

/* building the Envelope */
    bbox = gaiaAllocGeomColl ();
    bbox->Srid = srid;
    polyg = gaiaAddPolygonToGeomColl (bbox, 5, 0);
    rect = polyg->Exterior;
    gaiaSetPoint (rect->Coords, 0, minx, miny);	/* vertex # 1 */
    gaiaSetPoint (rect->Coords, 1, maxx, miny);	/* vertex # 2 */
    gaiaSetPoint (rect->Coords, 2, maxx, maxy);	/* vertex # 3 */
    gaiaSetPoint (rect->Coords, 3, minx, maxy);	/* vertex # 4 */
    gaiaSetPoint (rect->Coords, 4, minx, miny);	/* vertex # 5 [same as vertex # 1 to close the polygon] */
    return bbox;
}

SPATIALITE_DECLARE int
gaiaStatisticsInvalidate (sqlite3 * sqlite, const char *table,
			  const char *geometry)
{
/* attempting to immediately and unconditionally invalidate Statistics */
    int metadata_version = checkSpatialMetaData (sqlite);

    if (metadata_version == 3)
      {
	  /* current metadata style >= v.4.0.0 */
	  int ret;
	  char *errMsg = NULL;
	  char *sql_statement;
	  if (table != NULL && geometry != NULL)
	      sql_statement =
		  sqlite3_mprintf ("UPDATE geometry_columns_time SET "
				   "last_insert = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now'), "
				   "last_update = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now'), "
				   "last_delete = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now') "
				   "WHERE Lower(f_table_name) = Lower(%Q) AND "
				   "Lower(f_geometry_column) = Lower(%Q)",
				   table, geometry);
	  else if (table != NULL)
	      sql_statement =
		  sqlite3_mprintf ("UPDATE geometry_columns_time SET "
				   "last_insert = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now'), "
				   "last_update = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now'), "
				   "last_delete = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now') "
				   "WHERE Lower(f_table_name) = Lower(%Q)",
				   table);
	  else
	      sql_statement =
		  sqlite3_mprintf ("UPDATE geometry_columns_time SET "
				   "last_insert = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now'), "
				   "last_update = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now'), "
				   "last_delete = strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now')");
	  ret = sqlite3_exec (sqlite, sql_statement, NULL, NULL, &errMsg);
	  sqlite3_free (sql_statement);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("SQL error: %s: %s\n", sql_statement, errMsg);
		sqlite3_free (errMsg);
		return 0;
	    }
	  return 1;
      }
    else
	return 0;
}
