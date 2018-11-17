/*

 check_fdo2.c -- SpatiaLite Test Case

 Author: Sandro Furieri <a.furieri@lqt.it>

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
 
Portions created by the Initial Developer are Copyright (C) 2011
the Initial Developer. All Rights Reserved.

Contributor(s):
Brad Hards <bradh@frogmouth.net>

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"
#include "spatialite.h"

int
main (int argc, char *argv[])
{
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    int suffix_len = 16 * 1024;	/* 16 KB suffix */
    char *suffix;
    char *pt_2d_wkt;
    char *pt_3d_wkt;
    char *geometry;
    char *sql;
    char **results;
    int rows;
    int columns;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory database: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }

    spatialite_init_ex (handle, cache, 0);

/* FDO initialization */
    sql = "SELECT InitFDOSpatialMetadata()";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

/* setting up very long table and column names */
    suffix = malloc (suffix_len);
    memset (suffix, 'z', suffix_len);
    suffix[suffix_len - 1] = '\0';

    pt_2d_wkt = sqlite3_mprintf ("pt_2d_wkt_%s", suffix);
    pt_3d_wkt = sqlite3_mprintf ("pt_3d_wkt_%s", suffix);
    geometry = sqlite3_mprintf ("geometry_%s", suffix);

/* creating a Point 2D WKT table */
    sql =
	sqlite3_mprintf
	("CREATE TABLE %s (id INTEGER, name TEXT, value DOUBLE)", pt_2d_wkt);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }
    sql =
	sqlite3_mprintf ("SELECT AddFDOGeometryColumn(%Q, %Q, -1, 1, 2, 'WKT')",
			 pt_2d_wkt, geometry);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }

/* creating a Point 3D WKT table */
    sql =
	sqlite3_mprintf ("CREATE TABLE %s (id INTEGER, pic1 BLOB, pic2 BLOB)",
			 pt_3d_wkt);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }
    sql =
	sqlite3_mprintf ("SELECT AddFDOGeometryColumn(%Q, %Q, -1, 1, 3, 'WKT')",
			 pt_3d_wkt, geometry);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }

/* FDO start-up */
    sql = "SELECT AutoFDOStart()";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }

/* Inserting into pt_2d_wkt */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, value, %s) VALUES "
			   "(1, 'alpha', 0.1, GeomFromText('POINT(1 2)', -1))",
			   pt_2d_wkt, geometry);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -8;
      }

/* Inserting (take two) into pt_2d_wkt */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, value, %s) VALUES "
			   "(2, 'beta', 0.2, GeomFromText('POINT(2 3)', -1))",
			   pt_2d_wkt, geometry);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }

/* Updating pt_2d_wkt */
    sql = sqlite3_mprintf ("UPDATE %s SET name = 'beta', value = 0.2, "
			   "%s = GeomFromText('POINT(10 20)', -1) WHERE id = 1",
			   pt_2d_wkt, geometry);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }

/* Deleting form pt_2d_wkt */
    sql = sqlite3_mprintf ("DELETE FROM %s WHERE id = 1", pt_2d_wkt);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

/* Inserting into pt_3d_wkt */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, pic1, pic2, %s) VALUES "
			   "(1, zeroblob(16), NULL, GeomFromText('POINTZ(1 2 3)', -1))",
			   pt_3d_wkt, geometry);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }

/* checking pt_2d_wkt */
    sql = sqlite3_mprintf ("SELECT AsText(%s) FROM %s", geometry, pt_2d_wkt);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -14;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -15;
      }
    if (strcmp (results[1], "POINT(2 3)") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -16;
      }
    sqlite3_free_table (results);

/* checking pt_3d_wkt */
    sql = sqlite3_mprintf ("SELECT AsText(%s) FROM %s", geometry, pt_3d_wkt);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -17;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -18;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -19;
      }
    if (strcmp (results[1], "POINT Z(1 2 3)") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -20;
      }
    sqlite3_free_table (results);

/* dropping an FDO virtual table */
    sql = sqlite3_mprintf ("DROP TABLE %s", pt_2d_wkt);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }

/* discarding an FDO Geometry column */
    sql =
	sqlite3_mprintf ("SELECT DiscardFDOGeometryColumn(%Q, %Q)", pt_3d_wkt,
			 geometry);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }

/* recovering an FDO Geometry column */
    sql =
	sqlite3_mprintf
	("SELECT RecoverFDOGeometryColumn(%Q, %Q, -1, 1, 3, 'WKT')", pt_3d_wkt,
	 geometry);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }

/* FDO shut-down */
    sql = "SELECT AutoFDOStop()";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -25;
      }

    spatialite_cleanup_ex (cache);

    sqlite3_free (pt_2d_wkt);
    sqlite3_free (pt_3d_wkt);
    sqlite3_free (geometry);
    free (suffix);

    spatialite_shutdown ();
    return 0;
}
