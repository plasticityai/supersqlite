/*

 check_bufovflw.c -- SpatiaLite Test Case - buffer overflows

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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"
#include <spatialite/gaiaaux.h>

void
cleanup_shapefile (const char *filename)
{
    char nam[1000];

    if (!filename)
      {
	  return;
      }

    snprintf (nam, 1000, "%s.dbf", filename);
    unlink (nam);
    snprintf (nam, 1000, "%s.prj", filename);
    unlink (nam);
    snprintf (nam, 1000, "%s.shp", filename);
    unlink (nam);
    snprintf (nam, 1000, "%s.shx", filename);
    unlink (nam);
}

int
main (int argc, char *argv[])
{
    int ret;
    sqlite3 *handle;
    sqlite3_stmt *stmt;
    char *err_msg = NULL;
    int suffix_len = 16 * 1024;	/* 16 KB suffix */
    char *suffix;
    char *xtable;
    char *shape;
    char *shape2;
    char *shape2geom;
    char *dbf;
    char *table_a;
    char *table_b;
    char *topology;
    char *auth;
#ifndef OMIT_PROJ		/* including PROJ.4 */
    char *kml1;
    char *kml2;
#endif /* end including PROJ.4 */
    char *resvalue;
    char *pk;
    char *name;
    char *geom;
    char *sql;
    char **results;
    const char *value;
    int rows;
    int columns;
    char *string;
    int len;
    char frmt[2048];
    int row_count;
    char *dumpname = __FILE__ "dump";
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
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

/* setting up very long table and column names */
    suffix = malloc (suffix_len);
    memset (suffix, 'z', suffix_len);
    suffix[suffix_len - 1] = '\0';

    table_a = sqlite3_mprintf ("table_a_%s", suffix);
    table_b = sqlite3_mprintf ("table_b_%s", suffix);
    shape2 = sqlite3_mprintf ("shape_table_2_%s", suffix);
    shape2geom = sqlite3_mprintf ("shape_table_2_geom_%s", suffix);
    shape = sqlite3_mprintf ("shape_table_%s", suffix);
    dbf = sqlite3_mprintf ("dbf_tbale_%s", suffix);
    pk = sqlite3_mprintf ("id_%s", suffix);
    name = sqlite3_mprintf ("name_%s", suffix);
    geom = sqlite3_mprintf ("geom_%s", suffix);
    topology = sqlite3_mprintf ("topology_%s_", suffix);

/* creating table "A" */
    sql = sqlite3_mprintf ("CREATE TABLE %s (\n"
			   "%s INTEGER NOT NULL PRIMARY KEY,\n"
			   "%s TEXT NOT NULL)", table_a, pk, name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE-A error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -3;
      }

/* adding a POINT Geometry to table "A" */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, %Q, "
			   "4326, 'POINT', 'XY')", table_a, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -4;
      }

/* creating a Spatial Index on table "A" */
    sql = sqlite3_mprintf ("SELECT CreateSpatialIndex(%Q, %Q)", table_a, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CreateSpatialIndex (TABLE-A) error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -5;
      }

/* inserting few valid rows on table "A" */
    sql = sqlite3_mprintf ("INSERT INTO %s (%s, %s, %s) VALUES "
			   "(1, 'alpha', MakePoint(1, 10, 4326))", table_a, pk,
			   name, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -6;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (%s, %s, %s) VALUES "
			   "(2, 'beta', MakePoint(2, 20, 4326))", table_a, pk,
			   name, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -106;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (%s, %s, %s) VALUES "
			   "(3, 'gamma', MakePoint(3, 30, 4326))", table_a, pk,
			   name, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -206;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (%s, %s, %s) VALUES "
			   "(4, 'delta', MakePoint(4, 40, 4326))", table_a, pk,
			   name, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -306;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (%s, %s, %s) VALUES "
			   "(5, 'epsilon', MakePoint(5, 50, 4326))", table_a,
			   pk, name, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -406;
      }

/* inserting few invalid geometries on table "A" */
    sql = sqlite3_mprintf ("INSERT INTO %s (%s, %s, %s) VALUES "
			   "(6, 'zeta', MakePoint(10, 10))", table_a, pk, name,
			   geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  sql = sqlite3_mprintf ("%s.%s violates Geometry constraint "
				 "[geom-type or SRID not allowed]", table_a,
				 geom);
	  ret = strcmp (sql, err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_free (sql);
	  if (ret == 0)
	      goto test2;
	  sqlite3_close (handle);
	  return -7;
      }
  test2:
    sql = sqlite3_mprintf ("INSERT INTO %s (%s, %s, %s) VALUES "
			   "(7, 'eta', MakePointZ(20, 20, 20, 4326))", table_a,
			   pk, name, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  sql = sqlite3_mprintf ("%s.%s violates Geometry constraint "
				 "[geom-type or SRID not allowed]", table_a,
				 geom);
	  ret = strcmp (sql, err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_free (sql);
	  if (ret == 0)
	      goto test3;
	  sqlite3_close (handle);
	  return -8;
      }
  test3:
    sql = sqlite3_mprintf ("INSERT INTO %s (%s, %s, %s) VALUES "
			   "(8, 'theta', GeomFromText('LINESTRING(0 0, 1 1)', 4326))",
			   table_a, pk, name, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  sql = sqlite3_mprintf ("%s.%s violates Geometry constraint "
				 "[geom-type or SRID not allowed]", table_a,
				 geom);
	  ret = strcmp (sql, err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_free (sql);
	  if (ret == 0)
	      goto test4;
	  sqlite3_close (handle);
	  return -9;
      }
  test4:

/* checking for validity (table A) */
    sql = sqlite3_mprintf ("SELECT Count(*) FROM %s WHERE ROWID IN ("
			   "SELECT ROWID FROM SpatialIndex WHERE "
			   "f_table_name = %Q AND f_geometry_column = %Q "
			   "AND search_frame = BuildCircleMbr(3, 30, 1))",
			   table_a, table_a, geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Test TABLE-A error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -10;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr, "Unexpected rows/columns (TABLE-A): r=%d c=%d\n",
		   rows, columns);
	  return -11;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (TABLE-A): %s\n", results[1]);
	  return -12;
      }
    sqlite3_free_table (results);

/* creating table "B" */
    sql = sqlite3_mprintf ("CREATE TABLE %s AS "
			   "SELECT * FROM %s", table_b, table_a);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE-B error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -13;
      }

/* recovering Geometry for table "B" */
    sql = sqlite3_mprintf ("SELECT RecoverGeometryColumn(%Q, %Q, "
			   "4326, 'POINT', 'XY')", table_b, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "RecoverGeometryColumn error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -14;
      }

/* creating a Spatial Index on table "B" */
    sql = sqlite3_mprintf ("SELECT CreateSpatialIndex(%Q, %Q)", table_b, geom);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CreateSpatialIndex (TABLE-B) error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -15;
      }

