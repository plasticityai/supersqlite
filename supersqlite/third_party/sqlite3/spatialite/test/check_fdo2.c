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
    const char *sql;
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

/* creating a Point 2D WKT table */
    sql = "CREATE TABLE pt_2d_wkt (id INTEGER, name TEXT, value DOUBLE)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }
    sql = "SELECT AddFDOGeometryColumn('pt_2d_wkt', 'g', -1, 1, 2, 'WKT')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }

/* creating a Point 3D WKT table */
    sql = "CREATE TABLE pt_3d_wkt (id INTEGER, pic1 BLOB, pic2 BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }
    sql = "SELECT AddFDOGeometryColumn('pt_3d_wkt', 'g', -1, 1, 3, 'WKT')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }

/* creating a Point 2D WKB table */
    sql = "CREATE TABLE pt_2d_wkb (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }
    sql = "SELECT AddFDOGeometryColumn('pt_2d_wkb', 'g', -1, 1, 2, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }

/* creating a Point 3D WKB table */
    sql = "CREATE TABLE pt_3d_wkb (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -8;
      }
    sql = "SELECT AddFDOGeometryColumn('pt_3d_wkb', 'g', -1, 1, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }

/* creating a Point 3D WKB table */
    sql = "CREATE TABLE pt_3d_spl (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }
    sql =
	"SELECT AddFDOGeometryColumn('pt_3d_spl', 'g', -1, 1, 3, 'SPATIALITE')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

/* creating a Linestring 3D WKT table */
    sql = "CREATE TABLE ln_3d_wkt (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }
    sql = "SELECT AddFDOGeometryColumn('ln_3d_wkt', 'g', -1, 2, 3, 'WKT')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }

/* creating a Linestring 3D WKB table */
    sql = "CREATE TABLE ln_3d_wkb (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }
    sql = "SELECT AddFDOGeometryColumn('ln_3d_wkb', 'g', -1, 2, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -15;
      }

/* creating a Polygon 3D WKT table */
    sql = "CREATE TABLE pg_3d_wkt (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -16;
      }
    sql = "SELECT AddFDOGeometryColumn('pg_3d_wkt', 'g', -1, 3, 3, 'WKT')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -17;
      }

/* creating a Polygon 3D WKB table */
    sql = "CREATE TABLE pg_3d_wkb (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -18;
      }
    sql = "SELECT AddFDOGeometryColumn('pg_3d_wkb', 'g', -1, 3, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -19;
      }

/* creating a MultiPoint 3D WKT table */
    sql = "CREATE TABLE mpt_3d_wkt (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -20;
      }
    sql = "SELECT AddFDOGeometryColumn('mpt_3d_wkt', 'g', -1, 4, 3, 'WKT')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }

/* creating a MultiPoint 3D WKB table */
    sql = "CREATE TABLE mpt_3d_wkb (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }
    sql = "SELECT AddFDOGeometryColumn('mpt_3d_wkb', 'g', -1, 4, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }

/* creating a MultiLinestring 3D WKT table */
    sql = "CREATE TABLE mln_3d_wkt (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }
    sql = "SELECT AddFDOGeometryColumn('mln_3d_wkt', 'g', -1, 5, 3, 'WKT')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }

/* creating a MultiLinestring 3D WKB table */
    sql = "CREATE TABLE mln_3d_wkb (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -26;
      }
    sql = "SELECT AddFDOGeometryColumn('mln_3d_wkb', 'g', -1, 5, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -27;
      }

/* creating a MultiPolygon 3D WKT table */
    sql = "CREATE TABLE mpg_3d_wkt (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -28;
      }
    sql = "SELECT AddFDOGeometryColumn('mpg_3d_wkt', 'g', -1, 6, 3, 'WKT')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }

/* creating a MultiPolygon 3D WKB table */
    sql = "CREATE TABLE mpg_3d_wkb (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }
    sql = "SELECT AddFDOGeometryColumn('mpg_3d_wkb', 'g', -1, 6, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -31;
      }

/* creating a GeometryCollection 3D WKT table */
    sql = "CREATE TABLE gcoll_3d_wkt (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -32;
      }
    sql = "SELECT AddFDOGeometryColumn('gcoll_3d_wkt', 'g', -1, 7, 3, 'WKT')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -33;
      }

