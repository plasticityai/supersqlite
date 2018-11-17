/*

 check_recover_geom.c -- SpatiaLite Test Case

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

    ret =
	sqlite3_exec (handle, "SELECT InitSpatialMetadata(1)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "InitSpatialMetadata() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -2;
      }
/* creating a Point XY table */
    sql = "CREATE TABLE pt_xy (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }

/* creating a Point XYZ table */
    sql = "CREATE TABLE pt_xyz (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }

/* creating a Point XYM table */
    sql = "CREATE TABLE pt_xym (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }

/* creating a Point XYZM table */
    sql = "CREATE TABLE pt_xyzm (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }

/* creating a Linestring XY table */
    sql = "CREATE TABLE ln_xy (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }

/* creating a Linestring XYZ table */
    sql = "CREATE TABLE ln_xyz (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -8;
      }

/* creating a Linestring XYM table */
    sql = "CREATE TABLE ln_xym (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }

/* creating a Linestring XYZM table */
    sql = "CREATE TABLE ln_xyzm (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }

/* creating a Polygon XY table */
    sql = "CREATE TABLE pg_xy (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

/* creating a Polygon XYZ table */
    sql = "CREATE TABLE pg_xyz (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }

/* creating a Polygon XYM table */
    sql = "CREATE TABLE pg_xym (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }

/* creating a Polygon XYZM table */
    sql = "CREATE TABLE pg_xyzm (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }

/* creating a MultiPoint XY table */
    sql = "CREATE TABLE mpt_xy (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -15;
      }

/* creating a MultiPoint XYZ table */
    sql = "CREATE TABLE mpt_XYZ (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -16;
      }

/* creating a MultiPoint XYM table */
    sql = "CREATE TABLE mpt_xym (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -17;
      }

/* creating a MultiPoint XYZM table */
    sql = "CREATE TABLE mpt_xyzm (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -18;
      }

/* creating a MultiLinestring XY table */
    sql = "CREATE TABLE mln_xy (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -19;
      }

/* creating a MultiLinestring XYZ table */
    sql = "CREATE TABLE mln_xyz (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -20;
      }

/* creating a MultiLinestring XYM table */
    sql = "CREATE TABLE mln_xym (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }

/* creating a MultiLinestring XYZM table */
    sql = "CREATE TABLE mln_xyzm (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }

/* creating a MultiPolygon XY table */
    sql = "CREATE TABLE mpg_xy (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }

/* creating a MultiPolygon XYZ table */
    sql = "CREATE TABLE mpg_xyz (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }

/* creating a MultiPolygon XYM table */
    sql = "CREATE TABLE mpg_xym (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }

/* creating a MultiPolygon XYZM table */
    sql = "CREATE TABLE mpg_xyzm (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -26;
      }

/* creating a GeometryCollection XY table */
    sql = "CREATE TABLE gcoll_xy (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -27;
      }

/* creating a GeometryCollection XYZ table */
    sql = "CREATE TABLE gcoll_xyz (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -28;
      }

/* creating a GeometryCollection XYM table */
    sql = "CREATE TABLE gcoll_xym (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }

/* creating a GeometryCollection XYZM table */
    sql = "CREATE TABLE gcoll_xyzm (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }


/* creating a Geometry XY table */
    sql = "CREATE TABLE geom_xy (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -31;
      }

/* creating a Geometry XYZ table */
    sql = "CREATE TABLE geom_xyz (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -32;
      }

/* creating a Geometry XYM table */
    sql = "CREATE TABLE geom_xym (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -33;
      }

/* creating a Geometry XYZM table */
    sql = "CREATE TABLE geom_xyzm (id INTEGER, g BLOB)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -34;
      }

/* Inserting into pt_xy */
    sql =
	"INSERT INTO pt_xy (id, g) VALUES (1, GeomFromText('POINT(1 2)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -35;
      }

/* Inserting into pt_xyz */
    sql =
	"INSERT INTO pt_xyz (id, g) VALUES (1, GeomFromText('POINTZ(1 2 3)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -36;
      }

/* Inserting into pt_xym */
    sql =
	"INSERT INTO pt_xym (id, g) VALUES (1, GeomFromText('POINTM(1 2 10)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -37;
      }

/* Inserting into pt_xyzm */
    sql =
	"INSERT INTO pt_xyzm (id, g) VALUES (1, GeomFromText('POINTZM(1 2 3 10)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -38;
      }

