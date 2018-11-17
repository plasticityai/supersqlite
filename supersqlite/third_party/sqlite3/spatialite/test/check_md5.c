/*

 check_md5.c -- SpatiaLite Test Case

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gaiaaux.h"

int
main (int argc, char *argv[])
{
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    int i;
    double x = 0.0;
    double y = 0.0;
    char sql[256];
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
	sqlite3_exec (handle,
		      "CREATE TABLE test (id INTEGER PRIMARY KEY NOT NULL, name TEXT NOT NULL)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -3;
      }
    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('test', 'geom', 4326, 'POINT', 'XY')",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -4;
      }

    for (i = 0; i < 1000; i++)
      {
	  /* inserting thousand points */
	  sprintf (sql, "INSERT INTO test (id, name, geom) VALUES "
		   "(%d, 'qwertyUOIOP%d', MakePoint(%1.2f, %1.2f, 4326))", i, i,
		   x, y);
	  x += 0.01;
	  y += 0.25;
	  ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "Insert Point(%d) error: %s\n", i, err_msg);
		sqlite3_free (err_msg);
		sqlite3_close (handle);
		return -5;
	    }
      }

/* testing MD5Checksum() - BLOB */
    strcpy (sql,
	    "SELECT MD5Checksum(geom) FROM test WHERE id IN (10, 100, 750)");
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }
    if ((rows != 3) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: MD5Checksum (blob) bad result: %i/%i.\n",
		   rows, columns);
	  return -7;
      }
    if (strcmp (results[1], "66db2227f970c7881b57247a28e2f893") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: MD5Checksum blob-#1 bad result: %s.\n",
		   results[1]);
	  return -8;
      }
    if (strcmp (results[2], "6fc5f82a0f9093c5287617cac908b557") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: MD5Checksum blob-#2 bad result: %s.\n",
		   results[2]);
	  return -9;
      }
    if (strcmp (results[3], "5f92e335d709c85f5858772a5072f0e3") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: MD5Checksum blob-#3 bad result: %s.\n",
		   results[3]);
	  return -10;
      }
    sqlite3_free_table (results);

/* testing the aggregate function: MD5TotalChecksum() - BLOB */
    strcpy (sql, "SELECT MD5TotalChecksum(geom) FROM test");
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: MD5TotalChecksum (blob) bad result: %i/%i.\n",
		   rows, columns);
	  return -12;
      }
    if (strcmp (results[1], "480b0dd7524a3bb4a75e01123a3eedf0") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: MD5TotalChecksum() (blob) bad result: %s.\n",
		   results[1]);
	  return -13;
      }
    sqlite3_free_table (results);

/* testing MD5Checksum() - TEXT */
    strcpy (sql,
	    "SELECT MD5Checksum(name) FROM test WHERE id IN (10, 100, 750)");
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }
    if ((rows != 3) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: MD5Checksum (text) bad result: %i/%i.\n",
		   rows, columns);
	  return -15;
      }
    if (strcmp (results[1], "05bc33eb92fb4a319da50825571358ad") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: MD5Checksum text-#1 bad result: %s.\n",
		   results[1]);
	  return -16;
      }
    if (strcmp (results[2], "bbf88f3dfa445ea7a37feca8959b08ed") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: MD5Checksum text-#2 bad result: %s.\n",
		   results[2]);
	  return -17;
      }
    if (strcmp (results[3], "97805a371d30b02f7a4ee611ff027664") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: MD5Checksum text-#3 bad result: %s.\n",
		   results[3]);
	  return -18;
      }
    sqlite3_free_table (results);

/* testing the aggregate function: MD5TotalChecksum() - TEXT */
    strcpy (sql, "SELECT MD5TotalChecksum(name) FROM test");
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -19;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: MD5TotalChecksum (text) bad result: %i/%i.\n",
		   rows, columns);
	  return -20;
      }
    if (strcmp (results[1], "b775beceb3ef0ee9d9c16d4ddfe81879") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: MD5TotalChecksum() (text) bad result: %s.\n",
		   results[1]);
	  return -21;
      }
    sqlite3_free_table (results);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -22;
      }

    spatialite_cleanup_ex (cache);

    return 0;
}