/* creating a GeometryCollection 3D WKB table */
    sql = "CREATE TABLE gcoll_3d_wkb (id INTEGER)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -34;
      }
    sql = "SELECT AddFDOGeometryColumn('gcoll_3d_wkb', 'g', -1, 7, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -35;
      }

/* FDO start-up */
    sql = "SELECT AutoFDOStart()";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -36;
      }

/* Inserting into pt_2d_wkt */
    sql =
	"INSERT INTO fdo_pt_2d_wkt (id, name, value, g) VALUES (1, 'alpha', 0.1, GeomFromText('POINT(1 2)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -37;
      }

/* Updating pt_2d_wkt */
    sql =
	"UPDATE fdo_pt_2d_wkt SET name = 'beta', value = 0.2, g = GeomFromText('POINT(10 20)', -1) WHERE id = 1";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -38;
      }

/* Deleting form pt_2d_wkt */
    sql = "DELETE FROM fdo_pt_2d_wkt";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -39;
      }

/* Inserting into pt_3d_wkt */
    sql =
	"INSERT INTO fdo_pt_3d_wkt (id, pic1, pic2, g) VALUES (1, zeroblob(16), NULL, GeomFromText('POINTZ(1 2 3)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -40;
      }

/* Updating pt_3d_wkt */
    sql =
	"UPDATE fdo_pt_3d_wkt SET pic1 = NULL, pic2 = zeroblob(8), g = GeomFromText('POINTZ(10 20 30)', -1) WHERE id = 1";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -41;
      }

/* Inserting into pt_2d_wkb */
    sql =
	"INSERT INTO fdo_pt_2d_wkb (id, g) VALUES (1, GeomFromText('POINT(1 2)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -42;
      }

/* Updating pt_2d_wkb */
    sql =
	"UPDATE fdo_pt_2d_wkb SET g = GeomFromText('POINT(10 20)', -1) WHERE id = 1";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -43;
      }

/* Inserting into pt_3d_wkb */
    sql =
	"INSERT INTO fdo_pt_3d_wkb (id, g) VALUES (1, GeomFromText('POINTZ(1 2 3)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -44;
      }

/* Updating pt_3d_wkb */
    sql =
	"UPDATE fdo_pt_3d_wkb SET g = GeomFromText('POINTZ(10 20 30)', -1) WHERE id = 1";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -45;
      }

/* Inserting into pt_3d_spl */
    sql =
	"INSERT INTO fdo_pt_3d_spl (id, g) VALUES (1, GeomFromText('POINTZ(1 2 3)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -46;
      }

/* Updating pt_3d_spl */
    sql =
	"UPDATE fdo_pt_3d_spl SET g = GeomFromText('POINTZ(10 20 30)', -1) WHERE id = 1";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -47;
      }

/* Inserting into ln_3d_wkt */
    sql =
	"INSERT INTO fdo_ln_3d_wkt (id, g) VALUES (1, GeomFromText('LINESTRINGZ(1 2 3, 4 5 6)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -48;
      }

/* Inserting into ln_3d_wkb */
    sql =
	"INSERT INTO fdo_ln_3d_wkb (id, g) VALUES (1, GeomFromText('LINESTRINGZ(1 2 3, 4 5 6)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -49;
      }

/* Inserting into ln_3d_wkb */
    sql =
	"INSERT INTO fdo_ln_3d_wkb (id, g) VALUES (2, GeomFromText('LINESTRINGM(1 2 10, 4 5 11)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -50;
      }

/* Inserting into ln_3d_wkb */
    sql =
	"INSERT INTO fdo_ln_3d_wkb (id, g) VALUES (3, GeomFromText('LINESTRINGZM(1 2 3 10, 4 5 6 11)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -51;
      }

/* Inserting into ln_3d_wkb */
    sql =
	"INSERT INTO fdo_ln_3d_wkb (id, g) VALUES (4, GeomFromText('LINESTRING(1 2, 4 5)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -52;
      }

/* Inserting into pg_3d_wkt */
    sql =
	"INSERT INTO fdo_pg_3d_wkt (id, g) VALUES (1, GeomFromText('POLYGONZ((10 10 100, 15 10 101, 15 15 102, 10 15 103, 10 10 100), (11 11 100, 12 11 101, 1 12 102, 11 12 103, 11 11 100))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -53;
      }

