/*

 check_init2.c -- SpatiaLite Test Case

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
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    char **results;
    int rows;
    int columns;
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
	sqlite3_exec (handle, "SELECT InitSpatialMetadata(\"NONE\")", NULL,
		      NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "InitSpatialMetadata(\"NONE\") error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -2;
      }

    ret =
	sqlite3_exec (handle, "SELECT InsertEpsgSrid(4326)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Insert SRID 4326 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -3;
      }

#ifndef OMIT_EPSG
/* only if full EPSG support is enabled */
    ret =
	sqlite3_exec (handle, "SELECT InsertEpsgSrid(2998)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Insert SRID 2998 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -4;
      }
#endif

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -5;
      }

    spatialite_cleanup_ex (cache);

    cache = spatialite_alloc_connection ();
    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -6;
      }

    spatialite_init_ex (handle, cache, 0);

    ret =
	sqlite3_get_table (handle, "SELECT InitSpatialMetadata(3.4)", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result InitSpatialMetadata() bad result: %i/%i.\n",
		   rows, columns);
	  return -8;
      }
    if (strcmp (results[1], "0") != 0)
      {
	  fprintf (stderr,
		   "Unexpected result: InitSpatialMetadata() with non-text passed: %s.\n",
		   results[1]);
	  return -9;
      }
    sqlite3_free_table (results);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -10;
      }

    spatialite_cleanup_ex (cache);

    cache = spatialite_alloc_connection ();
    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -11;
      }

    spatialite_init_ex (handle, cache, 0);

    ret =
	sqlite3_get_table (handle, "SELECT InitSpatialMetadata(\"EMPTY\")",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result InitSpatialMetadata(\"EMPTY\") bad result: %i/%i.\n",
		   rows, columns);
	  return -13;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: InitSpatialMetadata(\"EMPTY\"): %s.\n",
		   results[1]);
	  return -14;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (handle, "SELECT InsertEpsgSrid(4326)", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -15;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result InsertEpsgSrid(4326) bad result: %i/%i.\n",
		   rows, columns);
	  return -16;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: InsertEpsgSrid(4326): %s.\n",
		   results[1]);
	  return -17;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (handle, "SELECT InsertEpsgSrid(\"Non-integer\")",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -18;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result InsertEpsgSrid(\"Non-integer\") bad result: %i/%i.\n",
		   rows, columns);
	  return -19;
      }
    if (strcmp (results[1], "0") != 0)
      {
	  fprintf (stderr,
		   "Unexpected result: InsertEpsgSrid() with non-integer passed: %s.\n",
		   results[1]);
	  return -20;
      }
    sqlite3_free_table (results);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -21;
      }

    spatialite_cleanup_ex (cache);

    cache = spatialite_alloc_connection ();
    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -22;
      }

    spatialite_init_ex (handle, cache, 0);

    ret =
	sqlite3_get_table (handle, "SELECT InitSpatialMetadata(\"WGS84\")",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result InitSpatialMetadata(\"WGS84\") bad result: %i/%i.\n",
		   rows, columns);
	  return -24;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: InitSpatialMetadata(\"WGS84\"): %s.\n",
		   results[1]);
	  return -25;
      }
    sqlite3_free_table (results);

#ifndef OMIT_EPSG
/* only if full EPSG support is enabled */
    ret =
	sqlite3_get_table (handle, "SELECT InsertEpsgSrid(3003)", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -26;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result InsertEpsgSrid(3003) bad result: %i/%i.\n",
		   rows, columns);
	  return -27;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: InsertEpsgSrid(3003): %s.\n",
		   results[1]);
	  return -28;
      }
    sqlite3_free_table (results);
#endif

    ret =
	sqlite3_get_table (handle, "SELECT InsertEpsgSrid(4326)", &results,
			   &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result InsertEpsgSrid(4326) (dupe) bad result: %i/%i.\n",
		   rows, columns);
	  return -30;
      }
    if (strcmp (results[1], "0") != 0)
      {
	  fprintf (stderr,
		   "Unexpected result: InsertEpsgSrid(4326) duplicate passed: %s.\n",
		   results[1]);
	  return -31;
      }
    sqlite3_free_table (results);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -32;
      }

    spatialite_cleanup_ex (cache);

    cache = spatialite_alloc_connection ();
    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -33;
      }

    spatialite_init_ex (handle, cache, 0);

    ret =
	sqlite3_get_table (handle, "SELECT InitSpatialMetadata(\"WGS84_only\")",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -34;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result InitSpatialMetadata(\"WGS84_ONLY\") bad result: %i/%i.\n",
		   rows, columns);
	  return -35;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: InitSpatialMetadata(\"WGS84_ONLY\"): %s.\n",
		   results[1]);
	  return -36;
      }
    sqlite3_free_table (results);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -32;
      }

    spatialite_cleanup_ex (cache);

    return 0;
}
