/*

 dxf_load_distinct.c -- implements DXF support
 [loding features into the DB - by distinct layers]

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
create_layer_text_table (sqlite3 * handle, const char *name, int srid,
			 int text3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Text-layer" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOTT NULL,\n"
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
	sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geometry', "
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
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n",
			name, sqlite3_errmsg (handle));
	  return 0;
      }
    if (!create_text_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_layer_point_table (sqlite3 * handle, const char *name, int srid,
			  int point3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Point-layer" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOTT NULL,\n"
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
	sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geometry', "
			 "%d, 'POINT', %Q)", name, srid,
			 point3D ? "XYZ" : "XY");
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
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n",
			name, sqlite3_errmsg (handle));
	  return 0;
      }
    if (!create_point_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_layer_line_table (sqlite3 * handle, const char *name, int srid,
			 int line3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Line-layer" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOTT NULL,\n"
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
	sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geometry', "
			 "%d, 'LINESTRING', %Q)", name, srid,
			 line3D ? "XYZ" : "XY");
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
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n",
			name, sqlite3_errmsg (handle));
	  return 0;
      }
    if (!create_line_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_layer_polyg_table (sqlite3 * handle, const char *name, int srid,
			  int polyg3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Polyg-layer" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOTT NULL,\n"
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
	sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geometry', "
			 "%d, 'POLYGON', %Q)", name, srid,
			 polyg3D ? "XYZ" : "XY");
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
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n",
			name, sqlite3_errmsg (handle));
	  return 0;
      }
    if (!create_polyg_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_layer_hatch_tables (sqlite3 * handle, const char *name, int srid,
			   sqlite3_stmt ** xstmt, sqlite3_stmt ** xstmt2)
{
/* attempting to create the "Hatch-layer" tables */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt2;
    char *xname;
    char *fk_name;
    char *xfk_name;
    char *pattern;
    char *xpattern;
    *xstmt = NULL;
    *xstmt2 = NULL;

