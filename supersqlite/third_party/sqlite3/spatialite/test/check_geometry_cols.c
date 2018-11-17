/*

 check_create.c -- SpatiaLite Test Case

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
		      "CREATE TABLE Point_Test (Name TEXT, Description TEXT)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -3;
      }

    ret =
	sqlite3_get_table (handle,
			   "SELECT AddGeometryColumn(26, 'geomZ', 4326, 'POINT', 'XYZ', 0)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result AddGeometryColumn int arg1 bad result: %i/%i.\n",
		   rows, columns);
	  return -5;
      }
    if (strcmp (results[1], "0") != 0)
      {
	  fprintf (stderr,
		   "Unexpected result: AddGeometryColumn with non-text arg1 passed: %s.\n",
		   results[1]);
	  return -6;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (handle,
			   "SELECT AddGeometryColumn('Point_Test', 8, 4326, 'POINT', 'XYZ', 0)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result AddGeometryColumn int arg2 bad result: %i/%i.\n",
		   rows, columns);
	  return -8;
      }
    if (strcmp (results[1], "0") != 0)
      {
	  fprintf (stderr,
		   "Unexpected result: AddGeometryColumn with non-text arg2 passed: %s.\n",
		   results[1]);
	  return -9;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (handle,
			   "SELECT AddGeometryColumn('Point_Test', 'geomZ', 'sometext', 'POINT', 'XYZ', 0)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result AddGeometryColumn text arg3 bad result: %i/%i.\n",
		   rows, columns);
	  return -11;
      }
    if (strcmp (results[1], "0") != 0)
      {
	  fprintf (stderr,
		   "Unexpected result: AddGeometryColumn with non-int arg3 passed: %s.\n",
		   results[1]);
	  return -12;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (handle,
			   "SELECT AddGeometryColumn('Point_Test', 'geomZ', 4326, 'POINT', 'XYZ', 0)",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result AddGeometryColumn bad result: %i/%i.\n",
		   rows, columns);
	  return -14;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: AddGeometryColumn with good args failed: %s.\n",
		   results[1]);
	  return -15;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Point_Test (Name, Description, geomZ) VALUES ('Point 1', 'Some point', GeomFromText('POINTZ(136 -33 365)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT POINT XYZ error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -16;
      }

    ret =
	sqlite3_get_table (handle,
			   "SELECT DiscardGeometryColumn('Point_Test', 'geomZ')",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -17;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result DiscardGeometryColumn bad result: %i/%i.\n",
		   rows, columns);
	  return -18;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: DiscardGeometryColumn failed: %s.\n",
		   results[1]);
	  return -19;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_get_table (handle,
			   "SELECT RecoverGeometryColumn('Point_Test', 'geomZ', 4326, 'POINT', 'XYZ')",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -20;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected result RecoverGeometryColumn bad result: %i/%i.\n",
		   rows, columns);
	  return -21;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: RecoverGeometryColumn failed: %s.\n",
		   results[1]);
	  return -22;
      }
    sqlite3_free_table (results);
    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -23;
      }

    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
