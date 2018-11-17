/*

 check_gpkgVirtual.c - Test case for GeoPackage Extensions

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

The Original Code is GeoPackage extensions

The Initial Developer of the Original Code is Sandro Furieri
 
Portions created by the Initial Developer are Copyright (C) 2014
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#ifdef ENABLE_GEOPACKAGE	/* only if GEOPACKAGE is enabled */

#include "sqlite3.h"
#include "spatialite.h"

static void
do_unlink_all ()
{
/* deleting all temporary files */
    unlink ("./copy-gpkg_test.gpkg");
}

static int
test_table (sqlite3 * handle, const char *table)
{
/* testing a 'raw' GPKG table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt = NULL;

    sql = sqlite3_mprintf ("SELECT geom, IsValidGPB(geom), ST_Srid(geom), "
			   "ST_MinX(geom), ST_MaxX(geom), ST_MinY(geom), ST_MaxY(geom), "
			   "ST_Is3D(geom), ST_IsMeasured(geom), ST_MinZ(geom), ST_MaxZ(geom), "
			   "ST_MinM(geom), ST_MaxM(geom), ST_GeometryType(geom) FROM %s",
			   table);

    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free(sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "SELECT FROM \"%s\" error: %s\n", table,
		   sqlite3_errmsg (handle));
	  goto stop;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		int is_null = 0;
		int has_z;
		int has_m;
		if (sqlite3_column_type (stmt, 0) == SQLITE_NULL)
		    is_null = 1;
		if (sqlite3_column_type (stmt, 1) != SQLITE_INTEGER)
		  {
		      fprintf (stderr,
			       "Unexpected ST_IsValidGPB: not an Integer\n");
		      goto stop;
		  }
		if (is_null)
		  {
		      if (sqlite3_column_int (stmt, 1) != 0)
			{
			    fprintf (stderr,
				     "Unexpected result (NULL) ST_IsValidGPB=%d\n",
				     sqlite3_column_int (stmt, 1));
			    goto stop;
			}
		      continue;
		  }
		else
		  {
		      if (sqlite3_column_int (stmt, 1) != 1)
			{
			    fprintf (stderr,
				     "Unexpected result ST_IsValidGPB=%d\n",
				     sqlite3_column_int (stmt, 1));
			    goto stop;
			}
		  }
		if (sqlite3_column_type (stmt, 2) != SQLITE_INTEGER)
		  {
		      fprintf (stderr, "Unexpected ST_SRID: not an Integer\n");
		      goto stop;
		  }
		if (sqlite3_column_int (stmt, 2) != 4326)
		  {
		      fprintf (stderr, "Unexpected result ST_SRID=%d\n",
			       sqlite3_column_int (stmt, 2));
		      goto stop;
		  }
		if (sqlite3_column_type (stmt, 3) != SQLITE_FLOAT)
		  {
		      fprintf (stderr, "Unexpected ST_MinX: not a Double\n");
		      goto stop;
		  }
		if (sqlite3_column_type (stmt, 4) != SQLITE_FLOAT)
		  {
		      fprintf (stderr, "Unexpected ST_MaxX: not a Double\n");
		      goto stop;
		  }
		if (sqlite3_column_type (stmt, 5) != SQLITE_FLOAT)
		  {
		      fprintf (stderr, "Unexpected ST_MinY: not a Double\n");
		      goto stop;
		  }
		if (sqlite3_column_type (stmt, 6) != SQLITE_FLOAT)
		  {
		      fprintf (stderr, "Unexpected ST_MaxY: not a Double\n");
		      goto stop;
		  }
		if (sqlite3_column_type (stmt, 7) != SQLITE_INTEGER)
		  {
		      fprintf (stderr, "Unexpected ST_Is3D: not an Integer\n");
		      goto stop;
		  }
		has_z = sqlite3_column_int (stmt, 7);
		if (sqlite3_column_type (stmt, 8) != SQLITE_INTEGER)
		  {
		      fprintf (stderr,
			       "Unexpected ST_IsMeasured: not an Integer\n");
		      goto stop;
		  }
		has_m = sqlite3_column_int (stmt, 8);
		if (has_z)
		  {
		      if (sqlite3_column_type (stmt, 9) != SQLITE_FLOAT)
			{
			    fprintf (stderr,
				     "Unexpected ST_MinZ: not a Double\n");
			    goto stop;
			}
		      if (sqlite3_column_type (stmt, 10) != SQLITE_FLOAT)
			{
			    fprintf (stderr,
				     "Unexpected ST_MaxZ: not a Double\n");
			    goto stop;
			}
		  }
		else
		  {
		      if (sqlite3_column_type (stmt, 9) != SQLITE_NULL)
			{
			    fprintf (stderr, "Unexpected ST_MinZ: not NULL\n");
			    goto stop;
			}
		      if (sqlite3_column_type (stmt, 10) != SQLITE_NULL)
			{
			    fprintf (stderr, "Unexpected ST_MaxZ: not NULL\n");
			    goto stop;
			}
		  }
		if (has_m)
		  {
		      if (sqlite3_column_type (stmt, 11) != SQLITE_FLOAT)
			{
			    fprintf (stderr,
				     "Unexpected ST_MinM: not a Double\n");
			    goto stop;
			}
		      if (sqlite3_column_type (stmt, 12) != SQLITE_FLOAT)
			{
			    fprintf (stderr,
				     "Unexpected ST_MaxM: not a Double\n");
			    goto stop;
			}
		  }
		else
		  {
		      if (sqlite3_column_type (stmt, 11) != SQLITE_NULL)
			{
			    fprintf (stderr, "Unexpected ST_MinM: not NULL\n");
			    goto stop;
			}
		      if (sqlite3_column_type (stmt, 12) != SQLITE_NULL)
			{
			    fprintf (stderr, "Unexpected ST_MaxM: not NULL\n");
			    goto stop;
			}
		  }
		if (sqlite3_column_type (stmt, 13) != SQLITE_TEXT)
		  {
		      fprintf (stderr,
			       "Unexpected ST_GeometryType: not TEXT\n");
		      goto stop;
		  }
	    }
	  else
	    {
		/* an unexpected error occurred */
		fprintf (stderr, "Error while querying from \"%s\": %s\n",
			 table, sqlite3_errmsg (handle));
		goto stop;
	    }
      }
    sqlite3_finalize (stmt);
    return 1;

  stop:
    if (stmt != NULL)
	sqlite3_finalize (stmt);
    return 0;
}

