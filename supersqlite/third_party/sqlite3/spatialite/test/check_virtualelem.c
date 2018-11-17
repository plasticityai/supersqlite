/*

 check_virtualelem.c -- SpatiaLite Test Case

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
 
Portions created by the Initial Developer are Copyright (C) 2013
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
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

static int
create_table (sqlite3 * sqlite, const char *table)
{
/* creating a test table */
    int ret;
    char *err_msg = NULL;
    char *sql;

    sql =
	sqlite3_mprintf
	("CREATE TABLE %s (id INTEGER PRIMARY KEY AUTOINCREMENT, "
	 "name TEXT NOT NULL)", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE \"%s\" error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

    return 1;
}

static int
test_table (sqlite3 * sqlite, const char *prefix, const char *table,
	    const char *column)
{
/* testing a resultset */
    int ret;
    char *sql;
    sqlite3_stmt *stmt = NULL;
    int row_no = 0;

    if (prefix == NULL && column == NULL)
	sql = sqlite3_mprintf ("SELECT t.id, t.name, Count(e.geometry) "
			       "FROM %s AS t JOIN ElementaryGeometries AS e ON "
			       "(e.f_table_name = %Q AND e.origin_rowid = t.ROWID) "
			       "GROUP BY t.id", table, table);
    else if (prefix != NULL && column != NULL)
	sql = sqlite3_mprintf ("SELECT t.id, t.name, Count(e.geometry) "
			       "FROM %s AS t JOIN ElementaryGeometries AS e ON "
			       "(e.db_prefix = %Q AND e.f_table_name = %Q AND "
			       "e.f_geometry_column = %Q AND e.origin_rowid = t.ROWID) "
			       "GROUP BY t.id", table, prefix, table, column);
    else if (prefix != NULL)
	sql = sqlite3_mprintf ("SELECT t.id, t.name, Count(e.geometry) "
			       "FROM %s AS t JOIN ElementaryGeometries AS e ON "
			       "(e.db_prefix = %Q AND e.f_table_name = %Q "
			       "AND e.origin_rowid = t.ROWID) "
			       "GROUP BY t.id", table, prefix, table);
    else
	sql = sqlite3_mprintf ("SELECT t.id, t.name, Count(e.geometry) "
			       "FROM %s AS t JOIN ElementaryGeometries AS e ON "
			       "(e.f_table_name = %Q AND e.f_geometry_column = %Q "
			       "AND e.origin_rowid = t.ROWID) "
			       "GROUP BY t.id", table, table, column);
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "SELECT FROM \"%s\": \"%s\"\n", table,
		   sqlite3_errmsg (sqlite));
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
		int id = sqlite3_column_int (stmt, 0);
		const char *name = (const char *) sqlite3_column_text (stmt, 1);
		int count = sqlite3_column_int (stmt, 2);
		switch (row_no)
		  {
		  case 0:
		      if (id == 1 && strcmp (name, "alpha") == 0 && count == 1)
			  ;
		      else
			{
			    fprintf (stderr,
				     "Unexpected result: table \"%s\" row=%d) [%d %s %d]\n",
				     table, row_no, id, name, count);
			    goto error;
			}
		      break;
		  case 1:
		      if (id == 2 && strcmp (name, "beta") == 0 && count == 2)
			  ;
		      else
			{
			    fprintf (stderr,
				     "Unexpected result: table \"%s\" row=%d) [%d %s %d]\n",
				     table, row_no, id, name, count);
			    goto error;
			}
		      break;
		  case 2:
		      if (id == 3 && strcmp (name, "gamma") == 0 && count == 3)
			  ;
		      else
			{
			    fprintf (stderr,
				     "Unexpected result: table \"%s\" row=%d) [%d %s %d]\n",
				     table, row_no, id, name, count);
			    goto error;
			}
		      break;
		  default:
		      fprintf (stderr, "Unexpected row %d (table \"%s\")\n",
			       row_no, table);
		      goto error;
		  };
	    }
	  else
	      goto error;
	  row_no++;
      }
    sqlite3_finalize (stmt);
    return 1;

  error:
    if (stmt != NULL)
	sqlite3_finalize (stmt);
    return 0;
}

static int
test_point_xy (sqlite3 * sqlite)
{
/* testing MULTIPOINT XY */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mpointxy";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTIPOINT', 'XY')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTIPOINT(1 1)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTIPOINT(1 1, 2 2)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTIPOINT(1 1, 2 2, 3 3)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, NULL, table, NULL);
    if (!ret)
	return 0;
    return 1;
}

static int
test_point_xyz (sqlite3 * sqlite)
{
/* testing MULTIPOINT XYZ */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mpointxyz";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTIPOINT', 'XYZ')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTIPOINTZ(1 1 1)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTIPOINTZ(1 1 1, 2 2 2)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTIPOINTZ(1 1 1, 2 2 2, 3 3 3)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, "main", table, NULL);
    if (!ret)
	return 0;
    return 1;
}

