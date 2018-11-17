/*

 dxf_loader.c -- implements DXF support 
 [loding features into the DB - common methods]

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

DXF_PRIVATE int
create_text_stmt (sqlite3 * handle, const char *name, sqlite3_stmt ** xstmt)
{
/* creating the "Text" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, label, rotation, geometry) "
	 "VALUES (NULL, ?, ?, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_point_stmt (sqlite3 * handle, const char *name, sqlite3_stmt ** xstmt)
{
/* creating the "Point" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, geometry) "
	 "VALUES (NULL, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_line_stmt (sqlite3 * handle, const char *name, sqlite3_stmt ** xstmt)
{
/* creating the "Line" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, geometry) "
	 "VALUES (NULL, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_polyg_stmt (sqlite3 * handle, const char *name, sqlite3_stmt ** xstmt)
{
/* creating the "Polyg" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, geometry) "
	 "VALUES (NULL, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_hatch_boundary_stmt (sqlite3 * handle, const char *name,
			    sqlite3_stmt ** xstmt)
{
/* creating the "Hatch-Boundary" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, geometry) "
	 "VALUES (NULL, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_hatch_pattern_stmt (sqlite3 * handle, const char *name,
			   sqlite3_stmt ** xstmt)
{
/* creating the "Hatch-Pattern" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xpattern;
    char *pattern;
    *xstmt = NULL;

    pattern = sqlite3_mprintf ("%s_pattern", name);
    xpattern = gaiaDoubleQuotedSql (pattern);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, geometry) "
	 "VALUES (?, ?, ?, ?)", xpattern);
    free (xpattern);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", pattern,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (pattern);
    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_insert_stmt (sqlite3 * handle, const char *name, sqlite3_stmt ** xstmt)
{
/* creating the "Insert" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, block_id, x, y, z, scale_x, scale_y, scale_z, angle) "
	 "VALUES (NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_extra_stmt (sqlite3 * handle, const char *extra_name,
		   sqlite3_stmt ** xstmt)
{
/* creating the Extra Attributes insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xextra_name;
    *xstmt = NULL;

    xextra_name = gaiaDoubleQuotedSql (extra_name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (attr_id, feature_id, attr_key, attr_value) "
	 "VALUES (NULL, ?, ?, ?)", xextra_name);
    free (xextra_name);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", extra_name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE char *
create_extra_attr_table_name (const char *name)
{
/* creating the Extra Attributes table name [Mixed Layers] */
    return sqlite3_mprintf ("%s_attr", name);
}

DXF_PRIVATE int
check_text_table (sqlite3 * handle, const char *name, int srid, int is3D)
{
/* checking if a Text table already exists */
    char *sql;
    int ok_geom = 0;
    int ok_data = 0;
    int ret;
    int i;
    char *xname;
    char **results;
    int n_rows;
    int n_columns;
    int metadata_version = checkSpatialMetaData (handle);

    if (metadata_version == 1)
      {
	  /* legacy metadata style <= v.3.1.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  int dims2d = 0;
	  int dims3d = 0;
	  sql = sqlite3_mprintf ("SELECT srid, type, coord_dimension "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (strcmp ("POINT", results[(i * n_columns) + 1]) == 0)
			  ok_type = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  dims2d = 1;
		      if (strcmp ("XYZ", results[(i * n_columns) + 2]) == 0)
			  dims3d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		if (is3D && dims3d)
		    ok_geom = 1;
		if (!is3D && dims2d)
		    ok_geom = 1;
	    }
      }
    else
      {
	  /* current metadata style >= v.4.0.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  sql = sqlite3_mprintf ("SELECT srid, geometry_type "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1 && !is3D)
			  ok_type = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1001 && is3D)
			  ok_type = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		ok_geom = 1;
	    }
      }

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  int ok_label = 0;
	  int ok_rotation = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
		if (strcasecmp ("label", results[(i * n_columns) + 1]) == 0)
		    ok_label = 1;
		if (strcasecmp ("rotation", results[(i * n_columns) + 1]) == 0)
		    ok_rotation = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer && ok_label
	      && ok_rotation)
	      ok_data = 1;
      }
    sqlite3_free_table (results);

    if (ok_geom && ok_data)
	return 1;
    return 0;
}

DXF_PRIVATE int
check_point_table (sqlite3 * handle, const char *name, int srid, int is3D)
{
/* checking if a Point table already exists */
    char *sql;
    int ok_geom = 0;
    int ok_data = 0;
    int ret;
    int i;
    char *xname;
    char **results;
    int n_rows;
    int n_columns;
    int metadata_version = checkSpatialMetaData (handle);

    if (metadata_version == 1)
      {
	  /* legacy metadata style <= v.3.1.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  int dims2d = 0;
	  int dims3d = 0;
	  sql = sqlite3_mprintf ("SELECT srid, type, coord_dimension "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (strcmp ("POINT", results[(i * n_columns) + 1]) == 0)
			  ok_type = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  dims2d = 1;
		      if (strcmp ("XYZ", results[(i * n_columns) + 2]) == 0)
			  dims3d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		if (is3D && dims3d)
		    ok_geom = 1;
		if (!is3D && dims2d)
		    ok_geom = 1;
	    }
      }
    else
      {
	  /* current metadata style >= v.4.0.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  sql = sqlite3_mprintf ("SELECT srid, geometry_type "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1 && !is3D)
			  ok_type = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1001 && is3D)
			  ok_type = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		ok_geom = 1;
	    }
      }

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer)
	      ok_data = 1;
      }
    sqlite3_free_table (results);

    if (ok_geom && ok_data)
	return 1;
    return 0;
}

DXF_PRIVATE int
check_line_table (sqlite3 * handle, const char *name, int srid, int is3D)
{
/* checking if a Line table already exists */
    char *sql;
    int ok_geom = 0;
    int ok_data = 0;
    int ret;
    int i;
    char *xname;
    char **results;
    int n_rows;
    int n_columns;
    int metadata_version = checkSpatialMetaData (handle);

    if (metadata_version == 1)
      {
	  /* legacy metadata style <= v.3.1.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  int dims2d = 0;
	  int dims3d = 0;
	  sql = sqlite3_mprintf ("SELECT srid, type, coord_dimension "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (strcmp ("LINESTRING", results[(i * n_columns) + 1]) ==
			  0)
			  ok_type = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  dims2d = 1;
		      if (strcmp ("XYZ", results[(i * n_columns) + 2]) == 0)
			  dims3d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		if (is3D && dims3d)
		    ok_geom = 1;
		if (!is3D && dims2d)
		    ok_geom = 1;
	    }
      }
    else
      {
	  /* current metadata style >= v.4.0.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  sql = sqlite3_mprintf ("SELECT srid, geometry_type "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 2 && !is3D)
			  ok_type = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1002 && is3D)
			  ok_type = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		ok_geom = 1;
	    }
      }

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer)
	      ok_data = 1;
      }
    sqlite3_free_table (results);

    if (ok_geom && ok_data)
	return 1;
    return 0;
}

DXF_PRIVATE int
check_polyg_table (sqlite3 * handle, const char *name, int srid, int is3D)
{
/* checking if a Polygon table already exists */
    char *sql;
    int ok_geom = 0;
    int ok_data = 0;
    int ret;
    int i;
    char *xname;
    char **results;
    int n_rows;
    int n_columns;
    int metadata_version = checkSpatialMetaData (handle);

    if (metadata_version == 1)
      {
	  /* legacy metadata style <= v.3.1.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  int dims2d = 0;
	  int dims3d = 0;
	  sql = sqlite3_mprintf ("SELECT srid, type, coord_dimension "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (strcmp ("POLYGON", results[(i * n_columns) + 1]) == 0)
			  ok_type = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  dims2d = 1;
		      if (strcmp ("XYZ", results[(i * n_columns) + 2]) == 0)
			  dims3d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		if (is3D && dims3d)
		    ok_geom = 1;
		if (!is3D && dims2d)
		    ok_geom = 1;
	    }
      }
    else
      {
	  /* current metadata style >= v.4.0.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  sql = sqlite3_mprintf ("SELECT srid, geometry_type "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 3 && !is3D)
			  ok_type = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1003 && is3D)
			  ok_type = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		ok_geom = 1;
	    }
      }

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer)
	      ok_data = 1;
      }
    sqlite3_free_table (results);

    if (ok_geom && ok_data)
	return 1;
    return 0;
}

DXF_PRIVATE int
check_hatch_tables (sqlite3 * handle, const char *name, int srid)
{
/* checking if Hatch tables already exist */
    char *sql;
    int ok_geom = 0;
    int ok_pdata = 0;
    int ok_bdata = 0;
    int ret;
    int i;
    char *xname;
    char *pattern = sqlite3_mprintf ("%s_pattern", name);
    char *xpattern;
    char **results;
    int n_rows;
    int n_columns;
    int metadata_version = checkSpatialMetaData (handle);

    if (metadata_version == 1)
      {
	  /* legacy metadata style <= v.3.1.0 */
	  int ok_bsrid = 0;
	  int ok_btype = 0;
	  int bdims2d = 0;
	  int ok_psrid = 0;
	  int ok_ptype = 0;
	  int pdims2d = 0;
	  sql =
	      sqlite3_mprintf
	      ("SELECT srid, type, coord_dimension "
	       "FROM geometry_columns "
	       "WHERE Lower(f_table_name) = Lower(%Q) AND "
	       "Lower(f_geometry_column) = Lower(%Q)", name, "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      goto stop;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_bsrid = 1;
		      if (strcmp
			  ("MULTIPOLYGON", results[(i * n_columns) + 1]) == 0)
			  ok_btype = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  bdims2d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  sql =
	      sqlite3_mprintf
	      ("SELECT srid, type, coord_dimension "
	       "FROM geometry_columns "
	       "WHERE Lower(f_table_name) = Lower(%Q) AND "
	       "Lower(f_geometry_column) = Lower(%Q)", pattern, "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      goto stop;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_psrid = 1;
		      if (strcmp
			  ("MULTILINESTRING",
			   results[(i * n_columns) + 1]) == 0)
			  ok_ptype = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  pdims2d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_bsrid && ok_btype && bdims2d && ok_psrid && ok_ptype
	      && pdims2d)
	      ok_geom = 1;
      }
    else
      {
	  /* current metadata style >= v.4.0.0 */
	  int ok_psrid = 0;
	  int ok_ptype = 0;
	  int ok_bsrid = 0;
	  int ok_btype = 0;
	  sql =
	      sqlite3_mprintf ("SELECT srid, geometry_type "
			       "FROM geometry_columns "
			       "WHERE Lower(f_table_name) = Lower(%Q) AND "
			       "Lower(f_geometry_column) = Lower(%Q)", name,
			       "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      goto stop;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_bsrid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 6)
			  ok_btype = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  sql =
	      sqlite3_mprintf ("SELECT srid, geometry_type "
			       "FROM geometry_columns "
			       "WHERE Lower(f_table_name) = Lower(%Q) AND "
			       "Lower(f_geometry_column) = Lower(%Q)", pattern,
			       "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      goto stop;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_psrid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 5)
			  ok_ptype = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_bsrid && ok_btype && ok_psrid && ok_ptype)
	      ok_geom = 1;
      }

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto stop;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer)
	      ok_bdata = 1;
      }
    sqlite3_free_table (results);

    xpattern = gaiaDoubleQuotedSql (pattern);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xpattern);
    free (xpattern);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto stop;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer)
	      ok_pdata = 1;
      }
    sqlite3_free_table (results);

    if (ok_geom && ok_bdata && ok_pdata)
      {
	  sqlite3_free (pattern);
	  return 1;
      }
  stop:
    sqlite3_free (pattern);
    return 0;
}

DXF_PRIVATE int
check_insert_table (sqlite3 * handle, const char *name)
{
/* checking if an Insert (indirect refs) table already exists */
    char *sql;
    int ok_data = 0;
    int ret;
    int i;
    char *xname;
    char **results;
    int n_rows;
    int n_columns;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  int ok_block_id = 0;
	  int ok_x = 0;
	  int ok_y = 0;
	  int ok_z = 0;
	  int ok_scale_x = 0;
	  int ok_scale_y = 0;
	  int ok_scale_z = 0;
	  int ok_angle = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
		if (strcasecmp ("block_id", results[(i * n_columns) + 1]) == 0)
		    ok_block_id = 1;
		if (strcasecmp ("x", results[(i * n_columns) + 1]) == 0)
		    ok_x = 1;
		if (strcasecmp ("y", results[(i * n_columns) + 1]) == 0)
		    ok_y = 1;
		if (strcasecmp ("z", results[(i * n_columns) + 1]) == 0)
		    ok_z = 1;
		if (strcasecmp ("scale_x", results[(i * n_columns) + 1]) == 0)
		    ok_scale_x = 1;
		if (strcasecmp ("scale_y", results[(i * n_columns) + 1]) == 0)
		    ok_scale_y = 1;
		if (strcasecmp ("scale_z", results[(i * n_columns) + 1]) == 0)
		    ok_scale_z = 1;
		if (strcasecmp ("angle", results[(i * n_columns) + 1]) == 0)
		    ok_angle = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer && ok_block_id && ok_x
	      && ok_y && ok_z && ok_scale_x && ok_scale_y && ok_scale_z
	      && ok_angle)
	      ok_data = 1;
      }
    sqlite3_free_table (results);

    if (ok_data)
	return 1;
    return 0;
}

