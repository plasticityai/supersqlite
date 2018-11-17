/*

 check_clone_table.c -- SpatiaLite Test Case
 
 This tests a couple of functions in gg_relations.c that aren't used in
 spatialite, but are provided for backwards compatibility.

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

#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gaiageo.h"

int
execute_check (sqlite3 * sqlite, const char *sql, char **error)
{
/* executing an SQL statement returning True/False */
    sqlite3_stmt *stmt;
    int ret;
    int retcode = 0;

    *error = NULL;
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  *error = sqlite3_mprintf ("%s", sqlite3_errmsg (sqlite));
	  return SQLITE_ERROR;
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
      {
	  if (sqlite3_column_int (stmt, 0) == 1)
	      retcode = 1;
      }
    sqlite3_finalize (stmt);
    if (retcode == 1)
	return SQLITE_OK;
    return SQLITE_ERROR;
}

int
create_origin ()
{
/* creating the origin DB */
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    const char *sql;
    void *cache = spatialite_alloc_connection ();

    ret =
	sqlite3_open_v2 ("clone_origin.sqlite", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open clone_origin database: %s\n",
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

/* creating the input table #1 */
    sql = "CREATE TABLE input_1 (id INTEGER NOT NULL PRIMARY KEY,"
	"name TEXT NOT NULL, measure DOUBLE NOT NULL)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }
/* populating the input table #1 */
    sql = "INSERT INTO input_1 (id, name, measure) VALUES "
	"(1, 'alpha', 1.51), (2, 'beta', 2.34), (3, 'gamma', 3.75), "
	"(4, 'delta', 4.81), (5, 'epsilon', 5.03)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }

/* creating the input table #2 */
    sql = "CREATE TABLE input_2 (first_name TEXT, last_name TEXT, "
	"birthday TEXT, address TEXT, town TEXT DEFAULT 'Arezzo', "
	"code_2 TEXT, code_1 TEXT, number INT, code_3 TEXT, "
	"CONSTRAINT pk_2 PRIMARY KEY (code_1, code_2, code_3))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }
/* populating the input table #2 */
    sql = "INSERT INTO input_2 (first_name, last_name, birthday, "
	" address, town, code_2, code_1, number, code_3) VALUES "
	"('Peter', 'Smith', '1935-05-28', 'Oak Road', 'London', 'uk', 'bb', 1, 'cc'), "
	"('Paul', 'Dupont', '1981-02-13', 'Rue Blanche', 'Paris', 'fr', 'kk', 2, 'zz'), "
	"('Daniel', 'Khun', '1967-11-12', 'Berner Strasse', 'Berlin', 'de', 'mm', 3, 'nn')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }
    sql = "INSERT INTO input_2 (first_name, last_name, address, "
	" code_2, code_1, number, code_3) VALUES "
	"('Mario', 'Rossi', NULL, 'it', 'aa', 1, '01'), "
	"('Lucia', 'Bianchi', NULL, 'it', 'aa', 1, '02'), "
	"('Tina', 'Gialli', 'Via Roma', 'it', 'ab', 1, '01'), "
	"('Paolo', 'Verdi', NULL, 'it', 'ab', 2, '02')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }

/* creating the input table #3 */
    sql = "CREATE TABLE input_3 (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -8;
      }
    sql = "SELECT AddGeometryColumn('input_3', 'geom', 4326, 'POLYGON', 'XY')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }
    sql = "SELECT CreateSpatialIndex('input_3', 'geom')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }
/* populating the input table #3 */
    sql = "INSERT INTO input_3 (id, name, geom) VALUES "
	"(NULL, 'one', BuildMbr(0, 0, 1, 1, 4326)), "
	"(NULL, 'two', BuildMbr(1, 1, 2, 2, 4326)), "
	"(NULL, 'three', BuildMbr(2, 2, 3, 3, 4326)), "
	"(NULL, 'four', BuildMbr(3, 3, 4, 4, 4326)), "
	"(NULL, 'five', BuildMbr(4, 4, 5, 5, 4326)), "
	"(1000, 'six', BuildMbr(5, 5, 6, 6, 4326)), "
	"(NULL, 'seven', BuildMbr(6, 6, 7, 7, 4326)), "
	"(NULL, 'height', BuildMbr(7, 7, 8, 8, 4326)), "
	"(NULL, 'nine', BuildMbr(8, 8, 9, 9, 4326)), "
	"(NULL, 'ten', BuildMbr(9, 9, 10, 10, 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

/* creating the input table #4 */
    sql = "CREATE TABLE input_4 (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT, tbl_1 INTEGER, code_2 TEXT NOT NULL, "
	"code_3 TEXT NOT NULL, code_1 TEXT NOT NULL, tbl_3 INTEGER, "
	"ignore_1 TEXT DEFAULT 'foo', ignore_2 TEXT DEFAULT 'foo', "
	"ignore_3 TEXT DEFAULT 'foo', ignore_4 TEXT DEFAULT 'foo', "
	"ignore_5 TEXT DEFAULT 'foo', ignore_6 TEXT DEFAULT 'foo', "
	"ignore_7 TEXT DEFAULT 'foo', ignore_8 TEXT DEFAULT 'foo', "
	"ignore_9 TEXT DEFAULT 'foo', ignore_10 TEXT DEFAULT 'foo', "
	"CONSTRAINT fk_tbl4_tbl1 FOREIGN KEY (tbl_1) REFERENCES input_1 (id), "
	"CONSTRAINT fk_tbl4_tbl2 FOREIGN KEY (code_1, code_2, code_3) "
	"REFERENCES input_2 (code_1, code_2, code_3) "
	"ON UPDATE CASCADE ON DELETE CASCADE)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }
    sql = "SELECT AddGeometryColumn('input_4', 'geom', 4326, 'POINT', 'XY')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }
/* adding several Indices */
    sql = "CREATE INDEX idx_4_tbl1 ON input_4 (tbl_1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }
    sql = "CREATE INDEX idx_4_tbl2 ON input_4 (code_1, code_2, code_3)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -15;
      }
    sql = "CREATE UNIQUE INDEX idx_4_name ON input_4 (name)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -16;
      }