/* checking for validity (table B) */
    sql = sqlite3_mprintf ("SELECT Count(*) FROM %s WHERE ROWID IN ("
			   "SELECT ROWID FROM SpatialIndex WHERE "
			   "f_table_name = %Q AND f_geometry_column = %Q "
			   "AND search_frame = BuildCircleMbr(2, 20, 1))",
			   table_b, table_b, geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Test TABLE-B error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -16;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr, "Unexpected rows/columns (TABLE-B): r=%d c=%d\n",
		   rows, columns);
	  return -17;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (TABLE-B): %s\n", results[1]);
	  return -18;
      }
    sqlite3_free_table (results);

/* deleting one row from Table B */
    sql = sqlite3_mprintf ("DELETE FROM %s WHERE %s = 2", table_b, pk);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DELETE (TABLE-B) error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -19;
      }

/* updating one row into Table B */
    sql = sqlite3_mprintf ("UPDATE %s SET %s = MakePoint(-3, -30, 4326) "
			   "WHERE %s = 4", table_b, geom, pk);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UPDATE (TABLE-B) error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -20;
      }

/* invalid updates table "B" */
    sql = sqlite3_mprintf ("UPDATE %s SET %s = MakePoint(-2, -20) "
			   "WHERE %s = 2", table_b, geom, pk);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  sql = sqlite3_mprintf ("%s.%s violates Geometry constraint "
				 "[geom-type or SRID not allowed]", table_b,
				 geom);
	  ret = strcmp (sql, err_msg);
	  sqlite3_free (sql);
	  if (ret == 0)
	      goto test5;
	  sqlite3_free (sql);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -21;
      }
  test5:
    sql = sqlite3_mprintf ("UPDATE %s SET %s = MakePointZ(-2, -20, 20, 4326) "
			   "WHERE %s = 2", table_b, geom, pk);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  sql = sqlite3_mprintf ("%s.%s violates Geometry constraint "
				 "[geom-type or SRID not allowed]", table_b,
				 geom);
	  ret = strcmp (sql, err_msg);
	  sqlite3_free (sql);
	  if (ret == 0)
	      goto test6;
	  sqlite3_free (sql);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -22;
      }
  test6:
    sql =
	sqlite3_mprintf
	("UPDATE %s SET %s = GeomFromText('LINESTRING(0 0, 1 1)', 4326) "
	 "WHERE %s = 2", table_b, geom, pk);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  sql = sqlite3_mprintf ("%s.%s violates Geometry constraint "
				 "[geom-type or SRID not allowed]", table_b,
				 geom);
	  ret = strcmp (sql, err_msg);
	  sqlite3_free (sql);
	  if (ret == 0)
	      goto test7;
	  sqlite3_free (sql);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -23;
      }
  test7:

/* VACUUM - thus invalidating TABLE-B Spatial Index */
    ret = sqlite3_exec (handle, "VACUUM", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VACUUM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -24;
      }

/* checking the Spatial Index on table "B" */
    sql = sqlite3_mprintf ("SELECT CheckSpatialIndex(%Q, %Q)", table_b, geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CheckSpatialIndex TABLE-B error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -25;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (check RTree TABLE-B): r=%d c=%d\n",
		   rows, columns);
	  return -26;
      }
    value = results[1];
    if (strcmp ("0", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (check RTree TABLE-B): %s\n",
		   results[1]);
	  return -27;
      }
    sqlite3_free_table (results);

/* checking the Spatial Index all table */
    ret =
	sqlite3_get_table (handle, "SELECT CheckSpatialIndex()", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CheckSpatialIndex ALL error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -28;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (check RTree ALL): r=%d c=%d\n",
		   rows, columns);
	  return -29;
      }
    value = results[1];
    if (strcmp ("0", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (check RTree ALL): %s\n",
		   results[1]);
	  return -30;
      }
    sqlite3_free_table (results);

/* recovering the Spatial Index all table */
    ret =
	sqlite3_get_table (handle, "SELECT RecoverSpatialIndex()", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "RecoverSpatialIndex ALL error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -34;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (recover RTree ALL): r=%d c=%d\n",
		   rows, columns);
	  return -35;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (recover RTree ALL): %s\n",
		   results[1]);
	  return -36;
      }
    sqlite3_free_table (results);

/* recovering the Spatial Index on table "B" */
    sql = sqlite3_mprintf ("SELECT RecoverSpatialIndex(%Q, %Q)", table_b, geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "RecoverSpatialIndex TABLE-B error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -37;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (recover RTree TABLE-B): r=%d c=%d\n",
		   rows, columns);
	  return -38;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (recover RTree TABLE-B): %s\n",
		   results[1]);
	  return -39;
      }
    sqlite3_free_table (results);

/* updating layer statistics for table "B" */
    sql =
	sqlite3_mprintf ("SELECT UpdateLayerStatistics(%Q, %Q)", table_b, geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UpdateLayerStatistics TABLE-B error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -40;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (statistics TABLE-B): r=%d c=%d\n",
		   rows, columns);
	  return -41;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (statistics TABLE-B): %s\n",
		   results[1]);
	  return -42;
      }
    sqlite3_free_table (results);

