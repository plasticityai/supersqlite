/*

 check_gpkgGetImageFormat_nonimage.c - Test case for GeoPackage Extensions

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

#include <sqlite3.h>
#include <spatialite.h>

#include "test_helpers.h"

int
main (int argc UNUSED, char *argv[]UNUSED)
{
    sqlite3 *db_handle = NULL;
    char *sql_statement;
    sqlite3_stmt *stmt;
    int ret;
    void *cache = spatialite_alloc_connection ();

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    /* For debugging / testing if required */
    /*
       ret = sqlite3_open_v2 ("check_gpkgGetImageFormat_nonimage.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
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

    sql_statement = "SELECT gpkgGetImageType(zeroblob(1000))";
    ret =
	sqlite3_prepare_v2 (db_handle, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "failed to prepare SQL statement: %i (%s)\n", ret,
		   sqlite3_errmsg (db_handle));
	  return -30;
      }
    ret = sqlite3_step (stmt);
    if (ret != SQLITE_ROW)
      {
	  fprintf (stderr, "unexpected return value for first step: %i\n", ret);
	  return -31;
      }
    if (sqlite3_column_type (stmt, 0) != SQLITE_TEXT)
      {
	  fprintf (stderr, "bad type for column 0: %i\n",
		   sqlite3_column_type (stmt, 0));
	  return -32;
      }
    if (strcmp ((const char *) sqlite3_column_text (stmt, 0), "unknown") != 0)
      {
	  fprintf (stderr, "wrong image type: %s\n",
		   sqlite3_column_text (stmt, 0));
	  return -33;
      }
    ret = sqlite3_step (stmt);
    if (ret != SQLITE_DONE)
      {
	  fprintf (stderr, "unexpected return value for second step: %i\n",
		   ret);
	  return -34;
      }
    ret = sqlite3_finalize (stmt);

    ret = sqlite3_close (db_handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (db_handle));
	  return -200;
      }

    spatialite_cleanup_ex (cache);

    return 0;
}
