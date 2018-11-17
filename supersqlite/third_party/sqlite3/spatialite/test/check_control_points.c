/*

 check_contro_points.c -- SpatiaLite Test Case

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

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

static int
test_query (sqlite3 * sqlite, const char *sql, const char *expected)
{
/* testing some SQL query */
    int ret;
    sqlite3_stmt *stmt = NULL;

    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "%s\n: \"%s\"\n", sql, sqlite3_errmsg (sqlite));
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
		const char *value;
		if (sqlite3_column_type (stmt, 0) == SQLITE_NULL)
		  {
		      if (expected == NULL)
			  continue;
		      else
			{
			    fprintf (stderr,
				     "Unexpected NULL result\nExpected: %s\n",
				     expected);
			    goto error;
			}
		  }
		value = (const char *) sqlite3_column_text (stmt, 0);
		if (expected == NULL)
		  {
		      fprintf (stderr, "Unexpected result %s\nExpected: NULL\n",
			       value);
		      goto error;
		  }
		if (strcasecmp (value, expected) != 0)
		  {
		      fprintf (stderr, "Unexpected result %s\nExpected: %s\n",
			       value, expected);
		      goto error;
		  }
	    }
	  else
	      goto error;
      }
    sqlite3_finalize (stmt);
    return 1;

  error:
    if (stmt != NULL)
	sqlite3_finalize (stmt);
    return 0;
}

static int
test_transform (sqlite3 * handle, int order)
{
/* testing GCP_Transform */
    char *sql;
    int ret;

/* resolving Control Points 2D - CGP_Transform XY */
    sql = sqlite3_mprintf ("SELECT AsText(GCP_Transform(g.geometry, b.gcp)) "
			   "FROM test_xy AS g, "
			   "(SELECT GCP_Compute(a.geometry, b.geometry, %d) AS gcp "
			   "FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id) AS b",
			   order);
    ret =
	test_query (handle, sql,
		    "GEOMETRYCOLLECTION(POINT(-161 -64), LINESTRING(-160 -63, -159 -62), "
		    "POLYGON((-151 -54, -141 -54, -141 -44, -151 -44, -151 -54), "
		    "(-147 -50, -145 -50, -145 -48, -147 -48, -147 -50)))");
    sqlite3_free (sql);
    if (!ret)
	return 0;

/* resolving Control Points 2D - CGP_Transform XYM */
    sql = sqlite3_mprintf ("SELECT AsText(GCP_Transform(g.geometry, b.gcp)) "
			   "FROM test_xym AS g, "
			   "(SELECT GCP_Compute(a.geometry, b.geometry, %d) AS gcp "
			   "FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id) AS b",
			   order);
    ret =
	test_query (handle, sql,
		    "GEOMETRYCOLLECTION M(POINT M(-161 -64 1), "
		    "LINESTRING M(-160 -63 1, -159 -62 3), "
		    "POLYGON M((-151 -54 1, -141 -54 2, -141 -44 3, -151 -44 2, -151 -54 1), "
		    "(-147 -50 1, -145 -50 2, -145 -48 2, -147 -48 2, -147 -50 1)))");
    sqlite3_free (sql);
    if (!ret)
	return 0;

/* resolving Control Points 3D - CGP_Transform XYZ */
    sql =
	sqlite3_mprintf
	("SELECT IsValid(GCP_Transform(g.geometry, b.gcp)) "
	 "FROM test_xyz AS g, "
	 "(SELECT GCP_Compute(a.geometry, b.geometry, %d) AS gcp "
	 "FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id) AS b",
	 order);
    ret = test_query (handle, sql, "1");
    sqlite3_free (sql);
    if (!ret)
	return 0;

/* resolving Control Points 3D - CGP_Transform XYZM */
    sql =
	sqlite3_mprintf
	("SELECT IsValid(GCP_Transform(g.geometry, b.gcp)) "
	 "FROM test_xyzm AS g, "
	 "(SELECT GCP_Compute(a.geometry, b.geometry, %d) AS gcp "
	 "FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id) AS b",
	 order);
    ret = test_query (handle, sql, "1");
    sqlite3_free (sql);
    if (!ret)
	return 0;

    return 1;
}

