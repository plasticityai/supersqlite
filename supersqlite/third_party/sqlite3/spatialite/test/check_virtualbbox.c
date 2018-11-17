/*

 check_virtualbbox.c -- SpatiaLite Test Case

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

int
main (int argc, char *argv[])
{
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
    char **results;
    int rows;
    int columns;
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

/* creating the BoundingBox table */
    ret = sqlite3_exec (db_handle, "CREATE TABLE BBox_Test (pk_uid INTEGER, "
			"min_x DOUBLE, min_y DOUBLE, max_x DOUBLE, max_y DOUBLE, srid INTEGER, "
			"name TEXT, count INTEGER, measure DOUBLE, odd BLOB)",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -3;
      }

/* inserting few rows into the BoundingBox table */
    ret = sqlite3_exec (db_handle, "INSERT INTO BBox_Test "
			"(pk_uid, min_x, min_y, max_x, max_y, srid, name, count, measure, odd) "
			"VALUES (1, 1, 2, 3, 4, 4326, 'alpha', 10, 10.1, zeroblob(4))",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT #1 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -4;
      }
    ret = sqlite3_exec (db_handle, "INSERT INTO BBox_Test "
			"(pk_uid, min_x, min_y, max_x, max_y, srid, name, count, measure, odd) "
			"VALUES (2, 2, 3, 4, 5, 4326, 'beta', 20, 20.2, zeroblob(4))",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT #2 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -5;
      }
    ret = sqlite3_exec (db_handle, "INSERT INTO BBox_Test "
			"(pk_uid, min_x, min_y, max_x, max_y, srid, name, count, measure, odd) "
			"VALUES (3, 3, 4, 5, 6, 3003, 'gamma', 30, 30.3, NULL)",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT #3 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -6;
      }
    ret = sqlite3_exec (db_handle, "INSERT INTO BBox_Test "
			"(pk_uid, min_x, min_y, max_x, max_y, srid, name, count, measure, odd) "
			"VALUES (4, 1, 2, NULL, 4, 4326, 'delta', 40, NULL, NULL)",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT #4 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -7;
      }
    ret = sqlite3_exec (db_handle, "INSERT INTO BBox_Test "
			"(pk_uid, min_x, min_y, max_x, max_y, srid, name, count, measure, odd) "
			"VALUES (5, 1, 2, 3, 4, NULL, 'epsilon', NULL, NULL, NULL)",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT #5 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -8;
      }

/* creating the VirtualBBox table #1 */
    ret = sqlite3_exec (db_handle, "CREATE VIRTUAL TABLE test1 USING "
			"VirtualBBox('BBox_Test', 'min_x', min_y, \"max_x\", 'max_y', "
			"srid, 0, pk_uid, name, count, measure, odd)", NULL,
			NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE VIRTUAL TABLE #1 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -9;
      }