static int
test_vtable (sqlite3 * handle, const char *table, int mode)
{
/* testing a VirtualGPKG table */
    char *sql;
    int ret;
    sqlite3_stmt *stmt = NULL;

    if (mode)
      {
	  sql = sqlite3_mprintf ("SELECT ROWID, first_name, "
				 "last_name, value1, value2 FROM vgpkg_%s",
				 table);
      }
    else
      {
	  sql = sqlite3_mprintf ("SELECT geom, ST_AsText(geom), id, name "
				 "FROM vgpkg_%s", table);
      }

    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "SELECT FROM \"vgpkg_%s\" error: %s\n", table,
		   sqlite3_errmsg (handle));
	  goto stop;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		if (mode)
		  {
		      if (sqlite3_column_type (stmt, 0) != SQLITE_INTEGER)
			{
			    fprintf (stderr,
				     "Unexpected ROWID: not an INTEGER\n");
			    goto stop;
			}
		      if (sqlite3_column_type (stmt, 1) != SQLITE_TEXT)
			{
			    fprintf (stderr,
				     "Unexpected \"first_name\": not a TEXT string\n");
			    goto stop;
			}
		      if (sqlite3_column_type (stmt, 2) != SQLITE_TEXT)
			{
			    fprintf (stderr,
				     "Unexpected \"first_name\": not a TEXT string\n");
			    goto stop;
			}
		      if (sqlite3_column_type (stmt, 3) != SQLITE_FLOAT)
			{
			    fprintf (stderr,
				     "Unexpected \"first_name\": not a DOUBLE\n");
			    goto stop;
			}
		      if (sqlite3_column_type (stmt, 4) != SQLITE_FLOAT)
			{
			    fprintf (stderr,
				     "Unexpected \"first_name\": not a DOUBLE\n");
			    goto stop;
			}
		  }
		else
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_NULL)
			  continue;
		      if (sqlite3_column_type (stmt, 1) != SQLITE_TEXT)
			{
			    fprintf (stderr,
				     "Unexpected ST_AsText: not a TEXT string\n");
			    goto stop;
			}
		      if (sqlite3_column_type (stmt, 2) != SQLITE_INTEGER)
			{
			    fprintf (stderr,
				     "Unexpected \"id\": not an INTEGER\n");
			    goto stop;
			}
		      if (sqlite3_column_type (stmt, 3) != SQLITE_TEXT)
			{
			    fprintf (stderr,
				     "Unexpected \"name\": not a TEXT string\n");
			    goto stop;
			}
		  }
	    }
	  else
	    {
		/* an unexpected error occurred */
		fprintf (stderr, "Error while querying from \"vgpkg_%s\": %s\n",
			 table, sqlite3_errmsg (handle));
		goto stop;
	    }
      }
    sqlite3_finalize (stmt);
    return 1;

  stop:
    if (stmt != NULL)
	sqlite3_finalize (stmt);
    return 0;
}