/* retieving the LayerExtent for table "B" */
    sql =
	sqlite3_mprintf ("SELECT ST_AsText(GetLayerExtent(%Q, %Q))", table_b,
			 geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "GetLayerExtent TABLE-B error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -140;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (GetLayerExtent TABLE-B): r=%d c=%d\n",
		   rows, columns);
	  return -141;
      }
    value = results[1];
    if (strcmp ("POLYGON((-3 -30, 5 -30, 5 50, -3 50, -3 -30))", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (GetLayerExtent TABLE-B): %s\n",
		   results[1]);
	  return -142;
      }
    sqlite3_free_table (results);

/* updating layer statistics ALL */
    ret =
	sqlite3_get_table (handle, "SELECT UpdateLayerStatistics()", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UpdateLayerStatistics ALL error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -43;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (statistics ALL): r=%d c=%d\n",
		   rows, columns);
	  return -44;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (statistics ALL): %s\n",
		   results[1]);
	  return -45;
      }
    sqlite3_free_table (results);

/* disabling the Spatial Index on table "B" */
    sql = sqlite3_mprintf ("SELECT DisableSpatialIndex(%Q, %Q)", table_b, geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DisableSpatialIndex TABLE-B error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -46;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (disable TABLE-B): r=%d c=%d\n",
		   rows, columns);
	  return -47;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (disable TABLE-B): %s\n",
		   results[1]);
	  return -48;
      }
    sqlite3_free_table (results);

/* creating an MbrCache on table "B" */
    sql = sqlite3_mprintf ("SELECT CreateMbrCache(%Q, %Q)", table_b, geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CreateMbrCache TABLE-B error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -49;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (disable TABLE-B): r=%d c=%d\n",
		   rows, columns);
	  return -50;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (disable TABLE-B): %s\n",
		   results[1]);
	  return -51;
      }
    sqlite3_free_table (results);

/* checking the MbrCache on table "B" */
    sql = sqlite3_mprintf ("SELECT Count(*) FROM %s WHERE ROWID IN ("
			   "SELECT ROWID FROM cache_%s_%s WHERE "
			   "mbr = FilterMbrIntersects(0, 0, 11, 11))", table_b,
			   table_b, geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Test MbrCache error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -52;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr, "Unexpected rows/columns (MbrCache): r=%d c=%d\n",
		   rows, columns);
	  return -53;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (MbrCache): %s\n", results[1]);
	  return -54;
      }
    sqlite3_free_table (results);

/* disabling the Spatial Index [MbrCache] on table "B" */
    sql = sqlite3_mprintf ("SELECT DisableSpatialIndex(%Q, %Q)", table_b, geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DisableSpatialIndex TABLE-B error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -55;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (disable TABLE-B): r=%d c=%d\n",
		   rows, columns);
	  return -56;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (disable TABLE-B): %s\n",
		   results[1]);
	  return -57;
      }
    sqlite3_free_table (results);

/* discarding geometry from table "B" */
    sql =
	sqlite3_mprintf ("SELECT DiscardGeometryColumn(%Q, %Q)", table_b, geom);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DiscardGeometryColumn TABLE-B error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -58;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (discard TABLE-B): r=%d c=%d\n",
		   rows, columns);
	  return -59;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (discard TABLE-B): %s\n",
		   results[1]);
	  return -60;
      }
    sqlite3_free_table (results);

/* creating a topology */
    sql =
	sqlite3_mprintf ("SELECT CreateTopologyTables(%Q, 4326, 'XY')",
			 topology);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CreateTopologyTables error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -61;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (CreateTopologyTables): r=%d c=%d\n",
		   rows, columns);
	  return -62;
      }
    value = results[1];
    if (strcmp ("1", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (CreateTopologyTables): %s\n",
		   results[1]);
	  return -63;
      }
    sqlite3_free_table (results);

    sqlite3_free (table_a);
    sqlite3_free (table_b);
    sqlite3_free (pk);
    sqlite3_free (name);
    sqlite3_free (geom);
    sqlite3_free (topology);

/* inserting a CRS (very long auth) */
    auth = sqlite3_mprintf ("authority_%s", suffix);
    sql =
	sqlite3_mprintf
	("INSERT INTO spatial_ref_sys (srid, auth_name, auth_srid, "
	 "ref_sys_name, proj4text, srtext) VALUES (NULL, %Q, 1122, "
	 "'silly CRS', 'silly proj def', 'silly wkt def')", auth);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT CRS error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -64;
      }

#ifndef OMIT_EPSG /* only if full EPSG support is enabled */
/* checking for validity (SRID from Auth) */
    sql = sqlite3_mprintf ("SELECT SridFromAuthCrs(%Q, %d)", auth, 1122);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Test TABLE-A error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -65;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr,
		   "Unexpected rows/columns (SRID from Auth): r=%d c=%d\n",
		   rows, columns);
	  return -66;
      }
    value = results[1];
    if (strcmp ("325834", value) != 0)
      {
	  fprintf (stderr, "Unexpected result (SRID from Auth): %s\n",
		   results[1]);
	  return -67;
      }
    sqlite3_free_table (results);
#endif

/* deleting the odd CRS */
    sql = "DELETE FROM spatial_ref_sys WHERE srid = 325834";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DELETE (CRS) error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -68;
      }

    sqlite3_free (auth);

#ifndef OMIT_PROJ		/* only if PROJ is supported */