/* Inserting into ln_xy */
    sql =
	"INSERT INTO ln_xy (id, g) VALUES (1, GeomFromText('LINESTRING(1 2, 4 5)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -39;
      }

/* Inserting into ln_xyz */
    sql =
	"INSERT INTO ln_xyz (id, g) VALUES (1, GeomFromText('LINESTRINGZ(1 2 3, 4 5 6)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -40;
      }

/* Inserting into ln_xym */
    sql =
	"INSERT INTO ln_xym (id, g) VALUES (1, GeomFromText('LINESTRINGM(1 2 10, 4 5 11)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -41;
      }

/* Inserting into ln_xyzm */
    sql =
	"INSERT INTO ln_xyzm (id, g) VALUES (2, GeomFromText('LINESTRINGZM(1 2 3 10, 4 5 6 11)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -42;
      }

/* Inserting into pg_xy */
    sql =
	"INSERT INTO pg_xy (id, g) VALUES (1, GeomFromText('POLYGON((10 10, 15 10, 15 15, 10 15, 10 10), (11 11, 12 11, 1 12, 11 12, 11 11))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -43;
      }

/* Inserting into pg_xyz */
    sql =
	"INSERT INTO pg_xyz (id, g) VALUES (1, GeomFromText('POLYGONZ((10 10 100, 15 10 101, 15 15 102, 10 15 103, 10 10 100), (11 11 100, 12 11 101, 1 12 102, 11 12 103, 11 11 100))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -44;
      }

/* Inserting into pg_xym */
    sql =
	"INSERT INTO pg_xym (id, g) VALUES (1, GeomFromText('POLYGONM((10 10 10, 15 10 11, 15 15 12, 10 15 13, 10 10 10), (11 11 10, 12 11 11, 1 12 12, 11 12 13, 11 11 10))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -45;
      }

/* Inserting into pg_xyzm */
    sql =
	"INSERT INTO pg_xyzm (id, g) VALUES (1, GeomFromText('POLYGONZM((10 10 100 10, 15 10 101 11, 15 15 102 12, 10 15 103 13, 10 10 100 10), (11 11 100 10, 12 11 101 11, 1 12 102 12, 11 12 103 13, 11 11 100 10))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -46;
      }

/* Inserting into mpt_xy */
    sql =
	"INSERT INTO mpt_xy (id, g) VALUES (1, GeomFromText('MULTIPOINT(1 2, 4 5)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -47;
      }

/* Inserting into mpt_xyz */
    sql =
	"INSERT INTO mpt_xyz (id, g) VALUES (1, GeomFromText('MULTIPOINTZ(1 2 3, 4 5 6)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -48;
      }

/* Inserting into mpt_xym */
    sql =
	"INSERT INTO mpt_xym (id, g) VALUES (1, GeomFromText('MULTIPOINTM(1 2 10, 4 5 11)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -49;
      }

/* Inserting into mpt_xyzm */
    sql =
	"INSERT INTO mpt_xyzm (id, g) VALUES (1, GeomFromText('MULTIPOINTZM(1 2 3 10, 4 5 6 10)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -50;
      }

/* Inserting into mln_xy */
    sql =
	"INSERT INTO mln_xy (id, g) VALUES (1, GeomFromText('MULTILINESTRING((1 2, 4 5), (7 8, 10 11))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -51;
      }

/* Inserting into mln_xyz */
    sql =
	"INSERT INTO mln_xyz (id, g) VALUES (1, GeomFromText('MULTILINESTRINGZ((1 2 3, 4 5 6), (7 8 9, 10 11 12))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -52;
      }

/* Inserting into mln_xym */
    sql =
	"INSERT INTO mln_xym (id, g) VALUES (1, GeomFromText('MULTILINESTRINGM((1 2 10, 4 5 11), (7 8 12, 10 11 13))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -53;
      }

/* Inserting into mln_xyzm */
    sql =
	"INSERT INTO mln_xyzm (id, g) VALUES (1, GeomFromText('MULTILINESTRINGZM((1 2 3 10, 4 5 6 11), (7 8 9 12, 10 11 12 13))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -54;
      }

/* Inserting into mpg_xy */
    sql =
	"INSERT INTO mpg_xy (id, g) VALUES (1, GeomFromText('MULTIPOLYGON(((10 10, 15 10, 15 15, 10 15, 10 10), (11 11, 12 11, 1 12, 11 12, 11 11)), ((0 0, 1 0, 1 1, 0 1, 0 0)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -55;
      }