static int
test_vtable_out (sqlite3 * handle)
{
/* testing VirtualGPKG insert/update/delete */
    const char *sql;
    int ret;
    char *sql_err = NULL;

    sql = "BEGIN";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "BEGIN error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    sql =
	"INSERT INTO vgpkg_test_pk (first_name, last_name, value1, value2, geom) "
	"VALUES ('z1', 'charlie', 1, 2, MakePoint(3, 3, 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT #1 error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    sql =
	"INSERT INTO vgpkg_test_pk (first_name, last_name, value1, value2, geom) "
	"VALUES ('z2', 'annie', 1.1, 2.2, MakePoint(4, 4, 4326))";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT #2 error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    sql = "INSERT INTO vgpkg_test_pk (first_name, last_name, value1, value2) "
	"VALUES ('z3', 'peter', 2.2, 3.3)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT #3 error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    sql = "UPDATE vgpkg_test_pk SET geom = MakePoint(14, 14, 4326) "
	"WHERE first_name = 'z1'";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UPDATE #1 error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    sql = "UPDATE vgpkg_test_pk SET value1 = 13.4 " "WHERE ROWID = 3";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UPDATE #2 error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    sql = "UPDATE vgpkg_test_pk SET geom = NULL, value2 = 51 "
	"WHERE ROWID = 2";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UPDATE #3 error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    sql = "DELETE FROM vgpkg_test_pk WHERE ROWID = 1";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DELETE #1 error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    sql = "DELETE FROM vgpkg_test_pk WHERE first_name > 'z'";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DELETE #2 error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    sql = "ROLLBACK";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "ROLLBACK error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    return 1;
}

int
main (int argc, char *argv[])
{
    sqlite3 *db_handle;
    int ret;
    const char *sql;
    void *cache = NULL;
    char *sql_err = NULL;
    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    do_unlink_all ();

/* directly testing GPKG */
    ret = system ("cp ./gpkg_test.gpkg copy-gpkg_test.gpkg");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot copy gpkg_test.gpkg database\n");
	  return -1;
      }

    cache = spatialite_alloc_connection ();
    ret =
	sqlite3_open_v2 ("./copy-gpkg_test.gpkg", &db_handle,
			 SQLITE_OPEN_READWRITE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open '%s': %s\n", "copy-gpkg_test.gpkg",
		   sqlite3_errmsg (db_handle));
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }
    spatialite_init_ex (db_handle, cache, 0);

    if (!test_table (db_handle, "pt2d"))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_table (db_handle, "ln3dz"))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_table (db_handle, "pg2dm"))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_table (db_handle, "mpt3dzm"))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_table (db_handle, "mln2dm"))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_table (db_handle, "mpg3dz"))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_table (db_handle, "gc3dz"))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

/* activating Auto GPKG Wrapping */
    sql = "SELECT AutoGPKGStart()";
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AutoGPKGStart error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

/* testing the Virtual Tables */
    if (!test_vtable (db_handle, "pt2d", 0))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_vtable (db_handle, "ln3dz", 0))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_vtable (db_handle, "pg2dm", 0))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_vtable (db_handle, "mpt3dzm", 0))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_vtable (db_handle, "mln2dm", 0))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_vtable (db_handle, "mpg3dz", 0))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_vtable (db_handle, "gc3dz", 0))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_vtable (db_handle, "test_pk", 1))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    if (!test_vtable_out (db_handle))
      {
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

/* quitting Auto GPKG Wrapping */
    sql = "SELECT AutoGPKGStop()";
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AutoGPKGStop error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  do_unlink_all ();
	  sqlite3_close (db_handle);
	  spatialite_cleanup_ex (cache);
	  spatialite_shutdown ();
	  return -1;
      }

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    do_unlink_all ();
    return 0;
}

#endif /* endif GEOPACKAGE enabled */