static int
test_invalid (sqlite3 * handle)
{
/* testing invalid calls */
    const char *sql;

    sql = "SELECT GCP_Compute(NULL, GeomFromText('POINT(1 1)'))";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_Compute(GeomFromText('POINT(1 1)'), NULL)";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql =
	"SELECT GCP_Compute(GeomFromText('POINT(1 1)'), GeomFromText('POINT(2 2)'), NULL)";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql =
	"SELECT GCP_Compute(GeomFromText('POINT(1 1)'), GeomFromText('POINT(2 2)'), -1)";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql =
	"SELECT GCP_Compute(GeomFromText('POINT(1 1)'), GeomFromText('POINT(2 2)'), 4)";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql =
	"SELECT GCP_Compute(GeomFromText('POINT(1 1)'), GeomFromText('POINTZ(2 2 2)'))";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql =
	"SELECT GCP_Compute(GeomFromText('POINT(1 1)'), GeomFromText('POINT(2 2)'))";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql =
	"SELECT GCP_Compute(GeomFromText('POINT(1 1)'), GeomFromText('LINESTRING(2 2, 3 2)'))";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql =
	"SELECT GCP_Compute(GeomFromText('LINESTRING(2 2, 3 2)'), GeomFromText('POINT(1 1)'))";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_Compute(GeomFromText('POINT(1 1)'), NULL)";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_Compute(GeomFromText('POINT(1 1)'), zeroblob(10))";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_Transform(NULL, b.gcp) "
	"FROM (SELECT GCP_Compute(a.geometry, b.geometry) AS gcp "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id) AS b";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_Transform(zeroblob(10), b.gcp) "
	"FROM (SELECT GCP_Compute(a.geometry, b.geometry) AS gcp "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id) AS b";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_Transform(GeomFromText('POINT(1 1)'), zeroblob(4))";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_IsValid(NULL)";
    if (!test_query (handle, sql, "-1"))
	return 0;
    sql = "SELECT GCP_IsValid(zeroblob(0))";
    if (!test_query (handle, sql, "0"))
	return 0;
    sql = "SELECT GCP_AsText(NULL)";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_AsText(zeroblob(0))";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_Transform(GeomFromText('POINT(1 1)'), b.gcp, 1.0) "
	"FROM (SELECT GCP_Compute(a.geometry, b.geometry) AS gcp "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id) AS b";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_Transform(zeroblob(4), b.gcp, 32632) "
	"FROM (SELECT GCP_Compute(a.geometry, b.geometry) AS gcp "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id) AS b";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP_Transform(GeomFromText('POINT(1 1)'), NULL)";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP2ATM(NULL)";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP2ATM(zeroblob(100))";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP2ATM(b.gcp) "
	"FROM (SELECT GCP_Compute(a.geometry, b.geometry, 2) AS gcp "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id) AS b";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP2ATM(b.gcp) "
	"FROM (SELECT GCP_Compute(a.geometry, b.geometry, 3) AS gcp "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id) AS b";
    if (!test_query (handle, sql, NULL))
	return 0;
    sql = "SELECT GCP2ATM(b.gcp) "
	"FROM (SELECT GCP_Compute(a.geometry, b.geometry, 0) AS gcp "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id) AS b";
    if (!test_query (handle, sql, NULL))
	return 0;

    return 1;
}