static int
test_point_xym (sqlite3 * sqlite)
{
/* testing MULTIPOINT XYM */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mpointxym";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTIPOINT', 'XYM')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTIPOINTM(1 1 1)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTIPOINTM(1 1 1, 2 2 2)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTIPOINTM(1 1 1, 2 2 2, 3 3 3)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, "main", table, "geom");
    if (!ret)
	return 0;
    return 1;
}

static int
test_point_xyzm (sqlite3 * sqlite)
{
/* testing MULTIPOINT XYZM */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mpointxyzm";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTIPOINT', 'XYZM')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTIPOINTZM(1 1 1 1)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTIPOINTZM(1 1 1 1, 2 2 2 2)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTIPOINTZM(1 1 1 1, 2 2 2 2, 3 3 3 3)', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, NULL, table, "geom");
    if (!ret)
	return 0;
    return 1;
}

static int
test_linestring_xy (sqlite3 * sqlite)
{
/* testing MULTILINESTRING XY */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mlinestringxy";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTILINESTRING', 'XY')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTILINESTRING((1 1, 2 2))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTILINESTRING((1 1, 2 2), "
			   "(3 3, 4 4))', 4326))", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTILINESTRING((1 1, 2 2), "
			   "(3 3, 4 4), (5 5, 6 6))', 4326))", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, NULL, table, NULL);
    if (!ret)
	return 0;
    return 1;
}

static int
test_linestring_xyz (sqlite3 * sqlite)
{
/* testing MULTILINESTRING XYZ */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mlinestringxyz";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTILINESTRING', 'XYZ')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTILINESTRINGZ((1 1 1, 2 2 2))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTILINESTRINGZ((1 1 1, 2 2 2), "
			   "(3 3 3, 4 4 4))', 4326))", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTILINESTRINGZ((1 1 1, 2 2 2), "
			   "(3 3 3, 4 4 4), (5 5 5, 6 6 6))', 4326))", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, "main", table, "geom");
    if (!ret)
	return 0;
    return 1;
}

static int
test_linestring_xym (sqlite3 * sqlite)
{
/* testing MULTILINESTRING XYM */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mlinestringxym";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTILINESTRING', 'XYM')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTILINESTRINGM((1 1 1, 2 2 2))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTILINESTRINGM((1 1 1, 2 2 2), "
			   "(3 3 3, 4 4 4))', 4326))", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTILINESTRINGM((1 1 1, 2 2 2), "
			   "(3 3 3, 4 4 4), (5 5 5, 6 6 6))', 4326))", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, "main", table, NULL);
    if (!ret)
	return 0;
    return 1;
}