/* testing the VirtualBBox table #1 */
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT ROWID, pk_uid, name, count, measure, Length(odd), GeometryType(Geometry), Srid(Geometry) FROM test1",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }
    if ((rows != 5) || (columns != 8))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -11;
      }
    if (strcasecmp (results[0], "ROWID") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[0]);
	  return -12;
      }
    if (strcasecmp (results[1], "pk_uid") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[1]);
	  return -13;
      }
    if (strcasecmp (results[2], "name") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[2]);
	  return -14;
      }
    if (strcasecmp (results[3], "count") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[3]);
	  return -15;
      }
    if (strcasecmp (results[4], "measure") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[4]);
	  return -16;
      }
    if (strcasecmp (results[5], "Length(odd)") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[5]);
	  return -17;
      }
    if (strcasecmp (results[6], "GeometryType(Geometry)") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[6]);
	  return -18;
      }
    if (strcasecmp (results[7], "Srid(Geometry)") != 0)
      {
	  fprintf (stderr, "Unexpected error: header() bad result: %s.\n",
		   results[7]);
	  return -19;
      }
    if (strcmp (results[8], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-1 bad result: %s.\n",
		   results[8]);
	  return -20;
      }
    if (strcmp (results[9], "1") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-1 bad result: %s.\n",
		   results[9]);
	  return -21;
      }
    if (strcmp (results[10], "alpha") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-1 bad result: %s.\n",
		   results[10]);
	  return -22;
      }
    if (strcmp (results[11], "10") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-1 bad result: %s.\n",
		   results[11]);
	  return -23;
      }
    if (strcmp (results[12], "10.1") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-1 bad result: %s.\n",
		   results[12]);
	  return -24;
      }
    if (strcmp (results[13], "4") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-1 bad result: %s.\n",
		   results[13]);
	  return -25;
      }
    if (strcmp (results[14], "POLYGON") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-1 bad result: %s.\n",
		   results[14]);
	  return -26;
      }
    if (strcmp (results[15], "4326") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-1 bad result: %s.\n",
		   results[15]);
	  return -27;
      }
    if (strcmp (results[16], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-2 bad result: %s.\n",
		   results[16]);
	  return -28;
      }
    if (strcmp (results[17], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-2 bad result: %s.\n",
		   results[17]);
	  return -29;
      }
    if (strcmp (results[18], "beta") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-2 bad result: %s.\n",
		   results[18]);
	  return -30;
      }
    if (strcmp (results[19], "20") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-2 bad result: %s.\n",
		   results[19]);
	  return -31;
      }
    if (strcmp (results[20], "20.2") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-2 bad result: %s.\n",
		   results[20]);
	  return -32;
      }
    if (strcmp (results[21], "4") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-2 bad result: %s.\n",
		   results[21]);
	  return -33;
      }
    if (strcmp (results[22], "POLYGON") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-2 bad result: %s.\n",
		   results[22]);
	  return -34;
      }
    if (strcmp (results[23], "4326") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-2 bad result: %s.\n",
		   results[23]);
	  return -35;
      }
    if (strcmp (results[24], "3") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-3 bad result: %s.\n",
		   results[24]);
	  return -36;
      }
    if (strcmp (results[25], "3") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-3 bad result: %s.\n",
		   results[25]);
	  return -37;
      }
    if (strcmp (results[26], "gamma") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-3 bad result: %s.\n",
		   results[26]);
	  return -38;
      }
    if (strcmp (results[27], "30") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-3 bad result: %s.\n",
		   results[27]);
	  return -39;
      }
    if (strcmp (results[28], "30.3") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-3 bad result: %s.\n",
		   results[28]);
	  return -40;
      }
    if (results[29] != NULL)
      {
	  fprintf (stderr, "Unexpected error: row-3 bad result: %s.\n",
		   results[29]);
	  return -41;
      }
    if (strcmp (results[30], "POLYGON") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-3 bad result: %s.\n",
		   results[30]);
	  return -42;
      }
    if (strcmp (results[31], "3003") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-3 bad result: %s.\n",
		   results[31]);
	  return -43;
      }
    if (strcmp (results[32], "4") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-4 bad result: %s.\n",
		   results[33]);
	  return -44;
      }
    if (strcmp (results[33], "4") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-4 bad result: %s.\n",
		   results[33]);
	  return -45;
      }
    if (strcmp (results[34], "delta") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-4 bad result: %s.\n",
		   results[34]);
	  return -46;
      }
    if (strcmp (results[35], "40") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-4 bad result: %s.\n",
		   results[35]);
	  return -47;
      }
    if (results[36] != NULL)
      {
	  fprintf (stderr, "Unexpected error: row-4 bad result: %s.\n",
		   results[36]);
	  return -48;
      }
    if (results[37] != NULL)
      {
	  fprintf (stderr, "Unexpected error: row-4 bad result: %s.\n",
		   results[37]);
	  return -49;
      }
    if (results[38] != NULL)
      {
	  fprintf (stderr, "Unexpected error: row-4 bad result: %s.\n",
		   results[38]);
	  return -50;
      }
    if (results[39] != NULL)
      {
	  fprintf (stderr, "Unexpected error: row-4 bad result: %s.\n",
		   results[39]);
	  return -51;
      }
    if (strcmp (results[40], "5") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-5 bad result: %s.\n",
		   results[40]);
	  return -52;
      }
    if (strcmp (results[41], "5") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-5 bad result: %s.\n",
		   results[41]);
	  return -53;
      }
    if (strcmp (results[42], "epsilon") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-5 bad result: %s.\n",
		   results[42]);
	  return -54;
      }
    if (results[43] != NULL)
      {
	  fprintf (stderr, "Unexpected error: row-5 bad result: %s.\n",
		   results[43]);
	  return -55;
      }
    if (results[44] != NULL)
      {
	  fprintf (stderr, "Unexpected error: row-5 bad result: %s.\n",
		   results[44]);
	  return -56;
      }
    if (results[45] != NULL)
      {
	  fprintf (stderr, "Unexpected error: row-5 bad result: %s.\n",
		   results[45]);
	  return -57;
      }
    if (strcmp (results[46], "POLYGON") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-5 bad result: %s.\n",
		   results[46]);
	  return -58;
      }
    if (strcmp (results[47], "0") != 0)
      {
	  fprintf (stderr, "Unexpected error: row-5 bad result: %s.\n",
		   results[47]);
	  return -59;
      }
    sqlite3_free_table (results);