/* Inserting into pg_3d_wkb */
    sql =
	"INSERT INTO fdo_pg_3d_wkb (id, g) VALUES (1, GeomFromText('POLYGONZ((10 10 100, 15 10 101, 15 15 102, 10 15 103, 10 10 100), (11 11 100, 12 11 101, 1 12 102, 11 12 103, 11 11 100))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -54;
      }

/* Inserting into pg_3d_wkb */
    sql =
	"INSERT INTO fdo_pg_3d_wkb (id, g) VALUES (2, GeomFromText('POLYGONZM((10 10 100 10, 15 10 101 11, 15 15 102 12, 10 15 103 13, 10 10 100 10), (11 11 100 10, 12 11 101 11, 1 12 102 12, 11 12 103 13, 11 11 100 10))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -55;
      }

/* Inserting into pg_3d_wkb */
    sql =
	"INSERT INTO fdo_pg_3d_wkb (id, g) VALUES (3, GeomFromText('POLYGONM((10 10 10, 15 10 11, 15 15 12, 10 15 13, 10 10 10), (11 11 10, 12 11 11, 1 12 12, 11 12 13, 11 11 10))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -56;
      }

/* Inserting into pg_3d_wkb */
    sql =
	"INSERT INTO fdo_pg_3d_wkb (id, g) VALUES (4, GeomFromText('POLYGON((10 10, 15 10, 15 15, 10 15, 10 10), (11 11, 12 11, 1 12, 11 12, 11 11))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -57;
      }

/* Inserting into mpt_3d_wkt */
    sql =
	"INSERT INTO fdo_mpt_3d_wkt (id, g) VALUES (1, GeomFromText('MULTIPOINTZ(1 2 3, 4 5 6)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -58;
      }

/* Inserting into mpt_3d_wkb */
    sql =
	"INSERT INTO fdo_mpt_3d_wkb (id, g) VALUES (1, GeomFromText('MULTIPOINTZ(1 2 3, 4 5 6)', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -59;
      }

/* Inserting into mln_3d_wkt */
    sql =
	"INSERT INTO fdo_mln_3d_wkt (id, g) VALUES (1, GeomFromText('MULTILINESTRINGZ((1 2 3, 4 5 6), (7 8 9, 10 11 12))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -60;
      }

/* Inserting into mln_3d_wkb */
    sql =
	"INSERT INTO fdo_mln_3d_wkb (id, g) VALUES (1, GeomFromText('MULTILINESTRINGZ((1 2 3, 4 5 6), (7 8 9, 10 11 12))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -61;
      }

/* Inserting into mpg_3d_wkt */
    sql =
	"INSERT INTO fdo_mpg_3d_wkt (id, g) VALUES (1, GeomFromText('MULTIPOLYGONZ(((10 10 100, 15 10 101, 15 15 102, 10 15 103, 10 10 100), (11 11 100, 12 11 101, 1 12 102, 11 12 103, 11 11 100)), ((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -62;
      }

/* Inserting into mpg_3d_wkb */
    sql =
	"INSERT INTO fdo_mpg_3d_wkb (id, g) VALUES (1, GeomFromText('MULTIPOLYGONZ(((10 10 100, 15 10 101, 15 15 102, 10 15 103, 10 10 100), (11 11 100, 12 11 101, 1 12 102, 11 12 103, 11 11 100)), ((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -63;
      }

/* Inserting into gcoll_3d_wkt */
    sql =
	"INSERT INTO fdo_gcoll_3d_wkt (id, g) VALUES (1, GeomFromText('GEOMETRYCOLLECTIONZ(POINTZ(10 10 100), LINESTRINGZ(5 5 10, 6 6 11), POLYGONZ((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -64;
      }

/* Inserting into gcoll_3d_wkb */
    sql =
	"INSERT INTO fdo_gcoll_3d_wkb (id, g) VALUES (1, GeomFromText('GEOMETRYCOLLECTIONZ(POINZ(10 10 100), LINESTRINGZ(5 5 10, 6 6 11), POLYGONZ((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))', -1))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -65;
      }

