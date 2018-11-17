/*

 check_fdo1.c -- SpatiaLite Test Case

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

static int
do_test (int legacy_mode)
{
#ifndef OMIT_GEOS		/* only if GEOS is supported */
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    const char *sql;
    int i;
    char **results;
    int rows;
    int columns;
    void *cache = NULL;
    if (!legacy_mode)
	cache = spatialite_alloc_connection ();
    else
	spatialite_init (0);

    ret = system ("cp sql_stmt_tests/testFDO.sqlite testFDO.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot copy testFDO.sqlite database\n");
	  return -1001;
      }

    ret =
	sqlite3_open_v2 ("testFDO.sqlite", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open testFDO.sqlite db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1000;
      }

    if (!legacy_mode)
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

/* testing aggregate Union() PointZ WKT "p02" */
    sql = "SELECT AsText(ST_Union(WKT_GEOMETRY)) FROM fdo_p02";
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
	(results[1],
	 "MULTIPOINT Z(664350.17954 5171957.915655 314.52, 664642.363686 5169415.339218 294.37, 664964.447225 5170571.245732 318.25)")
	!= 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result %s\n", results[1]);
	  return -5;
      }
    sqlite3_free_table (results);

/* testing aggregate Union() PointZ WKB "p03" */
    sql =
	"SELECT AsText(ST_Union(GEOMETRY)) FROM fdo_p03 WHERE text_dil IS NULL AND OGC_FID < 3";
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
    if (strcmp
	(results[1],
	 "MULTIPOINT Z(665216.306643 5169825.707161 296.06, 665224.506512 5169827.907054 296.16)")
	!= 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result |%s|\n",
		   results[1]);
	  return -9;
      }
    sqlite3_free_table (results);

/* testing aggregate Union() PointZ SpatiaLite "p05" */
    sql =
	"SELECT AsText(ST_Union(GEOMETRY)) FROM fdo_p05 WHERE text_dil IS NULL AND OGC_FID < 3";
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
	  fprintf (stderr, "Unexpected error: NULL result %s\n", results[1]);
	  return -12;
      }
    if (strcmp
	(results[1],
	 "MULTIPOINT Z(667687.978175 5169352.045712 583.140015, 667710.008189 5169402.894615 589.849976)")
	!= 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result b|%s|\n",
		   results[1]);
	  return -13;
      }
    sqlite3_free_table (results);

/* testing Sum(GLength()) LinestringZ WKT "l05" */
    sql = "SELECT Sum(GLength(WKT_GEOMETRY)) FROM fdo_l05";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -15;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -16;
      }
    if (strncmp (results[1], "59.417763", 9) != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -17;
      }
    sqlite3_free_table (results);

/* testing Sum(GLength()) LinestringZ WKB "l06" */
    sql = "SELECT Sum(GLength(GEOMETRY)) FROM fdo_l06";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -18;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -19;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -20;
      }
    if (strncmp (results[1], "273.076064", 10) != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -21;
      }
    sqlite3_free_table (results);

/* testing Sum(GLength()) LinestringZ SpatiaLite "l07" */
    sql = "SELECT Sum(GLength(GEOMETRY)) FROM fdo_l07";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -23;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -24;
      }
    if (strncmp (results[1], "219.459808", 10) != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -25;
      }
    sqlite3_free_table (results);

/* testing Sum(Area()) PolygonZ WKT "f04" */
    sql = "SELECT Sum(Area(WKT_GEOMETRY)) FROM fdo_f04";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -26;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -27;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -28;
      }
    if (strncmp (results[1], "9960.931239", 11) != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -29;
      }
    sqlite3_free_table (results);

/* testing Sum(Area()) PolygonZ WKB "f05" */
    sql = "SELECT Sum(Area(GEOMETRY)) FROM fdo_f05";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -31;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -32;
      }
    if (strncmp (results[1], "69972.113393", 12) != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -33;
      }
    sqlite3_free_table (results);

/* testing Sum(Area()) PolygonZ SpatiaLite "f06" */
    sql = "SELECT Sum(Area(GEOMETRY)) FROM fdo_f06";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -34;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -35;
      }
    if (results[1] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -36;
      }
    if (strncmp (results[1], "1125.064396", 11) != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -37;
      }
    sqlite3_free_table (results);