DXF_PRIVATE int
check_extra_attr_table (sqlite3 * handle, const char *name)
{
/* checking if an Extra Attributes table already exists */
    char *sql;
    int ok_data = 0;
    int ret;
    int i;
    char *xname;
    char **results;
    int n_rows;
    int n_columns;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  int ok_attr_id = 0;
	  int ok_feature_id = 0;
	  int ok_attr_key = 0;
	  int ok_attr_value = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("attr_id", results[(i * n_columns) + 1]) == 0)
		    ok_attr_id = 1;
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("attr_key", results[(i * n_columns) + 1]) == 0)
		    ok_attr_key = 1;
		if (strcasecmp ("attr_value", results[(i * n_columns) + 1]) ==
		    0)
		    ok_attr_value = 1;
	    }
	  if (ok_attr_id && ok_feature_id && ok_attr_key && ok_attr_value)
	      ok_data = 1;
      }
    sqlite3_free_table (results);

    if (ok_data)
	return 1;
    return 0;
}

DXF_PRIVATE int
create_block_text_stmt (sqlite3 * handle, const char *name,
			sqlite3_stmt ** xstmt)
{
/* creating the "Block-Text" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, block_id, label, rotation, geometry) "
	 "VALUES (NULL, ?, ?, ?, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

static int
create_block_point_stmt (sqlite3 * handle, const char *name,
			 sqlite3_stmt ** xstmt)
{
/* creating the "Block-Point" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, block_id, geometry) "
	 "VALUES (NULL, ?, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

static int
create_block_line_stmt (sqlite3 * handle, const char *name,
			sqlite3_stmt ** xstmt)
{
/* creating the "Block-Line" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, block_id, geometry) "
	 "VALUES (NULL, ?, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

static int
create_block_polyg_stmt (sqlite3 * handle, const char *name,
			 sqlite3_stmt ** xstmt)
{
/* creating the "Block-Polyg" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, block_id, geometry) "
	 "VALUES (NULL, ?, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

static int
create_block_hatch_boundary_stmt (sqlite3 * handle, const char *name,
				  sqlite3_stmt ** xstmt)
{
/* creating the "Block-Hatch-Boundary" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, block_id, geometry) "
	 "VALUES (NULL, ?, ?, ?, ?)", xname);
    free (xname);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    *xstmt = stmt;
    return 1;
}

static int
create_block_hatch_pattern_stmt (sqlite3 * handle, const char *name,
				 sqlite3_stmt ** xstmt)
{
/* creating the "Block-Hatch-Pattern" insert statement */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xpattern;
    char *pattern;
    *xstmt = NULL;

    pattern = sqlite3_mprintf ("%s_pattern", name);
    xpattern = gaiaDoubleQuotedSql (pattern);
    sql =
	sqlite3_mprintf
	("INSERT INTO \"%s\" (feature_id, filename, layer, block_id, geometry) "
	 "VALUES (?, ?, ?, ?, ?)", xpattern);
    free (xpattern);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE STATEMENT %s error: %s\n", pattern,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (pattern);
    *xstmt = stmt;
    return 1;
}