/* checking AsKML (1) */
    kml1 = sqlite3_mprintf ("kml_name_%s", suffix);
    kml2 = sqlite3_mprintf ("kml_description_%s", suffix);
    strcpy (frmt, "<Placemark><name>%s</name><description>%s</description>"
	    "<MultiGeometry><Point><coordinates>%1.0f,%1.0f"
	    "</coordinates></Point><LineString><coordinates>0,0 "
	    "%1.0f,%1.0f</coordinates></LineString>");
    strcat (frmt, "<Polygon><outerBoundaryIs><LinearRing><coordinates>"
	    "0,0 0,%1.0f %1.0f,%1.0f %1.0f,0 0,0"
	    "</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs>"
	    "<LinearRing><coordinates>5,5 5,%1.0f %1.0f,%1.0f "
	    "%1.0f,5 5,5</coordinates></LinearRing></innerBoundaryIs>"
	    "</Polygon></MultiGeometry></Placemark>");
    resvalue =
	sqlite3_mprintf (frmt, kml1, kml2, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf
	("SELECT AsKml(%Q, %Q, GeomFromText('GEOMETRYCOLLECTION("
	 "POINT(%1.0f %1.0f), LINESTRING(0 0, %1.0f %1.0f), "
	 "POLYGON((0 0, 0 %1.0f, %1.0f %1.0f, %1.0f 0, 0 0), "
	 "(5 5, 5 %1.0f, %1.0f %1.0f, %1.0f 5, 5 5)))'" ", 4326), 300)", kml1,
	 kml2, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
	 1e128, 1e128, 1e128);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Test AsKML#1 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -69;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr, "Unexpected rows/columns (AsKML#1): r=%d c=%d\n",
		   rows, columns);
	  return -70;
      }
    value = results[1];
    if (strcmp (resvalue, value) != 0)
      {
	  fprintf (stderr, "Unexpected result (AsKML#1): %s\n", results[1]);
	  return -71;
      }
    sqlite3_free_table (results);
    sqlite3_free (kml1);
    sqlite3_free (kml2);
    sqlite3_free (resvalue);

/* checking AsKML (2) */
    kml1 = sqlite3_mprintf ("%1.0f", 1e128);
    kml2 = sqlite3_mprintf ("%2.0f", 2e128);
    strcpy (frmt, "<Placemark><name>%s</name><description>%s</description>"
	    "<MultiGeometry><Point><coordinates>%1.0f,%1.0f,%1.0f"
	    "</coordinates></Point><LineString><coordinates>0,0,0 "
	    "%1.0f,%1.0f,%1.0f</coordinates></LineString>");
    strcat (frmt, "<Polygon><outerBoundaryIs><LinearRing><coordinates>"
	    "0,0,0 0,%1.0f,0 %1.0f,%1.0f,%1.0f %1.0f,0,0 0,0,0"
	    "</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs>"
	    "<LinearRing><coordinates>5,5,0 5,%1.0f,0 %1.0f,%1.0f,%1.0f "
	    "%1.0f,5,0 5,5,0</coordinates></LinearRing></innerBoundaryIs>"
	    "</Polygon></MultiGeometry></Placemark>");
    resvalue =
	sqlite3_mprintf (frmt, kml1, kml2, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf
	("SELECT AsKml(%Q, %Q, GeomFromText('GEOMETRYCOLLECTIONZ("
	 "POINTZ(%1.0f %1.0f %1.0f), LINESTRINGZ(0 0 0, %1.0f %1.0f %1.0f), "
	 "POLYGONZ((0 0 0, 0 %1.0f 0, %1.0f %1.0f %1.0f, %1.0f 0 0, 0 0 0), "
	 "(5 5 0, 5 %1.0f 0, %1.0f %1.0f %1.0f, %1.0f 5 0, 5 5 0)))'"
	 ", 4326), 300)", kml1, kml2, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
	 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
	 1e128, 1e128);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Test AsKML#2 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -72;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr, "Unexpected rows/columns (AsKML#2): r=%d c=%d\n",
		   rows, columns);
	  return -73;
      }
    value = results[1];
    if (strcmp (resvalue, value) != 0)
      {
	  fprintf (stderr, "Unexpected result (AsKML#2): %s\n", results[1]);
	  return -74;
      }
    sqlite3_free_table (results);
    sqlite3_free (kml1);
    sqlite3_free (kml2);
    sqlite3_free (resvalue);

#endif /* end including PROJ.4 */