/* Inserting into mpg_xyz */
    sql =
	"INSERT INTO mpg_xyz (id, g) VALUES (1, GeomFromText('MULTIPOLYGONZ(((10 10 100, 15 10 101, 15 15 102, 10 15 103, 10 10 100), (11 11 100, 12 11 101, 1 12 102, 11 12 103, 11 11 100)), ((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -56;
      }

/* Inserting into mpg_xym */
    sql =
	"INSERT INTO mpg_xym (id, g) VALUES (1, GeomFromText('MULTIPOLYGONM(((10 10 11, 15 10 12, 15 15 13, 10 15 14, 10 10 11), (11 11 5, 12 11 6, 12 12 7, 11 12 8, 11 11 5)), ((0 0 11, 1 0 12, 1 1 13, 0 1 14, 0 0 11)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -57;
      }

/* Inserting into mpg_xyzm */
    sql =
	"INSERT INTO mpg_xyzm (id, g) VALUES (1, GeomFromText('MULTIPOLYGONZM(((10 10 100 11, 15 10 101 12, 15 15 102 13, 10 15 103 14, 10 10 100 11), (11 11 100 5, 12 11 101 6, 1 12 102 7, 11 12 103 8, 11 11 100 5)), ((0 0 1 11, 1 0 2 12, 1 1 3 13, 0 1 4 14, 0 0 1 11)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -58;
      }

/* Inserting into gcoll_xy */
    sql =
	"INSERT INTO gcoll_xy (id, g) VALUES (1, GeomFromText('GEOMETRYCOLLECTION(POINT(10 10), LINESTRING(5 5, 6 6), POLYGON((0 0, 1 0, 1 1, 0 1, 0 0)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -59;
      }

/* Inserting into gcoll_xyz */
    sql =
	"INSERT INTO gcoll_xyz (id, g) VALUES (1, GeomFromText('GEOMETRYCOLLECTIONZ(POINTZ(10 10 100), LINESTRINGZ(5 5 10, 6 6 11), POLYGONZ((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -60;
      }

/* Inserting into gcoll_xym */
    sql =
	"INSERT INTO gcoll_xym (id, g) VALUES (1, GeomFromText('GEOMETRYCOLLECTIONM(POINTM(10 10 100), LINESTRINGM(5 5 10, 6 6 11), POLYGONM((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -61;
      }

/* Inserting into gcoll_xyzm */
    sql =
	"INSERT INTO gcoll_xyzm (id, g) VALUES (1, GeomFromText('GEOMETRYCOLLECTIONZM(POINTZM(10 10 100 11), LINESTRINGZM(5 5 10 11, 6 6 11 12), POLYGONZM((0 0 1 10, 1 0 2 11, 1 1 3 12, 0 1 4 13, 0 0 1 10)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -62;
      }

/* Inserting into geom_xy */
    sql =
	"INSERT INTO geom_xy (id, g) VALUES (1, GeomFromText('GEOMETRYCOLLECTION(POINT(10 10), LINESTRING(5 5, 6 6), POLYGON((0 0, 1 0, 1 1, 0 1, 0 0)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -63;
      }
    sql =
	"INSERT INTO geom_xy (id, g) VALUES (2, GeomFromText('POINT(10 10)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -64;
      }

/* Inserting into geom_xyz */
    sql =
	"INSERT INTO geom_xyz (id, g) VALUES (1, GeomFromText('GEOMETRYCOLLECTIONZ(POINTZ(10 10 100), LINESTRINGZ(5 5 10, 6 6 11), POLYGONZ((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -65;
      }
    sql =
	"INSERT INTO geom_xyz (id, g) VALUES (2, GeomFromText('POINTZ(10 10 100)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -66;
      }

/* Inserting into geom_xym */
    sql =
	"INSERT INTO geom_xym (id, g) VALUES (1, GeomFromText('GEOMETRYCOLLECTIONM(POINTM(10 10 100), LINESTRINGM(5 5 10, 6 6 11), POLYGONM((0 0 1, 1 0 2, 1 1 3, 0 1 4, 0 0 1)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -67;
      }
    sql =
	"INSERT INTO geom_xym (id, g) VALUES (2, GeomFromText('POINTM(10 10 100)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -68;
      }