/* testing IsValid() LinestringZ WKB "l06" */
    sql = "SELECT IsValid(GEOMETRY) FROM fdo_l06";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -38;
      }
    if ((rows != 12) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -39;
      }
    for (i = 1; i <= rows; i++)
      {
	  if (results[i] == NULL)
	    {
		fprintf (stderr, "Unexpected error: NULL result\n");
		return -40;
	    }
	  if (strcmp (results[i], "0") == 0)
	    {
		const char *geos_msg;
		if (legacy_mode)
		    geos_msg = gaiaGetGeosErrorMsg ();
		else
		    geos_msg = gaiaGetGeosErrorMsg_r (cache);
		if (geos_msg == NULL)
		  {
		      if (legacy_mode)
			  geos_msg = gaiaGetGeosWarningMsg ();
		      else
			  geos_msg = gaiaGetGeosWarningMsg_r (cache);
		  }
		if (geos_msg == NULL)
		  {
		      fprintf (stderr, "Unexpected error: invalid result\n");
		      return -41;
		  }
	    }
      }
    sqlite3_free_table (results);

/* testing IsValid() PolygonZ WKT "f04" */
    sql = "SELECT IsValid(WKT_GEOMETRY) FROM fdo_f04";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -42;
      }
    if ((rows != 16) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -43;
      }
    for (i = 1; i <= rows; i++)
      {
	  if (results[i] == NULL)
	    {
		fprintf (stderr, "Unexpected error: NULL result\n");
		return -44;
	    }
	  if (strcmp (results[i], "0") == 0)
	    {
		const char *geos_msg;
		if (legacy_mode)
		    geos_msg = gaiaGetGeosErrorMsg ();
		else
		    geos_msg = gaiaGetGeosErrorMsg_r (cache);
		if (geos_msg == NULL)
		  {
		      if (legacy_mode)
			  geos_msg = gaiaGetGeosWarningMsg ();
		      else
			  geos_msg = gaiaGetGeosWarningMsg_r (cache);
		  }
		if (geos_msg == NULL)
		  {
		      fprintf (stderr, "Unexpected error: invalid result\n");
		      return -45;
		  }
	    }
      }
    sqlite3_free_table (results);

/* testing IsValid() PolygonZ WKB "f05" */
    sql = "SELECT IsValid(GEOMETRY) FROM fdo_f05";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -46;
      }
    if ((rows != 13) || (columns != 1))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -47;
      }
    for (i = 1; i <= rows; i++)
      {
	  if (results[i] == NULL)
	    {
		fprintf (stderr, "Unexpected error: NULL result\n");
		return -48;
	    }
	  if (strcmp (results[i], "0") == 0)
	    {
		const char *geos_msg;
		if (legacy_mode)
		    geos_msg = gaiaGetGeosErrorMsg ();
		else
		    geos_msg = gaiaGetGeosErrorMsg_r (cache);
		if (geos_msg == NULL)
		  {
		      if (legacy_mode)
			  geos_msg = gaiaGetGeosWarningMsg ();
		      else
			  geos_msg = gaiaGetGeosWarningMsg_r (cache);
		  }
		if (geos_msg == NULL)
		  {
		      fprintf (stderr, "Unexpected error: invalid result\n");
		      return -49;
		  }
	    }
      }
    sqlite3_free_table (results);

/* testing DOUBLE and TEXT columns */
    sql = "SELECT datum, hoehe FROM fdo_p05 WHERE OGC_FID = 5";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -50;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr, "Unexpected error: bad result: %i/%i.\n", rows,
		   columns);
	  return -51;
      }
    if (results[2] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -52;
      }
    if (strcmp (results[2], "1997/03/07") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -53;
      }
    if (results[3] == NULL)
      {
	  fprintf (stderr, "Unexpected error: NULL result\n");
	  return -54;
      }
    if (strcmp (results[3], "277.55") != 0)
      {
	  fprintf (stderr, "Unexpected error: invalid result\n");
	  return -54;
      }
    sqlite3_free_table (results);

/* FDO shut-down */
    sql = "SELECT AutoFDOStop()";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -55;
      }

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -56;
      }

    spatialite_cleanup_ex (cache);
    ret = unlink ("testFDO.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot remove testFDO database\n");
	  return -57;
      }
#endif /* end GEOS conditional */

    return 0;
}

int
main (int argc, char *argv[])
{
    int ret;

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    fprintf (stderr, "********* testing in current mode\n");
    ret = do_test (0);
    if (ret != 0)
	return ret;

    fprintf (stderr, "********* testing in legacy mode\n");
    ret = do_test (1);
    if (ret != 0)
	return ret;

    spatialite_shutdown ();
    return 0;
}