/* creating the Hatch-Boundary table */
    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOTT NULL,\n"
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
	sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geometry', "
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
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n",
			name, sqlite3_errmsg (handle));
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
			   "    filename TEXT NOTT NULL,\n"
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
	sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geometry', "
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
	  spatialite_e ("CREATE SPATIAL INDEX %s error: %s\n",
			pattern, sqlite3_errmsg (handle));
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
create_layer_text_extra_attr_table (sqlite3 * handle, const char *name,
				    char *attr_name, sqlite3_stmt ** xstmt_ext)
{
/* attempting to create the "Text-layer-extra-attr" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt_ext;
    char *xname;
    char *xattr_name;
    char *fk_name;
    char *xfk_name;
    char *idx_name;
    char *xidx_name;
    char *view_name;
    char *xview_name;
    *xstmt_ext = NULL;

    fk_name = sqlite3_mprintf ("fk_%s_attr", name);
    xfk_name = gaiaDoubleQuotedSql (fk_name);
    xattr_name = gaiaDoubleQuotedSql (attr_name);
    xname = gaiaDoubleQuotedSql (name);
    sqlite3_free (fk_name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    attr_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    feature_id INTEGER NOT NULL,\n"
			   "    attr_key TEXT NOT NULL,\n"
			   "    attr_value TEXT NOT NULL,\n"
			   "    CONSTRAINT \"%s\" FOREIGN KEY (feature_id) "
			   "REFERENCES \"%s\" (feature_id))",
			   xattr_name, xfk_name, xname);
    free (xfk_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", attr_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s_attr", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (feature_id)", xidx_name, xname);
    free (xidx_name);
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
    sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
			   "SELECT f.feature_id AS feature_id, f.layer AS layer, f.label AS label, "
			   "f.rotation AS rotation, f.geometry AS geometry, "
			   "a.attr_id AS attr_id, a.attr_key AS attr_key, a.attr_value AS attr_value "
			   "FROM \"%s\" AS f "
			   "LEFT JOIN \"%s\" AS a ON (f.feature_id = a.feature_id)",
			   xview_name, xname, xattr_name);
    free (xview_name);
    free (xattr_name);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view_name);
    if (!create_extra_stmt (handle, attr_name, &stmt_ext))
	return 0;

    *xstmt_ext = stmt_ext;
    return 1;
}

static int
create_layer_point_extra_attr_table (sqlite3 * handle, const char *name,
				     char *attr_name, sqlite3_stmt ** xstmt_ext)
{
/* attempting to create the "Point-layer-extra-attr" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt_ext;
    char *xname;
    char *xattr_name;
    char *fk_name;
    char *xfk_name;
    char *idx_name;
    char *xidx_name;
    char *view_name;
    char *xview_name;
    *xstmt_ext = NULL;

    fk_name = sqlite3_mprintf ("fk_%s_attr", name);
    xfk_name = gaiaDoubleQuotedSql (fk_name);
    xattr_name = gaiaDoubleQuotedSql (attr_name);
    xname = gaiaDoubleQuotedSql (name);
    sqlite3_free (fk_name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    attr_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    feature_id INTEGER NOT NULL,\n"
			   "    attr_key TEXT NOT NULL,\n"
			   "    attr_value TEXT NOT NULL,\n"
			   "    CONSTRAINT \"%s\" FOREIGN KEY (feature_id) "
			   "REFERENCES \"%s\" (feature_id))",
			   xattr_name, xfk_name, xname);
    free (xfk_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", attr_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s_attr", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (feature_id)", xidx_name, xname);
    free (xidx_name);
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
    sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
			   "SELECT f.feature_id AS feature_id, f.layer AS layer, f.geometry AS geometry, "
			   "a.attr_id AS attr_id, a.attr_key AS attr_key, a.attr_value AS attr_value "
			   "FROM \"%s\" AS f "
			   "LEFT JOIN \"%s\" AS a ON (f.feature_id = a.feature_id)",
			   xview_name, xname, xattr_name);
    free (xview_name);
    free (xattr_name);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view_name);
    if (!create_extra_stmt (handle, attr_name, &stmt_ext))
	return 0;

    *xstmt_ext = stmt_ext;
    return 1;
}

static int
create_layer_line_extra_attr_table (sqlite3 * handle, const char *name,
				    char *attr_name, sqlite3_stmt ** xstmt_ext)
{
/* attempting to create the "Line-layer-extra-attr" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt_ext;
    char *xname;
    char *xattr_name;
    char *fk_name;
    char *xfk_name;
    char *idx_name;
    char *xidx_name;
    char *view_name;
    char *xview_name;
    *xstmt_ext = NULL;

    fk_name = sqlite3_mprintf ("fk_%s_attr", name);
    xfk_name = gaiaDoubleQuotedSql (fk_name);
    xattr_name = gaiaDoubleQuotedSql (attr_name);
    xname = gaiaDoubleQuotedSql (name);
    sqlite3_free (fk_name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    attr_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    feature_id INTEGER NOT NULL,\n"
			   "    attr_key TEXT NOT NULL,\n"
			   "    attr_value TEXT NOT NULL,\n"
			   "    CONSTRAINT \"%s\" FOREIGN KEY (feature_id) "
			   "REFERENCES \"%s\" (feature_id))",
			   xattr_name, xfk_name, xname);
    free (xfk_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", attr_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s_attr", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (feature_id)", xidx_name, xname);
    free (xidx_name);
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
    sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
			   "SELECT f.feature_id AS feature_id, f.layer AS layer, f.geometry AS geometry, "
			   "a.attr_id AS attr_id, a.attr_key AS attr_key, a.attr_value AS attr_value "
			   "FROM \"%s\" AS f "
			   "LEFT JOIN \"%s\" AS a ON (f.feature_id = a.feature_id)",
			   xview_name, xname, xattr_name);
    free (xview_name);
    free (xattr_name);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view_name);
    if (!create_extra_stmt (handle, attr_name, &stmt_ext))
	return 0;

    *xstmt_ext = stmt_ext;
    return 1;
}

static int
create_layer_polyg_extra_attr_table (sqlite3 * handle, const char *name,
				     char *attr_name, sqlite3_stmt ** xstmt_ext)
{
/* attempting to create the "Polyg-layer-extra-attr" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt_ext;
    char *xname;
    char *xattr_name;
    char *fk_name;
    char *xfk_name;
    char *idx_name;
    char *xidx_name;
    char *view_name;
    char *xview_name;
    *xstmt_ext = NULL;

    fk_name = sqlite3_mprintf ("fk_%s_attr", name);
    xfk_name = gaiaDoubleQuotedSql (fk_name);
    xattr_name = gaiaDoubleQuotedSql (attr_name);
    xname = gaiaDoubleQuotedSql (name);
    sqlite3_free (fk_name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    attr_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    feature_id INTEGER NOT NULL,\n"
			   "    attr_key TEXT NOT NULL,\n"
			   "    attr_value TEXT NOT NULL,\n"
			   "    CONSTRAINT \"%s\" FOREIGN KEY (feature_id) "
			   "REFERENCES \"%s\" (feature_id))",
			   xattr_name, xfk_name, xname);
    free (xfk_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", attr_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s_attr", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (feature_id)", xidx_name, xname);
    free (xidx_name);
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
    sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
			   "SELECT f.feature_id AS feature_id, f.layer AS layer, f.geometry AS geometry, "
			   "a.attr_id AS attr_id, a.attr_key AS attr_key, a.attr_value AS attr_value "
			   "FROM \"%s\" AS f "
			   "LEFT JOIN \"%s\" AS a ON (f.feature_id = a.feature_id)",
			   xview_name, xname, xattr_name);
    free (xview_name);
    free (xattr_name);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view_name);
    if (!create_extra_stmt (handle, attr_name, &stmt_ext))
	return 0;

    *xstmt_ext = stmt_ext;
    return 1;
}

DXF_PRIVATE int
import_by_layer (sqlite3 * handle, gaiaDxfParserPtr dxf, int append)
{
/* populating the target DB - by distinct layers */
    int ret;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt_ext;
    sqlite3_stmt *stmt_pattern;
    unsigned char *blob;
    int blob_size;
    gaiaGeomCollPtr geom;
    gaiaLinestringPtr p_ln;
    gaiaPolygonPtr p_pg;
    gaiaRingPtr p_rng;
    int iv;
    char *name;
    char *attr_name;
    char *block;
    gaiaDxfTextPtr txt;
    gaiaDxfPointPtr pt;
    gaiaDxfPolylinePtr ln;
    gaiaDxfPolylinePtr pg;
    gaiaDxfHatchPtr p_hatch;
    gaiaDxfInsertPtr ins;

    gaiaDxfLayerPtr lyr = dxf->first_layer;
    while (lyr != NULL)
      {
	  /* looping on layers */
	  int text = 0;
	  int point = 0;
	  int line = 0;
	  int polyg = 0;
	  int hatch = 0;
	  int ins_text = 0;
	  int ins_point = 0;
	  int ins_line = 0;
	  int ins_polyg = 0;
	  int ins_hatch = 0;
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
	      ins_text = 1;
	  if (lyr->first_ins_point != NULL)
	      ins_point = 1;
	  if (lyr->first_ins_line != NULL)
	      ins_line = 1;
	  if (lyr->first_ins_polyg != NULL)
	      ins_polyg = 1;
	  if (lyr->first_ins_hatch != NULL)
	      ins_hatch = 1;
	  if (text)
	    {
		/* creating and populating the TEXT-layer */
		stmt_ext = NULL;
		attr_name = NULL;
		if (dxf->prefix == NULL)
		    name =
			sqlite3_mprintf ("%s_text_%s", lyr->layer_name,
					 lyr->is3Dtext ? "3d" : "2d");
		else
		    name =
			sqlite3_mprintf ("%s%s_text_%s", dxf->prefix,
					 lyr->layer_name,
					 lyr->is3Dtext ? "3d" : "2d");
		if (append
		    && check_text_table (handle, name, dxf->srid,
					 lyr->is3Dtext))
		  {
		      /* appending into the already existing table */
		      if (!create_text_stmt (handle, name, &stmt))
			  return 0;
		  }
		else
		  {
		      /* creating a new table */
		      if (!create_layer_text_table
			  (handle, name, dxf->srid, lyr->is3Dtext, &stmt))
			{
			    sqlite3_free (name);
			    return 0;
			}
		  }
		if (lyr->hasExtraText)
		  {
		      attr_name = create_extra_attr_table_name (name);
		      if (append && check_extra_attr_table (handle, attr_name))
			{
			    /* appending into the already existing table */
			    if (!create_extra_stmt
				(handle, attr_name, &stmt_ext))
				return 0;
			}
		      else
			{
			    /* creating the Extra Attribute table */
			    if (!create_layer_text_extra_attr_table
				(handle, name, attr_name, &stmt_ext))
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
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		txt = lyr->first_text;
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
		      if (lyr->is3Dtext)
			  geom = gaiaAllocGeomCollXYZ ();
		      else
			  geom = gaiaAllocGeomColl ();
		      geom->Srid = dxf->srid;
		      if (lyr->is3Dtext)
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
			    sqlite3_free (name);
			    if (attr_name)
				sqlite3_free (attr_name);
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
						      attr_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					sqlite3_free (name);
					if (attr_name)
					    sqlite3_free (attr_name);
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
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("COMMIT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		sqlite3_free (name);
		if (attr_name)
		    sqlite3_free (attr_name);
	    }

	  if (point)
	    {
		/* creating and populating the POINT-layer */
		stmt_ext = NULL;
		attr_name = NULL;
		if (dxf->prefix == NULL)
		    name =
			sqlite3_mprintf ("%s_point_%s", lyr->layer_name,
					 lyr->is3Dpoint ? "3d" : "2d");
		else
		    name =
			sqlite3_mprintf ("%s%s_point_%s", dxf->prefix,
					 lyr->layer_name,
					 lyr->is3Dpoint ? "3d" : "2d");
		if (append
		    && check_point_table (handle, name, dxf->srid,
					  lyr->is3Dpoint))
		  {
		      /* appending into the already existing table */
		      if (!create_point_stmt (handle, name, &stmt))
			  return 0;
		  }
		else
		  {
		      /* creating a new table */
		      if (!create_layer_point_table
			  (handle, name, dxf->srid, lyr->is3Dpoint, &stmt))
			{

			    sqlite3_free (name);
			    return 0;
			}
		  }
		if (lyr->hasExtraPoint)
		  {
		      attr_name = create_extra_attr_table_name (name);
		      if (append && check_extra_attr_table (handle, attr_name))
			{
			    /* appending into the already existing table */
			    if (!create_extra_stmt
				(handle, attr_name, &stmt_ext))
				return 0;
			}
		      else
			{
			    /* creating the Extra Attribute table */
			    if (!create_layer_point_extra_attr_table
				(handle, name, attr_name, &stmt_ext))
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
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		pt = lyr->first_point;
		while (pt != NULL)
		  {
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      if (lyr->is3Dpoint)
			  geom = gaiaAllocGeomCollXYZ ();
		      else
			  geom = gaiaAllocGeomColl ();
		      geom->Srid = dxf->srid;
		      if (lyr->is3Dpoint)
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
			    sqlite3_free (name);
			    if (attr_name)
				sqlite3_free (attr_name);
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
						      attr_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					sqlite3_free (name);
					if (attr_name)
					    sqlite3_free (attr_name);
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
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("COMMIT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		sqlite3_free (name);
		if (attr_name)
		    sqlite3_free (attr_name);
	    }

	  if (line)
	    {
		/* creating and populating the LINE-layer */
		stmt_ext = NULL;
		attr_name = NULL;
		if (dxf->prefix == NULL)
		    name =
			sqlite3_mprintf ("%s_line_%s", lyr->layer_name,
					 lyr->is3Dline ? "3d" : "2d");
		else
		    name =
			sqlite3_mprintf ("%s%s_line_%s", dxf->prefix,
					 lyr->layer_name,
					 lyr->is3Dline ? "3d" : "2d");
		if (append
		    && check_line_table (handle, name, dxf->srid,
					 lyr->is3Dline))
		  {
		      /* appending into the already existing table */
		      if (!create_line_stmt (handle, name, &stmt))
			  return 0;
		  }
		else
		  {
		      /* creating a new table */
		      if (!create_layer_line_table
			  (handle, name, dxf->srid, lyr->is3Dline, &stmt))
			{

			    sqlite3_free (name);
			    return 0;
			}
		  }
		if (lyr->hasExtraLine)
		  {
		      attr_name = create_extra_attr_table_name (name);
		      if (append && check_extra_attr_table (handle, attr_name))
			{
			    /* appending into the already existing table */
			    if (!create_extra_stmt
				(handle, attr_name, &stmt_ext))
				return 0;
			}
		      else
			{
			    /* creating the Extra Attribute table */
			    if (!create_layer_line_extra_attr_table
				(handle, name, attr_name, &stmt_ext))
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
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		ln = lyr->first_line;
		while (ln != NULL)
		  {
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      if (lyr->is3Dline)
			  geom = gaiaAllocGeomCollXYZ ();
		      else
			  geom = gaiaAllocGeomColl ();
		      geom->Srid = dxf->srid;
		      gaiaAddLinestringToGeomColl (geom, ln->points);
		      p_ln = geom->FirstLinestring;
		      for (iv = 0; iv < ln->points; iv++)
			{
			    if (lyr->is3Dline)
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
			    sqlite3_free (name);
			    if (attr_name)
				sqlite3_free (attr_name);
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
						      attr_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					sqlite3_free (name);
					if (attr_name)
					    sqlite3_free (attr_name);
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
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("COMMIT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		sqlite3_free (name);
		if (attr_name)
		    sqlite3_free (attr_name);
	    }

	  if (polyg)
	    {
		/* creating and populating the POLYG-layer */
		stmt_ext = NULL;
		attr_name = NULL;
		if (dxf->prefix == NULL)
		    name =
			sqlite3_mprintf ("%s_polyg_%s", lyr->layer_name,
					 lyr->is3Dpolyg ? "3d" : "2d");
		else
		    name =
			sqlite3_mprintf ("%s%s_polyg_%s", dxf->prefix,
					 lyr->layer_name,
					 lyr->is3Dpolyg ? "3d" : "2d");
		if (append
		    && check_polyg_table (handle, name, dxf->srid,
					  lyr->is3Dpolyg))
		  {
		      /* appending into the already existing table */
		      if (!create_polyg_stmt (handle, name, &stmt))
			  return 0;
		  }
		else
		  {
		      /* creating a new table */
		      if (!create_layer_polyg_table
			  (handle, name, dxf->srid, lyr->is3Dpolyg, &stmt))
			{

			    sqlite3_free (name);
			    return 0;
			}
		  }
		if (lyr->hasExtraPolyg)
		  {
		      attr_name = create_extra_attr_table_name (name);
		      if (append && check_extra_attr_table (handle, attr_name))
			{
			    /* appending into the already existing table */
			    if (!create_extra_stmt
				(handle, attr_name, &stmt_ext))
				return 0;
			}
		      else
			{
			    /* creating the Extra Attribute table */
			    if (!create_layer_polyg_extra_attr_table
				(handle, name, attr_name, &stmt_ext))
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
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		pg = lyr->first_polyg;
		while (pg != NULL)
		  {
		      int unclosed = check_unclosed_polyg (pg, lyr->is3Dpolyg);
		      int num_holes;
		      gaiaDxfHolePtr hole;
		      sqlite3_reset (stmt);
		      sqlite3_clear_bindings (stmt);
		      sqlite3_bind_text (stmt, 1, dxf->filename,
					 strlen (dxf->filename), SQLITE_STATIC);
		      sqlite3_bind_text (stmt, 2, lyr->layer_name,
					 strlen (lyr->layer_name),
					 SQLITE_STATIC);
		      if (lyr->is3Dpolyg)
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
			    sqlite3_free (name);
			    if (attr_name)
				sqlite3_free (attr_name);
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
						      attr_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					sqlite3_free (name);
					if (attr_name)
					    sqlite3_free (attr_name);
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
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("COMMIT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		sqlite3_free (name);
		if (attr_name)
		    sqlite3_free (attr_name);
	    }

	  if (hatch)
	    {
		/* creating and populating the HATCH-layer */
		if (dxf->prefix == NULL)
		    name = sqlite3_mprintf ("%s_hatch_2d", lyr->layer_name);
		else
		    name =
			sqlite3_mprintf ("%s%s_hatch_2d", dxf->prefix,
					 lyr->layer_name);
		if (append && check_hatch_tables (handle, name, dxf->srid))
		  {
		      /* appending into the already existing table */
		      if (!create_hatch_boundary_stmt (handle, name, &stmt))
			  return 0;
		      if (!create_hatch_pattern_stmt
			  (handle, name, &stmt_pattern))
			  return 0;
		  }
		else
		  {
		      /* creating a new table */
		      if (!create_layer_hatch_tables
			  (handle, name, dxf->srid, &stmt, &stmt_pattern))
			{

			    sqlite3_free (name);
			    return 0;
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
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		p_hatch = lyr->first_hatch;
		while (p_hatch != NULL)
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
		      if (p_hatch->boundary == NULL)
			  sqlite3_bind_null (stmt, 2);
		      else
			{
			    gaiaToSpatiaLiteBlobWkb (p_hatch->boundary, &blob,
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
			    sqlite3_free (name);
			    if (attr_name)
				sqlite3_free (attr_name);
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
		      if (p_hatch->first_out == NULL)
			  sqlite3_bind_null (stmt_pattern, 4);
		      else
			{
			    geom = gaiaAllocGeomColl ();
			    geom->Srid = dxf->srid;
			    geom->DeclaredType = GAIA_MULTILINESTRING;
			    segm = p_hatch->first_out;
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
			    sqlite3_free (name);
			    if (attr_name)
				sqlite3_free (attr_name);
			    return 0;
			}
		      p_hatch = p_hatch->next;
		  }
		sqlite3_finalize (stmt);
		sqlite3_finalize (stmt_pattern);
		ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("COMMIT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		sqlite3_free (name);
		if (attr_name)
		    sqlite3_free (attr_name);
	    }

	  if (ins_text)
	    {
		/* creating and populating the INSERT (Text reference) layer */
		stmt_ext = NULL;
		attr_name = NULL;
		if (dxf->prefix == NULL)
		    name =
			sqlite3_mprintf ("%s_instext_%s", lyr->layer_name,
					 lyr->is3DinsText ? "3d" : "2d");
		else
		    name =
			sqlite3_mprintf ("%s%s_instext_%s", dxf->prefix,
					 lyr->layer_name,
					 lyr->is3DinsText ? "3d" : "2d");
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
					       lyr->is3DinsText ? "3d" : "2d");
		      else
			  block =
			      sqlite3_mprintf ("%sblock_text_%s", dxf->prefix,
					       lyr->is3DinsText ? "3d" : "2d");
		      if (!create_instext_table
			  (handle, name, block, lyr->is3Dtext, &stmt))
			{
			    sqlite3_free (name);
			    return 0;
			}
		      sqlite3_free (block);
		  }
		if (lyr->hasExtraInsText)
		  {
		      attr_name = create_extra_attr_table_name (name);
		      if (append && check_extra_attr_table (handle, attr_name))
			{
			    /* appending into the already existing table */
			    if (!create_extra_stmt
				(handle, attr_name, &stmt_ext))
				return 0;
			}
		      else
			{
			    /* creating the Extra Attribute table */
			    if (!create_insert_extra_attr_table
				(handle, name, attr_name, &stmt_ext))
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
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		ins = lyr->first_ins_text;
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
			    sqlite3_free (name);
			    if (attr_name)
				sqlite3_free (attr_name);
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
						      attr_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					sqlite3_free (name);
					if (attr_name)
					    sqlite3_free (attr_name);
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
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("COMMIT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		sqlite3_free (name);
		if (attr_name)
		    sqlite3_free (attr_name);
	    }

	  if (ins_point)
	    {
		/* creating and populating the INSERT (Point reference) layer */
		stmt_ext = NULL;
		attr_name = NULL;
		if (dxf->prefix == NULL)
		    name =
			sqlite3_mprintf ("%s_inspoint_%s", lyr->layer_name,
					 lyr->is3DinsPoint ? "3d" : "2d");
		else
		    name =
			sqlite3_mprintf ("%s%s_inspoint_%s", dxf->prefix,
					 lyr->layer_name,
					 lyr->is3DinsPoint ? "3d" : "2d");
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
					       lyr->is3DinsPoint ? "3d" : "2d");
		      else
			  block =
			      sqlite3_mprintf ("%sblock_point_%s", dxf->prefix,
					       lyr->is3DinsPoint ? "3d" : "2d");
		      if (!create_inspoint_table
			  (handle, name, block, lyr->is3Dpoint, &stmt))
			{
			    sqlite3_free (name);
			    return 0;
			}
		      sqlite3_free (block);
		  }
		if (lyr->hasExtraInsPoint)
		  {
		      attr_name = create_extra_attr_table_name (name);
		      if (append && check_extra_attr_table (handle, attr_name))
			{
			    /* appending into the already existing table */
			    if (!create_extra_stmt
				(handle, attr_name, &stmt_ext))
				return 0;
			}
		      else
			{
			    /* creating the Extra Attribute table */
			    if (!create_insert_extra_attr_table
				(handle, name, attr_name, &stmt_ext))
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
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		ins = lyr->first_ins_point;
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
			    sqlite3_free (name);
			    if (attr_name)
				sqlite3_free (attr_name);
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
						      attr_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					sqlite3_free (name);
					if (attr_name)
					    sqlite3_free (attr_name);
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
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("COMMIT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		sqlite3_free (name);
		if (attr_name)
		    sqlite3_free (attr_name);
	    }

	  if (ins_line)
	    {
		/* creating and populating the INSERT (Line reference) layer */
		stmt_ext = NULL;
		attr_name = NULL;
		if (dxf->prefix == NULL)
		    name =
			sqlite3_mprintf ("%s_insline_%s", lyr->layer_name,
					 lyr->is3DinsLine ? "3d" : "2d");
		else
		    name =
			sqlite3_mprintf ("%s%s_insline_%s", dxf->prefix,
					 lyr->layer_name,
					 lyr->is3DinsLine ? "3d" : "2d");
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
					       lyr->is3DinsLine ? "3d" : "2d");
		      else
			  block =
			      sqlite3_mprintf ("%sblock_line_%s", dxf->prefix,
					       lyr->is3DinsLine ? "3d" : "2d");
		      if (!create_insline_table
			  (handle, name, block, lyr->is3Dline, &stmt))
			{
			    sqlite3_free (name);
			    return 0;
			}
		      sqlite3_free (block);
		  }
		if (lyr->hasExtraInsLine)
		  {
		      attr_name = create_extra_attr_table_name (name);
		      if (append && check_extra_attr_table (handle, attr_name))
			{
			    /* appending into the already existing table */
			    if (!create_extra_stmt
				(handle, attr_name, &stmt_ext))
				return 0;
			}
		      else
			{
			    /* creating the Extra Attribute table */
			    if (!create_insert_extra_attr_table
				(handle, name, attr_name, &stmt_ext))
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
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		ins = lyr->first_ins_line;
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
			    sqlite3_free (name);
			    if (attr_name)
				sqlite3_free (attr_name);
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
						      attr_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					sqlite3_free (name);
					if (attr_name)
					    sqlite3_free (attr_name);
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
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("COMMIT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		sqlite3_free (name);
		if (attr_name)
		    sqlite3_free (attr_name);
	    }

	  if (ins_polyg)
	    {
		/* creating and populating the INSERT (Polygon reference) layer */
		stmt_ext = NULL;
		attr_name = NULL;
		if (dxf->prefix == NULL)
		    name =
			sqlite3_mprintf ("%s_inspolyg_%s", lyr->layer_name,
					 lyr->is3DinsPolyg ? "3d" : "2d");
		else
		    name =
			sqlite3_mprintf ("%s%s_inspolyg_%s", dxf->prefix,
					 lyr->layer_name,
					 lyr->is3DinsPolyg ? "3d" : "2d");
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
					       lyr->is3DinsPolyg ? "3d" : "2d");
		      else
			  block =
			      sqlite3_mprintf ("%sblock_polyg_%s", dxf->prefix,
					       lyr->is3DinsPolyg ? "3d" : "2d");
		      if (!create_inspolyg_table
			  (handle, name, block, lyr->is3Dpolyg, &stmt))
			{
			    sqlite3_free (name);
			    return 0;
			}
		      sqlite3_free (block);
		  }
		if (lyr->hasExtraInsPolyg)
		  {
		      attr_name = create_extra_attr_table_name (name);
		      if (append && check_extra_attr_table (handle, attr_name))
			{
			    /* appending into the already existing table */
			    if (!create_extra_stmt
				(handle, attr_name, &stmt_ext))
				return 0;
			}
		      else
			{
			    /* creating the Extra Attribute table */
			    if (!create_insert_extra_attr_table
				(handle, name, attr_name, &stmt_ext))
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
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		ins = lyr->first_ins_polyg;
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
			    sqlite3_free (name);
			    if (attr_name)
				sqlite3_free (attr_name);
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
						      attr_name,
						      sqlite3_errmsg (handle));
					sqlite3_finalize (stmt);
					sqlite3_finalize (stmt_ext);
					sqlite3_free (name);
					if (attr_name)
					    sqlite3_free (attr_name);
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
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("COMMIT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		sqlite3_free (name);
		if (attr_name)
		    sqlite3_free (attr_name);
	    }

	  if (ins_hatch)
	    {
		/* creating and populating the INSERT (Hatch reference) layer */
		stmt_ext = NULL;
		attr_name = NULL;
		if (dxf->prefix == NULL)
		    name = sqlite3_mprintf ("%s_inshatch_2d", lyr->layer_name);
		else
		    name =
			sqlite3_mprintf ("%s%s_inspolyg_2d", dxf->prefix,
					 lyr->layer_name);
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
			  block =
			      sqlite3_mprintf ("%sblock_polyg_2d", dxf->prefix);
		      if (!create_inshatch_table (handle, name, block, &stmt))
			{
			    sqlite3_free (name);
			    return 0;
			}
		      sqlite3_free (block);
		  }
		ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("BEGIN %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_finalize (stmt);
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		ins = lyr->first_ins_polyg;
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
			    sqlite3_free (name);
			    return 0;
			}
		      ins = ins->next;
		  }
		sqlite3_finalize (stmt);
		if (stmt_ext != NULL)
		    sqlite3_finalize (stmt_ext);
		ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("COMMIT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      sqlite3_free (name);
		      if (attr_name)
			  sqlite3_free (attr_name);
		      return 0;
		  }
		sqlite3_free (name);
		if (attr_name)
		    sqlite3_free (attr_name);
	    }
	  lyr = lyr->next;
      }
    return 1;
}