int
main (int argc, char *argv[])
{
#ifdef ENABLE_GCP		/* only if Control Points (Grass) is enabled */
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    const char *sql;
    int order;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory databse: %s\n",
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

/* creating and populating a test table - XY */
    ret = sqlite3_exec (handle, "CREATE TABLE test_xy ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT)", NULL, NULL,
			&err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE test_xy error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -3;
      }
    ret = sqlite3_exec (handle, "SELECT AddGeometryColumn("
			"'test_xy', 'geometry', 4326, 'GEOMETRYCOLLECTION', 'XY')",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE test_xy Geometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -4;
      }
    ret = sqlite3_exec (handle, "INSERT INTO test_xy VALUES (NULL, "
			"GeomFromText('GEOMETRYCOLLECTION(POINT(0 0), LINESTRING(1 1, 2 2), "
			"POLYGON((10 10, 20 10, 20 20, 10 20, 10 10), "
			"(14 14, 16 14, 16 16, 14 16, 14 14)))', 4326))", NULL,
			NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO test_xy Geometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -5;
      }

/* creating and populating a test table - XYZ */
    ret = sqlite3_exec (handle, "CREATE TABLE test_xyz ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT)", NULL, NULL,
			&err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE test_xyz error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -6;
      }
    ret = sqlite3_exec (handle, "SELECT AddGeometryColumn("
			"'test_xyz', 'geometry', 4326, 'GEOMETRYCOLLECTION', 'XYz')",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE test_xyz Geometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -7;
      }
    ret = sqlite3_exec (handle, "INSERT INTO test_xyz VALUES (NULL, "
			"GeomFromText('GEOMETRYCOLLECTIONZ(POINTZ(0 0 1), "
			"LINESTRINGZ(1 1 1, 2 2 3), "
			"POLYGONZ((10 10 1, 20 10 2, 20 20 3, 10 20 2, 10 10 1), "
			"(14 14 1, 16 14 2, 16 16 2, 14 16 2, 14 14 1)))', 4326))",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO test_xyz Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -8;
      }

/* creating and populating a test table - XYM */
    ret = sqlite3_exec (handle, "CREATE TABLE test_xym ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT)", NULL, NULL,
			&err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE test_xym error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -9;
      }
    ret = sqlite3_exec (handle, "SELECT AddGeometryColumn("
			"'test_xym', 'geometry', 4326, 'GEOMETRYCOLLECTION', 'XYM')",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE test_xym Geometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -10;
      }
    ret = sqlite3_exec (handle, "INSERT INTO test_xym VALUES (NULL, "
			"GeomFromText('GEOMETRYCOLLECTIONM(POINTM(0 0 1), "
			"LINESTRINGM(1 1 1, 2 2 3), "
			"POLYGONM((10 10 1, 20 10 2, 20 20 3, 10 20 2, 10 10 1), "
			"(14 14 1, 16 14 2, 16 16 2, 14 16 2, 14 14 1)))', 4326))",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO test_xym Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -11;
      }

/* creating and populating a test table - XYZM */
    ret = sqlite3_exec (handle, "CREATE TABLE test_xyzm ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT)", NULL, NULL,
			&err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE test_xyzm error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -12;
      }
    ret = sqlite3_exec (handle, "SELECT AddGeometryColumn("
			"'test_xyzm', 'geometry', 4326, 'GEOMETRYCOLLECTION', 'XYZM')",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE test_xyzm Geometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -13;
      }
    ret = sqlite3_exec (handle, "INSERT INTO test_xyzm VALUES (NULL, "
			"GeomFromText('GEOMETRYCOLLECTIONZM(POINTZM(0 0 1 1), "
			"LINESTRINGZM(1 1 1 1, 2 2 3 3), "
			"POLYGONZM((10 10 1 1, 20 10 2 2, 20 20 3 3, 10 20 2 2, 10 10 1 1), "
			"(14 14 1 1, 16 14 2 2, 16 16 2 2, 14 16 2 2, 14 14 1 1)))', 4326))",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO test_xyzm Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -14;
      }

/* creating and populating Point table #1 - XY */
    ret = sqlite3_exec (handle, "CREATE TABLE point_a_xy ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT)", NULL, NULL,
			&err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE point_a_xy error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -15;
      }
    ret = sqlite3_exec (handle, "SELECT AddGeometryColumn("
			"'point_a_xy', 'geometry', 4326, 'POINT', 'XY')", NULL,
			NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE point_a_xy Geometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -16;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_a_xy VALUES "
			"(NULL, MakePoint(0, 0, 4326)), (NULL, MakePoint(1, 1, 4326)), "
			"(NULL, MakePoint(10, 10, 4326)), (NULL, MakePoint(11, 11, 4326)), "
			"(NULL, MakePoint(10, 0, 4326)), (NULL, MakePoint(11, 1, 4326)), "
			"(NULL, MakePoint(0, 10, 4326)), (NULL, MakePoint(1, 11, 4326)) ",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_a_xy Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -17;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_a_xy VALUES "
			"(NULL, MakePoint(5, 4, 4326)), (NULL, MakePoint(4, 5, 4326)), "
			"(NULL, MakePoint(8, 2, 4326)), (NULL, MakePoint(2, 8, 4326)), "
			"(NULL, MakePoint(20, 20, 4326)), (NULL, MakePoint(21, 21, 4326)), "
			"(NULL, MakePoint(30, 30, 4326)), (NULL, MakePoint(31, 31, 4326)), "
			"(NULL, MakePoint(30, 20, 4326)), (NULL, MakePoint(31, 21, 4326)) ",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_a_xy Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -18;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_a_xy VALUES "
			"(NULL, MakePoint(20, 30, 4326)), (NULL, MakePoint(21, 31, 4326)), "
			"(NULL, MakePoint(25, 24, 4326)), (NULL, MakePoint(24, 25, 4326)), "
			"(NULL, MakePoint(28, 22, 4326)), (NULL, MakePoint(22, 28, 4326)), "
			"(NULL, MakePoint(30, 0, 4326)), (NULL, MakePoint(0, 30, 4326)) ",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_a_xy Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -19;
      }

/* creating and populating Point table #2 - XY */
    ret = sqlite3_exec (handle, "CREATE TABLE point_b_xy ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT)", NULL, NULL,
			&err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE point_b_xy error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -20;
      }
    ret = sqlite3_exec (handle, "SELECT AddGeometryColumn("
			"'point_b_xy', 'geometry', 4326, 'POINT', 'XY')", NULL,
			NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE point_b_xy Geometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -21;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_b_xy VALUES "
			"(NULL, MakePoint(-161 + 0, -64 + 0, 4326)), "
			"(NULL, MakePoint(-161 + 1, -64 + 1, 4326)), "
			"(NULL, MakePoint(-161 + 10, -64 + 10, 4326)), "
			"(NULL, MakePoint(-161 + 11, -64 + 11, 4326)), "
			"(NULL, MakePoint(-161 + 10, -64 + 0, 4326)), "
			"(NULL, MakePoint(-161 + 11, -64 + 1, 4326)) ", NULL,
			NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_b_xy Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -22;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_b_xy VALUES "
			"(NULL, MakePoint(-161 + 0, -64 + 10, 4326)), "
			"(NULL, MakePoint(-161 + 1, -64 + 11, 4326)), "
			"(NULL, MakePoint(-161 + 5, -64 + 4, 4326)), "
			"(NULL, MakePoint(-161 + 4, -64 + 5, 4326)), "
			"(NULL, MakePoint(-161 + 8, -64 + 2, 4326)), "
			"(NULL, MakePoint(-161 + 2, -64 + 8, 4326)) ", NULL,
			NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_b_xy Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -23;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_b_xy VALUES "
			"(NULL, MakePoint(-161 + 20, -64 + 20, 4326)), "
			"(NULL, MakePoint(-161 + 21, -64 + 21, 4326)), "
			"(NULL, MakePoint(-161 + 30, -64 + 30, 4326)), "
			"(NULL, MakePoint(-161 + 31, -64 + 31, 4326)), "
			"(NULL, MakePoint(-161 + 30, -64 + 20, 4326)), "
			"(NULL, MakePoint(-161 + 31, -64 + 21, 4326)) ", NULL,
			NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_b_xy Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -24;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_b_xy VALUES "
			"(NULL, MakePoint(-161 + 20, -64 + 30, 4326)), "
			"(NULL, MakePoint(-161 + 21, -64 + 31, 4326)), "
			"(NULL, MakePoint(-161 + 25, -64 + 24, 4326)), "
			"(NULL, MakePoint(-161 + 24, -64 + 25, 4326)), "
			"(NULL, MakePoint(-161 + 28, -64 + 22, 4326)), "
			"(NULL, MakePoint(-161 + 22, -64 + 28, 4326)), "
			"(NULL, MakePoint(-161 + 30, -64 + 0, 4326)), "
			"(NULL, MakePoint(-161 + 0, -64 + 30, 4326)) ", NULL,
			NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_b_xy Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -25;
      }

/* creating and populating Point table #1 - XYZ */
    ret = sqlite3_exec (handle, "CREATE TABLE point_a_xyz ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT)", NULL, NULL,
			&err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE point_a_xyz error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -26;
      }
    ret = sqlite3_exec (handle, "SELECT AddGeometryColumn("
			"'point_a_xyz', 'geometry', 4326, 'POINT', 'XYZ')",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE point_a_xy Geometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -27;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_a_xyz VALUES "
			"(NULL, MakePointZ(0, 0, 1, 4326)), (NULL, MakePointZ(1, 1, 2, 4326)), "
			"(NULL, MakePointZ(10, 10, 1, 4326)), (NULL, MakePointZ(11, 11, 2, 4326)), "
			"(NULL, MakePointZ(10, 0, 1, 4326)), (NULL, MakePointZ(11, 1, 2, 4326)), "
			"(NULL, MakePointZ(0, 10, 1, 4326)), (NULL, MakePointZ(1, 11, 2, 4326)) ",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_a_xyz Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -28;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_a_xyz VALUES "
			"(NULL, MakePointZ(5, 4, 1, 4326)), (NULL, MakePointZ(4, 5, 2, 4326)), "
			"(NULL, MakePointZ(8, 2, 1, 4326)), (NULL, MakePointZ(2, 8, 2, 4326)), "
			"(NULL, MakePointZ(20, 20, 1, 4326)), (NULL, MakePointZ(21, 21, 2, 4326)), "
			"(NULL, MakePointZ(70, 70, 1, 4326)), (NULL, MakePointZ(51, 51, 2, 4326)), "
			"(NULL, MakePointZ(30, 20, 1, 4326)), (NULL, MakePointZ(31, 21, 2, 4326)) ",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_a_xyz Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -29;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_a_xyz VALUES "
			"(NULL, MakePointZ(20, 30, 1, 4326)), (NULL, MakePointZ(21, 31, 2, 4326)), "
			"(NULL, MakePointZ(25, 24, 1, 4326)), (NULL, MakePointZ(24, 25, 2, 4326)), "
			"(NULL, MakePointZ(28, 22, 1, 4326)), (NULL, MakePointZ(22, 28, 2, 4326)), "
			"(NULL, MakePointZ(30, 0, 1, 4326)), (NULL, MakePointZ(0, 30, 2, 4326)) ",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_a_xyz Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -30;
      }

/* creating and populating Point table #2 - XYZ */
    ret = sqlite3_exec (handle, "CREATE TABLE point_b_xyz ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT)", NULL, NULL,
			&err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE point_b_xyz error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -31;
      }
    ret = sqlite3_exec (handle, "SELECT AddGeometryColumn("
			"'point_b_xyz', 'geometry', 4326, 'POINT', 'XYZ')",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE point_b_xyz Geometry error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -32;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_b_xyz VALUES "
			"(NULL, MakePointZ(-161 + 0, -64 + 0, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 1, -64 + 1, 12, 4326)), "
			"(NULL, MakePointZ(-161 + 10, -64 + 10, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 11, -64 + 11, 12, 4326)), "
			"(NULL, MakePointZ(-161 + 10, -64 + 0, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 11, -64 + 1, 12, 4326)) ",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_b_xyz Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -33;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_b_xyz VALUES "
			"(NULL, MakePointZ(-161 + 0, -64 + 10, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 1, -64 + 11, 12, 4326)), "
			"(NULL, MakePointZ(-161 + 5, -64 + 4, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 4, -64 + 5, 12, 4326)), "
			"(NULL, MakePointZ(-161 + 8, -64 + 2, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 2, -64 + 8, 12, 4326)) ",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_b_xyz Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -34;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_b_xyz VALUES "
			"(NULL, MakePointZ(-161 + 20, -64 + 20, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 21, -64 + 21, 12, 4326)), "
			"(NULL, MakePointZ(-161 + 70, -64 + 70, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 51, -64 + 51, 12, 4326)), "
			"(NULL, MakePointZ(-161 + 30, -64 + 20, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 31, -64 + 21, 12, 4326)), "
			"(NULL, MakePointZ(-161 + 20, -64 + 30, 11, 4326)) ",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_b_xyz Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -35;
      }
    ret = sqlite3_exec (handle, "INSERT INTO point_b_xyz VALUES "
			"(NULL, MakePointZ(-161 + 21, -64 + 31, 12, 4326)), "
			"(NULL, MakePointZ(-161 + 25, -64 + 24, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 24, -64 + 25, 12, 4326)), "
			"(NULL, MakePointZ(-161 + 28, -64 + 22, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 22, -64 + 28, 12, 4326)), "
			"(NULL, MakePointZ(-161 + 30, -64 + 0, 11, 4326)), "
			"(NULL, MakePointZ(-161 + 0, -64 + 30, 12, 4326)) ",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO point_b_xyz Geometry error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -36;
      }

/* resolving Control Points 2D - 1st order - IsValid */
    sql = "SELECT GCP_IsValid(GCP_Compute(a.geometry, b.geometry, 1)) "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -37;
/* resolving Control Points 2D  - 1st order- AsText */
    sql = "SELECT GCP_AsText(GCP_Compute(a.geometry, b.geometry)) "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id";
    if (!test_query
	(handle, sql,
	 "E{-161.0000000000,1.0000000000,-0.0000000000}, "
	 "N{-64.0000000000,-0.0000000000,1.0000000000}"))
	return -38;

/* resolving Control Points 2D - 1st order - GCP2ATM */
    sql = "SELECT ATM_IsValid(GCP2ATM(GCP_Compute(a.geometry, b.geometry))) "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -39;

/* resolving Control Points 3D - 1st order - IsValid */
    sql = "SELECT GCP_IsValid(GCP_Compute(a.geometry, b.geometry, 1)) "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -40;

/* resolving Control Points 3D - 1st order - AsText */
    sql = "SELECT GCP_AsText(GCP_Compute(a.geometry, b.geometry)) "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id";
    if (!test_query (handle, sql,
		     "E{-161.0000000000,1.0000000000,-0.0000000000,0.0000000000}, "
		     "N{-64.0000000000,0.0000000000,1.0000000000,0.0000000000}, "
		     "Z{10.0000000000,-0.0000000000,0.0000000000,1.0000000000}"))
	return -41;

/* resolving Control Points 3D - 1st order - GCP2ATM */
    sql = "SELECT ATM_IsValid(GCP2ATM(GCP_Compute(a.geometry, b.geometry))) "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -42;

/* resolving Control Points 2D - 2nd order - IsValid */
    sql = "SELECT GCP_IsValid(GCP_Compute(a.geometry, b.geometry, 2)) "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -43;

/* resolving Control Points 2D  - 2nd order- AsText */
    sql = "SELECT GCP_IsValid(GCP_Compute(a.geometry, b.geometry, 2)) "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id";
    if (!test_query	(handle, sql,"1"))
	return -44;

/* resolving Control Points 2D - 2nd order - GCP2ATM */
    sql = "SELECT ATM_IsValid(GCP2ATM(GCP_Compute(a.geometry, b.geometry, 2))) "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "-1"))
	return -45;