/* dropping the VirtualBBox table #1 */
    ret = sqlite3_exec (db_handle, "DROP TABLE test1", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE #1 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -60;
      }

/* creating the VirtualBBox table #2 */
    ret = sqlite3_exec (db_handle, "CREATE VIRTUAL TABLE test2 USING "
			"VirtualBBox('BBox_Test', 'min_x', min_y, \"max_x\", 'max_y', 3003, 0)",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE VIRTUAL TABLE #2 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -61;
      }

/* testing the VirtualBBox table #2 */
    ret =
	sqlite3_get_table (db_handle,
			   "SELECT DISTINCT GeometryType(Geometry), Srid(Geometry) FROM test2",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -62;
      }
    if ((rows != 2) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -63;
      }
    if (strcasecmp (results[2], "POLYGON") != 0)
      {
	  fprintf (stderr, "Unexpected error: distinct-row-1 bad result: %s.\n",
		   results[2]);
	  return -64;
      }
    if (strcasecmp (results[3], "3003") != 0)
      {
	  fprintf (stderr, "Unexpected error: distinct-row-1 bad result: %s.\n",
		   results[3]);
	  return -65;
      }
    if (results[4] != NULL)
      {
	  fprintf (stderr, "Unexpected error: distinct-row-2 bad result: %s.\n",
		   results[4]);
	  return -66;
      }
    if (results[5] != NULL)
      {
	  fprintf (stderr, "Unexpected error: distinct-row-2 bad result: %s.\n",
		   results[5]);
	  return -67;
      }
    sqlite3_free_table (results);

/* dropping the VirtualBBox table #2 */
    ret = sqlite3_exec (db_handle, "DROP TABLE test2", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE #2 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -68;
      }

/* creating the VirtualBBox table #3 - failure */
    ret = sqlite3_exec (db_handle, "CREATE VIRTUAL TABLE test3 USING "
			"VirtualBBox('BBox_Test', 'min_x', min_y, \"max_x\", 'max_y', 3003)",
			NULL, NULL, &err_msg);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "CREATE VIRTUAL TABLE #3 unexpected result: %s\n",
		   err_msg);
	  sqlite3_close (db_handle);
	  return -69;
      }
    sqlite3_free (err_msg);

/* creating the VirtualBBox table #4 - failure */
    ret = sqlite3_exec (db_handle, "CREATE VIRTUAL TABLE test3 USING "
			"VirtualBBox('BBox_Test', 'min_x', min_y, \"max_x\", 'wrong', 3003, 0)",
			NULL, NULL, &err_msg);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "CREATE VIRTUAL TABLE #4 unexpected result: %s\n",
		   err_msg);
	  sqlite3_close (db_handle);
	  return -70;
      }
    sqlite3_free (err_msg);

/* creating the VirtualBBox table #46 - failure */
    ret = sqlite3_exec (db_handle, "CREATE VIRTUAL TABLE test3 USING "
			"VirtualBBox('wrong', 'min_x', min_y, \"max_x\", 'max_y', 3003, 0)",
			NULL, NULL, &err_msg);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "CREATE VIRTUAL TABLE #5 unexpected result: %s\n",
		   err_msg);
	  sqlite3_close (db_handle);
	  return -71;
      }
    sqlite3_free (err_msg);


    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