/* checking AsGML (1) */
    strcpy (frmt,
	    "<gml:MultiGeometry srsName=\"EPSG:4326\"><gml:geometryMember>"
	    "<gml:Point><gml:coordinates>%1.0f,%1.0f</gml:coordinates>"
	    "</gml:Point></gml:geometryMember><gml:geometryMember><gml:LineString>"
	    "<gml:coordinates>0,0 %1.0f,%1.0f</gml:coordinates></gml:LineString>"
	    "</gml:geometryMember><gml:geometryMember>");
    strcat (frmt,
	    "<gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>"
	    "0,0 0,%1.0f %1.0f,%1.0f %1.0f,0 0,0</gml:coordinates></gml:LinearRing>"
	    "</gml:outerBoundaryIs><gml:innerBoundaryIs><gml:LinearRing><gml:coordinates>"
	    "5,5 5,%1.0f %1.0f,%1.0f %1.0f,5 5,5</gml:coordinates></gml:LinearRing>"
	    "</gml:innerBoundaryIs></gml:Polygon></gml:geometryMember></gml:MultiGeometry>");
    resvalue =
	sqlite3_mprintf (frmt, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsGml(GeomFromText('GEOMETRYCOLLECTION("
			 "POINT(%1.0f %1.0f), LINESTRING(0 0, %1.0f %1.0f), "
			 "POLYGON((0 0, 0 %1.0f, %1.0f %1.0f, %1.0f 0, 0 0), "
			 "(5 5, 5 %1.0f, %1.0f %1.0f, %1.0f 5, 5 5)))'"
			 ", 4326), 300)", 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Test AsGML#1 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -75;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr, "Unexpected rows/columns (AsGML#1): r=%d c=%d\n",
		   rows, columns);
	  return -76;
      }
    value = results[1];
    if (strcmp (resvalue, value) != 0)
      {
	  fprintf (stderr, "Unexpected result (AsGML#1): %s\n", results[1]);
	  return -77;
      }
    sqlite3_free_table (results);
    sqlite3_free (resvalue);

/* checking AsGML (2) */
    strcpy (frmt,
	    "<gml:MultiGeometry srsName=\"EPSG:4326\"><gml:geometryMember>"
	    "<gml:Point><gml:pos srsDimension=\"3\">%1.0f %1.0f %1.0f</gml:pos>"
	    "</gml:Point></gml:geometryMember><gml:geometryMember><gml:Curve><gml:segments>"
	    "<gml:LineStringSegment><gml:posList srsDimension=\"3\">0 0 0 "
	    "%1.0f %1.0f %1.0f</gml:posList></gml:LineStringSegment></gml:segments>"
	    "</gml:Curve></gml:geometryMember><gml:geometryMember>");
    strcat (frmt,
	    "<gml:Polygon><gml:exterior><gml:LinearRing><gml:posList srsDimension=\"3\">"
	    "0 0 0 0 %1.0f 0 %1.0f %1.0f %1.0f %1.0f 0 0 0 0 0</gml:posList>"
	    "</gml:LinearRing></gml:exterior><gml:interior><gml:LinearRing>"
	    "<gml:posList srsDimension=\"3\">5 5 0 5 %1.0f 0 %1.0f %1.0f %1.0f "
	    "%1.0f 5 0 5 5 0</gml:posList></gml:LinearRing></gml:interior></gml:Polygon>"
	    "</gml:geometryMember></gml:MultiGeometry>");
    resvalue =
	sqlite3_mprintf (frmt, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsGml(3, GeomFromText('GEOMETRYCOLLECTIONZ("
			 "POINTZ(%1.0f %1.0f %1.0f), LINESTRINGZ(0 0 0, %1.0f %1.0f %1.0f), "
			 "POLYGONZ((0 0 0, 0 %1.0f 0, %1.0f %1.0f %1.0f, %1.0f 0 0, 0 0 0), "
			 "(5 5 0, 5 %1.0f 0, %1.0f %1.0f %1.0f, %1.0f 5 0, 5 5 0)))'"
			 ", 4326), 300)", 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Test AsGML#2 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -78;
      }
    if (rows != 1 || columns != 1)
      {
	  fprintf (stderr, "Unexpected rows/columns (AsGML#2): r=%d c=%d\n",
		   rows, columns);
	  return -79;
      }
    value = results[1];
    if (strcmp (resvalue, value) != 0)
      {
	  fprintf (stderr, "Unexpected result (AsGML#2): %s\n", results[1]);
	  return -80;
      }
    sqlite3_free_table (results);
    sqlite3_free (resvalue);

/* checking WKT 2D */
    resvalue = sqlite3_mprintf ("GEOMETRYCOLLECTION(POINT(%1.0f %1.0f), "
				"LINESTRING(0 0, %1.0f %1.0f), "
				"POLYGON((0 0, %1.0f 0, %1.0f %1.0f, 0 %1.0f, 0 0), "
				"(5 5, %1.0f 5, %1.0f %1.0f, 5 %1.0f, 5 5)))",
				1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
				1e128, 1e128, 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsText(GeomFromText('GEOMETRYCOLLECTION("
			 "POINT(%1.0f %1.0f), LINESTRING(0 0, %1.0f %1.0f), "
			 "POLYGON((0 0, %1.0f 0, %1.0f %1.0f, 0 %1.0f, 0 0), "
			 "(5 5, %1.0f 5, %1.0f %1.0f, 5 %1.0f, 5 5)))'))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "WKT 2D: error \"%s\"\n", sqlite3_errmsg (handle));
	  return -81;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (WKT 2D)\n");
	  return -82;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (WKT 2D): %s\n", string);
	  return -83;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking WKT 3D */
    resvalue =
	sqlite3_mprintf ("GEOMETRYCOLLECTION Z(POINT Z(%1.0f %1.0f %1.0f), "
			 "LINESTRING Z(0 0 0, %1.0f %1.0f %1.0f), "
			 "POLYGON Z((0 0 0, %1.0f 0 0, %1.0f %1.0f %1.0f, 0 %1.0f 0, 0 0 0), "
			 "(5 5 0, %1.0f 5 0, %1.0f %1.0f %1.0f, 5 %1.0f 0, 5 5 0)))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsText(GeomFromText('GEOMETRYCOLLECTIONZ("
			 "POINTZ(%1.0f %1.0f %1.0f), LINESTRINGZ(0 0 0, %1.0f %1.0f %1.0f), "
			 "POLYGONZ((0 0 0, %1.0f 0 0, %1.0f %1.0f %1.0f, 0 %1.0f 0, 0 0 0), "
			 "(5 5 0, %1.0f 5 0, %1.0f %1.0f %1.0f, 5 %1.0f 0, 5 5 0)))'))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "WKT 3D: error \"%s\"\n", sqlite3_errmsg (handle));
	  return -84;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (WKT 3D)\n");
	  return -85;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (WKT 3D): %s\n", string);
	  return -86;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking WKT M */
    resvalue =
	sqlite3_mprintf ("GEOMETRYCOLLECTION M(POINT M(%1.0f %1.0f %1.0f), "
			 "LINESTRING M(0 0 0, %1.0f %1.0f %1.0f), "
			 "POLYGON M((0 0 0, %1.0f 0 0, %1.0f %1.0f %1.0f, 0 %1.0f 0, 0 0 0), "
			 "(5 5 0, %1.0f 5 0, %1.0f %1.0f %1.0f, 5 %1.0f 0, 5 5 0)))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsText(GeomFromText('GEOMETRYCOLLECTIONM("
			 "POINTM(%1.0f %1.0f %1.0f), LINESTRINGM(0 0 0, %1.0f %1.0f %1.0f), "
			 "POLYGONM((0 0 0, %1.0f 0 0, %1.0f %1.0f %1.0f, 0 %1.0f 0, 0 0 0), "
			 "(5 5 0, %1.0f 5 0, %1.0f %1.0f %1.0f, 5 %1.0f 0, 5 5 0)))'))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "WKT M: error \"%s\"\n", sqlite3_errmsg (handle));
	  return -87;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (WKT M)\n");
	  return -88;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (WKT M): %s\n", string);
	  return -89;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking WKT 3D M */
    resvalue =
	sqlite3_mprintf
	("GEOMETRYCOLLECTION ZM(POINT ZM(%1.0f %1.0f %1.0f %1.0f), "
	 "LINESTRING ZM(0 0 0 0, %1.0f %1.0f %1.0f %1.0f), "
	 "POLYGON ZM((0 0 0 0, %1.0f 0 0 0, %1.0f %1.0f %1.0f %1.0f, 0 %1.0f 0 0, 0 0 0 0), "
	 "(5 5 0 0, %1.0f 5 0 0, %1.0f %1.0f %1.0f %1.0f, 5 %1.0f 0 0, 5 5 0 0)))",
	 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
	 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsText(GeomFromText('GEOMETRYCOLLECTIONZM("
			 "POINTZM(%1.0f %1.0f %1.0f %1.0f), LINESTRINGZM(0 0 0 0, %1.0f %1.0f %1.0f %1.0f), "
			 "POLYGONZM((0 0 0 0, %1.0f 0 0 0, %1.0f %1.0f %1.0f %1.0f, 0 %1.0f 0 0, 0 0 0 0), "
			 "(5 5 0 0, %1.0f 5 0 0, %1.0f %1.0f %1.0f %1.0f, 5 %1.0f 0 0, 5 5 0 0)))'))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "WKT 3D M: error \"%s\"\n", sqlite3_errmsg (handle));
	  return -90;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (WKT 3D M)\n");
	  return -91;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (WKT 3D M): %s\n", string);
	  return -92;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking EWKT 2D */
    resvalue =
	sqlite3_mprintf ("SRID=4326;GEOMETRYCOLLECTION(POINT(%1.0f %1.0f),"
			 "LINESTRING(0 0,%1.0f %1.0f),"
			 "POLYGON((0 0,%1.0f 0,%1.0f %1.0f,0 %1.0f,0 0),"
			 "(5 5,%1.0f 5,%1.0f %1.0f,5 %1.0f,5 5)))", 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsEWKT(GeomFromText('GEOMETRYCOLLECTION("
			 "POINT(%1.0f %1.0f), LINESTRING(0 0, %1.0f %1.0f), "
			 "POLYGON((0 0, %1.0f 0, %1.0f %1.0f, 0 %1.0f, 0 0), "
			 "(5 5, %1.0f 5, %1.0f %1.0f, 5 %1.0f, 5 5)))', 4326))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "EWKT 2D: error \"%s\"\n", sqlite3_errmsg (handle));
	  return -93;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (EWKT 2D)\n");
	  return -94;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (EWKT 2D): %s\n", string);
	  return -95;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking EWKT 3D */
    resvalue =
	sqlite3_mprintf
	("SRID=4326;GEOMETRYCOLLECTION(POINT(%1.0f %1.0f %1.0f),"
	 "LINESTRING(0 0 0,%1.0f %1.0f %1.0f),"
	 "POLYGON((0 0 0,%1.0f 0 0,%1.0f %1.0f %1.0f,0 %1.0f 0,0 0 0),"
	 "(5 5 0,%1.0f 5 0,%1.0f %1.0f %1.0f,5 %1.0f 0,5 5 0)))", 1e128, 1e128,
	 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
	 1e128, 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsEWKT(GeomFromText('GEOMETRYCOLLECTIONZ("
			 "POINTZ(%1.0f %1.0f %1.0f), LINESTRINGZ(0 0 0, %1.0f %1.0f %1.0f), "
			 "POLYGONZ((0 0 0, %1.0f 0 0, %1.0f %1.0f %1.0f, 0 %1.0f 0, 0 0 0), "
			 "(5 5 0, %1.0f 5 0, %1.0f %1.0f %1.0f, 5 %1.0f 0, 5 5 0)))', 4326))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "EWKT 3D: error \"%s\"\n", sqlite3_errmsg (handle));
	  return -96;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (EWKT 3D)\n");
	  return -97;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (EWKT 3D): %s\n", string);
	  return -98;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking EWKT M */
    resvalue =
	sqlite3_mprintf
	("SRID=4326;GEOMETRYCOLLECTIONM(POINTM(%1.0f %1.0f %1.0f),"
	 "LINESTRINGM(0 0 0,%1.0f %1.0f %1.0f),"
	 "POLYGONM((0 0 0,%1.0f 0 0,%1.0f %1.0f %1.0f,0 %1.0f 0,0 0 0),"
	 "(5 5 0,%1.0f 5 0,%1.0f %1.0f %1.0f,5 %1.0f 0,5 5 0)))", 1e128, 1e128,
	 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
	 1e128, 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsEWKT(GeomFromText('GEOMETRYCOLLECTIONM("
			 "POINTM(%1.0f %1.0f %1.0f), LINESTRINGM(0 0 0, %1.0f %1.0f %1.0f), "
			 "POLYGONM((0 0 0, %1.0f 0 0, %1.0f %1.0f %1.0f, 0 %1.0f 0, 0 0 0), "
			 "(5 5 0, %1.0f 5 0, %1.0f %1.0f %1.0f, 5 %1.0f 0, 5 5 0)))', 4326))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "EWKT M: error \"%s\"\n", sqlite3_errmsg (handle));
	  return -99;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (EWKT M)\n");
	  return -100;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (EWKT M): %s\n", string);
	  return -101;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking EWKT 3D M */
    resvalue =
	sqlite3_mprintf
	("SRID=4326;GEOMETRYCOLLECTION(POINT(%1.0f %1.0f %1.0f %1.0f),"
	 "LINESTRING(0 0 0 0,%1.0f %1.0f %1.0f %1.0f),"
	 "POLYGON((0 0 0 0,%1.0f 0 0 0,%1.0f %1.0f %1.0f %1.0f,0 %1.0f 0 0,0 0 0 0),"
	 "(5 5 0 0,%1.0f 5 0 0,%1.0f %1.0f %1.0f %1.0f,5 %1.0f 0 0,5 5 0 0)))",
	 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
	 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsEWKT(GeomFromText('GEOMETRYCOLLECTIONZM("
			 "POINTZM(%1.0f %1.0f %1.0f %1.0f), LINESTRINGZM(0 0 0 0, %1.0f %1.0f %1.0f %1.0f), "
			 "POLYGONZM((0 0 0 0, %1.0f 0 0 0, %1.0f %1.0f %1.0f %1.0f, 0 %1.0f 0 0, 0 0 0 0), "
			 "(5 5 0 0, %1.0f 5 0 0, %1.0f %1.0f %1.0f %1.0f, 5 %1.0f 0 0, 5 5 0 0)))', 4326))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "EWKT 3D M: error \"%s\"\n",
		   sqlite3_errmsg (handle));
	  return -102;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (EWKT 3D M)\n");
	  return -103;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (EWKT 3D M): %s\n", string);
	  return -104;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking WKT Strict */
    resvalue = sqlite3_mprintf ("GEOMETRYCOLLECTION(POINT(%1.0f %1.0f),"
				"LINESTRING(0 0,%1.0f %1.0f),"
				"POLYGON((0 0,%1.0f 0,%1.0f %1.0f,0 %1.0f,0 0),"
				"(5 5,%1.0f 5,%1.0f %1.0f,5 %1.0f,5 5)))",
				1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
				1e128, 1e128, 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsWKT(GeomFromText('GEOMETRYCOLLECTIONZM("
			 "POINTZM(%1.0f %1.0f %1.0f %1.0f), LINESTRINGZM(0 0 0 0, %1.0f %1.0f %1.0f %1.0f), "
			 "POLYGONZM((0 0 0 0, %1.0f 0 0 0, %1.0f %1.0f %1.0f %1.0f, 0 %1.0f 0 0, 0 0 0 0), "
			 "(5 5 0 0, %1.0f 5 0 0, %1.0f %1.0f %1.0f %1.0f, 5 %1.0f 0 0, 5 5 0 0)))', 4326))",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "WKT Strict: error \"%s\"\n",
		   sqlite3_errmsg (handle));
	  return -105;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (WKT Strict)\n");
	  return -106;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (WKT Strict): %s\n", string);
	  return -107;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking GeoJSON 2D */
    resvalue =
	sqlite3_mprintf
	("{\"type\":\"GeometryCollection\",\"crs\":{\"type\":\"name\",\"properties\":"
	 "{\"name\":\"urn:ogc:def:crs:EPSG:4326\"}},\"bbox\":[0,0,%1.0f,%1.0f],"
	 "\"geometries\":[{\"type\":\"Point\",\"coordinates\":[%1.0f,%1.0f]},"
	 "{\"type\":\"LineString\",\"coordinates\":[[0,0],[%1.0f,%1.0f]]},"
	 "{\"type\":\"Polygon\",\"coordinates\":"
	 "[[[0,0],[%1.0f,0],[%1.0f,%1.0f],[0,%1.0f],[0,0]],"
	 "[[5,5],[%1.0f,5],[%1.0f,%1.0f],[5,%1.0f],[5,5]]]}]}", 1e128, 1e128,
	 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
	 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsGeoJSON(GeomFromText('GEOMETRYCOLLECTION("
			 "POINT(%1.0f %1.0f), LINESTRING(0 0, %1.0f %1.0f), "
			 "POLYGON((0 0, %1.0f 0, %1.0f %1.0f, 0 %1.0f, 0 0), "
			 "(5 5, %1.0f 5, %1.0f %1.0f, 5 %1.0f, 5 5)))', 4326), 2, 5)",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "GeoJSON 2D: error \"%s\"\n",
		   sqlite3_errmsg (handle));
	  return -108;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (GeoJSON 2D)\n");
	  return -109;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (GeoJSON 2D): %s\n", string);
	  return -110;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking GeoJSON 3D */
    resvalue =
	sqlite3_mprintf
	("{\"type\":\"GeometryCollection\",\"crs\":{\"type\":\"name\",\"properties\":"
	 "{\"name\":\"urn:ogc:def:crs:EPSG:4326\"}},\"bbox\":[0,0,%1.0f,%1.0f],"
	 "\"geometries\":[{\"type\":\"Point\",\"coordinates\":[%1.0f,%1.0f,%1.0f]},"
	 "{\"type\":\"LineString\",\"coordinates\":[[0,0,0],[%1.0f,%1.0f,%1.0f]]},"
	 "{\"type\":\"Polygon\",\"coordinates\":"
	 "[[[0,0,0],[%1.0f,0,0],[%1.0f,%1.0f,%1.0f],[0,%1.0f,0],[0,0,0]],"
	 "[[5,5,0],[%1.0f,5,0],[%1.0f,%1.0f,%1.0f],[5,%1.0f,0],[5,5,0]]]}]}",
	 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
	 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128);
    sql =
	sqlite3_mprintf ("SELECT AsGeoJSON(GeomFromText('GEOMETRYCOLLECTIONZ("
			 "POINTZ(%1.0f %1.0f %1.0f), LINESTRINGZ(0 0 0, %1.0f %1.0f %1.0f), "
			 "POLYGONZ((0 0 0, %1.0f 0 0, %1.0f %1.0f %1.0f, 0 %1.0f 0, 0 0 0), "
			 "(5 5 0, %1.0f 5 0, %1.0f %1.0f %1.0f, 5 %1.0f 0, 5 5 0)))', 4326), 2, 5)",
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128, 1e128, 1e128, 1e128, 1e128, 1e128, 1e128,
			 1e128);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "GeoJSON 3D: error \"%s\"\n",
		   sqlite3_errmsg (handle));
	  return -111;
      }
    string = NULL;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
		  {
		      value = (const char *) sqlite3_column_text (stmt, 0);
		      len = sqlite3_column_bytes (stmt, 0);
		      string = malloc (len + 1);
		      strcpy (string, value);
		  }
	    }
      }
    ret = sqlite3_finalize (stmt);
    if (string == NULL)
      {
	  fprintf (stderr, "Unexpected NULL result (GeoJSON 3D)\n");
	  return -112;
      }
    if (strcmp (resvalue, string) != 0)
      {
	  fprintf (stderr, "Unexpected result (GeoJSON 3D): %s\n", string);
	  return -113;
      }
    free (string);
    sqlite3_free (resvalue);

/* checking gaiaDequotedSql */
    resvalue = gaiaDequotedSql ("'a''b''c''d''e''f''ghiklmnopqrst''u''vwy''z'");
    if (strcmp (resvalue, "a'b'c'd'e'f'ghiklmnopqrst'u'vwy'z") != 0)
      {
	  fprintf (stderr, "Unexpected result (Dequote 1): %s\n", resvalue);
	  return -114;
      }
    free (resvalue);

    resvalue = gaiaDequotedSql ("\"abcdef\"\"ghijkmlmnopqsrt\"\"uvwy\"\"z\"");
    if (strcmp (resvalue, "abcdef\"ghijkmlmnopqsrt\"uvwy\"z") != 0)
      {
	  fprintf (stderr, "Unexpected result (Dequote 2): %s\n", resvalue);
	  return -115;
      }
    free (resvalue);

    resvalue = gaiaDequotedSql ("abcdefghijkmlnopqrtsuvwyz");
    if (strcmp (resvalue, "abcdefghijkmlnopqrtsuvwyz") != 0)
      {
	  fprintf (stderr, "Unexpected result (Dequote 3): %s\n", resvalue);
	  return -116;
      }
    free (resvalue);

    resvalue = gaiaDequotedSql ("'a''b''c''d''e''f''ghiklmnopqrst''u'vwy''z'");
    if (resvalue != NULL)
      {
	  fprintf (stderr, "Unexpected result (Dequote 4): %s\n", resvalue);
	  return -117;
      }

/* checking load_shapefile */
    ret = load_shapefile (handle, "./shp/gaza/route", shape, "UTF-8", 4326,
			  NULL, 1, 0, 1, 1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "load_shapefile() error: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -118;
      }
    if (row_count != 2)
      {
	  fprintf (stderr, "unexpected row count for load_shapefile: %i\n",
		   row_count);
	  sqlite3_close (handle);
	  return -119;
      }
/* checking dump_shapefile */
    ret =
	dump_shapefile (handle, shape, "Geometry", dumpname, "UTF-8", "", 1,
			&row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -120;
      }
    cleanup_shapefile (dumpname);
    if (row_count != 2)
      {
	  fprintf (stderr, "unexpected dump row count for shapefile: %i\n",
		   row_count);
	  sqlite3_close (handle);
	  return -121;
      }

#ifndef OMIT_PROJ		/* including PROJ.4 */
/* checking dump_kml */
    ret =
	dump_kml (handle, shape, "geometry", dumpname, "sub_type", "name", 10);
    if (!ret)
      {
	  fprintf (stderr, "dump_kml() error: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -122;
      }
    unlink (dumpname);
#endif /* end including PROJ.4 */

/* checking dump_geojson */
    ret = dump_geojson (handle, shape, "col1", dumpname, 10, 5);
    if (!ret)
      {
	  fprintf (stderr, "dump_geojson() error: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -123;
      }
    unlink (dumpname);
/* dropping virtual geometry */
    sql = sqlite3_mprintf ("SELECT DropVirtualGeometry(%Q)", shape);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "DROP VIRTUAL GEOMETRT /shp/gaza/barrier error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -124;
      }
    sqlite3_free (shape);

/* checking load_dbf */
    ret =
	load_dbf (handle, "./shapetest1.dbf", dbf, "UTF-8", 1, &row_count,
		  err_msg);
    if (!ret)
      {
	  fprintf (stderr, "load_dbf() error: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -125;
      }
    if (row_count != 2)
      {
	  fprintf (stderr, "unexpected row count for load_dbf: %i\n",
		   row_count);
	  sqlite3_close (handle);
	  return -126;
      }
/* checking dump_dbf */
    ret = dump_dbf (handle, dbf, dumpname, "CP1252", err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_dbf() error for points: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -127;
      }
    unlink (dumpname);
    sqlite3_free (dbf);

/* checking load_shapefile (2) */
    ret =
	load_shapefile (handle, "./shp/merano-3d/polygons", shape2, "CP1252",
			25832, shape2geom, 0, 1, 1, 0, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "load_shapefile() #2 error: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -128;
      }
    if (row_count != 10)
      {
	  fprintf (stderr, "unexpected row count for load_shapefile #2: %i\n",
		   row_count);
	  sqlite3_close (handle);
	  return -129;
      }
    xtable = gaiaDoubleQuotedSql (shape2);
    sql = sqlite3_mprintf ("INSERT INTO \"%s\" (FEATURE_ID, DATUM, HAUSNR) "
			   "VALUES (1250000, 0.1, 'alpha')", xtable);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT polygons (1) error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -130;
      }
    sql = sqlite3_mprintf ("INSERT INTO \"%s\" (FEATURE_ID, DATUM, HAUSNR) "
			   "VALUES (1250000, 0.1, 'alpha')", xtable);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT polygons (1) error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -131;
      }
    sql = sqlite3_mprintf ("INSERT INTO \"%s\" (FEATURE_ID, DATUM, TEXT_I) "
			   "VALUES (1250000, 0.1, 'alpha')", xtable);
    free (xtable);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT polygons (1) error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -132;
      }

    remove_duplicated_rows (handle, shape2);
    elementary_geometries (handle, shape2, shape2geom, "elem_poly", "pk_elem",
			   "mul_id");
    sqlite3_free (shape2);
    sqlite3_free (shape2geom);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -133;
      }

    spatialite_cleanup_ex (cache);
    free (suffix);
    spatialite_shutdown ();

    return 0;
}