static int
check_block_text_table (sqlite3 * handle, const char *name, int srid, int is3D)
{
/* checking if a Block-Text table already exists */
    char *sql;
    int ok_geom = 0;
    int ok_data = 0;
    int ret;
    int i;
    char *xname;
    char **results;
    int n_rows;
    int n_columns;
    int metadata_version = checkSpatialMetaData (handle);

    if (metadata_version == 1)
      {
	  /* legacy metadata style <= v.3.1.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  int dims2d = 0;
	  int dims3d = 0;
	  sql = sqlite3_mprintf ("SELECT srid, type, coord_dimension "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (strcmp ("POINT", results[(i * n_columns) + 1]) == 0)
			  ok_type = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  dims2d = 1;
		      if (strcmp ("XYZ", results[(i * n_columns) + 2]) == 0)
			  dims3d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		if (is3D && dims3d)
		    ok_geom = 1;
		if (!is3D && dims2d)
		    ok_geom = 1;
	    }
      }
    else
      {
	  /* current metadata style >= v.4.0.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  sql = sqlite3_mprintf ("SELECT srid, geometry_type "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1 && !is3D)
			  ok_type = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1001 && is3D)
			  ok_type = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		ok_geom = 1;
	    }
      }

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  int ok_block_id = 0;
	  int ok_label = 0;
	  int ok_rotation = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
		if (strcasecmp ("block_id", results[(i * n_columns) + 1]) == 0)
		    ok_block_id = 1;
		if (strcasecmp ("label", results[(i * n_columns) + 1]) == 0)
		    ok_label = 1;
		if (strcasecmp ("rotation", results[(i * n_columns) + 1]) == 0)
		    ok_rotation = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer && ok_block_id
	      && ok_label && ok_rotation)
	      ok_data = 1;
      }
    sqlite3_free_table (results);

    if (ok_geom && ok_data)
	return 1;
    return 0;
}

static int
has_viewgeom_rdonly (sqlite3 * handle)
{
/* testing if "views_geometry_columns" has a "read_only" column */
    int has_rdonly = 0;
    int ret;
    int i;
    char **results;
    int n_rows;
    int n_columns;
    char *sql = "PRAGMA table_info(views_geometry_columns)";
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("read_only", results[(i * n_columns) + 1]) == 0)
		    has_rdonly = 1;
	    }
      }
    sqlite3_free_table (results);
    return has_rdonly;
}