/* creating Triggers on input table #4 */
    sql = "CREATE TRIGGER find_polygon_ins AFTER INSERT ON input_4 "
	"BEGIN UPDATE input_4 SET tbl_3 = "
	"(SELECT id FROM input_3 WHERE ST_Intersects(geom, NEW.geom) = 1) "
	"WHERE id = NEW.id; END";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -17;
      }
    sql = "CREATE TRIGGER find_polygon_upd AFTER UPDATE OF geom ON input_4 "
	"BEGIN UPDATE input_4 SET tbl_3 = "
	"(SELECT id FROM input_3 WHERE ST_Intersects(geom, NEW.geom) = 1) "
	"WHERE id = NEW.id; END";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -18;
      }

/* populating the input table #4 */
    sql =
	"INSERT INTO input_4 (id, name, tbl_1, code_2, code_3, code_1, geom) VALUES "
	"(NULL, 'alpha', 1, 'it', '01', 'aa', MakePoint(0.5, 0.5, 4326)), "
	"(NULL, 'beta', 1, 'fr', 'zz', 'kk', MakePoint(1.5, 1.5, 4326)), "
	"(NULL, 'gamma', 2, 'uk', 'cc', 'bb', MakePoint(2.5, 2.5, 4326)), "
	"(100, 'delta', 2, 'de', 'nn', 'mm', MakePoint(3.5, 3.5, 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -19;
      }
    sql =
	"INSERT INTO input_4 (id, name, tbl_1, code_2, code_3, code_1, geom) VALUES "
	"(NULL, 'epsilon', 3, 'it', '01', 'ab', MakePoint(4.5, 4.5, 4326)), "
	"(200, 'zeta', 3, 'it', '01', 'ab', MakePoint(5.5, 5.5, 4326)), "
	"(300, 'eta', 4, 'it', '02', 'aa', MakePoint(6.5, 6.5, 4326)), "
	"(NULL, 'iota', 5, 'it', '02', 'ab', MakePoint(7.5, 7.5, 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -20;
      }

/* creating the input table Linestring XY */
    sql = "CREATE TABLE ln_xy (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }
    sql = "SELECT AddGeometryColumn('ln_xy', 'geom', 4326, 'LINESTRING', 'XY')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }
/* populating the input table Linestring XY */
    sql = "INSERT INTO ln_xy (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('LINESTRING(0 0, 1 1)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }

/* creating the input table Linestring XYZ */
    sql = "CREATE TABLE ln_xyz (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }
    sql =
	"SELECT AddGeometryColumn('ln_xyz', 'geom', 4326, 'LINESTRING', 'XYZ')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }
/* populating the input table Linestring XYZ */
    sql = "INSERT INTO ln_xyz (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('LINESTRINGZ(0 0 0, 1 1 1)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -26;
      }

/* creating the input table Linestring XYM */
    sql = "CREATE TABLE ln_xym (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -27;
      }
    sql =
	"SELECT AddGeometryColumn('ln_xym', 'geom', 4326, 'LINESTRING', 'XYM')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -28;
      }
/* populating the input table Linestring XYM */
    sql = "INSERT INTO ln_xym (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('LINESTRINGM(0 0 0, 1 1 1)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }

/* creating the input table Linestring XYZM */
    sql = "CREATE TABLE ln_xyzm (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -27;
      }
    sql =
	"SELECT AddGeometryColumn('ln_xyzm', 'geom', 4326, 'LINESTRING', 'XYZM')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -28;
      }
/* populating the input table Linestring XYZM */
    sql = "INSERT INTO ln_xyzm (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('LINESTRINGZM(0 0 0 0, 1 1 1 1)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }

/* creating the input table Polygon XYZ */
    sql = "CREATE TABLE pg_xyz (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }
    sql = "SELECT AddGeometryColumn('pg_xyz', 'geom', 4326, 'POLYGON', 'XYZ')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -31;
      }
/* populating the input table Linestring XYZ */
    sql = "INSERT INTO pg_xyz (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POLYGONZ(0 0 0, 0 1 1, 1 1 1, 1 0 1, 0 0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -32;
      }

/* creating the input table Polygon XYM */
    sql = "CREATE TABLE pg_xym (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -33;
      }
    sql = "SELECT AddGeometryColumn('pg_xym', 'geom', 4326, 'POLYGON', 'XYM')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -34;
      }
/* populating the input table Linestring XYM */
    sql = "INSERT INTO pg_xym (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POLYGONM(0 0 0, 0 1 1, 1 1 1, 1 0 1, 0 0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -35;
      }

/* creating the input table Polygon XYZM */
    sql = "CREATE TABLE pg_xyzm (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -36;
      }
    sql =
	"SELECT AddGeometryColumn('pg_xyzm', 'geom', 4326, 'POLYGON', 'XYZM')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -37;
      }
/* populating the input table Linestring XYZM */
    sql = "INSERT INTO pg_xyz (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POLYGONZM(0 0 0 0, 0 1 1 1, 1 1 1 1, 1 0 1 1, 0 0 0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -38;
      }

/* creating the input table Point XY */
    sql = "CREATE TABLE pt_xy (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -39;
      }
    sql = "SELECT AddGeometryColumn('pt_xy', 'geom', 4326, 'POINT', 'XY', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -40;
      }
/* populating the input table Point XY */
    sql = "INSERT INTO pt_xy (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POINT(0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -41;
      }

/* creating the input table Point XYZ */
    sql = "CREATE TABLE pt_xyz (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -42;
      }
    sql = "SELECT AddGeometryColumn('pt_xyz', 'geom', 4326, 'POINT', 'XYZ', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -43;
      }
/* populating the input table Point XYZ */
    sql = "INSERT INTO pt_xyz (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POINTZ(0 0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -44;
      }

/* creating the input table Point XYM */
    sql = "CREATE TABLE pt_xym (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -45;
      }
    sql = "SELECT AddGeometryColumn('pt_xym', 'geom', 4326, 'POINT', 'XYM', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -46;
      }
/* populating the input table Point XYM */
    sql = "INSERT INTO pt_xym (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POINTM(0 0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -47;
      }

/* creating the input table Point XYZM */
    sql = "CREATE TABLE pt_xyzm (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -48;
      }
    sql =
	"SELECT AddGeometryColumn('pt_xyzm', 'geom', 4326, 'POINT', 'XYZM', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -49;
      }
/* populating the input table Point XYZM */
    sql = "INSERT INTO pt_xyzm (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POINTZM(0 0 0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -50;
      }

/* creating the input table GeometryCollection XY */
    sql = "CREATE TABLE gc_xy (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -51;
      }
    sql =
	"SELECT AddGeometryColumn('gc_xy', 'geom', 4326, 'GEOMETRYCOLLECTION', 'XY', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -52;
      }