/* Inserting into geom_xyzm */
    sql =
	"INSERT INTO geom_xyzm (id, g) VALUES (1, GeomFromText('GEOMETRYCOLLECTIONZM(POINTZM(10 10 100 11), LINESTRINGZM(5 5 10 11, 6 6 11 12), POLYGONZM((0 0 1 10, 1 0 2 11, 1 1 3 12, 0 1 4 13, 0 0 1 10)))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -69;
      }
    sql =
	"INSERT INTO geom_xyzm (id, g) VALUES (2, GeomFromText('POINTZM(10 10 100 11)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -70;
      }

/* recovering pt_xy */
    sql = "SELECT RecoverGeometryColumn('pt_xy', 'g', 4326, 'POINT', 2);";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -71;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -72;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -73;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -74;
      }
    sqlite3_free_table (results);

/* recovering pt_xyz */
    sql = "SELECT RecoverGeometryColumn('pt_xyz', 'g', 4326, 'POINT', 3);";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -75;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -76;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -77;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -78;
      }
    sqlite3_free_table (results);

/* recovering pt_xym */
    sql = "SELECT RecoverGeometryColumn('pt_xym', 'g', 4326, 'POINT', 'XYM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -79;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -80;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -81;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -82;
      }
    sqlite3_free_table (results);

/* recovering pt_xyzm */
    sql =
	"SELECT RecoverGeometryColumn('pt_xyzm', 'g', 4326, 'POINT', 'XYZM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -83;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -84;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -85;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -86;
      }
    sqlite3_free_table (results);

/* recovering ln_xy */
    sql =
	"SELECT RecoverGeometryColumn('ln_xy', 'g', 4326, 'LINESTRING', 'XY');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -87;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -88;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -89;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -90;
      }
    sqlite3_free_table (results);

/* recovering ln_xyz */
    sql =
	"SELECT RecoverGeometryColumn('ln_xyz', 'g', 4326, 'LINESTRING', 'XYZ');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -91;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -92;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -93;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -94;
      }
    sqlite3_free_table (results);

/* recovering ln_xym */
    sql =
	"SELECT RecoverGeometryColumn('ln_xym', 'g', 4326, 'LINESTRING', 'XYM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -95;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -96;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -97;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -98;
      }
    sqlite3_free_table (results);

/* recovering ln_xyzm */
    sql =
	"SELECT RecoverGeometryColumn('ln_xyzm', 'g', 4326, 'LINESTRING', 'XYZM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -99;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -100;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -101;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -102;
      }
    sqlite3_free_table (results);

/* recovering pg_xy */
    sql = "SELECT RecoverGeometryColumn('pg_xy', 'g', 4326, 'POLYGON', 'XY');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -103;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -104;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -105;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -106;
      }
    sqlite3_free_table (results);

/* recovering pg_xyz */
    sql =
	"SELECT RecoverGeometryColumn('pg_xyz', 'g', 4326, 'POLYGON', 'XYZ');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -107;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -108;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -109;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -110;
      }
    sqlite3_free_table (results);

/* recovering pg_xym */
    sql =
	"SELECT RecoverGeometryColumn('pg_xym', 'g', 4326, 'POLYGON', 'XYM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -111;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -112;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -113;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -114;
      }
    sqlite3_free_table (results);

/* recovering pg_xyzm */
    sql =
	"SELECT RecoverGeometryColumn('pg_xyzm', 'g', 4326, 'POLYGON', 'XYZM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -115;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -116;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -117;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -118;
      }
    sqlite3_free_table (results);

/* recovering mpt_xy */
    sql =
	"SELECT RecoverGeometryColumn('mpt_xy', 'g', 4326, 'MULTIPOINT', 'XY');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -119;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -120;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -121;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -122;
      }
    sqlite3_free_table (results);

/* recovering mpt_xyz */
    sql =
	"SELECT RecoverGeometryColumn('mpt_xyz', 'g', 4326, 'MULTIPOINT', 'XYZ');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -123;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -124;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -125;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -126;
      }
    sqlite3_free_table (results);

/* recovering mpt_xym */
    sql =
	"SELECT RecoverGeometryColumn('mpt_xym', 'g', 4326, 'MULTIPOINT', 'XYM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -127;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -128;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -129;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -130;
      }
    sqlite3_free_table (results);

/* recovering mpt_xyzm */
    sql =
	"SELECT RecoverGeometryColumn('mpt_xyzm', 'g', 4326, 'MULTIPOINT', 'XYZM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -131;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -132;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -133;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -134;
      }
    sqlite3_free_table (results);