static int
test_linestring_xyzm (sqlite3 * sqlite)
{
/* testing MULTILINESTRING XYZ */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mlinestringxyzm";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTILINESTRING', 'XYZM')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTILINESTRINGZM((1 1 1 1, 2 2 2 2))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTILINESTRINGZM((1 1 1 1, 2 2 2 2), "
			   "(3 3 3 3, 4 4 4 4))', 4326))", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTILINESTRINGZM((1 1 1 1, 2 2 2 2), "
			   "(3 3 3 3, 4 4 4 4), (5 5 5 5, 6 6 6 6))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, NULL, table, "geom");
    if (!ret)
	return 0;
    return 1;
}

static int
test_polygon_xy (sqlite3 * sqlite)
{
/* testing MULTIPOLYGON XY */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mpolygonxy";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTIPOLYGON', 'XY')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTIPOLYGON("
			   "((1 1, 2 1, 2 2, 1 2, 1 1)))', 4326))", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTIPOLYGON("
			   "((1 1, 2 1, 2 2, 1 2, 1 1)), "
			   "((10 10, 11 10, 11 11, 10 11, 10 10)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTIPOLYGON("
			   "((1 1, 2 1, 2 2, 1 2, 1 1)), "
			   "((10 10, 11 10, 11 11, 10 11, 10 10)), "
			   "((20 20, 31 20, 31 31, 20 31, 20 20), "
			   "(25 25, 25 26, 26 26, 26 25, 25 25)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, NULL, table, NULL);
    if (!ret)
	return 0;
    return 1;
}

static int
test_polygon_xyz (sqlite3 * sqlite)
{
/* testing MULTIPOLYGON XYZ */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mpolygonxyz";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTIPOLYGON', 'XYZ')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTIPOLYGONZ("
			   "((1 1 1, 2 1 1, 2 2 2, 1 2 2, 1 1 1)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTIPOLYGONZ("
			   "((1 1 1, 2 1 1, 2 2 2, 1 2 2, 1 1 1)), "
			   "((10 10 10, 11 10 10, 11 11 11, 10 11 11, 10 10 10)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTIPOLYGONZ("
			   "((1 1 1, 2 1 1, 2 2 2, 1 2 2, 1 1 1)), "
			   "((10 10 10, 11 10 10, 11 11 11, 10 11 11, 10 10 10)), "
			   "((20 20 20, 31 20 20, 31 31 31, 20 31 31, 20 20 20), "
			   "(25 25 25, 25 26 26, 26 26 26, 26 25 25, 25 25 25)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, "main", table, "geom");
    if (!ret)
	return 0;
    return 1;
}

static int
test_polygon_xym (sqlite3 * sqlite)
{
/* testing MULTIPOLYGON XYM */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mpolygonxym";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTIPOLYGON', 'XYM')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTIPOLYGONM("
			   "((1 1 1, 2 1 1, 2 2 2, 1 2 2, 1 1 1)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTIPOLYGONM("
			   "((1 1 1, 2 1 1, 2 2 2, 1 2 2, 1 1 1)), "
			   "((10 10 10, 11 10 10, 11 11 11, 10 11 11, 10 10 10)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTIPOLYGONM("
			   "((1 1 1, 2 1 1, 2 2 2, 1 2 2, 1 1 1)), "
			   "((10 10 10, 11 10 10, 11 11 11, 10 11 11, 10 10 10)), "
			   "((20 20 20, 31 20 20, 31 31 31, 20 31 31, 20 20 20), "
			   "(25 25 25, 25 26 26, 26 26 26, 26 25 25, 25 25 25)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, "main", table, NULL);
    if (!ret)
	return 0;
    return 1;
}

static int
test_polygon_xyzm (sqlite3 * sqlite)
{
/* testing MULTIPOLYGON XYZM */
    int ret;
    char *err_msg = NULL;
    char *sql;
    const char *table = "mpolygonxyzm";

/* creating the base Table */
    ret = create_table (sqlite, table);
    if (!ret)
	return 0;
/* adding the Geometry */
    sql = sqlite3_mprintf ("SELECT AddGeometryColumn(%Q, 'geom', 4326, "
			   "'MULTIPOLYGON', 'XYZM')", table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn \"%s\" error: %s\n", table,
		   err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
/* populating the test table */
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'alpha', ST_GeomFromText('MULTIPOLYGONZM("
			   "((1 1 1 1, 2 1 1 1, 2 2 2 2, 1 2 2 2, 1 1 1 1)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #1 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'beta', ST_GeomFromText('MULTIPOLYGONZM("
			   "((1 1 1 1, 2 1 1 1, 2 2 2 2, 1 2 2 2, 1 1 1 1)), "
			   "((10 10 10 10, 11 10 10 10, 11 11 11 11, 10 11 11 11, 10 10 10 10)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #2 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    sql = sqlite3_mprintf ("INSERT INTO %s (id, name, geom) VALUES "
			   "(NULL, 'gamma', ST_GeomFromText('MULTIPOLYGONZM("
			   "((1 1 1 1, 2 1 1 1, 2 2 2 2, 1 2 2 2, 1 1 1 1)), "
			   "((10 10 10 10, 11 10 10 10, 11 11 11 11, 10 11 11 11, 10 10 10 10)), "
			   "((20 20 20 20, 31 20 20 20, 31 31 31 31, 20 31 31 31, 20 20 20 20), "
			   "(25 25 25 25, 25 26 26 26, 26 26 26 26, 26 25 25 25, 25 25 25 25)))', 4326))",
			   table);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" #3 error: %s\n", table, err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

/* testing the table */
    ret = test_table (sqlite, NULL, table, "geom");
    if (!ret)
	return 0;
    return 1;
}

int
main (int argc, char *argv[])
{
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  db_handle = NULL;
	  return -1;
      }

    spatialite_init_ex (db_handle, cache, 0);

    ret =
	sqlite3_exec (db_handle, "SELECT InitSpatialMetadata(1)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "InitSpatialMetadata() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -2;
      }

/* Testing MULTIPOINTs */
    ret = test_point_xy (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -3;
      }
    ret = test_point_xyz (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -4;
      }
    ret = test_point_xym (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -5;
      }
    ret = test_point_xyzm (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -6;
      }

/* Testing MULTILINESTRINGs */
    ret = test_linestring_xy (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -7;
      }
    ret = test_linestring_xyz (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -8;
      }
    ret = test_linestring_xym (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -9;
      }
    ret = test_linestring_xyzm (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -10;
      }

/* Testing MULTIPOLYGONs */
    ret = test_polygon_xy (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -11;
      }
    ret = test_polygon_xyz (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -12;
      }
    ret = test_polygon_xym (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -13;
      }
    ret = test_polygon_xyzm (db_handle);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -14;
      }

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