DXF_PRIVATE int
create_instext_table (sqlite3 * handle, const char *name,
		      const char *block, int is3d, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Insert-mixed-Text" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    char *idx_name;
    char *xidx_name;
    char *view;
    char *xview;
    char *xblock;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    block_id TEXT NOT NULL,\n"
			   "    x DOUBLE NOT NULL,\n"
			   "    y DOUBLE NOT NULL,\n"
			   "    z DOUBLE NOT NULL,\n"
			   "    scale_x DOUBLE NOT NULL,\n"
			   "    scale_y DOUBLE NOT NULL,\n"
			   "    scale_z DOUBLE NOT NULL,\n"
			   "    angle DOUBLE NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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

    view = sqlite3_mprintf ("%s_view", name);
    xview = gaiaDoubleQuotedSql (view);
    xname = gaiaDoubleQuotedSql (name);
    xblock = gaiaDoubleQuotedSql (block);
    if (is3d)
      {
	  sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
				 "SELECT b.ROWID AS rowid, i.feature_id AS feature_id, "
				 "i.filename AS filename, i.layer AS layer, i.block_id AS block_id, "
				 "ST_Translate(ScaleCoords(RotateCoords(b.geometry, i.angle), "
				 "i.scale_x, i.scale_y), i.x, i.y, i.z) AS geometry "
				 "FROM \"%s\" AS i JOIN \"%s\" AS b ON "
				 "(b.layer = i.layer AND b.block_id = i.block_id)",
				 xview, xname, xblock);
      }
    else
      {
	  sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
				 "SELECT b.ROWID AS rowid, i.feature_id AS feature_id, "
				 "i.filename AS filename, i.layer AS layer, i.block_id AS block_id, "
				 "ShiftCoords(ScaleCoords(RotateCoords(b.geometry, i.angle), "
				 "i.scale_x, i.scale_y), i.x, i.y) AS geometry "
				 "FROM \"%s\" AS i JOIN \"%s\" AS b ON "
				 "(b.layer = i.layer AND b.block_id = i.block_id)",
				 xview, xname, xblock);
      }
    free (xname);
    free (xview);
    free (xblock);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view,
			sqlite3_errmsg (handle));
	  return 0;
      }
    if (has_viewgeom_rdonly (handle))
	sql = sqlite3_mprintf ("INSERT INTO views_geometry_columns ("
			       "view_name, view_geometry, view_rowid, f_table_name, "
			       "f_geometry_column, read_only) VALUES (Lower(%Q), Lower(%Q), "
			       "Lower(%Q), Lower(%Q), Lower(%Q), 1)", view,
			       "geometry", "rowid", block, "geometry");
    else
	sql = sqlite3_mprintf ("INSERT INTO views_geometry_columns ("
			       "view_name, view_geometry, view_rowid, f_table_name, "
			       "f_geometry_column) VALUES (Lower(%Q), Lower(%Q), "
			       "Lower(%Q), Lower(%Q), Lower(%Q))", view,
			       "geometry", "rowid", block, "geometry");
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("Register Spatial VIEW %s error: %s\n", view,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view);

    if (!create_insert_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_inspoint_table (sqlite3 * handle, const char *name,
		       const char *block, int is3d, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Insert-mixed-Point" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    char *idx_name;
    char *xidx_name;
    char *view;
    char *xview;
    char *xblock;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    block_id TEXT NOT NULL,\n"
			   "    x DOUBLE NOT NULL,\n"
			   "    y DOUBLE NOT NULL,\n"
			   "    z DOUBLE NOT NULL,\n"
			   "    scale_x DOUBLE NOT NULL,\n"
			   "    scale_y DOUBLE NOT NULL,\n"
			   "    scale_z DOUBLE NOT NULL,\n"
			   "    angle DOUBLE NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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

    view = sqlite3_mprintf ("%s_view", name);
    xview = gaiaDoubleQuotedSql (view);
    xname = gaiaDoubleQuotedSql (name);
    xblock = gaiaDoubleQuotedSql (block);
    if (is3d)
      {
	  sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
				 "SELECT b.ROWID AS rowid, i.feature_id AS feature_id, "
				 "i.filename AS filename, i.layer AS layer, i.block_id AS block_id, "
				 "ST_Translate(ScaleCoords(RotateCoords(b.geometry, i.angle), "
				 "i.scale_x, i.scale_y), i.x, i.y, i.z) AS geometry "
				 "FROM \"%s\" AS i JOIN \"%s\" AS b ON "
				 "(b.layer = i.layer AND b.block_id = i.block_id)",
				 xview, xname, xblock);
      }
    else
      {
	  sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
				 "SELECT b.ROWID as rowid, i.feature_id AS feature_id, "
				 "i.filename AS filename, i.layer AS layer, i.block_id AS block_id, "
				 "ShiftCoords(ScaleCoords(RotateCoords(b.geometry, i.angle), "
				 "i.scale_x, i.scale_y), i.x, i.y) AS geometry "
				 "FROM \"%s\" AS i JOIN \"%s\" AS b ON "
				 "(b.layer = i.layer AND b.block_id = i.block_id)",
				 xview, xname, xblock);
      }
    free (xname);
    free (xview);
    free (xblock);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view,
			sqlite3_errmsg (handle));
	  return 0;
      }
    if (has_viewgeom_rdonly (handle))
	sql = sqlite3_mprintf ("INSERT INTO views_geometry_columns ("
			       "view_name, view_geometry, view_rowid, f_table_name, "
			       "f_geometry_column, read_only) VALUES (Lower(%Q), Lower(%Q), "
			       "Lower(%Q), Lower(%Q), Lower(%Q), 1)", view,
			       "geometry", "rowid", block, "geometry");
    else
	sql = sqlite3_mprintf ("INSERT INTO views_geometry_columns ("
			       "view_name, view_geometry, view_rowid, f_table_name, "
			       "f_geometry_column) VALUES (Lower(%Q), Lower(%Q), "
			       "Lower(%Q), Lower(%Q), Lower(%Q))", view,
			       "geometry", "rowid", block, "geometry");
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("Register Spatial VIEW %s error: %s\n", view,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view);

    if (!create_insert_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_insline_table (sqlite3 * handle, const char *name,
		      const char *block, int is3d, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Insert-mixed-Line" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    char *idx_name;
    char *xidx_name;
    char *view;
    char *xview;
    char *xblock;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    block_id TEXT NOT NULL,\n"
			   "    x DOUBLE NOT NULL,\n"
			   "    y DOUBLE NOT NULL,\n"
			   "    z DOUBLE NOT NULL,\n"
			   "    scale_x DOUBLE NOT NULL,\n"
			   "    scale_y DOUBLE NOT NULL,\n"
			   "    scale_z DOUBLE NOT NULL,\n"
			   "    angle DOUBLE NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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

    view = sqlite3_mprintf ("%s_view", name);
    xview = gaiaDoubleQuotedSql (view);
    xname = gaiaDoubleQuotedSql (name);
    xblock = gaiaDoubleQuotedSql (block);
    if (is3d)
      {
	  sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
				 "SELECT b.ROWID AS rowid, i.feature_id AS feature_id, "
				 "i.filename AS filename, i.layer AS layer, i.block_id AS block_id, "
				 "ST_Translate(ScaleCoords(RotateCoords(b.geometry, i.angle), "
				 "i.scale_x, i.scale_y), i.x, i.y, i.z) AS geometry "
				 "FROM \"%s\" AS i JOIN \"%s\" AS b ON "
				 "(b.layer = i.layer AND b.block_id = i.block_id)",
				 xview, xname, xblock);
      }
    else
      {
	  sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
				 "SELECT b.ROWID AS rowid, i.feature_id AS feature_id, "
				 "i.filename AS filename, i.layer AS layer, i.block_id AS block_id, "
				 "ShiftCoords(ScaleCoords(RotateCoords(b.geometry, i.angle), "
				 "i.scale_x, i.scale_y), i.x, i.y) AS geometry "
				 "FROM \"%s\" AS i JOIN \"%s\" AS b ON "
				 "(b.layer = i.layer AND b.block_id = i.block_id)",
				 xview, xname, xblock);
      }
    free (xname);
    free (xview);
    free (xblock);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view,
			sqlite3_errmsg (handle));
	  return 0;
      }
    if (has_viewgeom_rdonly (handle))
	sql = sqlite3_mprintf ("INSERT INTO views_geometry_columns ("
			       "view_name, view_geometry, view_rowid, f_table_name, "
			       "f_geometry_column, read_only) VALUES (Lower(%Q), Lower(%Q), "
			       "Lower(%Q), Lower(%Q), Lower(%Q), 1)", view,
			       "geometry", "rowid", block, "geometry");
    else
	sql = sqlite3_mprintf ("INSERT INTO views_geometry_columns ("
			       "view_name, view_geometry, view_rowid, f_table_name, "
			       "f_geometry_column) VALUES (Lower(%Q), Lower(%Q), "
			       "Lower(%Q), Lower(%Q), Lower(%Q))", view,
			       "geometry", "rowid", block, "geometry");
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("Register Spatial VIEW %s error: %s\n", view,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view);

    if (!create_insert_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_inspolyg_table (sqlite3 * handle, const char *name,
		       const char *block, int is3d, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Insert-mixed-Polyg" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    char *idx_name;
    char *xidx_name;
    char *view;
    char *xview;
    char *xblock;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    block_id TEXT NOT NULL,\n"
			   "    x DOUBLE NOT NULL,\n"
			   "    y DOUBLE NOT NULL,\n"
			   "    z DOUBLE NOT NULL,\n"
			   "    scale_x DOUBLE NOT NULL,\n"
			   "    scale_y DOUBLE NOT NULL,\n"
			   "    scale_z DOUBLE NOT NULL,\n"
			   "    angle DOUBLE NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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

    view = sqlite3_mprintf ("%s_view", name);
    xview = gaiaDoubleQuotedSql (view);
    xname = gaiaDoubleQuotedSql (name);
    xblock = gaiaDoubleQuotedSql (block);
    if (is3d)
      {
	  sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
				 "SELECT b.ROWID AS rowid, i.feature_id AS feature_id, "
				 "i.filename AS filename, i.layer AS layer, i.block_id AS block_id, "
				 "ST_Translate(ScaleCoords(RotateCoords(b.geometry, i.angle), "
				 "i.scale_x, i.scale_y), i.x, i.y, i.z) AS geometry "
				 "FROM \"%s\" AS i JOIN \"%s\" AS b ON "
				 "(b.layer = i.layer AND b.block_id = i.block_id)",
				 xview, xname, xblock);
      }
    else
      {
	  sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
				 "SELECT b.ROWID AS rowid, i.feature_id AS feature_id, "
				 "i.filename AS filename, i.layer AS layer, i.block_id AS block_id, "
				 "ShiftCoords(ScaleCoords(RotateCoords(b.geometry, i.angle), "
				 "i.scale_x, i.scale_y), i.x, i.y) AS geometry "
				 "FROM \"%s\" AS i JOIN \"%s\" AS b ON "
				 "(b.layer = i.layer AND b.block_id = i.block_id)",
				 xview, xname, xblock);
      }
    free (xname);
    free (xview);
    free (xblock);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view,
			sqlite3_errmsg (handle));
	  return 0;
      }
    if (has_viewgeom_rdonly (handle))
	sql = sqlite3_mprintf ("INSERT INTO views_geometry_columns ("
			       "view_name, view_geometry, view_rowid, f_table_name, "
			       "f_geometry_column, read_only) VALUES (Lower(%Q), Lower(%Q), "
			       "Lower(%Q), Lower(%Q), Lower(%Q), 1)", view,
			       "geometry", "rowid", block, "geometry");
    else
	sql = sqlite3_mprintf ("INSERT INTO views_geometry_columns ("
			       "view_name, view_geometry, view_rowid, f_table_name, "
			       "f_geometry_column) VALUES (Lower(%Q), Lower(%Q), "
			       "Lower(%Q), Lower(%Q), Lower(%Q))", view,
			       "geometry", "rowid", block, "geometry");
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("Register Spatial VIEW %s error: %s\n", view,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view);

    if (!create_insert_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_inshatch_table (sqlite3 * handle, const char *name,
		       const char *block, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Insert-mixed-Hatch" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    char *idx_name;
    char *xidx_name;
    char *view;
    char *xview;
    char *xblock;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    block_id TEXT NOT NULL,\n"
			   "    x DOUBLE NOT NULL,\n"
			   "    y DOUBLE NOT NULL,\n"
			   "    z DOUBLE NOT NULL,\n"
			   "    scale_x DOUBLE NOT NULL,\n"
			   "    scale_y DOUBLE NOT NULL,\n"
			   "    scale_z DOUBLE NOT NULL,\n"
			   "    angle DOUBLE NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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

    view = sqlite3_mprintf ("%s_view", name);
    xview = gaiaDoubleQuotedSql (view);
    xname = gaiaDoubleQuotedSql (name);
    xblock = gaiaDoubleQuotedSql (block);
    sql = sqlite3_mprintf ("CREATE VIEW \"%s\" AS "
			   "SELECT b.ROWID AS rowid, i.feature_id AS feature_id, "
			   "i.filename AS filename, i.layer AS layer, i.block_id AS block_id, "
			   "ShiftCoords(ScaleCoords(RotateCoords(b.geometry, i.angle), "
			   "i.scale_x, i.scale_y), i.x, i.y) AS geometry "
			   "FROM \"%s\" AS i JOIN \"%s\" AS b ON "
			   "(b.layer = i.layer AND b.block_id = i.block_id)",
			   xview, xname, xblock);
    free (xname);
    free (xview);
    free (xblock);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE VIEW %s error: %s\n", view,
			sqlite3_errmsg (handle));
	  return 0;
      }
    if (has_viewgeom_rdonly (handle))
	sql = sqlite3_mprintf ("INSERT INTO views_geometry_columns ("
			       "view_name, view_geometry, view_rowid, f_table_name, "
			       "f_geometry_column, read_only) VALUES (Lower(%Q), Lower(%Q), "
			       "Lower(%Q), Lower(%Q), Lower(%Q), 1)", view,
			       "geometry", "rowid", block, "geometry");
    else
	sql = sqlite3_mprintf ("INSERT INTO views_geometry_columns ("
			       "view_name, view_geometry, view_rowid, f_table_name, "
			       "f_geometry_column) VALUES (Lower(%Q), Lower(%Q), "
			       "Lower(%Q), Lower(%Q), Lower(%Q))", view,
			       "geometry", "rowid", block, "geometry");
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("Register Spatial VIEW %s error: %s\n", view,
			sqlite3_errmsg (handle));
	  return 0;
      }
    sqlite3_free (view);

    if (!create_insert_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

DXF_PRIVATE int
create_insert_extra_attr_table (sqlite3 * handle, const char *name,
				char *extra_name, sqlite3_stmt ** xstmt_ext)
{
/* attempting to create the "Insert-mixed-extra-attr" table */
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
			   "f.layer AS layer, f.x AS x, f.y AS y, f.z AS z, f.scale_x AS scale_x, "
			   "f.scale_y AS scale_y, f.scale_z AS scale_z, f.angle AS angle, "
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
check_block_point_table (sqlite3 * handle, const char *name, int srid, int is3D)
{
/* checking if a Block-Point table already exists */
    char *sql;
    int ok_geom = 0;
    int ok_data = 0;
    int ret;
    int i;
    char *xname;
    char **results;
    int n_rows;
    int n_columns;
    int metadata_version = checkSpatialMetaData (handle);

    if (metadata_version == 1)
      {
	  /* legacy metadata style <= v.3.1.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  int dims2d = 0;
	  int dims3d = 0;
	  sql = sqlite3_mprintf ("SELECT srid, type, coord_dimension "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (strcmp ("POINT", results[(i * n_columns) + 1]) == 0)
			  ok_type = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  dims2d = 1;
		      if (strcmp ("XYZ", results[(i * n_columns) + 2]) == 0)
			  dims3d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		if (is3D && dims3d)
		    ok_geom = 1;
		if (!is3D && dims2d)
		    ok_geom = 1;
	    }
      }
    else
      {
	  /* current metadata style >= v.4.0.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  sql = sqlite3_mprintf ("SELECT srid, geometry_type "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1 && !is3D)
			  ok_type = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1001 && is3D)
			  ok_type = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		ok_geom = 1;
	    }
      }

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  int ok_block_id = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
		if (strcasecmp ("block_id", results[(i * n_columns) + 1]) == 0)
		    ok_block_id = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer && ok_block_id)
	      ok_data = 1;
      }
    sqlite3_free_table (results);

    if (ok_geom && ok_data)
	return 1;
    return 0;
}

static int
check_block_line_table (sqlite3 * handle, const char *name, int srid, int is3D)
{
/* checking if a Block-Line table already exists */
    char *sql;
    int ok_geom = 0;
    int ok_data = 0;
    int ret;
    int i;
    char *xname;
    char **results;
    int n_rows;
    int n_columns;
    int metadata_version = checkSpatialMetaData (handle);

    if (metadata_version == 1)
      {
	  /* legacy metadata style <= v.3.1.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  int dims2d = 0;
	  int dims3d = 0;
	  sql = sqlite3_mprintf ("SELECT srid, type, coord_dimension "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (strcmp ("LINESTRING", results[(i * n_columns) + 1]) ==
			  0)
			  ok_type = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  dims2d = 1;
		      if (strcmp ("XYZ", results[(i * n_columns) + 2]) == 0)
			  dims3d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		if (is3D && dims3d)
		    ok_geom = 1;
		if (!is3D && dims2d)
		    ok_geom = 1;
	    }
      }
    else
      {
	  /* current metadata style >= v.4.0.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  sql = sqlite3_mprintf ("SELECT srid, geometry_type "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 2 && !is3D)
			  ok_type = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1002 && is3D)
			  ok_type = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		ok_geom = 1;
	    }
      }

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  int ok_block_id = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
		if (strcasecmp ("block_id", results[(i * n_columns) + 1]) == 0)
		    ok_block_id = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer && ok_block_id)
	      ok_data = 1;
      }
    sqlite3_free_table (results);

    if (ok_geom && ok_data)
	return 1;
    return 0;
}

static int
check_block_polyg_table (sqlite3 * handle, const char *name, int srid, int is3D)
{
/* checking if a Block-Polygon table already exists */
    char *sql;
    int ok_geom = 0;
    int ok_data = 0;
    int ret;
    int i;
    char *xname;
    char **results;
    int n_rows;
    int n_columns;
    int metadata_version = checkSpatialMetaData (handle);

    if (metadata_version == 1)
      {
	  /* legacy metadata style <= v.3.1.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  int dims2d = 0;
	  int dims3d = 0;
	  sql = sqlite3_mprintf ("SELECT srid, type, coord_dimension "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (strcmp ("POLYGON", results[(i * n_columns) + 1]) == 0)
			  ok_type = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  dims2d = 1;
		      if (strcmp ("XYZ", results[(i * n_columns) + 2]) == 0)
			  dims3d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		if (is3D && dims3d)
		    ok_geom = 1;
		if (!is3D && dims2d)
		    ok_geom = 1;
	    }
      }
    else
      {
	  /* current metadata style >= v.4.0.0 */
	  int ok_srid = 0;
	  int ok_type = 0;
	  sql = sqlite3_mprintf ("SELECT srid, geometry_type "
				 "FROM geometry_columns "
				 "WHERE Lower(f_table_name) = Lower(%Q) AND "
				 "Lower(f_geometry_column) = Lower(%Q)", name,
				 "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      return 0;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_srid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 3 && !is3D)
			  ok_type = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 1003 && is3D)
			  ok_type = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_srid && ok_type)
	    {
		ok_geom = 1;
	    }
      }

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  int ok_block_id = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
		if (strcasecmp ("block_id", results[(i * n_columns) + 1]) == 0)
		    ok_block_id = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer && ok_block_id)
	      ok_data = 1;
      }
    sqlite3_free_table (results);

    if (ok_geom && ok_data)
	return 1;
    return 0;
}

static int
check_block_hatch_tables (sqlite3 * handle, const char *name, int srid)
{
/* checking if Block-Hatch tables already exist */
    char *sql;
    int ok_geom = 0;
    int ok_pdata = 0;
    int ok_bdata = 0;
    int ret;
    int i;
    char *xname;
    char *pattern = sqlite3_mprintf ("%s_pattern", name);
    char *xpattern;
    char **results;
    int n_rows;
    int n_columns;
    int metadata_version = checkSpatialMetaData (handle);

    if (metadata_version == 1)
      {
	  /* legacy metadata style <= v.3.1.0 */
	  int ok_bsrid = 0;
	  int ok_btype = 0;
	  int bdims2d = 0;
	  int ok_psrid = 0;
	  int ok_ptype = 0;
	  int pdims2d = 0;
	  sql =
	      sqlite3_mprintf
	      ("SELECT srid, type, coord_dimension "
	       "FROM geometry_columns "
	       "WHERE Lower(f_table_name) = Lower(%Q) AND "
	       "Lower(f_geometry_column) = Lower(%Q)", name, "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      goto stop;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_bsrid = 1;
		      if (strcmp
			  ("MULTIPOLYGON", results[(i * n_columns) + 1]) == 0)
			  ok_btype = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  bdims2d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  sql =
	      sqlite3_mprintf
	      ("SELECT srid, type, coord_dimension "
	       "FROM geometry_columns "
	       "WHERE Lower(f_table_name) = Lower(%Q) AND "
	       "Lower(f_geometry_column) = Lower(%Q)", pattern, "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      goto stop;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_psrid = 1;
		      if (strcmp
			  ("MULTILINESTRING",
			   results[(i * n_columns) + 1]) == 0)
			  ok_ptype = 1;
		      if (strcmp ("XY", results[(i * n_columns) + 2]) == 0)
			  pdims2d = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_bsrid && ok_btype && bdims2d && ok_psrid && ok_ptype
	      && pdims2d)
	      ok_geom = 1;
      }
    else
      {
	  /* current metadata style >= v.4.0.0 */
	  int ok_psrid = 0;
	  int ok_ptype = 0;
	  int ok_bsrid = 0;
	  int ok_btype = 0;
	  sql =
	      sqlite3_mprintf ("SELECT srid, geometry_type "
			       "FROM geometry_columns "
			       "WHERE Lower(f_table_name) = Lower(%Q) AND "
			       "Lower(f_geometry_column) = Lower(%Q)", name,
			       "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      goto stop;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_bsrid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 6)
			  ok_btype = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  sql =
	      sqlite3_mprintf ("SELECT srid, geometry_type "
			       "FROM geometry_columns "
			       "WHERE Lower(f_table_name) = Lower(%Q) AND "
			       "Lower(f_geometry_column) = Lower(%Q)", pattern,
			       "geometry");
	  ret =
	      sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
				 NULL);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	      goto stop;
	  if (n_rows > 0)
	    {
		for (i = 1; i <= n_rows; i++)
		  {
		      if (atoi (results[(i * n_columns) + 0]) == srid)
			  ok_psrid = 1;
		      if (atoi (results[(i * n_columns) + 1]) == 5)
			  ok_ptype = 1;
		  }
	    }
	  sqlite3_free_table (results);
	  if (ok_bsrid && ok_btype && ok_psrid && ok_ptype)
	      ok_geom = 1;
      }

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto stop;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  int ok_block_id = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
		if (strcasecmp ("block_id", results[(i * n_columns) + 1]) == 0)
		    ok_block_id = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer && ok_block_id)
	      ok_bdata = 1;
      }
    sqlite3_free_table (results);

    xpattern = gaiaDoubleQuotedSql (pattern);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xpattern);
    free (xpattern);
    ret = sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto stop;
    if (n_rows > 0)
      {
	  int ok_feature_id = 0;
	  int ok_filename = 0;
	  int ok_layer = 0;
	  int ok_block_id = 0;
	  for (i = 1; i <= n_rows; i++)
	    {
		if (strcasecmp ("feature_id", results[(i * n_columns) + 1]) ==
		    0)
		    ok_feature_id = 1;
		if (strcasecmp ("filename", results[(i * n_columns) + 1]) == 0)
		    ok_filename = 1;
		if (strcasecmp ("layer", results[(i * n_columns) + 1]) == 0)
		    ok_layer = 1;
		if (strcasecmp ("block_id", results[(i * n_columns) + 1]) == 0)
		    ok_block_id = 1;
	    }
	  if (ok_feature_id && ok_filename && ok_layer && ok_block_id)
	      ok_pdata = 1;
      }
    sqlite3_free_table (results);

    if (ok_geom && ok_bdata && ok_pdata)
      {
	  sqlite3_free (pattern);
	  return 1;
      }
  stop:
    sqlite3_free (pattern);
    return 0;
}

