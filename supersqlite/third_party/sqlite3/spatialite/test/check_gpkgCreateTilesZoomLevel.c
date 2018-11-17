/*

 check_gpkgCreateTilesZoomLevel.c - Test case for GeoPackage Extensions

 Author: Brad Hards <bradh@frogmouth.net>

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

The Initial Developer of the Original Code is Brad Hards
 
Portions created by the Initial Developer are Copyright (C) 2011,2013
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
#include <stdio.h>
#include <string.h>

#include <sqlite3.h>
#include <spatialite.h>

#include "test_helpers.h"

int
main (int argc UNUSED, char *argv[]UNUSED)
{
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
    void *cache = spatialite_alloc_connection ();
    char *old_SPATIALITE_SECURITY_ENV = NULL;
#ifdef _WIN32
    char *env;
#endif /* not WIN32 */

    old_SPATIALITE_SECURITY_ENV = getenv ("SPATIALITE_SECURITY");
#ifdef _WIN32
    putenv ("SPATIALITE_SECURITY=relaxed");
#else /* not WIN32 */
    setenv ("SPATIALITE_SECURITY", "relaxed", 1);
#endif

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    /* For debugging / testing if required */
    /*
       ret = sqlite3_open_v2 ("check_gpkgCreateTilesZoomLevel.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
     */
    spatialite_init_ex (db_handle, cache, 0);

    if (old_SPATIALITE_SECURITY_ENV)
      {
#ifdef _WIN32
	  env =
	      sqlite3_mprintf ("SPATIALITE_SECURITY=%s",
			       old_SPATIALITE_SECURITY_ENV);
	  putenv (env);
	  sqlite3_free (env);
#else /* not WIN32 */
	  setenv ("SPATIALITE_SECURITY", old_SPATIALITE_SECURITY_ENV, 1);
#endif
      }
    else
      {
#ifdef _WIN32
	  putenv ("SPATIALITE_SECURITY=");
#else /* not WIN32 */
	  unsetenv ("SPATIALITE_SECURITY");
#endif
      }
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  db_handle = NULL;
	  return -1;
      }

    ret =
	sqlite3_exec (db_handle, "SELECT gpkgCreateBaseTables()", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateBaseTables() result: %i, (%s)\n", ret,
		   err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesTable(\"test1_matrix_tiles\", 4326, -180.0, -90.0, 180.0, 90.0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateTilesTable() result: %i, (%s)\n", ret,
		   err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }

    /* create matrix level 0 */
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(\"test1_matrix_tiles\", 0, 360, 180)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateTilesZoomLevel(0) result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }

    /* try duplicate entry */
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(\"test1_matrix_tiles\", 0, 360, 180)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add zoom level, duplicate entry, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -10;
      }
    sqlite3_free (err_msg);

    /*  try some bad arguments */
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(9.4, 4, 360, 180)", NULL,
		      NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add zoom level, non-string table name, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -18;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesZoomLevel() error: argument 1 [table] is not of the String type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 1 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(\"test1_matrix_tiles\", 4.2, 360, 180)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add zoom level, non-integer zoom, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -20;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesZoomLevel() error: argument 2 [zoom level] is not of the integer type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 2 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(\"test1_matrix_tiles\", 4, \"x\", 180)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add zoom level, non-numeric extent, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -22;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesZoomLevel() error: argument 3 [extent_width] is not of a numerical type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 3 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(\"test1_matrix_tiles\", 4, 360, \"y\")",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add zoom level, non-numeric height extent, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -24;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesZoomLevel() error: argument 4 [extent_height] is not of a numerical type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 4 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(\"test1_matrix_tiles\", -1, 360, 180)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for add zoom level, negative zoom, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -26;
      }
    if (strcmp
	(err_msg,
	 "gpkgCreateTilesZoomLevel() error: argument 2 [zoom level] must be >= 0")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgAddGeometryColumn arg 2 bad value: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -27;
      }
    sqlite3_free (err_msg);

    /* Try float extent */
    ret =
	sqlite3_exec (db_handle,
		      "SELECT gpkgCreateTilesZoomLevel(\"test1_matrix_tiles\", 1, 360.0, 180.0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateTilesZoomLevel(1) result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }

    ret = sqlite3_close (db_handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (db_handle));
	  return -200;
      }

    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