/* checking MultiPoint 3D WKT */
    sql = "SELECT AsText(g) FROM fdo_mpt_3d_wkt";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -66;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -67;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -68;
      }
    if (strcmp (results[1], "MULTIPOINT Z(1 2 3, 4 5 6)") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -69;
      }
    sqlite3_free_table (results);

/* checking MultiLinestring 3D WKT */
    sql = "SELECT AsText(g) FROM fdo_mln_3d_wkt";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -70;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -71;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -72;
      }
    if (strcmp
	(results[1],
	 "MULTILINESTRING Z((1 2 3, 4 5 6), (7 8 9, 10 11 12))") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -73;
      }
    sqlite3_free_table (results);

/* checking MultiPolygon 3D WKT */
    sql = "SELECT AsText(g) FROM fdo_mpg_3d_wkt";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -74;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -75;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -76;
      }
    if (strcmp
	(results[1],
	 "MULTIPOLYGON Z(((10 10 100, 15 10 101, 15 15 102, 10 15 103, 10 10 100), (11 11 100, 12 11 101, 1 12 102, 11 12 103, 11 11 100)), ((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))")
	!= 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -77;
      }
    sqlite3_free_table (results);

/* checking GeometryCollection 3D WKT */
    sql = "SELECT AsText(g) FROM fdo_gcoll_3d_wkt";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -78;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -79;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -80;
      }
    if (strcmp
	(results[1],
	 "GEOMETRYCOLLECTION Z(POINT Z(10 10 100), LINESTRING Z(5 5 10, 6 6 11), POLYGON Z((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))")
	!= 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -81;
      }
    sqlite3_free_table (results);

/* dropping an FDO virtual table */
    sql = "DROP TABLE fdo_mpt_3d_wkt";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -82;
      }

/* discarding an FDO Geometry column */
    sql = "SELECT DiscardFDOGeometryColumn('fdo_mpt_3d_wkt', 'g')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -83;
      }
    sql = "SELECT DiscardFDOGeometryColumn(1, 'g')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -84;
      }
    sql = "SELECT DiscardFDOGeometryColumn('mpt_3d_wkt', 2)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -85;
      }

/* recovering an FDO Geometry column */
    sql = "SELECT RecoverFDOGeometryColumn('mpt_3d_wkt', 'g', -1, 4, 3, 'WKT')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -86;
      }

/* FDO shut-down */
    sql = "SELECT AutoFDOStop()";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -66;
      }

    sql = "SELECT AddFDOGeometryColumn(1, 'g', -1, 7, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -67;
      }
    sql = "SELECT AddFDOGeometryColumn('a', 2, -1, 7, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -68;
      }
    sql = "SELECT AddFDOGeometryColumn('a', 'g', -1.5, 7, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -69;
      }
    sql = "SELECT AddFDOGeometryColumn('a', 'g', -1, 'a', 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -70;
      }
    sql = "SELECT AddFDOGeometryColumn('a', 'g', -1, 7, 'a', 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -71;
      }
    sql = "SELECT AddFDOGeometryColumn('a', 'g', -1, 17, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -72;
      }
    sql = "SELECT AddFDOGeometryColumn('a', 'g', -1, 7, 13, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -73;
      }
    sql = "SELECT AddFDOGeometryColumn('a', 'g', -1, 7, 3, 'DUMMY')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -74;
      }

    sql = "SELECT RecoverFDOGeometryColumn(1, 'g', -1, 7, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -75;
      }
    sql = "SELECT RecoverFDOGeometryColumn('a', 2, -1, 7, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -76;
      }
    sql = "SELECT RecoverFDOGeometryColumn('a', 'g', -1.5, 7, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -77;
      }
    sql = "SELECT RecoverFDOGeometryColumn('a', 'g', -1, 'a', 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -78;
      }
    sql = "SELECT RecoverFDOGeometryColumn('a', 'g', -1, 7, 'a', 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -79;
      }
    sql = "SELECT RecoverFDOGeometryColumn('a', 'g', -1, 17, 3, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -80;
      }
    sql = "SELECT RecoverFDOGeometryColumn('a', 'g', -1, 7, 13, 'WKB')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -81;
      }
    sql = "SELECT RecoverFDOGeometryColumn('a', 'g', -1, 7, 3, 'DUMMY')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -82;
      }

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -83;
      }

    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
