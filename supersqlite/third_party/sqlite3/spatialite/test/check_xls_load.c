/*

 check_xls_load.c -- SpatiaLite Test Case

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

int
main (int argc, char *argv[])
{
#ifndef OMIT_FREEXL		/* only if FreeXL is supported */
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    unsigned int row_count;
    int rcnt;
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

    ret =
	load_XL (handle, "./testcase1.xls", "test1", 0, 0, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "load_XL() error: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -3;
      }
    if (row_count != 17)
      {
	  fprintf (stderr, "load_XL() unexpected row count: %u\n", row_count);
	  sqlite3_close (handle);
	  return -4;
      }

    ret =
	load_XL (handle, "./testcase1.xls", "test2", 1, 1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "load_XL() error sheet 2: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -5;
      }
    if (row_count != 19)
      {
	  fprintf (stderr, "load_XL() unexpected row count sheet 2: %u\n",
		   row_count);
	  sqlite3_close (handle);
	  return -6;
      }

    check_duplicated_rows (handle, "test1", &rcnt);
    if (rcnt != 0)
      {
	  fprintf (stderr,
		   "check_duplicated_rows() unexpected duplicate count: %d\n",
		   rcnt);
	  sqlite3_close (handle);
	  return -8;
      }

    check_duplicated_rows (handle, "test2", &rcnt);
    if (rcnt != 2)
      {
	  fprintf (stderr,
		   "check_duplicated_rows() unexpected duplicate count sheet 2: %d\n",
		   rcnt);
	  sqlite3_close (handle);
	  return -10;
      }

    remove_duplicated_rows (handle, "test1");

    remove_duplicated_rows (handle, "test2");

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -11;
      }

    spatialite_cleanup_ex (cache);
#endif /* end FreeXL conditional */

    spatialite_shutdown ();
    return 0;
}
