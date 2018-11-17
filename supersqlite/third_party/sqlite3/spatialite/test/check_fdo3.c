/*

 check_fdo3.c -- SpatiaLite Test Case

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
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

#include "spatialite/gaiageo.h"

#ifndef OMIT_GEOS		/* including GEOS */
#include <geos_c.h>
#endif

int
main (int argc, char *argv[])
{
#ifndef OMIT_GEOS		/* only if GEOS is supported */
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    const char *sql;
    char **results;
    int rows;
    int columns;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret = system ("cp sql_stmt_tests/testFGF.sqlite testFGF.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot copy testFGF.sqlite database\n");
	  return -1001;
      }
    ret =
	sqlite3_open_v2 ("testFGF.sqlite", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open testFGF.sqlite db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1000;
      }

    spatialite_init_ex (handle, cache, 0);

/* FDO start-up */
    sql = "SELECT AutoFDOStart()";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -1;
      }

/* testing aggregate Union() Point FGF "HouseNumbers" */
    sql =
	"SELECT AsText(ST_Union(Geometry)) FROM fdo_HouseNumbers WHERE TEXT_LABEL LIKE '11%'";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -3;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -4;
      }
    if (strcmp
	(results[1], "MULTIPOINT(430417.1 5448290.9, 430666.6 5448125.4)") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result |%s|\n",
		   results[1]);
	  return -5;
      }
    sqlite3_free_table (results);

/* testing Sum(GLength()) Linestring FGF "Centrerlines" */
    sql = "SELECT Sum(GLength(Geometry)) FROM fdo_Centerlines";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -7;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -8;
      }
    if (strncmp (results[1], "895.3351", 7) != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -9;
      }
    sqlite3_free_table (results);

/* testing Sum(Area()) Polygon FGF "AssessmentParcels" */
    sql = "SELECT Sum(Area(Geometry)) FROM fdo_AssessmentParcels";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -11;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -12;
      }
    if (strncmp (results[1], "9022.1792", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -13;
      }
    sqlite3_free_table (results);

/* FDO shut-down */
    sql = "SELECT AutoFDOStop()";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -15;
      }

    spatialite_cleanup_ex (cache);
    ret = unlink ("testFGF.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot remove testFGF database\n");
	  return -16;
      }
#endif /* end GEOS conditional */

    spatialite_shutdown ();
    return 0;
}