/* recovering mln_xy */
    sql =
	"SELECT RecoverGeometryColumn('mln_xy', 'g', 4326, 'MULTILINESTRING', 'XY');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -135;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -136;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -137;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -138;
      }
    sqlite3_free_table (results);

/* recovering mln_xyz */
    sql =
	"SELECT RecoverGeometryColumn('mln_xyz', 'g', 4326, 'MULTILINESTRING', 'XYZ');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -139;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -140;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -141;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -142;
      }
    sqlite3_free_table (results);

/* recovering mln_xym */
    sql =
	"SELECT RecoverGeometryColumn('mln_xym', 'g', 4326, 'MULTILINESTRING', 'XYM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -143;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -144;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -145;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -146;
      }
    sqlite3_free_table (results);

/* recovering mln_xyzm */
    sql =
	"SELECT RecoverGeometryColumn('mln_xyzm', 'g', 4326, 'MULTILINESTRING', 'XYZM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -147;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -148;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -149;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -150;
      }
    sqlite3_free_table (results);

/* recovering mpg_xy */
    sql =
	"SELECT RecoverGeometryColumn('mpg_xy', 'g', 4326, 'MULTIPOLYGON', 'XY');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -151;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -152;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -153;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -154;
      }
    sqlite3_free_table (results);

/* recovering mpg_xyz */
    sql =
	"SELECT RecoverGeometryColumn('mpg_xyz', 'g', 4326, 'MULTIPOLYGON', 'XYZ');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -155;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -156;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -157;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -158;
      }
    sqlite3_free_table (results);

/* recovering mpg_xym */
    sql =
	"SELECT RecoverGeometryColumn('mpg_xym', 'g', 4326, 'MULTIPOLYGON', 'XYM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -159;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -160;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -161;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -162;
      }
    sqlite3_free_table (results);

/* recovering mpg_xyzm */
    sql =
	"SELECT RecoverGeometryColumn('mpg_xyzm', 'g', 4326, 'MULTIPOLYGON', 'XYZM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -163;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -164;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -165;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -166;
      }
    sqlite3_free_table (results);

/* recovering gcoll_xy */
    sql =
	"SELECT RecoverGeometryColumn('gcoll_xy', 'g', 4326, 'GEOMETRYCOLLECTION', 'XY');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -167;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -168;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -169;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -170;
      }
    sqlite3_free_table (results);

/* recovering gcoll_xyz */
    sql =
	"SELECT RecoverGeometryColumn('gcoll_xyz', 'g', 4326, 'GEOMETRYCOLLECTION', 'XYZ');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -171;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -172;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -173;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -174;
      }
    sqlite3_free_table (results);

/* recovering gcoll_xym */
    sql =
	"SELECT RecoverGeometryColumn('gcoll_xym', 'g', 4326, 'GEOMETRYCOLLECTION', 'XYM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -175;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -176;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -177;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -178;
      }
    sqlite3_free_table (results);

/* recovering gcoll_xyzm */
    sql =
	"SELECT RecoverGeometryColumn('gcoll_xyzm', 'g', 4326, 'GEOMETRYCOLLECTION', 'XYZM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -179;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -180;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -181;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -182;
      }
    sqlite3_free_table (results);

/* recovering geom_xy */
    sql =
	"SELECT RecoverGeometryColumn('geom_xy', 'g', 4326, 'GEOMETRY', 'XY');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -183;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -184;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -185;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -186;
      }
    sqlite3_free_table (results);

/* recovering geom_xyz */
    sql =
	"SELECT RecoverGeometryColumn('geom_xyz', 'g', 4326, 'GEOMETRY', 'XYZ');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -187;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -188;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -189;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -190;
      }
    sqlite3_free_table (results);

/* recovering geom_xym */
    sql =
	"SELECT RecoverGeometryColumn('geom_xym', 'g', 4326, 'GEOMETRY', 'XYM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -191;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -192;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -193;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -194;
      }
    sqlite3_free_table (results);

/* recovering geom_xyzm */
    sql =
	"SELECT RecoverGeometryColumn('geom_xyzm', 'g', 4326, 'GEOMETRY', 'XYZM');";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -195;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -196;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -197;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -198;
      }
    sqlite3_free_table (results);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -199;
      }

    spatialite_cleanup_ex (cache);

    return 0;
}
