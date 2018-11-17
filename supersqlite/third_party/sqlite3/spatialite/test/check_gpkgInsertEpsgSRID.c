/*

 check_gpkgInsertEpsgSRID.c - Test case for GeoPackage Extensions

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
 
Portions created by the Initial Developer are Copyright (C) 2012
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

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

#include "test_helpers.h"

int
main (int argc UNUSED, char *argv[]UNUSED)
{
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
#ifndef OMIT_EPSG /* only if full EPSG support is enabled */
    char *sql_statement;
    sqlite3_stmt *stmt;
#endif
    void *cache = spatialite_alloc_connection ();

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    /* For debugging / testing if required */
    /*
       ret = sqlite3_open_v2 ("check_gpkgInsertEpsgSRID.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
     */
    spatialite_init_ex (db_handle, cache, 0);
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
	  return -100;
      }

#ifndef OMIT_EPSG /* only if full EPSG support is enabled */
    ret =
	sqlite3_exec (db_handle, "SELECT gpkgInsertEpsgSRID(3857)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Unexpected gpkgInsertEpsgSRID() result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -101;
      }

    /* check insert is OK */
    sql_statement =
	"SELECT srs_name, organization, organization_coordsys_id FROM gpkg_spatial_ref_sys WHERE srs_id=3857";
    ret =
	sqlite3_prepare_v2 (db_handle, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "failed to prepare SELECT ... FROM gpkg_spatial_ref_sys: %i (%s)\n",
		   ret, sqlite3_errmsg (db_handle));
	  return -102;
      }
    ret = sqlite3_step (stmt);
    if (ret != SQLITE_ROW)
      {
	  fprintf (stderr, "unexpected return value for first step: %i (%s)\n",
		   ret, sqlite3_errmsg (db_handle));
	  return -103;
      }
    if (sqlite3_column_type (stmt, 0) != SQLITE_TEXT)
      {
	  fprintf (stderr, "bad type for column 0: %i\n",
		   sqlite3_column_type (stmt, 0));
	  return -104;
      }
    if (strcasecmp
	((const char *) sqlite3_column_text (stmt, 0),
	 "WGS 84 / Pseudo-Mercator") != 0)
      {
	  fprintf (stderr, "wrong srs_name: %s\n",
		   sqlite3_column_text (stmt, 0));
	  return -105;
      }
    if (sqlite3_column_type (stmt, 1) != SQLITE_TEXT)
      {
	  fprintf (stderr, "bad type for column 1: %i\n",
		   sqlite3_column_type (stmt, 1));
	  return -106;
      }
    if (strcasecmp ((const char *) sqlite3_column_text (stmt, 1), "epsg") != 0)
      {
	  fprintf (stderr, "wrong organization: %s\n",
		   sqlite3_column_text (stmt, 1));
	  return -107;
      }
    if (sqlite3_column_type (stmt, 2) != SQLITE_INTEGER)
      {
	  fprintf (stderr, "bad type for column 2: %i\n",
		   sqlite3_column_type (stmt, 2));
	  return -108;
      }
    if (sqlite3_column_int (stmt, 2) != 3857)
      {
	  fprintf (stderr, "wrong organization coordinate system ID: %i\n",
		   sqlite3_column_int (stmt, 2));
	  return -109;
      }
    ret = sqlite3_step (stmt);
    if (ret != SQLITE_DONE)
      {
	  fprintf (stderr, "unexpected return value for second step: %i\n",
		   ret);
	  return -129;
      }
    ret = sqlite3_finalize (stmt);

    sqlite3_free (err_msg);
    
    /* try no WKT, something of a hack here */
    ret =
	sqlite3_exec (db_handle, "SELECT gpkgInsertEpsgSRID(40001)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgInsertEpsgSRID(40001) result: %i, (%s)\n",
		   ret, err_msg);
	  sqlite3_free (err_msg);
	  return -130;
      }
#endif

    /* try some bad arguments */
    ret =
	sqlite3_exec (db_handle, "SELECT gpkgInsertEpsgSRID(34.4)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for insert value, non-integer id, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -200;
      }
    if (strcmp
	(err_msg,
	 "gpkgInsertEpsgSRID() error: argument 1 [srid] is not of the integer type")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgInsertEpsgSRID arg 1 bad type: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -201;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle, "SELECT gpkgInsertEpsgSRID(9999999)", NULL,
		      NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for insert value, invalid id, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -202;
      }
    if (strcmp
	(err_msg,
	 "gpkgInsertEpsgSRID() error: srid is not defined in the EPSG inlined dataset")
	!= 0)
      {
	  fprintf (stderr,
		   "Unexpected error message for gpkgInsertEpsgSRID arg 1 bad value: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -203;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle, "SELECT gpkgInsertEpsgSRID(3857)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr,
		   "Expected error for insert value, duplicate entry, got %i\n",
		   ret);
	  sqlite3_free (err_msg);
	  return -204;
      }
    sqlite3_free (err_msg);

    ret = sqlite3_close (db_handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (db_handle));
	  return -400;
      }

    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
