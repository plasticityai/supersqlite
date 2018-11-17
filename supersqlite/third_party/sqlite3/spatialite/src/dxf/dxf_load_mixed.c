/*

 dxf_load_mixed.c -- implements DXF support
 [loding features into the DB - mixed layers by geometry type]

 version 4.3, 2015 June 29

 Author: Sandro Furieri a.furieri@lqt.it

 -----------------------------------------------------------------------------
 
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

/*
 
CREDITS:

inital development of the DXF module has been funded by:
Regione Toscana - Settore Sistema Informativo Territoriale ed Ambientale

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>
#include <spatialite/debug.h>

#include <spatialite/gaiageo.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/gg_dxf.h>
#include <spatialite.h>
#include <spatialite_private.h>

#include "dxf_private.h"

#if defined(_WIN32) && !defined(__MINGW32__)
#define strcasecmp	_stricmp
#endif /* not WIN32 */

static int
create_mixed_text_table (sqlite3 * handle, const char *name, int srid,
			 int text3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Text-mixed" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    label TEXT NOT NULL,\n"
			   "    rotation DOUBLE NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql =
	sqlite3_mprintf
	("SELECT AddGeometryColumn(%Q, 'geometry', "
	 "%d, 'POINT', %Q)", name, srid, text3D ? "XYZ" : "XY");
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("ADD GEOMETRY %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql = sqlite3_mprintf ("SELECT CreateSpatialIndex(%Q, 'geometry')", name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    if (!create_text_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_mixed_point_table (sqlite3 * handle, const char *name, int srid,
			  int point3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Point-mixed" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql =
	sqlite3_mprintf
	("SELECT AddGeometryColumn(%Q, 'geometry', "
	 "%d, 'POINT', %Q)", name, srid, point3D ? "XYZ" : "XY");
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("ADD GEOMETRY %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql = sqlite3_mprintf ("SELECT CreateSpatialIndex(%Q, 'geometry')", name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    if (!create_point_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_mixed_line_table (sqlite3 * handle, const char *name, int srid,
			 int line3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Line-mixed" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql =
	sqlite3_mprintf
	("SELECT AddGeometryColumn(%Q, 'geometry', "
	 "%d, 'LINESTRING', %Q)", name, srid, line3D ? "XYZ" : "XY");
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("ADD GEOMETRY %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql = sqlite3_mprintf ("SELECT CreateSpatialIndex(%Q, 'geometry')", name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    if (!create_line_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_mixed_polyg_table (sqlite3 * handle, const char *name, int srid,
			  int polyg3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Polyg-mixed" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql =
	sqlite3_mprintf
	("SELECT AddGeometryColumn(%Q, 'geometry', "
	 "%d, 'POLYGON', %Q)", name, srid, polyg3D ? "XYZ" : "XY");
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("ADD GEOMETRY %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql = sqlite3_mprintf ("SELECT CreateSpatialIndex(%Q, 'geometry')", name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    if (!create_polyg_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_mixed_hatch_table (sqlite3 * handle, const char *name, int srid,
			  sqlite3_stmt ** xstmt, sqlite3_stmt ** xstmt2)
{
/* attempting to create the "Hatch-mixed" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt2;
    char *xname;
    char *pattern;
    char *xpattern;
    char *fk_name;
    char *xfk_name;
    *xstmt = NULL;
    *xstmt2 = NULL;

/* creating the Hatch-Boundary table */
    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql =
	sqlite3_mprintf
	("SELECT AddGeometryColumn(%Q, 'geometry', "
	 "%d, 'MULTIPOLYGON', 'XY')", name, srid);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("ADD GEOMETRY %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql = sqlite3_mprintf ("SELECT CreateSpatialIndex(%Q, 'geometry')", name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }

/* creating the Hatch-Pattern table */
    xname = gaiaDoubleQuotedSql (name);
    pattern = sqlite3_mprintf ("%s_pattern", name);
    xpattern = gaiaDoubleQuotedSql (pattern);
    fk_name = sqlite3_mprintf ("fk_%s_pattern", name);
    xfk_name = gaiaDoubleQuotedSql (fk_name);
    sqlite3_free (fk_name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY NOT NULL,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    CONSTRAINT \"%s\" FOREIGN KEY (feature_id) "
			   "    REFERENCES \"%s\" (feature_id))", xpattern,
			   xfk_name, xname);
    free (xname);
    free (xfk_name);
    free (xpattern);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", pattern,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql =
	sqlite3_mprintf
	("SELECT AddGeometryColumn(%Q, 'geometry', "
	 "%d, 'MULTILINESTRING', 'XY')", pattern, srid);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("ADD GEOMETRY %s error: %s\n", pattern,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sql =
	sqlite3_mprintf ("SELECT CreateSpatialIndex(%Q, 'geometry')", pattern);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n", pattern,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (pattern);
    if (!create_hatch_boundary_stmt (handle, name, &stmt))
	return 0;
    if (!create_hatch_pattern_stmt (handle, name, &stmt2))
	return 0;

    *xstmt = stmt;
    *xstmt2 = stmt2;
    return 1;
}

static int
create_mixed_text_extra_attr_table (sqlite3 * handle, const char *name,
				    char *extra_name, sqlite3_stmt ** xstmt_ext)
{
/* attempting to create the "Text-mixed-extra-attr" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt_ext;
    char *xname;
    char *xextra_name;
    char *fk_name;
    char *xfk_name;
    char *idx_name;
    char *xidx_name;
    char *view_name;
    char *xview_name;
    *xstmt_ext = NULL;

    fk_name = sqlite3_mprintf ("fk_%s", extra_name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    xfk_name = gaiaDoubleQuotedSql (fk_name);
    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    attr_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    feature_id INTEGER NOT NULL,\n"
			   "    attr_key TEXT NOT NULL,\n"
			   "    attr_value TEXT NOT NULL,\n"
			   "    CONSTRAINT \"%s\" FOREIGN KEY (feature_id) "
			   "REFERENCES \"%s\" (feature_id))",
			   xextra_name, xfk_name, xname);
    free (xextra_name);
    free (xfk_name);
    free (xname);
    sqlite3_free (fk_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", extra_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", extra_name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (feature_id)", xidx_name, xextra_name);
    free (xidx_name);
    free (xextra_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE INDEX %s error: %s\n", idx_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (idx_name);
    view_name = sqlite3_mprintf ("%s_view", name);
    xview_name = gaiaDoubleQuotedSql (view_name);
    xname = gaiaDoubleQuotedSql (name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
			   "SELECT f.feature_id AS feature_id, f.filename AS filename, "
			   "f.layer AS layer, f.label AS label, "
			   "f.rotation AS rotation, f.geometry AS geometry, "
			   "a.attr_id AS attr_id, a.attr_key AS attr_key, a.attr_value AS attr_value "
			   "FROM \"%s\" AS f "
			   "LEFT JOIN \"%s\" AS a ON (f.feature_id = a.feature_id)",
			   xview_name, xname, xextra_name);
    free (xview_name);
    free (xname);
    free (xextra_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view_name);
    if (!create_extra_stmt (handle, extra_name, &stmt_ext))
	return 0;

    *xstmt_ext = stmt_ext;
    return 1;
}

static int
create_mixed_point_extra_attr_table (sqlite3 * handle, const char *name,
				     char *extra_name,
				     sqlite3_stmt ** xstmt_ext)
{
/* attempting to create the "Point-mixed-extra-attr" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt_ext;
    char *xname;
    char *xextra_name;
    char *fk_name;
    char *xfk_name;
    char *idx_name;
    char *xidx_name;
    char *view_name;
    char *xview_name;
    *xstmt_ext = NULL;

    fk_name = sqlite3_mprintf ("fk_%s", extra_name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    xfk_name = gaiaDoubleQuotedSql (fk_name);
    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    attr_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    feature_id INTEGER NOT NULL,\n"
			   "    attr_key TEXT NOT NULL,\n"
			   "    attr_value TEXT NOT NULL,\n"
			   "    CONSTRAINT \"%s\" FOREIGN KEY (feature_id) "
			   "REFERENCES \"%s\" (feature_id))",
			   xextra_name, xfk_name, xname);
    free (xfk_name);
    free (xname);
    free (xextra_name);
    sqlite3_free (fk_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", extra_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", extra_name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (feature_id)", xidx_name, xname);
    free (xidx_name);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE INDEX %s error: %s\n", idx_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (idx_name);
    view_name = sqlite3_mprintf ("%s_view", name);
    xview_name = gaiaDoubleQuotedSql (view_name);
    xname = gaiaDoubleQuotedSql (name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
			   "SELECT f.feature_id AS feature_id, f.filename AS filename, "
			   "f.layer AS layer, f.geometry AS geometry, "
			   "a.attr_id AS attr_id, a.attr_key AS attr_key, a.attr_value AS attr_value "
			   "FROM \"%s\" AS f "
			   "LEFT JOIN \"%s\" AS a ON (f.feature_id = a.feature_id)",
			   xview_name, xname, xextra_name);
    free (xview_name);
    free (xname);
    free (xextra_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view_name);
    if (!create_extra_stmt (handle, extra_name, &stmt_ext))
	return 0;

    *xstmt_ext = stmt_ext;
    return 1;
}

static int
create_mixed_line_extra_attr_table (sqlite3 * handle, const char *name,
				    char *extra_name, sqlite3_stmt ** xstmt_ext)
{
/* attempting to create the "Line-mixed-extra-attr" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt_ext;
    char *xname;
    char *xextra_name;
    char *fk_name;
    char *xfk_name;
    char *idx_name;
    char *xidx_name;
    char *view_name;
    char *xview_name;
    *xstmt_ext = NULL;

    fk_name = sqlite3_mprintf ("fk_%s", extra_name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    xfk_name = gaiaDoubleQuotedSql (fk_name);
    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    attr_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    feature_id INTEGER NOT NULL,\n"
			   "    attr_key TEXT NOT NULL,\n"
			   "    attr_value TEXT NOT NULL,\n"
			   "    CONSTRAINT \"%s\" FOREIGN KEY (feature_id) "
			   "REFERENCES \"%s\" (feature_id))",
			   xextra_name, xfk_name, xname);
    free (xextra_name);
    free (xfk_name);
    free (xname);
    sqlite3_free (fk_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", extra_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", extra_name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (feature_id)", xidx_name, xextra_name);
    free (xidx_name);
    free (xextra_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE INDEX %s error: %s\n", idx_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (idx_name);
    view_name = sqlite3_mprintf ("%s_view", extra_name);
    xview_name = gaiaDoubleQuotedSql (view_name);
    xname = gaiaDoubleQuotedSql (name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
			   "SELECT f.feature_id AS feature_id, f.filename AS filename, "
			   "f.layer AS layer, f.geometry AS geometry, "
			   "a.attr_id AS attr_id, a.attr_key AS attr_key, a.attr_value AS attr_value "
			   "FROM \"%s\" AS f "
			   "LEFT JOIN \"%s\" AS a ON (f.feature_id = a.feature_id)",
			   xview_name, xname, xextra_name);
    free (xview_name);
    free (xname);
    free (xextra_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view_name);
    if (!create_extra_stmt (handle, extra_name, &stmt_ext))
	return 0;

    *xstmt_ext = stmt_ext;
    return 1;
}

static int
create_mixed_polyg_extra_attr_table (sqlite3 * handle, const char *name,
				     char *extra_name,
				     sqlite3_stmt ** xstmt_ext)
{
/* attempting to create the "Polyg-mixed-extra-attr" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt_ext;
    char *xname;
    char *xextra_name;
    char *fk_name;
    char *xfk_name;
    char *idx_name;
    char *xidx_name;
    char *view_name;
    char *xview_name;
    *xstmt_ext = NULL;

    fk_name = sqlite3_mprintf ("fk_%s", extra_name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    xfk_name = gaiaDoubleQuotedSql (fk_name);
    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    attr_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    feature_id INTEGER NOT NULL,\n"
			   "    attr_key TEXT NOT NULL,\n"
			   "    attr_value TEXT NOT NULL,\n"
			   "    CONSTRAINT \"%s\" FOREIGN KEY (feature_id) "
			   "REFERENCES \"%s\" (feature_id))",
			   xextra_name, xfk_name, xname);
    free (xextra_name);
    free (xfk_name);
    free (xname);
    sqlite3_free (fk_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", extra_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", extra_name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (feature_id)", xidx_name, xextra_name);
    free (xidx_name);
    free (xextra_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE INDEX %s error: %s\n", idx_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (idx_name);
    view_name = sqlite3_mprintf ("%s_view", name);
    xview_name = gaiaDoubleQuotedSql (view_name);
    xname = gaiaDoubleQuotedSql (name);
    xextra_name = gaiaDoubleQuotedSql (extra_name);
    sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
			   "SELECT f.feature_id AS feature_id, f.filename AS filename, "
			   "f.layer AS layer, f.geometry AS geometry, "
			   "a.attr_id AS attr_id, a.attr_key AS attr_key, a.attr_value AS attr_value "
			   "FROM \"%s\" AS f "
			   "LEFT JOIN \"%s\" AS a ON (f.feature_id = a.feature_id)",
			   xview_name, xname, xextra_name);
    free (xview_name);
    free (xname);
    free (xextra_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view_name);
    if (!create_extra_stmt (handle, extra_name, &stmt_ext))
	return 0;

    *xstmt_ext = stmt_ext;
    return 1;
}

DXF_PRIVATE int
import_mixed (sqlite3 * handle, gaiaDxfParserPtr dxf, int append)
{
/* populating the target DB - all layers mixed altogether */
    int text = 0;
    int point = 0;
    int line = 0;
    int polyg = 0;
    int hatch = 0;
    int insText = 0;
    int insPoint = 0;
    int insLine = 0;
    int insPolyg = 0;
    int insHatch = 0;
    int hasExtraText = 0;
    int hasExtraPoint = 0;
    int hasExtraLine = 0;
    int hasExtraPolyg = 0;
    int hasExtraInsText = 0;
    int hasExtraInsPoint = 0;
    int hasExtraInsLine = 0;
    int hasExtraInsPolyg = 0;
    int text3D = 0;
    int point3D = 0;
    int line3D = 0;
    int polyg3D = 0;
    int insText3D = 0;
    int insPoint3D = 0;
    int insLine3D = 0;
    int insPolyg3D = 0;
    int ret;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt_ext;
    sqlite3_stmt *stmt_pattern;
    unsigned char *blob;
    int blob_size;
    gaiaGeomCollPtr geom;
    char *name;
    char *extra_name;
    char *block;

    gaiaDxfLayerPtr lyr = dxf->first_layer;
    while (lyr != NULL)
      {
	  /* exploring Layers by type */
	  if (lyr->first_text != NULL)
	      text = 1;
	  if (lyr->first_point != NULL)
	      point = 1;
	  if (lyr->first_line != NULL)
	      line = 1;
	  if (lyr->first_polyg != NULL)
	      polyg = 1;
	  if (lyr->first_hatch != NULL)
	      hatch = 1;
	  if (lyr->first_ins_text != NULL)
	      insText = 1;
	  if (lyr->first_ins_point != NULL)
	      insPoint = 1;
	  if (lyr->first_ins_line != NULL)
	      insLine = 1;
	  if (lyr->first_ins_polyg != NULL)
	      insPolyg = 1;
	  if (lyr->first_ins_hatch != NULL)
	      insHatch = 1;
	  if (lyr->hasExtraText)
	      hasExtraText = 1;
	  if (lyr->hasExtraPoint)
	      hasExtraPoint = 1;
	  if (lyr->hasExtraLine)
	      hasExtraLine = 1;
	  if (lyr->hasExtraPolyg)
	      hasExtraPolyg = 1;
	  if (lyr->hasExtraInsText)
	      hasExtraInsText = 1;
	  if (lyr->hasExtraInsPoint)
	      hasExtraInsPoint = 1;
	  if (lyr->hasExtraInsLine)
	      hasExtraInsLine = 1;
	  if (lyr->hasExtraInsPolyg)
	      hasExtraInsPolyg = 1;
	  if (lyr->is3Dtext)
	      text3D = 1;
	  if (lyr->is3Dpoint)
	      point3D = 1;
	  if (lyr->is3Dline)
	      line3D = 1;
	  if (lyr->is3Dpolyg)
	      polyg3D = 1;
	  if (lyr->is3DinsText)
	      insText3D = 1;
	  if (lyr->is3DinsPoint)
	      insPoint3D = 1;
	  if (lyr->is3DinsLine)
	      insLine3D = 1;
	  if (lyr->is3DinsPolyg)
	      insPolyg3D = 1;
	  lyr = lyr->next;
      }

    if (text)
      {
	  /* creating and populating the TEXT layer */
	  stmt_ext = NULL;
	  extra_name = NULL;
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("text_layer_%s", text3D ? "3d" : "2d");
	  else
	      name =
		  sqlite3_mprintf ("%stext_layer_%s", dxf->prefix,
				   text3D ? "3d" : "2d");
	  if (append && check_text_table (handle, name, dxf->srid, text3D))
	    {
		/* appending into the already existing table */
		if (!create_text_stmt (handle, name, &stmt))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_mixed_text_table
		    (handle, name, dxf->srid, text3D, &stmt))
		    return 0;
	    }
	  if (hasExtraText)
	    {
		extra_name = create_extra_attr_table_name (name);
		if (append && check_extra_attr_table (handle, extra_name))
		  {
		      /* appending into the already existing table */
		      if (!create_extra_stmt (handle, extra_name, &stmt_ext))
			  return 0;
		  }
		else
		  {
		      /* creating the Extra Attribute table */
		      if (!create_mixed_text_extra_attr_table
			  (handle, name, extra_name, &stmt_ext))
			{
			    sqlite3_finalize (stmt);
			    return 0;
			}
		  }
	    }
	  ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("BEGIN %s error: %s\n", name,
			      sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		return 0;
	    }
	  lyr = dxf->first_layer;
	  while (lyr != NULL)
	    {
		gaiaDxfTextPtr txt = lyr->first_text;
		while (txt != NULL)
		  {
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 3, txt->label,
					 strlen (txt->label), SQLITE_STATIC);
		      sqlite3_bind_double (stmt, 4, txt->angle);
		      if (text3D)
			  geom = gaiaAllocGeomCollXYZ ();
		      else
			  geom = gaiaAllocGeomColl ();
		      geom->Srid = dxf->srid;
		      if (text3D)
			  gaiaAddPointToGeomCollXYZ (geom, txt->x, txt->y,
						     txt->z);
		      else
			  gaiaAddPointToGeomColl (geom, txt->x, txt->y);
		      gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_size);
		      gaiaFreeGeomColl (geom);
		      sqlite3_bind_blob (stmt, 5, blob, blob_size, free);
		      ret = sqlite3_step (stmt);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    if (stmt_ext != NULL)
				sqlite3_finalize (stmt_ext);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      if (stmt_ext != NULL)
			{
			    /* inserting all Extra Attributes */
			    sqlite3_int64 feature_id =
				sqlite3_last_insert_rowid (handle);
			    gaiaDxfExtraAttrPtr ext = txt->first;
			    while (ext != NULL)
			      {
				  sqlite3_reset (stmt_ext);
				  sqlite3_clear_bindings (stmt_ext);
				  sqlite3_bind_int64 (stmt_ext, 1, feature_id);
				  sqlite3_bind_text (stmt_ext, 2, ext->key,
						     strlen (ext->key),
						     SQLITE_STATIC);
				  sqlite3_bind_text (stmt_ext, 3, ext->value,
						     strlen (ext->value),
						     SQLITE_STATIC);
				  ret = sqlite3_step (stmt_ext);
				  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
				      ;
				  else
				    {
					spatialite_e ("INSERT %s error: %s\n",
						      extra_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					ret =
					    sqlite3_exec (handle, "ROLLBACK",
							  NULL, NULL, NULL);
					return 0;
				    }
				  ext = ext->next;
			      }
			}
		      txt = txt->next;
		  }
		lyr = lyr->next;
	    }
	  sqlite3_free (name);
	  if (extra_name)
	      sqlite3_free (extra_name);
	  sqlite3_finalize (stmt);
	  if (stmt_ext != NULL)
	      sqlite3_finalize (stmt_ext);
	  ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("COMMIT text_layer error: %s\n",
			      sqlite3_errmsg (handle));
		return 0;
	    }
      }

    if (point)
      {
	  /* creating and populating the POINT layer */
	  stmt_ext = NULL;
	  extra_name = NULL;
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("point_layer_%s", point3D ? "3d" : "2d");
	  else
	      name =
		  sqlite3_mprintf ("%spoint_layer_%s", dxf->prefix,
				   point3D ? "3d" : "2d");
	  if (append && check_point_table (handle, name, dxf->srid, point3D))
	    {
		/* appending into the already existing table */
		if (!create_point_stmt (handle, name, &stmt))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_mixed_point_table
		    (handle, name, dxf->srid, point3D, &stmt))
		    return 0;
	    }
	  if (hasExtraPoint)
	    {
		extra_name = create_extra_attr_table_name (name);
		if (append && check_extra_attr_table (handle, extra_name))
		  {
		      /* appending into the already existing table */
		      if (!create_extra_stmt (handle, extra_name, &stmt_ext))
			  return 0;
		  }
		else
		  {
		      /* creating the Extra Attribute table */
		      if (!create_mixed_point_extra_attr_table
			  (handle, name, extra_name, &stmt_ext))
			{
			    sqlite3_finalize (stmt);
			    return 0;
			}
		  }
	    }
	  ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("BEGIN %s error: %s\n", name,
			      sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		return 0;
	    }
	  lyr = dxf->first_layer;
	  while (lyr != NULL)
	    {
		gaiaDxfPointPtr pt = lyr->first_point;
		while (pt != NULL)
		  {
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      if (point3D)
			  geom = gaiaAllocGeomCollXYZ ();
		      else
			  geom = gaiaAllocGeomColl ();
		      geom->Srid = dxf->srid;
		      if (point3D)
			  gaiaAddPointToGeomCollXYZ (geom, pt->x, pt->y, pt->z);
		      else
			  gaiaAddPointToGeomColl (geom, pt->x, pt->y);
		      gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_size);
		      gaiaFreeGeomColl (geom);
		      sqlite3_bind_blob (stmt, 3, blob, blob_size, free);
		      ret = sqlite3_step (stmt);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    if (stmt_ext != NULL)
				sqlite3_finalize (stmt_ext);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      if (stmt_ext != NULL)
			{
			    /* inserting all Extra Attributes */
			    sqlite3_int64 feature_id =
				sqlite3_last_insert_rowid (handle);
			    gaiaDxfExtraAttrPtr ext = pt->first;
			    while (ext != NULL)
			      {
				  sqlite3_reset (stmt_ext);
				  sqlite3_clear_bindings (stmt_ext);
				  sqlite3_bind_int64 (stmt_ext, 1, feature_id);
				  sqlite3_bind_text (stmt_ext, 2, ext->key,
						     strlen (ext->key),
						     SQLITE_STATIC);
				  sqlite3_bind_text (stmt_ext, 3, ext->value,
						     strlen (ext->value),
						     SQLITE_STATIC);
				  ret = sqlite3_step (stmt_ext);
				  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
				      ;
				  else
				    {
					spatialite_e ("INSERT %s error: %s\n",
						      extra_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					ret =
					    sqlite3_exec (handle, "ROLLBACK",
							  NULL, NULL, NULL);
					return 0;
				    }
				  ext = ext->next;
			      }
			}
		      pt = pt->next;
		  }
		lyr = lyr->next;
	    }
	  sqlite3_free (name);
	  if (extra_name)
	      sqlite3_free (extra_name);
	  sqlite3_finalize (stmt);
	  if (stmt_ext != NULL)
	      sqlite3_finalize (stmt_ext);
	  ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("COMMIT point_layer error: %s\n",
			      sqlite3_errmsg (handle));
		return 0;
	    }
      }

    if (line)
      {
	  /* creating and populating the LINE layer */
	  stmt_ext = NULL;
	  extra_name = NULL;
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("line_layer_%s", line3D ? "3d" : "2d");
	  else
	      name =
		  sqlite3_mprintf ("%sline_layer_%s", dxf->prefix,
				   line3D ? "3d" : "2d");
	  if (append && check_line_table (handle, name, dxf->srid, line3D))
	    {
		/* appending into the already existing table */
		if (!create_line_stmt (handle, name, &stmt))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_mixed_line_table
		    (handle, name, dxf->srid, line3D, &stmt))
		    return 0;
	    }
	  if (hasExtraLine)
	    {
		extra_name = create_extra_attr_table_name (name);
		if (append && check_extra_attr_table (handle, extra_name))
		  {
		      /* appending into the already existing table */
		      if (!create_extra_stmt (handle, extra_name, &stmt_ext))
			  return 0;
		  }
		else
		  {
		      /* creating the Extra Attribute table */
		      if (!create_mixed_line_extra_attr_table
			  (handle, name, extra_name, &stmt_ext))
			{
			    sqlite3_finalize (stmt);
			    return 0;
			}
		  }
	    }
	  ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("BEGIN %s error: %s\n", name,
			      sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		return 0;
	    }
	  lyr = dxf->first_layer;
	  while (lyr != NULL)
	    {
		gaiaDxfPolylinePtr ln = lyr->first_line;
		while (ln != NULL)
		  {
		      int iv;
		      gaiaLinestringPtr p_ln;
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      if (line3D)
			  geom = gaiaAllocGeomCollXYZ ();
		      else
			  geom = gaiaAllocGeomColl ();
		      geom->Srid = dxf->srid;
		      gaiaAddLinestringToGeomColl (geom, ln->points);
		      p_ln = geom->FirstLinestring;
		      for (iv = 0; iv < ln->points; iv++)
			{
			    if (line3D)
			      {
				  gaiaSetPointXYZ (p_ln->Coords, iv,
						   *(ln->x + iv), *(ln->y + iv),
						   *(ln->z + iv));
			      }
			    else
			      {
				  gaiaSetPoint (p_ln->Coords, iv, *(ln->x + iv),
						*(ln->y + iv));
			      }
			}
		      gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_size);
		      gaiaFreeGeomColl (geom);
		      sqlite3_bind_blob (stmt, 3, blob, blob_size, free);
		      ret = sqlite3_step (stmt);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    if (stmt_ext != NULL)
				sqlite3_finalize (stmt_ext);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      if (stmt_ext != NULL)
			{
			    /* inserting all Extra Attributes */
			    sqlite3_int64 feature_id =
				sqlite3_last_insert_rowid (handle);
			    gaiaDxfExtraAttrPtr ext = ln->first;
			    while (ext != NULL)
			      {
				  sqlite3_reset (stmt_ext);
				  sqlite3_clear_bindings (stmt_ext);
				  sqlite3_bind_int64 (stmt_ext, 1, feature_id);
				  sqlite3_bind_text (stmt_ext, 2, ext->key,
						     strlen (ext->key),
						     SQLITE_STATIC);
				  sqlite3_bind_text (stmt_ext, 3, ext->value,
						     strlen (ext->value),
						     SQLITE_STATIC);
				  ret = sqlite3_step (stmt_ext);
				  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
				      ;
				  else
				    {
					spatialite_e ("INSERT %s error: %s\n",
						      extra_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					ret =
					    sqlite3_exec (handle, "ROLLBACK",
							  NULL, NULL, NULL);
					return 0;
				    }
				  ext = ext->next;
			      }
			}
		      ln = ln->next;
		  }
		lyr = lyr->next;
	    }
	  sqlite3_free (name);
	  if (extra_name)
	      sqlite3_free (extra_name);
	  sqlite3_finalize (stmt);
	  if (stmt_ext != NULL)
	      sqlite3_finalize (stmt_ext);
	  ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("COMMIT line_layer error: %s\n",
			      sqlite3_errmsg (handle));
		return 0;
	    }
      }

    if (polyg)
      {
	  /* creating and populating the POLYG layer */
	  stmt_ext = NULL;
	  extra_name = NULL;
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("polyg_layer_%s", polyg3D ? "3d" : "2d");
	  else
	      name =
		  sqlite3_mprintf ("%spolyg_layer_%s", dxf->prefix,
				   polyg3D ? "3d" : "2d");
	  if (append && check_polyg_table (handle, name, dxf->srid, polyg3D))
	    {
		/* appending into the already existing table */
		if (!create_polyg_stmt (handle, name, &stmt))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_mixed_polyg_table
		    (handle, name, dxf->srid, polyg3D, &stmt))
		    return 0;
	    }
	  if (hasExtraPolyg)
	    {
		extra_name = create_extra_attr_table_name (name);
		if (append && check_extra_attr_table (handle, extra_name))
		  {
		      /* appending into the already existing table */
		      if (!create_extra_stmt (handle, extra_name, &stmt_ext))
			  return 0;
		  }
		else
		  {
		      /* creating the Extra Attribute table */
		      if (!create_mixed_polyg_extra_attr_table
			  (handle, name, extra_name, &stmt_ext))
			{
			    sqlite3_finalize (stmt);
			    return 0;
			}
		  }
	    }
	  ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("BEGIN %s error: %s\n", name,
			      sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		return 0;
	    }
	  lyr = dxf->first_layer;
	  while (lyr != NULL)
	    {
		gaiaDxfPolylinePtr pg = lyr->first_polyg;
		while (pg != NULL)
		  {
		      int unclosed = check_unclosed_polyg (pg, lyr->is3Dpolyg);
		      gaiaDxfHolePtr hole;
		      int num_holes;
		      int iv;
		      gaiaPolygonPtr p_pg;
		      gaiaRingPtr p_rng;
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      if (polyg3D)
			  geom = gaiaAllocGeomCollXYZ ();
		      else
			  geom = gaiaAllocGeomColl ();
		      geom->Srid = dxf->srid;
		      num_holes = 0;
		      hole = pg->first_hole;
		      while (hole != NULL)
			{
			    num_holes++;
			    hole = hole->next;
			}
		      gaiaAddPolygonToGeomColl (geom, pg->points + unclosed,
						num_holes);
		      p_pg = geom->FirstPolygon;
		      p_rng = p_pg->Exterior;
		      for (iv = 0; iv < pg->points; iv++)
			{
			    if (lyr->is3Dpolyg)
			      {
				  gaiaSetPointXYZ (p_rng->Coords, iv,
						   *(pg->x + iv), *(pg->y + iv),
						   *(pg->z + iv));
			      }
			    else
			      {
				  gaiaSetPoint (p_rng->Coords, iv,
						*(pg->x + iv), *(pg->y + iv));
			      }
			}
		      if (unclosed)
			{
			    /* forcing the Ring to be closed */
			    if (lyr->is3Dpolyg)
			      {
				  gaiaSetPointXYZ (p_rng->Coords, pg->points,
						   *(pg->x + 0), *(pg->y + 0),
						   *(pg->z + 0));
			      }
			    else
			      {
				  gaiaSetPoint (p_rng->Coords, pg->points,
						*(pg->x + 0), *(pg->y + 0));
			      }
			}
		      num_holes = 0;
		      hole = pg->first_hole;
		      while (hole != NULL)
			{
			    int unclosed =
				check_unclosed_hole (hole, lyr->is3Dpolyg);
			    p_rng =
				gaiaAddInteriorRing (p_pg, num_holes,
						     hole->points + unclosed);
			    for (iv = 0; iv < hole->points; iv++)
			      {
				  if (lyr->is3Dpolyg)
				    {
					gaiaSetPointXYZ (p_rng->Coords, iv,
							 *(hole->x + iv),
							 *(hole->y + iv),
							 *(hole->z + iv));
				    }
				  else
				    {
					gaiaSetPoint (p_rng->Coords, iv,
						      *(hole->x + iv),
						      *(hole->y + iv));
				    }
			      }
			    if (unclosed)
			      {
				  /* forcing the Ring to be closed */
				  if (lyr->is3Dpolyg)
				    {
					gaiaSetPointXYZ (p_rng->Coords,
							 hole->points,
							 *(hole->x + 0),
							 *(hole->y + 0),
							 *(hole->z + 0));
				    }
				  else
				    {
					gaiaSetPoint (p_rng->Coords,
						      hole->points,
						      *(hole->x + 0),
						      *(hole->y + 0));
				    }
			      }
			    num_holes++;
			    hole = hole->next;
			}
		      gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_size);
		      gaiaFreeGeomColl (geom);
		      sqlite3_bind_blob (stmt, 3, blob, blob_size, free);
		      ret = sqlite3_step (stmt);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    if (stmt_ext != NULL)
				sqlite3_finalize (stmt_ext);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      if (stmt_ext != NULL)
			{
			    /* inserting all Extra Attributes */
			    sqlite3_int64 feature_id =
				sqlite3_last_insert_rowid (handle);
			    gaiaDxfExtraAttrPtr ext = pg->first;
			    while (ext != NULL)
			      {
				  sqlite3_reset (stmt_ext);
				  sqlite3_clear_bindings (stmt_ext);
				  sqlite3_bind_int64 (stmt_ext, 1, feature_id);
				  sqlite3_bind_text (stmt_ext, 2, ext->key,
						     strlen (ext->key),
						     SQLITE_STATIC);
				  sqlite3_bind_text (stmt_ext, 3, ext->value,
						     strlen (ext->value),
						     SQLITE_STATIC);
				  ret = sqlite3_step (stmt_ext);
				  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
				      ;
				  else
				    {
					spatialite_e ("INSERT %s error: %s\n",
						      extra_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					if (stmt_ext != NULL)
					    sqlite3_finalize (stmt_ext);
					ret =
					    sqlite3_exec (handle, "ROLLBACK",
							  NULL, NULL, NULL);
					return 0;
				    }
				  ext = ext->next;
			      }
			}
		      pg = pg->next;
		  }
		lyr = lyr->next;
	    }
	  sqlite3_free (name);
	  if (extra_name)
	      sqlite3_free (extra_name);
	  sqlite3_finalize (stmt);
	  if (stmt_ext != NULL)
	      sqlite3_finalize (stmt_ext);
	  ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("COMMIT polyg_layer error: %s\n",
			      sqlite3_errmsg (handle));
		return 0;
	    }
      }

    if (hatch)
      {
	  /* creating and populating the HATCH layer */
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("hatch_layer_2d");
	  else
	      name = sqlite3_mprintf ("%shatch_layer_2d", dxf->prefix);
	  if (append && check_hatch_tables (handle, name, dxf->srid))
	    {
		/* appending into the already existing tables */
		if (!create_hatch_boundary_stmt (handle, name, &stmt))
		    return 0;
		if (!create_hatch_pattern_stmt (handle, name, &stmt_pattern))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_mixed_hatch_table
		    (handle, name, dxf->srid, &stmt, &stmt_pattern))
		    return 0;
	    }
	  ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("BEGIN %s error: %s\n", name,
			      sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  lyr = dxf->first_layer;
	  while (lyr != NULL)
	    {
		gaiaDxfHatchPtr hatch = lyr->first_hatch;
		while (hatch != NULL)
		  {
		      sqlite3_int64 feature_id;
		      gaiaDxfHatchSegmPtr segm;
		      /* inserting the Boundary Geometry */
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      if (hatch->boundary == NULL)
			  sqlite3_bind_null (stmt, 3);
		      else
			{
			    gaiaToSpatiaLiteBlobWkb (hatch->boundary, &blob,
						     &blob_size);
			    sqlite3_bind_blob (stmt, 3, blob, blob_size, free);
			}
		      ret = sqlite3_step (stmt);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    sqlite3_finalize (stmt_pattern);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      feature_id = sqlite3_last_insert_rowid (handle);

		      /* inserting the Pattern Geometry */
		      sqlite3_reset (stmt_pattern);
		      sqlite3_clear_bindings (stmt_pattern);
		      sqlite3_bind_int64 (stmt_pattern, 1, feature_id);
		      sqlite3_bind_text (stmt_pattern, 2, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt_pattern, 3, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      if (hatch->first_out == NULL)
			  sqlite3_bind_null (stmt_pattern, 4);
		      else
			{
			    geom = gaiaAllocGeomColl ();
			    geom->Srid = dxf->srid;
			    geom->DeclaredType = GAIA_MULTILINESTRING;
			    segm = hatch->first_out;
			    while (segm != NULL)
			      {
				  gaiaLinestringPtr p_ln =
				      gaiaAddLinestringToGeomColl (geom, 2);
				  gaiaSetPoint (p_ln->Coords, 0, segm->x0,
						segm->y0);
				  gaiaSetPoint (p_ln->Coords, 1, segm->x1,
						segm->y1);
				  segm = segm->next;
			      }
			    gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_size);
			    gaiaFreeGeomColl (geom);
			    sqlite3_bind_blob (stmt_pattern, 4, blob, blob_size,
					       free);
			}
		      ret = sqlite3_step (stmt_pattern);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    sqlite3_finalize (stmt_pattern);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      hatch = hatch->next;
		  }
		lyr = lyr->next;
	    }
	  sqlite3_free (name);
	  sqlite3_finalize (stmt);
	  sqlite3_finalize (stmt_pattern);
	  ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("COMMIT hatch_layer error: %s\n",
			      sqlite3_errmsg (handle));
		return 0;
	    }
      }

    if (insText)
      {
	  /* creating and populating the INSERT (Text reference) layer */
	  stmt_ext = NULL;
	  extra_name = NULL;
	  if (dxf->prefix == NULL)
	      name =
		  sqlite3_mprintf ("instext_layer_%s", insText3D ? "3d" : "2d");
	  else
	      name =
		  sqlite3_mprintf ("%sinstext_layer_%s", dxf->prefix,
				   insText3D ? "3d" : "2d");
	  if (append && check_insert_table (handle, name))
	    {
		/* appending into the already existing table */
		if (!create_insert_stmt (handle, name, &stmt))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (dxf->prefix == NULL)
		    block =
			sqlite3_mprintf ("block_text_%s",
					 insText3D ? "3d" : "2d");
		else
		    block =
			sqlite3_mprintf ("%sblock_text_%s", dxf->prefix,
					 insText3D ? "3d" : "2d");
		if (!create_instext_table
		    (handle, name, block, insText3D, &stmt))
		    return 0;
		sqlite3_free (block);
	    }
	  if (hasExtraInsText)
	    {
		extra_name = create_extra_attr_table_name (name);
		if (append && check_extra_attr_table (handle, extra_name))
		  {
		      /* appending into the already existing table */
		      if (!create_extra_stmt (handle, extra_name, &stmt_ext))
			  return 0;
		  }
		else
		  {
		      /* creating the Extra Attribute table */
		      if (!create_insert_extra_attr_table
			  (handle, name, extra_name, &stmt_ext))
			{
			    sqlite3_finalize (stmt);
			    return 0;
			}
		  }
	    }
	  ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("BEGIN %s error: %s\n", name,
			      sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		return 0;
	    }
	  lyr = dxf->first_layer;
	  while (lyr != NULL)
	    {
		gaiaDxfInsertPtr ins = lyr->first_ins_text;
		while (ins != NULL)
		  {
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 3, ins->block_id,
					 strlen (ins->block_id), SQLITE_STATIC);
		      sqlite3_bind_double (stmt, 4, ins->x);
		      sqlite3_bind_double (stmt, 5, ins->y);
		      sqlite3_bind_double (stmt, 6, ins->z);
		      sqlite3_bind_double (stmt, 7, ins->scale_x);
		      sqlite3_bind_double (stmt, 8, ins->scale_y);
		      sqlite3_bind_double (stmt, 9, ins->scale_z);
		      sqlite3_bind_double (stmt, 10, ins->angle);
		      ret = sqlite3_step (stmt);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    if (stmt_ext != NULL)
				sqlite3_finalize (stmt_ext);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      if (stmt_ext != NULL)
			{
			    /* inserting all Extra Attributes */
			    sqlite3_int64 feature_id =
				sqlite3_last_insert_rowid (handle);
			    gaiaDxfExtraAttrPtr ext = ins->first;
			    while (ext != NULL)
			      {
				  sqlite3_reset (stmt_ext);
				  sqlite3_clear_bindings (stmt_ext);
				  sqlite3_bind_int64 (stmt_ext, 1, feature_id);
				  sqlite3_bind_text (stmt_ext, 2, ext->key,
						     strlen (ext->key),
						     SQLITE_STATIC);
				  sqlite3_bind_text (stmt_ext, 3, ext->value,
						     strlen (ext->value),
						     SQLITE_STATIC);
				  ret = sqlite3_step (stmt_ext);
				  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
				      ;
				  else
				    {
					spatialite_e ("INSERT %s error: %s\n",
						      extra_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					if (stmt_ext != NULL)
					    sqlite3_finalize (stmt_ext);
					ret =
					    sqlite3_exec (handle, "ROLLBACK",
							  NULL, NULL, NULL);
					return 0;
				    }
				  ext = ext->next;
			      }
			}
		      ins = ins->next;
		  }
		lyr = lyr->next;
	    }
	  sqlite3_free (name);
	  if (extra_name)
	      sqlite3_free (extra_name);
	  sqlite3_finalize (stmt);
	  if (stmt_ext != NULL)
	      sqlite3_finalize (stmt_ext);
	  ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("COMMIT hatch_layer error: %s\n",
			      sqlite3_errmsg (handle));
		return 0;
	    }
      }

    if (insPoint)
      {
	  /* creating and populating the INSERT (Point reference) layer */
	  stmt_ext = NULL;
	  extra_name = NULL;
	  if (dxf->prefix == NULL)
	      name =
		  sqlite3_mprintf ("inspoint_layer_%s",
				   insPoint3D ? "3d" : "2d");
	  else
	      name =
		  sqlite3_mprintf ("%sinspoint_layer_%s", dxf->prefix,
				   insPoint3D ? "3d" : "2d");
	  if (append && check_insert_table (handle, name))
	    {
		/* appending into the already existing table */
		if (!create_insert_stmt (handle, name, &stmt))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (dxf->prefix == NULL)
		    block =
			sqlite3_mprintf ("block_point_%s",
					 insPoint3D ? "3d" : "2d");
		else
		    block =
			sqlite3_mprintf ("%sblock_point_%s", dxf->prefix,
					 insPoint3D ? "3d" : "2d");
		if (!create_inspoint_table
		    (handle, name, block, insPoint3D, &stmt))
		    return 0;
		sqlite3_free (block);
	    }
	  if (hasExtraInsPoint)
	    {
		extra_name = create_extra_attr_table_name (name);
		if (append && check_extra_attr_table (handle, extra_name))
		  {
		      /* appending into the already existing table */
		      if (!create_extra_stmt (handle, extra_name, &stmt_ext))
			  return 0;
		  }
		else
		  {
		      /* creating the Extra Attribute table */
		      if (!create_insert_extra_attr_table
			  (handle, name, extra_name, &stmt_ext))
			{
			    sqlite3_finalize (stmt);
			    return 0;
			}
		  }
	    }
	  ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("BEGIN %s error: %s\n", name,
			      sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		return 0;
	    }
	  lyr = dxf->first_layer;
	  while (lyr != NULL)
	    {
		gaiaDxfInsertPtr ins = lyr->first_ins_point;
		while (ins != NULL)
		  {
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 3, ins->block_id,
					 strlen (ins->block_id), SQLITE_STATIC);
		      sqlite3_bind_double (stmt, 4, ins->x);
		      sqlite3_bind_double (stmt, 5, ins->y);
		      sqlite3_bind_double (stmt, 6, ins->z);
		      sqlite3_bind_double (stmt, 7, ins->scale_x);
		      sqlite3_bind_double (stmt, 8, ins->scale_y);
		      sqlite3_bind_double (stmt, 9, ins->scale_z);
		      sqlite3_bind_double (stmt, 10, ins->angle);
		      ret = sqlite3_step (stmt);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    if (stmt_ext != NULL)
				sqlite3_finalize (stmt_ext);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      if (stmt_ext != NULL)
			{
			    /* inserting all Extra Attributes */
			    sqlite3_int64 feature_id =
				sqlite3_last_insert_rowid (handle);
			    gaiaDxfExtraAttrPtr ext = ins->first;
			    while (ext != NULL)
			      {
				  sqlite3_reset (stmt_ext);
				  sqlite3_clear_bindings (stmt_ext);
				  sqlite3_bind_int64 (stmt_ext, 1, feature_id);
				  sqlite3_bind_text (stmt_ext, 2, ext->key,
						     strlen (ext->key),
						     SQLITE_STATIC);
				  sqlite3_bind_text (stmt_ext, 3, ext->value,
						     strlen (ext->value),
						     SQLITE_STATIC);
				  ret = sqlite3_step (stmt_ext);
				  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
				      ;
				  else
				    {
					spatialite_e ("INSERT %s error: %s\n",
						      extra_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					if (stmt_ext != NULL)
					    sqlite3_finalize (stmt_ext);
					ret =
					    sqlite3_exec (handle, "ROLLBACK",
							  NULL, NULL, NULL);
					return 0;
				    }
				  ext = ext->next;
			      }
			}
		      ins = ins->next;
		  }
		lyr = lyr->next;
	    }
	  sqlite3_free (name);
	  if (extra_name)
	      sqlite3_free (extra_name);
	  sqlite3_finalize (stmt);
	  if (stmt_ext != NULL)
	      sqlite3_finalize (stmt_ext);
	  ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("COMMIT hatch_layer error: %s\n",
			      sqlite3_errmsg (handle));
		return 0;
	    }
      }

    if (insLine)
      {
	  /* creating and populating the INSERT (Linestring reference) layer */
	  stmt_ext = NULL;
	  extra_name = NULL;
	  if (dxf->prefix == NULL)
	      name =
		  sqlite3_mprintf ("insline_layer_%s", insLine3D ? "3d" : "2d");
	  else
	      name =
		  sqlite3_mprintf ("%sinsline_layer_%s", dxf->prefix,
				   insLine3D ? "3d" : "2d");
	  if (append && check_insert_table (handle, name))
	    {
		/* appending into the already existing table */
		if (!create_insert_stmt (handle, name, &stmt))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (dxf->prefix == NULL)
		    block =
			sqlite3_mprintf ("block_line_%s",
					 insLine3D ? "3d" : "2d");
		else
		    block =
			sqlite3_mprintf ("%sblock_line_%s", dxf->prefix,
					 insLine3D ? "3d" : "2d");
		if (!create_insline_table
		    (handle, name, block, insLine3D, &stmt))
		    return 0;
		sqlite3_free (block);
	    }
	  if (hasExtraInsLine)
	    {
		extra_name = create_extra_attr_table_name (name);
		if (append && check_extra_attr_table (handle, extra_name))
		  {
		      /* appending into the already existing table */
		      if (!create_extra_stmt (handle, extra_name, &stmt_ext))
			  return 0;
		  }
		else
		  {
		      /* creating the Extra Attribute table */
		      if (!create_insert_extra_attr_table
			  (handle, name, extra_name, &stmt_ext))
			{
			    sqlite3_finalize (stmt);
			    return 0;
			}
		  }
	    }
	  ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("BEGIN %s error: %s\n", name,
			      sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		return 0;
	    }
	  lyr = dxf->first_layer;
	  while (lyr != NULL)
	    {
		gaiaDxfInsertPtr ins = lyr->first_ins_line;
		while (ins != NULL)
		  {
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 3, ins->block_id,
					 strlen (ins->block_id), SQLITE_STATIC);
		      sqlite3_bind_double (stmt, 4, ins->x);
		      sqlite3_bind_double (stmt, 5, ins->y);
		      sqlite3_bind_double (stmt, 6, ins->z);
		      sqlite3_bind_double (stmt, 7, ins->scale_x);
		      sqlite3_bind_double (stmt, 8, ins->scale_y);
		      sqlite3_bind_double (stmt, 9, ins->scale_z);
		      sqlite3_bind_double (stmt, 10, ins->angle);
		      ret = sqlite3_step (stmt);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    if (stmt_ext != NULL)
				sqlite3_finalize (stmt_ext);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      if (stmt_ext != NULL)
			{
			    /* inserting all Extra Attributes */
			    sqlite3_int64 feature_id =
				sqlite3_last_insert_rowid (handle);
			    gaiaDxfExtraAttrPtr ext = ins->first;
			    while (ext != NULL)
			      {
				  sqlite3_reset (stmt_ext);
				  sqlite3_clear_bindings (stmt_ext);
				  sqlite3_bind_int64 (stmt_ext, 1, feature_id);
				  sqlite3_bind_text (stmt_ext, 2, ext->key,
						     strlen (ext->key),
						     SQLITE_STATIC);
				  sqlite3_bind_text (stmt_ext, 3, ext->value,
						     strlen (ext->value),
						     SQLITE_STATIC);
				  ret = sqlite3_step (stmt_ext);
				  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
				      ;
				  else
				    {
					spatialite_e ("INSERT %s error: %s\n",
						      extra_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					if (stmt_ext != NULL)
					    sqlite3_finalize (stmt_ext);
					ret =
					    sqlite3_exec (handle, "ROLLBACK",
							  NULL, NULL, NULL);
					return 0;
				    }
				  ext = ext->next;
			      }
			}
		      ins = ins->next;
		  }
		lyr = lyr->next;
	    }
	  sqlite3_free (name);
	  if (extra_name)
	      sqlite3_free (extra_name);
	  sqlite3_finalize (stmt);
	  if (stmt_ext != NULL)
	      sqlite3_finalize (stmt_ext);
	  ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("COMMIT hatch_layer error: %s\n",
			      sqlite3_errmsg (handle));
		return 0;
	    }
      }

    if (insPolyg)
      {
	  /* creating and populating the INSERT (Polygon reference) layer */
	  stmt_ext = NULL;
	  extra_name = NULL;
	  if (dxf->prefix == NULL)
	      name =
		  sqlite3_mprintf ("inspolyg_layer_%s",
				   insPolyg3D ? "3d" : "2d");
	  else
	      name =
		  sqlite3_mprintf ("%sinspolyg_layer_%s", dxf->prefix,
				   insPolyg3D ? "3d" : "2d");
	  if (append && check_insert_table (handle, name))
	    {
		/* appending into the already existing table */
		if (!create_insert_stmt (handle, name, &stmt))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (dxf->prefix == NULL)
		    block =
			sqlite3_mprintf ("block_polyg_%s",
					 insPolyg3D ? "3d" : "2d");
		else
		    block =
			sqlite3_mprintf ("%sblock_polyg_%s", dxf->prefix,
					 insPolyg3D ? "3d" : "2d");
		if (!create_inspolyg_table
		    (handle, name, block, insPolyg3D, &stmt))
		    return 0;
		sqlite3_free (block);
	    }
	  if (hasExtraInsPolyg)
	    {
		extra_name = create_extra_attr_table_name (name);
		if (append && check_extra_attr_table (handle, extra_name))
		  {
		      /* appending into the already existing table */
		      if (!create_extra_stmt (handle, extra_name, &stmt_ext))
			  return 0;
		  }
		else
		  {
		      /* creating the Extra Attribute table */
		      if (!create_insert_extra_attr_table
			  (handle, name, extra_name, &stmt_ext))
			{
			    sqlite3_finalize (stmt);
			    return 0;
			}
		  }
	    }
	  ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("BEGIN %s error: %s\n", name,
			      sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		return 0;
	    }
	  lyr = dxf->first_layer;
	  while (lyr != NULL)
	    {
		gaiaDxfInsertPtr ins = lyr->first_ins_polyg;
		while (ins != NULL)
		  {
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 3, ins->block_id,
					 strlen (ins->block_id), SQLITE_STATIC);
		      sqlite3_bind_double (stmt, 4, ins->x);
		      sqlite3_bind_double (stmt, 5, ins->y);
		      sqlite3_bind_double (stmt, 6, ins->z);
		      sqlite3_bind_double (stmt, 7, ins->scale_x);
		      sqlite3_bind_double (stmt, 8, ins->scale_y);
		      sqlite3_bind_double (stmt, 9, ins->scale_z);
		      sqlite3_bind_double (stmt, 10, ins->angle);
		      ret = sqlite3_step (stmt);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    if (stmt_ext != NULL)
				sqlite3_finalize (stmt_ext);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      if (stmt_ext != NULL)
			{
			    /* inserting all Extra Attributes */
			    sqlite3_int64 feature_id =
				sqlite3_last_insert_rowid (handle);
			    gaiaDxfExtraAttrPtr ext = ins->first;
			    while (ext != NULL)
			      {
				  sqlite3_reset (stmt_ext);
				  sqlite3_clear_bindings (stmt_ext);
				  sqlite3_bind_int64 (stmt_ext, 1, feature_id);
				  sqlite3_bind_text (stmt_ext, 2, ext->key,
						     strlen (ext->key),
						     SQLITE_STATIC);
				  sqlite3_bind_text (stmt_ext, 3, ext->value,
						     strlen (ext->value),
						     SQLITE_STATIC);
				  ret = sqlite3_step (stmt_ext);
				  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
				      ;
				  else
				    {
					spatialite_e ("INSERT %s error: %s\n",
						      extra_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					if (stmt_ext != NULL)
					    sqlite3_finalize (stmt_ext);
					ret =
					    sqlite3_exec (handle, "ROLLBACK",
							  NULL, NULL, NULL);
					return 0;
				    }
				  ext = ext->next;
			      }
			}
		      ins = ins->next;
		  }
		lyr = lyr->next;
	    }
	  sqlite3_free (name);
	  if (extra_name)
	      sqlite3_free (extra_name);
	  sqlite3_finalize (stmt);
	  if (stmt_ext != NULL)
	      sqlite3_finalize (stmt_ext);
	  ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("COMMIT hatch_layer error: %s\n",
			      sqlite3_errmsg (handle));
		return 0;
	    }
      }

    if (insHatch)
      {
	  /* creating and populating the INSERT (Hatch reference) layer */
	  stmt_ext = NULL;
	  extra_name = NULL;
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("inshatch_layer_2d");
	  else
	      name = sqlite3_mprintf ("%sinshatch_layer_2d", dxf->prefix);
	  if (append && check_insert_table (handle, name))
	    {
		/* appending into the already existing table */
		if (!create_insert_stmt (handle, name, &stmt))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (dxf->prefix == NULL)
		    block = sqlite3_mprintf ("block_hatch_2d");
		else
		    block = sqlite3_mprintf ("%sblock_hatch_2d", dxf->prefix);
		if (!create_inshatch_table (handle, name, block, &stmt))
		    return 0;
		sqlite3_free (block);
	    }
	  ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("BEGIN %s error: %s\n", name,
			      sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  lyr = dxf->first_layer;
	  while (lyr != NULL)
	    {
		gaiaDxfInsertPtr ins = lyr->first_ins_hatch;
		while (ins != NULL)
		  {
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 3, ins->block_id,
					 strlen (ins->block_id), SQLITE_STATIC);
		      sqlite3_bind_double (stmt, 4, ins->x);
		      sqlite3_bind_double (stmt, 5, ins->y);
		      sqlite3_bind_double (stmt, 6, ins->z);
		      sqlite3_bind_double (stmt, 7, ins->scale_x);
		      sqlite3_bind_double (stmt, 8, ins->scale_y);
		      sqlite3_bind_double (stmt, 9, ins->scale_z);
		      sqlite3_bind_double (stmt, 10, ins->angle);
		      ret = sqlite3_step (stmt);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    spatialite_e ("INSERT %s error: %s\n", name,
					  sqlite3_errmsg (handle));
			    sqlite3_finalize (stmt);
			    ret =
				sqlite3_exec (handle, "ROLLBACK", NULL, NULL,
					      NULL);
			    return 0;
			}
		      ins = ins->next;
		  }
		lyr = lyr->next;
	    }
	  sqlite3_free (name);
	  if (extra_name)
	      sqlite3_free (extra_name);
	  sqlite3_finalize (stmt);
	  if (stmt_ext != NULL)
	      sqlite3_finalize (stmt_ext);
	  ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("COMMIT hatch_layer error: %s\n",
			      sqlite3_errmsg (handle));
		return 0;
	    }
      }
    return 1;
}