static int
create_block_text_table (sqlite3 * handle, const char *name, int srid,
			 int text3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Block-Text" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    char *idx_name;
    char *xidx_name;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    block_id TEXT NOT NULL,\n"
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
    idx_name = sqlite3_mprintf ("idx_%s", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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
    if (!create_block_text_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_block_point_table (sqlite3 * handle, const char *name, int srid,
			  int point3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Block-Point" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    char *idx_name;
    char *xidx_name;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    block_id TEXT NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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
    if (!create_block_point_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_block_line_table (sqlite3 * handle, const char *name, int srid,
			 int line3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Block-Line" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    char *idx_name;
    char *xidx_name;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    block_id TEXT NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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
    if (!create_block_line_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_block_polyg_table (sqlite3 * handle, const char *name, int srid,
			  int polyg3D, sqlite3_stmt ** xstmt)
{
/* attempting to create the "Block-Polyg" table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    char *xname;
    char *idx_name;
    char *xidx_name;
    *xstmt = NULL;

    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    block_id TEXT NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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
    if (!create_block_polyg_stmt (handle, name, &stmt))
	return 0;

    *xstmt = stmt;
    return 1;
}

static int
create_block_hatch_tables (sqlite3 * handle, const char *name, int srid,
			   sqlite3_stmt ** xstmt, sqlite3_stmt ** xstmt2)
{
/* attempting to create the "Block-Hatch" tables */
    char *sql;
    int ret;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt2;
    char *xname;
    char *pattern;
    char *xpattern;
    char *fk_name;
    char *xfk_name;
    char *idx_name;
    char *xidx_name;
    *xstmt = NULL;
    *xstmt2 = NULL;

/* creating the Block-Hatch-Boundary table */
    xname = gaiaDoubleQuotedSql (name);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ("
			   "    feature_id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
			   "    filename TEXT NOT NULL, \n"
			   "    layer TEXT NOT NULL,\n"
			   "    block_id TEXT NOT NULL)", xname);
    free (xname);
    ret = sqlite3_exec (handle, sql, NULL, NULL, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE %s error: %s\n", name,
			sqlite3_errmsg (handle));
	  return 0;
      }
    idx_name = sqlite3_mprintf ("idx_%s", name);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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

/* creating the Block-Hatch-Pattern table */
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
			   "    block_id TEXT NOT NULL,\n"
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
    idx_name = sqlite3_mprintf ("idx_%s", pattern);
    xidx_name = gaiaDoubleQuotedSql (idx_name);
    xname = gaiaDoubleQuotedSql (name);
    sql =
	sqlite3_mprintf
	("CREATE INDEX \"%s\" ON \"%s\" (layer, block_id)", xidx_name, xname);
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
    if (!create_block_hatch_boundary_stmt (handle, name, &stmt))
	return 0;
    if (!create_block_hatch_pattern_stmt (handle, name, &stmt2))
	return 0;

    *xstmt = stmt;
    *xstmt2 = stmt2;
    return 1;
}

DXF_PRIVATE int
check_unclosed_polyg (gaiaDxfPolylinePtr pg, int is3d)
{
/* checking for unclosed Rings (exterior) */
    int last = pg->points - 1;
    if (is3d)
      {
	  if (*(pg->x + 0) == *(pg->x + last) && *(pg->y + 0) == *(pg->y + last)
	      && *(pg->z + 0) == *(pg->z + last))
	      return 0;
      }
    else
      {
	  if (*(pg->x + 0) == *(pg->x + last)
	      && *(pg->y + 0) == *(pg->y + last))
	      return 0;
      }
    return 1;
}

DXF_PRIVATE int
check_unclosed_hole (gaiaDxfHolePtr hole, int is3d)
{
/* checking for unclosed Rings (interior) */
    int last = hole->points - 1;
    if (is3d)
      {
	  if (*(hole->x + 0) == *(hole->x + last)
	      && *(hole->y + 0) == *(hole->y + last)
	      && *(hole->z + 0) == *(hole->z + last))
	      return 0;
      }
    else
      {
	  if (*(hole->x + 0) == *(hole->x + last)
	      && *(hole->y + 0) == *(hole->y + last))
	      return 0;
      }
    return 1;
}

static int
import_blocks (sqlite3 * handle, gaiaDxfParserPtr dxf, int append)
{
/* populating the target DB - importing BLOCK geometries */
    gaiaDxfTextPtr txt;
    gaiaDxfPointPtr pt;
    gaiaDxfPolylinePtr ln;
    gaiaDxfPolylinePtr pg;
    gaiaDxfHatchPtr htc;
    int ret;
    unsigned char *blob;
    int blob_size;
    gaiaGeomCollPtr geom;
    int error = 0;
    char *name;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt_text_2d = NULL;
    sqlite3_stmt *stmt_text_3d = NULL;
    sqlite3_stmt *stmt_point_2d = NULL;
    sqlite3_stmt *stmt_point_3d = NULL;
    sqlite3_stmt *stmt_line_2d = NULL;
    sqlite3_stmt *stmt_line_3d = NULL;
    sqlite3_stmt *stmt_polyg_2d = NULL;
    sqlite3_stmt *stmt_polyg_3d = NULL;
    sqlite3_stmt *stmt_hatch_boundary = NULL;
    sqlite3_stmt *stmt_hatch_pattern = NULL;
    int text = 0;
    int point = 0;
    int line = 0;
    int polyg = 0;
    int hatch = 0;
    int text2D = 0;
    int point2D = 0;
    int line2D = 0;
    int polyg2D = 0;
    int text3D = 0;
    int point3D = 0;
    int line3D = 0;
    int polyg3D = 0;
    int iv;
    gaiaDxfHolePtr hole;
    int num_holes;
    gaiaLinestringPtr p_ln;
    gaiaPolygonPtr p_pg;
    gaiaRingPtr p_rng;
    sqlite3_int64 feature_id;
    gaiaDxfHatchSegmPtr segm;

    gaiaDxfBlockPtr blk = dxf->first_block;
    while (blk != NULL)
      {
	  /* exploring Blocks by type */
	  if (blk->hasInsert == 0)
	    {
		blk = blk->next;
		continue;
	    }
	  if (blk->first_text != NULL)
	    {
		text = 1;
		if (blk->is3Dtext)
		    text3D = 1;
		else
		    text2D = 1;
	    }
	  if (blk->first_point != NULL)
	    {
		point = 1;
		if (blk->is3Dpoint)
		    point3D = 1;
		else
		    point2D = 1;
	    }
	  if (blk->first_line != NULL)
	    {
		line = 1;
		if (blk->is3Dline)
		    line3D = 1;
		else
		    line2D = 1;
	    }
	  if (blk->first_polyg != NULL)
	    {
		polyg = 1;
		if (blk->is3Dpolyg)
		    polyg3D = 1;
		else
		    polyg2D = 1;
	    }
	  if (blk->first_hatch != NULL)
	      hatch = 1;
	  blk = blk->next;
      }

    if (text && text2D)
      {
	  /* preparing to insert into the Block Text 2D table */
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("block_text_2d");
	  else
	      name = sqlite3_mprintf ("%sblock_text_2d", dxf->prefix);
	  if (append && check_block_text_table (handle, name, dxf->srid, 0))
	    {
		/* appending into the already existing table */
		if (!create_block_text_stmt (handle, name, &stmt_text_2d))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_block_text_table
		    (handle, name, dxf->srid, 0, &stmt_text_2d))
		    return 0;
	    }
	  sqlite3_free (name);
      }
    if (text && text3D)
      {
	  /* preparing to insert into the Block Text 3D table */
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("block_text_3d");
	  else
	      name = sqlite3_mprintf ("%sblock_text_3d", dxf->prefix);
	  if (append && check_block_text_table (handle, name, dxf->srid, 1))
	    {
		/* appending into the already existing table */
		if (!create_block_text_stmt (handle, name, &stmt_text_3d))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_block_text_table
		    (handle, name, dxf->srid, 1, &stmt_text_3d))
		    return 0;
	    }
	  sqlite3_free (name);
      }
    if (point && point2D)
      {
	  /* preparing to insert into the Block Point 2D table */
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("block_point_2d");
	  else
	      name = sqlite3_mprintf ("%sblock_point_2d", dxf->prefix);
	  if (append && check_block_point_table (handle, name, dxf->srid, 0))
	    {
		/* appending into the already existing table */
		if (!create_block_point_stmt (handle, name, &stmt_point_2d))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_block_point_table
		    (handle, name, dxf->srid, 0, &stmt_point_2d))
		    return 0;
	    }
	  sqlite3_free (name);
      }
    if (point && point3D)
      {
	  /* preparing to insert into the Block Point 3D table */
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("block_point_3d");
	  else
	      name = sqlite3_mprintf ("%sblock_point_3d", dxf->prefix);
	  if (append && check_block_point_table (handle, name, dxf->srid, 1))
	    {
		/* appending into the already existing table */
		if (!create_block_point_stmt (handle, name, &stmt_point_3d))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_block_point_table
		    (handle, name, dxf->srid, 1, &stmt_point_3d))
		    return 0;
	    }
	  sqlite3_free (name);
      }
    if (line && line2D)
      {
	  /* preparing to insert into the Block Line 2D table */
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("block_line_2d");
	  else
	      name = sqlite3_mprintf ("%sblock_line_2d", dxf->prefix);
	  if (append && check_block_line_table (handle, name, dxf->srid, 0))
	    {
		/* appending into the already existing table */
		if (!create_block_line_stmt (handle, name, &stmt_line_2d))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_block_line_table
		    (handle, name, dxf->srid, 0, &stmt_line_2d))
		    return 0;
	    }
	  sqlite3_free (name);
      }
    if (line && line3D)
      {
	  /* preparing to insert into the Block Line 3D table */
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("block_line_3d");
	  else
	      name = sqlite3_mprintf ("%sblock_line_3d", dxf->prefix);
	  if (append && check_block_line_table (handle, name, dxf->srid, 1))
	    {
		/* appending into the already existing table */
		if (!create_block_line_stmt (handle, name, &stmt_line_3d))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_block_line_table
		    (handle, name, dxf->srid, 1, &stmt_line_3d))
		    return 0;
	    }
	  sqlite3_free (name);
      }
    if (polyg && polyg2D)
      {
	  /* preparing to insert into the Block Polyg 2D table */
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("block_polyg_2d");
	  else
	      name = sqlite3_mprintf ("%sblock_polyg_2d", dxf->prefix);
	  if (append && check_block_polyg_table (handle, name, dxf->srid, 0))
	    {
		/* appending into the already existing table */
		if (!create_block_polyg_stmt (handle, name, &stmt_polyg_2d))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_block_polyg_table
		    (handle, name, dxf->srid, 0, &stmt_polyg_2d))
		    return 0;
	    }
	  sqlite3_free (name);
      }
    if (polyg && polyg3D)
      {
	  /* preparing to insert into the Block Polyg 3D table */
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("block_polyg_3d");
	  else
	      name = sqlite3_mprintf ("%sblock_polyg_3d", dxf->prefix);
	  if (append && check_block_polyg_table (handle, name, dxf->srid, 1))
	    {
		/* appending into the already existing table */
		if (!create_block_polyg_stmt (handle, name, &stmt_polyg_3d))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_block_polyg_table
		    (handle, name, dxf->srid, 1, &stmt_polyg_3d))
		    return 0;
	    }
	  sqlite3_free (name);
      }
    if (hatch)
      {
	  /* preparing to insert into the Block Hatch table */
	  if (dxf->prefix == NULL)
	      name = sqlite3_mprintf ("block_hatch_2d");
	  else
	      name = sqlite3_mprintf ("%sblock_hatch_2d", dxf->prefix);
	  if (append && check_block_hatch_tables (handle, name, dxf->srid))
	    {
		/* appending into the already existing table */
		if (!create_block_hatch_boundary_stmt
		    (handle, name, &stmt_hatch_boundary))
		    return 0;
		if (!create_block_hatch_pattern_stmt
		    (handle, name, &stmt_hatch_pattern))
		    return 0;
	    }
	  else
	    {
		/* creating a new table */
		if (!create_block_hatch_tables
		    (handle, name, dxf->srid, &stmt_hatch_boundary,
		     &stmt_hatch_pattern))
		    return 0;
	    }
	  sqlite3_free (name);
      }

    ret = sqlite3_exec (handle, "BEGIN", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("BEGIN %s error: %s\n", name, sqlite3_errmsg (handle));
	  error = 1;
	  goto stop;
      }

    blk = dxf->first_block;
    while (blk != NULL)
      {
	  if (blk->hasInsert == 0)
	    {
		blk = blk->next;
		continue;
	    }
	  txt = blk->first_text;
	  while (txt)
	    {
		if (blk->is3Dtext)
		    stmt = stmt_text_3d;
		else
		    stmt = stmt_text_2d;
		sqlite3_reset (stmt);
		sqlite3_clear_bindings (stmt);
		sqlite3_bind_text (stmt, 1, dxf->filename,
				   strlen (dxf->filename), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 2, blk->layer_name,
				   strlen (blk->layer_name), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 3, blk->block_id,
				   strlen (blk->block_id), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 4, txt->label,
				   strlen (txt->label), SQLITE_STATIC);
		sqlite3_bind_double (stmt, 5, txt->angle);
		if (blk->is3Dtext)
		    geom = gaiaAllocGeomCollXYZ ();
		else
		    geom = gaiaAllocGeomColl ();
		geom->Srid = dxf->srid;
		if (blk->is3Dtext)
		    gaiaAddPointToGeomCollXYZ (geom, txt->x, txt->y, txt->z);
		else
		    gaiaAddPointToGeomColl (geom, txt->x, txt->y);
		gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_size);
		gaiaFreeGeomColl (geom);
		sqlite3_bind_blob (stmt, 6, blob, blob_size, free);
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      spatialite_e ("INSERT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      ret = sqlite3_exec (handle, "ROLLBACK", NULL, NULL, NULL);
		      error = 1;
		      goto stop;
		  }
		txt = txt->next;
	    }
	  pt = blk->first_point;
	  while (pt)
	    {
		if (blk->is3Dpoint)
		    stmt = stmt_point_3d;
		else
		    stmt = stmt_point_2d;
		sqlite3_reset (stmt);
		sqlite3_clear_bindings (stmt);
		sqlite3_bind_text (stmt, 1, dxf->filename,
				   strlen (dxf->filename), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 2, blk->layer_name,
				   strlen (blk->layer_name), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 3, blk->block_id,
				   strlen (blk->block_id), SQLITE_STATIC);
		if (blk->is3Dpoint)
		    geom = gaiaAllocGeomCollXYZ ();
		else
		    geom = gaiaAllocGeomColl ();
		geom->Srid = dxf->srid;
		if (blk->is3Dpoint)
		    gaiaAddPointToGeomCollXYZ (geom, pt->x, pt->y, pt->z);
		else
		    gaiaAddPointToGeomColl (geom, pt->x, pt->y);
		gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_size);
		gaiaFreeGeomColl (geom);
		sqlite3_bind_blob (stmt, 4, blob, blob_size, free);
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      spatialite_e ("INSERT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      ret = sqlite3_exec (handle, "ROLLBACK", NULL, NULL, NULL);
		      error = 1;
		      goto stop;
		  }
		pt = pt->next;
	    }
	  ln = blk->first_line;
	  while (ln)
	    {
		if (blk->is3Dline)
		    stmt = stmt_line_3d;
		else
		    stmt = stmt_line_2d;
		sqlite3_reset (stmt);
		sqlite3_clear_bindings (stmt);
		sqlite3_bind_text (stmt, 1, dxf->filename,
				   strlen (dxf->filename), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 2, blk->layer_name,
				   strlen (blk->layer_name), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 3, blk->block_id,
				   strlen (blk->block_id), SQLITE_STATIC);
		if (blk->is3Dline)
		    geom = gaiaAllocGeomCollXYZ ();
		else
		    geom = gaiaAllocGeomColl ();
		geom->Srid = dxf->srid;
		gaiaAddLinestringToGeomColl (geom, ln->points);
		p_ln = geom->FirstLinestring;
		for (iv = 0; iv < ln->points; iv++)
		  {
		      if (blk->is3Dline)
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
		sqlite3_bind_blob (stmt, 4, blob, blob_size, free);
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      spatialite_e ("INSERT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      ret = sqlite3_exec (handle, "ROLLBACK", NULL, NULL, NULL);
		      error = 1;
		      goto stop;
		  }
		ln = ln->next;
	    }
	  pg = blk->first_polyg;
	  while (pg)
	    {
		int unclosed = check_unclosed_polyg (pg, blk->is3Dpolyg);
		if (blk->is3Dpolyg)
		    stmt = stmt_polyg_3d;
		else
		    stmt = stmt_polyg_2d;
		sqlite3_reset (stmt);
		sqlite3_clear_bindings (stmt);
		sqlite3_bind_text (stmt, 1, dxf->filename,
				   strlen (dxf->filename), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 2, blk->layer_name,
				   strlen (blk->layer_name), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 3, blk->block_id,
				   strlen (blk->block_id), SQLITE_STATIC);
		if (blk->is3Dpolyg)
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
		      if (blk->is3Dpolyg)
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
		      if (blk->is3Dpolyg)
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
		      int unclosed = check_unclosed_hole (hole, polyg3D);
		      p_rng =
			  gaiaAddInteriorRing (p_pg, num_holes,
					       hole->points + unclosed);
		      for (iv = 0; iv < hole->points; iv++)
			{
			    if (polyg3D)
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
			    if (polyg3D)
			      {
				  gaiaSetPointXYZ (p_rng->Coords, hole->points,
						   *(hole->x + 0),
						   *(hole->y + 0),
						   *(hole->z + 0));
			      }
			    else
			      {
				  gaiaSetPoint (p_rng->Coords, hole->points,
						*(hole->x + 0), *(hole->y + 0));
			      }
			}
		      num_holes++;
		      hole = hole->next;
		  }
		gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_size);
		gaiaFreeGeomColl (geom);
		sqlite3_bind_blob (stmt, 4, blob, blob_size, free);
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      spatialite_e ("INSERT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      ret = sqlite3_exec (handle, "ROLLBACK", NULL, NULL, NULL);
		      error = 1;
		      goto stop;
		  }
		pg = pg->next;
	    }
	  htc = blk->first_hatch;
	  while (htc)
	    {
		/* inserting the Boundary Geometry */
		stmt = stmt_hatch_boundary;
		sqlite3_reset (stmt);
		sqlite3_clear_bindings (stmt);
		sqlite3_bind_text (stmt, 1, dxf->filename,
				   strlen (dxf->filename), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 2, blk->layer_name,
				   strlen (blk->layer_name), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 3, blk->block_id,
				   strlen (blk->block_id), SQLITE_STATIC);
		if (htc->boundary == NULL)
		    sqlite3_bind_null (stmt, 4);
		else
		  {
		      gaiaToSpatiaLiteBlobWkb (htc->boundary, &blob,
					       &blob_size);
		      sqlite3_bind_blob (stmt, 4, blob, blob_size, free);
		  }
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      spatialite_e ("INSERT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      ret = sqlite3_exec (handle, "ROLLBACK", NULL, NULL, NULL);
		      error = 1;
		      goto stop;
		  }
		feature_id = sqlite3_last_insert_rowid (handle);

		/* inserting the Pattern Geometry */
		stmt = stmt_hatch_pattern;
		sqlite3_reset (stmt);
		sqlite3_clear_bindings (stmt);
		sqlite3_bind_int64 (stmt, 1, feature_id);
		sqlite3_bind_text (stmt, 2, dxf->filename,
				   strlen (dxf->filename), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 3, blk->layer_name,
				   strlen (blk->layer_name), SQLITE_STATIC);
		sqlite3_bind_text (stmt, 4, blk->block_id,
				   strlen (blk->block_id), SQLITE_STATIC);
		if (htc->first_out == NULL)
		    sqlite3_bind_null (stmt, 5);
		else
		  {
		      geom = gaiaAllocGeomColl ();
		      geom->Srid = dxf->srid;
		      geom->DeclaredType = GAIA_MULTILINESTRING;
		      segm = htc->first_out;
		      while (segm != NULL)
			{
			    gaiaLinestringPtr p_ln =
				gaiaAddLinestringToGeomColl (geom, 2);
			    gaiaSetPoint (p_ln->Coords, 0, segm->x0, segm->y0);
			    gaiaSetPoint (p_ln->Coords, 1, segm->x1, segm->y1);
			    segm = segm->next;
			}
		      gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_size);
		      gaiaFreeGeomColl (geom);
		      sqlite3_bind_blob (stmt, 5, blob, blob_size, free);
		  }
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      spatialite_e ("INSERT %s error: %s\n", name,
				    sqlite3_errmsg (handle));
		      ret = sqlite3_exec (handle, "ROLLBACK", NULL, NULL, NULL);
		      error = 1;
		      goto stop;
		  }
		htc = htc->next;
	    }
	  blk = blk->next;
      }

    ret = sqlite3_exec (handle, "COMMIT", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("COMMIT text_layer error: %s\n",
			sqlite3_errmsg (handle));
	  error = 1;
	  goto stop;
      }

  stop:
    if (stmt_text_2d != NULL)
	sqlite3_finalize (stmt_text_2d);
    if (stmt_text_3d != NULL)
	sqlite3_finalize (stmt_text_3d);
    if (stmt_point_2d != NULL)
	sqlite3_finalize (stmt_point_2d);
    if (stmt_point_3d != NULL)
	sqlite3_finalize (stmt_point_3d);
    if (stmt_line_2d != NULL)
	sqlite3_finalize (stmt_line_2d);
    if (stmt_line_3d != NULL)
	sqlite3_finalize (stmt_line_3d);
    if (stmt_polyg_2d != NULL)
	sqlite3_finalize (stmt_polyg_2d);
    if (stmt_polyg_3d != NULL)
	sqlite3_finalize (stmt_polyg_3d);
    if (stmt_hatch_boundary != NULL)
	sqlite3_finalize (stmt_hatch_boundary);
    if (stmt_hatch_pattern != NULL)
	sqlite3_finalize (stmt_hatch_pattern);
    if (error)
	return 0;
    return 1;
}

GAIAGEO_DECLARE int
gaiaLoadFromDxfParser (sqlite3 * handle,
		       gaiaDxfParserPtr dxf, int mode, int append)
{
/* populating the target DB */
    int ret;

    if (dxf == NULL)
	return 0;
    if (dxf->first_layer == NULL)
	return 0;

    if (dxf->first_block != NULL)
      {
	  if (!import_blocks (handle, dxf, append))
	      return 0;
      }

    if (mode == GAIA_DXF_IMPORT_MIXED)
	ret = import_mixed (handle, dxf, append);
    else
	ret = import_by_layer (handle, dxf, append);
    return ret;
}
