/*

 check_gpkgConvert.c - Test case for GeoPackage Extensions

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
#include <errno.h>

#include "config.h"

#ifdef ENABLE_GEOPACKAGE	/* only if GEOPACKAGE is enabled */

#include "sqlite3.h"
#include "spatialite.h"

static void
do_unlink_all ()
{
/* deleting all temporary files */
    unlink ("./copy-gpkg_test.sqlite");
    unlink ("./copy-test-legacy-3.0.1.sqlite");
    unlink ("./out1.gpkg");
    unlink ("./out2.gpkg");
    unlink ("./out1.sqlite");
    unlink ("./out2.sqlite");
}

static int
do_load_legacy (const char *path)
{
/* loading the Legacy Test DB */
    sqlite3 *db_handle;
    int ret;
    void *cache = NULL;
    const char *sql;
    char *sql_err = NULL;

    cache = spatialite_alloc_connection ();
    ret = sqlite3_open_v2 (path, &db_handle, SQLITE_OPEN_READWRITE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open '%s': %s\n", path,
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  return 0;
      }
    spatialite_init_ex (db_handle, cache, 0);

/* creating a table */
    sql = "CREATE TABLE test0 (id INTEGER NOT NULL PRIMARY KEY)";
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

/* adding a geometry */
    sql = "SELECT AddGeometryColumn('test0', 'geom', 4326, 'POINT', 'XY')";
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

/* inserting one row */
    sql = "INSERT INTO test0 (id, geom) VALUES(1, MakePoint(1, 2, 4326))";
    ret = sqlite3_exec (db_handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
    return 1;
}

static sqlite3 *
connect_db (const char *path, int flags, void *cache)
{
/* attempting to connect a SQLite DB file */
    sqlite3 *db_handle;
    int ret;

    ret = sqlite3_open_v2 (path, &db_handle, flags, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open '%s': %s\n", path,
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  return NULL;
      }
    spatialite_init_ex (db_handle, cache, 0);
    return db_handle;
}

static int
already_existing (const char *path)
{
/* check if a file already exists */
    FILE *in = fopen (path, "rb");
    if (in != NULL)
      {
	  /* already existing and accessible */
	  fclose (in);
	  return 1;
      }
    if (errno == ENOENT)
	return 0;
    return 1;
}

static int
open_connections (const char *path_origin, const char *path_destination,
		  void *cache_in, void *cache_out, sqlite3 ** xhandle_in,
		  sqlite3 ** xhandle_out)
{
/* establishing IN and OUT DB connections */
    sqlite3 *handle_in = NULL;
    sqlite3 *handle_out = NULL;

    if (already_existing (path_destination))
      {
	  fprintf (stderr, "Already existing output destination:\n\"%s\"\n",
		   path_destination);
	  return 0;
      }

/* attempting to connect the IN DB */
    handle_in = connect_db (path_origin, SQLITE_OPEN_READONLY, cache_in);
    if (handle_in == NULL)
	goto error;

/* attempting to connect the OUT DB */
    handle_out =
	connect_db (path_destination,
		    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, cache_out);
    if (handle_out == NULL)
	goto error;
    *xhandle_in = handle_in;
    *xhandle_out = handle_out;
    return 1;

  error:
    if (handle_in != NULL)
	sqlite3_close (handle_in);
    if (handle_out != NULL)
	sqlite3_close (handle_out);
    return 0;
}

int
main (int argc, char *argv[])
{
    sqlite3 *handle_in = NULL;
    sqlite3 *handle_out = NULL;
    void *cache_in = NULL;
    void *cache_out = NULL;
    const char *path_origin;
    const char *path_destination;
    int ret;
    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    do_unlink_all ();

/* converting from SpatiaLite v4 to GPKG */
    ret = system ("cp ./gpkg_test.sqlite copy-gpkg_test.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot copy gpkg_test.sqlite database\n");
	  return -1;
      }
    path_origin = "./copy-gpkg_test.sqlite";
    path_destination = "./out1.gpkg";
    cache_in = spatialite_alloc_connection ();
    cache_out = spatialite_alloc_connection ();
    if (!open_connections
	(path_origin, path_destination, cache_in, cache_out, &handle_in,
	 &handle_out))
      {
	  do_unlink_all ();
	  return -1;
      }
    if (!gaiaSpatialite2GPKG
	(handle_in, path_origin, handle_out, path_destination))
      {
	  do_unlink_all ();
	  return -1;
      }
    sqlite3_close (handle_in);
    sqlite3_close (handle_out);
    spatialite_cleanup_ex (cache_in);
    spatialite_cleanup_ex (cache_out);

/* converting from GPKG to SpatiaLite */
    path_origin = "./out1.gpkg";
    path_destination = "./out1.sqlite";
    cache_in = spatialite_alloc_connection ();
    cache_out = spatialite_alloc_connection ();
    if (!open_connections
	(path_origin, path_destination, cache_in, cache_out, &handle_in,
	 &handle_out))
      {
	  do_unlink_all ();
	  return -1;
      }
    if (!gaiaGPKG2Spatialite
	(handle_in, path_origin, handle_out, path_destination))
      {
	  do_unlink_all ();
	  return -1;
      }
    sqlite3_close (handle_in);
    sqlite3_close (handle_out);
    spatialite_cleanup_ex (cache_in);
    spatialite_cleanup_ex (cache_out);

/* converting from SpatiaLite v3 to GPKG */
    ret =
	system ("cp ./test-legacy-3.0.1.sqlite copy-test-legacy-3.0.1.sqlite");
    if (ret != 0)
      {
	  do_unlink_all ();
	  fprintf (stderr, "cannot copy test-legacy-3.0.1.sqlite database\n");
	  return -1;
      }
    if (!do_load_legacy ("./copy-test-legacy-3.0.1.sqlite"))
      {
	  do_unlink_all ();
	  return -1;
      }
    path_origin = "./copy-test-legacy-3.0.1.sqlite";
    path_destination = "./out2.sqlite";
    cache_in = spatialite_alloc_connection ();
    cache_out = spatialite_alloc_connection ();
    if (!open_connections
	(path_origin, path_destination, cache_in, cache_out, &handle_in,
	 &handle_out))
      {
	  do_unlink_all ();
	  return -1;
      }
    if (!gaiaSpatialite2GPKG
	(handle_in, path_origin, handle_out, path_destination))
      {
	  do_unlink_all ();
	  return -1;
      }
    sqlite3_close (handle_in);
    sqlite3_close (handle_out);
    spatialite_cleanup_ex (cache_in);
    spatialite_cleanup_ex (cache_out);

    do_unlink_all ();
    return 0;
}

#endif /* endif GEOPACKAGE enabled */