/* populating the input table GeometryCollection XY */
    sql = "INSERT INTO gc_xy (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('GEOMETRYCOLLECTION(POINT(0 0))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -53;
      }

/* creating the input table GeometryCollection XYZ */
    sql = "CREATE TABLE gc_xyz (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -54;
      }
    sql =
	"SELECT AddGeometryColumn('gc_xyz', 'geom', 4326, 'GEOMETRYCOLLECTION', 'XYZ', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -55;
      }
/* populating the input table GeometryCollection XYZ */
    sql = "INSERT INTO gc_xyz (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('GEOMETRYCOLLECTIONZ(POINTZ(0 0 0))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -56;
      }

/* creating the input table GeometryCollection XYM */
    sql = "CREATE TABLE gc_xym (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -57;
      }
    sql =
	"SELECT AddGeometryColumn('gc_xym', 'geom', 4326, 'GEOMETRYCOLLECTION', 'XYM', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -58;
      }
/* populating the input table GeometryCollection XYM */
    sql = "INSERT INTO gc_xym (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('GEOMETRYCOLLECTIONM(POINTM(0 0 0))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -59;
      }

/* creating the input table GeometryCollection XYZM */
    sql = "CREATE TABLE gc_xyzm (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -60;
      }
    sql =
	"SELECT AddGeometryColumn('gc_xyzm', 'geom', 4326, 'GEOMETRYCOLLECTION', 'XYZM', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -61;
      }
/* populating the input table GeometryCollection XYZM */
    sql = "INSERT INTO gc_xyzm (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('GEOMETRYCOLLECTIONZM(POINTZM(0 0 0 0))', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -62;
      }

/* creating the input table Geometry XY */
    sql = "CREATE TABLE geo_xy (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -63;
      }
    sql =
	"SELECT AddGeometryColumn('geo_xy', 'geom', 4326, 'GEOMETRY', 'XY', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -64;
      }
/* populating the input table Geometry XY */
    sql = "INSERT INTO geo_xy (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POINT(0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -65;
      }

/* creating the input table Geometry XYZ */
    sql = "CREATE TABLE geo_xyz (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -66;
      }
    sql =
	"SELECT AddGeometryColumn('geo_xyz', 'geom', 4326, 'GEOMETRY', 'XYZ', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -67;
      }
/* populating the input table Geometry XYZ */
    sql = "INSERT INTO geo_xyz (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POINTZ(0 0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -68;
      }

/* creating the input table Geometry XYM */
    sql = "CREATE TABLE geo_xym (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -69;
      }
    sql =
	"SELECT AddGeometryColumn('geo_xym', 'geom', 4326, 'GEOMETRY', 'XYM', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -70;
      }
/* populating the input table Geometry XYM */
    sql = "INSERT INTO geo_xym (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POINTM(0 0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -71;
      }

/* creating the input table Geometry XYZM */
    sql = "CREATE TABLE geo_xyzm (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -72;
      }
    sql =
	"SELECT AddGeometryColumn('geo_xyzm', 'geom', 4326, 'GEOMETRY', 'XYZM', 1)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -73;
      }
/* populating the input table Geometry XYZM */
    sql = "INSERT INTO geo_xyzm (id, name, geom) VALUES "
	"(NULL, 'one', GeomFromText('POINTZM(0 0 0 0)', 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -74;
      }

/* creating the input table #5 (append) */
    sql = "CREATE TABLE input_5 (id INTEGER PRIMARY KEY AUTOINCREMENT,"
	"name TEXT, code_2 TEXT NOT NULL, "
	"code_3 TEXT NOT NULL, code_1 TEXT NOT NULL, tbl_3 INTEGER, "
	"ignore_1 TEXT DEFAULT 'foo', ignore_2 TEXT DEFAULT 'foo', "
	"ignore_3 TEXT DEFAULT 'foo', ignore_4 TEXT DEFAULT 'foo', "
	"extra_1 TEXT DEFAULT 'extra_foo', extra_2 TEXT DEFAULT 'extra_foo', "
	"extra_3 TEXT DEFAULT 'extra_foo', extra_4 TEXT DEFAULT 'extra_foo', "
	"extra_5 TEXT DEFAULT 'extra_foo', extra_6 TEXT DEFAULT 'extra_foo', "
	"extra_7 TEXT DEFAULT 'extra_foo', extra_8 TEXT DEFAULT 'extra_foo', "
	"extra_9 TEXT DEFAULT 'extra_foo', extra_10 TEXT DEFAULT 'extra_foo')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -75;
      }
    sql = "SELECT AddGeometryColumn('input_5', 'geom', 4326, 'POINT', 'XY')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -76;
      }
    sql =
	"SELECT AddGeometryColumn('input_5', 'extra_geom', 4326, 'LINESTRING', 'XY')";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -77;
      }