/* resolving Control Points 3D - 2nd order - IsValid */
    sql = "SELECT GCP_IsValid(GCP_Compute(a.geometry, b.geometry, 2)) "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -46;

/* resolving Control Points 3D - 2nd order - AsText */
    sql = "SELECT GCP_IsValid(GCP_Compute(a.geometry, b.geometry, 2)) "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -47;

/* resolving Control Points 3D - 2nd order - GCP2ATM */
    sql = "SELECT ATM_IsValid(GCP2ATM(GCP_Compute(a.geometry, b.geometry, 2))) "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "-1"))
	return -48;

/* resolving Control Points 2D - 3nd order - IsValid */
    sql = "SELECT GCP_IsValid(GCP_Compute(a.geometry, b.geometry, 3)) "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -49;

/* resolving Control Points 2D  - 3nd order- AsText */
    sql = "SELECT GCP_IsValid(GCP_Compute(a.geometry, b.geometry, 3)) "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -50;

/* resolving Control Points 2D - 3nd order - GCP2ATM */
    sql = "SELECT ATM_IsValid(GCP2ATM(GCP_Compute(a.geometry, b.geometry, 3))) "
	"FROM point_a_xy AS a, point_b_xy AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "-1"))
	return -51;

/* resolving Control Points 3D - 3nd order - IsValid */
    sql = "SELECT GCP_IsValid(GCP_Compute(a.geometry, b.geometry, 3)) "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -52;

/* resolving Control Points 3D - 3nd order - AsText */
    sql = "SELECT GCP_IsValid(GCP_Compute(a.geometry, b.geometry, 3)) "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "1"))
	return -53;

/* resolving Control Points 3D - 3nd order - GCP2ATM */
    sql = "SELECT ATM_IsValid(GCP2ATM(GCP_Compute(a.geometry, b.geometry, 3))) "
	"FROM point_a_xyz AS a, point_b_xyz AS b WHERE a.id = b.id";
    if (!test_query (handle, sql, "-1"))
	return -54;

    for (order = 0; order <= 3; order++)
      {
	  /* GCP_Trasform */
	  if (!test_transform (handle, order))
	      return -55;
      }

    if (!test_invalid (handle))
	return -56;

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -57;
      }

    spatialite_cleanup_ex (cache);

#endif /* end CGP conditional */

    spatialite_shutdown ();
    return 0;
}
