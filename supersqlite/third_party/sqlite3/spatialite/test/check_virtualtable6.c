/*

 check_virtualtable6.c -- SpatiaLite Test Case

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
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

struct test_step
{
    const char *sql;
    const int num_rows;
};

struct test_step steps[] = {
    {"select DATUM, Geometry from shapetest WHERE DATUM = 2450514.5;", 18},
    {"select DATUM from shapetest WHERE DATUM < 2450514.5;", 0},
    {"select DATUM from shapetest WHERE DATUM > 2450514.5;", 0},
    {"select DATUM from shapetest WHERE DATUM >= 2450514.5;", 18},
    {"select DATUM from shapetest WHERE DATUM <= 2450514.5;", 18},
    {"select DATUM from shapetest WHERE DATUM = 2450514.5;", 18},
    {"select Length, HOEHE from shapetest WHERE HOEHE = 0.0;", 18},
    {"select Length, HOEHE from shapetest WHERE HOEHE = 1.0;", 0},
    {"select Length, HOEHE, Geometry from shapetest WHERE HOEHE != 0.0;", 0},
    {"select Length, HOEHE from shapetest WHERE Length > 0.0;", 18},
    {"select Length, HOEHE from shapetest WHERE Length >= 0.0;", 18},
    {"select Length, HOEHE from shapetest WHERE Length <= 0.0;", 0},
    {"select Length, HOEHE from shapetest WHERE Length > 1.0;", 0},
    {"select Length, HOEHE from shapetest WHERE Length > 0.1;", 10},
    {"select Length, HOEHE from shapetest WHERE Length < 0.1;", 8},
    {"select Length, HOEHE from shapetest WHERE Length >= 0.1;", 10},
    {"select Length, HOEHE from shapetest WHERE HOEHE = 0;", 18},
    {"select Length, HOEHE from shapetest WHERE HOEHE >= 0;", 18},
    {"select Length, HOEHE from shapetest WHERE HOEHE <= 0;", 18},
    {"select Length, HOEHE from shapetest WHERE HOEHE > 0;", 0},
    {"select Length, HOEHE from shapetest WHERE HOEHE < 0;", 0},
    {"select Length, HOEHE from shapetest WHERE HOEHE = 1;", 0},
    {"select Length, HOEHE from shapetest WHERE HOEHE <= 1;", 18},
    {"select Length, HOEHE from shapetest WHERE HOEHE >= 1;", 0},
    {"select Length, HOEHE from shapetest WHERE HOEHE < 1;", 18},
    {"select Length, HOEHE from shapetest WHERE HOEHE > -1;", 18},
    {"select Length, HOEHE from shapetest WHERE HOEHE < -1;", 0},
    {"select Length, HOEHE from shapetest WHERE HOEHE <= -1;", 0},
    {"select * from shapetest LIMIT 5 OFFSET 2;", 5},
    {NULL, 0}
};

int
main (int argc, char *argv[])
{
#ifndef OMIT_ICONV		/* only if ICONV is supported */
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
    int i;
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
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE shapetest USING VirtualShape('shp/merano-3d/roads', CP1252, 25832);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualShape error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

    ret =
	sqlite3_exec (db_handle, "DROP TABLE shapetest;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE shapetest USING VirtualShape(\"shp/merano-3d/roads\", 'CP1252', -1);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualShape error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }

    ret =
	sqlite3_exec (db_handle, "DROP TABLE shapetest;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE unquoted USING VirtualShape(shapetest1, UTF8, -1);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualShape error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }

    ret =
	sqlite3_exec (db_handle, "DROP TABLE unquoted;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE shapetest USING VirtualShape('shp/merano-3d/roads', \"CP1252\", 25832);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualShape error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -8;
      }

    for (i = 0; steps[i].sql; ++i)
      {
	  ret =
	      sqlite3_get_table (db_handle, steps[i].sql, &results, &rows,
				 &columns, &err_msg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return -9;
	    }
	  if (rows != steps[i].num_rows)
	    {
		fprintf (stderr, "Unexpected num of rows for test %i: %i.\n", i,
			 rows);
		return -10;
	    }
	  sqlite3_free_table (results);
      }

    ret =
	sqlite3_exec (db_handle, "DROP TABLE shapetest;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE nosuchfile USING VirtualShape(nosuchfile, UTF8, -1);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VirtualShape error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }

    ret =
	sqlite3_exec (db_handle, "DROP TABLE nosuchfile;", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE toofewargs USING VirtualShape(\"shapetest1\", UTF8);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "VirtualShape unexpected result: %i\n", ret);
	  return -14;
      }
    sqlite3_free (err_msg);

    ret =
	sqlite3_exec (db_handle,
		      "create VIRTUAL TABLE toomanyargs USING VirtualShape(\"shapetest1\", UTF8, 4386, 1, 1);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_ERROR)
      {
	  fprintf (stderr, "VirtualShape unexpected result: %i\n", ret);
	  return -15;
      }
    sqlite3_free (err_msg);

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
#endif /* end ICONV conditional */

    spatialite_shutdown ();
    return 0;
}