/* populating the input table #5 */
    sql =
	"INSERT INTO input_5 (id, name, code_2, code_3, code_1, geom, extra_geom) VALUES "
	"(NULL, 'phi', 'it', '01', 'aa', MakePoint(0.5, 0.5, 4326), "
	"MakeLine(MakePoint(10, 10, 4326), MakePoint(11, 11, 4326))), "
	"(NULL, 'chi', 'fr', 'zz', 'kk', MakePoint(1.5, 1.5, 4326), "
	"MakeLine(MakePoint(11, 11, 4326), MakePoint(12, 12, 4326))), "
	"(NULL, 'psi', 'uk', 'cc', 'bb', MakePoint(2.5, 2.5, 4326), "
	"MakeLine(MakePoint(12, 12, 4326), MakePoint(13, 13, 4326))), "
	"(100, 'omega', 'de', 'nn', 'mm', MakePoint(3.5, 3.5, 4326), "
	"MakeLine(MakePoint(13, 13, 4326), MakePoint(14, 14, 4326)))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -78;
      }

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -79;
      }

    spatialite_cleanup_ex (cache);

    return 0;
}

int
test_clone_table (int base, int with_extra, int ignore, int resequence,
		  int cast2multi)
{
/* performing a CloneTable testcase */
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    const char *sql;
    int retcode = 0;
    void *cache = spatialite_alloc_connection ();

    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory database: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  retcode = -1;
	  goto end;
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
	  retcode = -2;
	  goto end;
      }

/* attaching the origin DB */
    sql = "ATTACH DATABASE \"./clone_origin.sqlite\" AS \"input\"";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }

/* cloning input_1 */
    if (with_extra)
	sql =
	    "SELECT CloneTable('input', 'input_1', 'input_1', 1, '::with-foreign-keys::', '::with-triggers::')";
    else
	sql = "SELECT CloneTable('input', 'input_1', 'input_1', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }

/* cloning input_2 */
    if (with_extra)
	sql =
	    "SELECT CloneTable('input', 'input_2', 'input_2', 1, '::with-foreign-keys::', '::with-triggers::')";
    else
	sql = "SELECT CloneTable('input', 'input_2', 'input_2', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }

/* cloning input_3 */
    if (with_extra)
	sql =
	    "SELECT CloneTable('input', 'input_3', 'input_3', 1, '::with-foreign-keys::', '::with-triggers::')";
    else if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'input_3', 'input_3', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'input_3', 'input_3', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }

/* cloning input_4 */
    if (with_extra)
	sql =
	    "SELECT CloneTable('input', 'input_4', 'input_4', 1, '::with-foreign-keys::', '::with-triggers::')";
    else if (ignore)
	sql =
	    "SELECT CloneTable('input', 'input_4', 'input_4', 1, '::ignore::ignore_1', '::ignore::ignore_2', "
	    "'::ignore::ignore_3', '::ignore::ignore_4', '::ignore::ignore_5', '::ignore::ignore_6', "
	    "'::ignore::ignore_7', '::ignore::ignore_8', '::ignore::ignore_9', '::ignore::ignore_10')";
    else if (resequence)
	sql =
	    "SELECT CloneTable('input', 'input_4', 'input_4', 1, '::resequence::')";
    else
	sql = "SELECT CloneTable('input', 'input_4', 'input_4', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }

/* cloning Linestring XY */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'ln_xy', 'ln_xy', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'ln_xy', 'ln_xy', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -8;
      }

/* cloning Linestring XYZ */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'ln_xyz', 'ln_xyz', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'ln_xyz', 'ln_xyz', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }

/* cloning Linestring XYM */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'ln_xym', 'ln_xym', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'ln_xym', 'ln_xym', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }

/* cloning Linestring XYZM */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'ln_xyzm', 'ln_xyzm', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'ln_xyzm', 'ln_xyzm', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

/* cloning Polygon XYZ */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'pg_xyz', 'pg_xyz', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'pg_xyz', 'pg_xyz', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }

/* cloning Polygon XYM */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'pg_xym', 'pg_xym', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'pg_xym', 'pg_xym', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }

/* cloning Polygon XYZM */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'pg_xyzm', 'pg_xyzm', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'pg_xyzm', 'pg_xyzm', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }

/* cloning Point XY */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'pt_xy', 'pt_xy', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'pt_xy', 'pt_xy', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -15;
      }

/* cloning Point XYZ */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'pt_xyz', 'pt_xyz', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'pt_xyz', 'pt_xyz', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -16;
      }

/* cloning Point XYM */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'pt_xym', 'pt_xym', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'pt_xym', 'pt_xym', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -17;
      }

/* cloning Point XYZM */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'pt_xyzm', 'pt_xyzm', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'pt_xyzm', 'pt_xyzm', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -18;
      }

/* cloning GeometryCollection XY */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'gc_xy', 'gc_xy', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'gc_xy', 'gc_xy', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -19;
      }

/* cloning GeometryCollection XYZ */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'gc_xyz', 'gc_xyz', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'gc_xyz', 'gc_xyz', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -20;
      }

/* cloning GeometryCollection XYM */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'gc_xym', 'gc_xym', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'gc_xym', 'gc_xym', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }

/* cloning GeometryCollection XYZM */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'gc_xyzm', 'gc_xyzm', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'gc_xyzm', 'gc_xyzm', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }

/* cloning Geometry XY */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'geo_xy', 'geo_xy', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'geo_xy', 'geo_xy', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }

/* cloning Geometry XYZ */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'geo_xyz', 'geo_xyz', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'geo_xyz', 'geo_xyz', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }

/* cloning Geometry XYM */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'geo_xym', 'geo_xym', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'geo_xym', 'geo_xym', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }

/* cloning Geometry XYZM */
    if (cast2multi)
	sql =
	    "SELECT CloneTable('input', 'geo_xyzm', 'geo_xyzm', 1, '::cast2multi::geom')";
    else
	sql = "SELECT CloneTable('input', 'geo_xyzm', 'geo_xyzm', 1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }

/* cloning input_5 (APPEND) */
    if (ignore)
	sql =
	    "SELECT CloneTable('input', 'input_5', 'input_4', 1, '::ignore::ignore_1', '::ignore::ignore_2', "
	    "'::ignore::ignore_3', '::ignore::ignore_4', '::resequence::', '::append::', "
	    "'::ignore::extra_7', '::ignore::extra_8', '::ignore::extra_9', '::ignore::extra_10')";
    else
	sql =
	    "SELECT CloneTable('input', 'input_5', 'input_4', 1, '::resequence::', '::append::')";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }

/* detaching the origin DB */
    sql = "DETACH DATABASE \"input\"";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  retcode = -9;
	  goto end;
      }

    spatialite_cleanup_ex (cache);

  end:
    if (retcode < 0)
	return base + retcode;
    return 0;
}


int
main (int argc, char *argv[])
{
    int ret;
    int retcode = 0;

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    if (create_origin () < 0)
      {
	  retcode = -66;
	  goto end;
      }

/* base test: no options */
    ret = test_clone_table (-100, 0, 0, 0, 0);
    if (ret < 0)
      {
	  retcode = ret;
	  goto end;
      }
/* advanced test: with-foreign-keys and with-triggers */
    ret = test_clone_table (-200, 1, 0, 0, 0);
    if (ret < 0)
      {
	  retcode = ret;
	  goto end;
      }
/* advanced test: ignore */
    ret = test_clone_table (-300, 0, 1, 0, 0);
    if (ret < 0)
      {
	  retcode = ret;
	  goto end;
      }
/* advanced test: resequence */
    ret = test_clone_table (-400, 0, 0, 1, 0);
    if (ret < 0)
      {
	  retcode = ret;
	  goto end;
      }
/* advanced test: cast2multi */
    ret = test_clone_table (-400, 0, 0, 0, 1);
    if (ret < 0)
      {
	  retcode = ret;
	  goto end;
      }

  end:
/* removing the origin DB */
    unlink ("clone_origin.sqlite");
    spatialite_shutdown ();
    return retcode;
}
