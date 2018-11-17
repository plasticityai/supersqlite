/*

 check_mbrcache.c -- SpatiaLite Test Case

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
#ifndef OMIT_ICONV		/* only if ICONV is supported */
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    int row_count;
    char **results;
    int rows;
    int columns;
    int pt;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory database: %s\n",
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

    ret = load_shapefile (handle, "shp/foggia/local_councils", "Councils",
			  "CP1252", 23032, "geom", 1, 0, 1, 0, &row_count,
			  err_msg);
    if (!ret)
      {
	  fprintf (stderr, "load_shapefile() error: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -3;
      }
    if (row_count != 61)
      {
	  fprintf (stderr, "unexpected number of rows loaded: %i\n", row_count);
	  sqlite3_close (handle);
	  return -4;
      }

    ret =
	sqlite3_get_table (handle,
			   "SELECT lc_name FROM Councils WHERE MbrWithin(geom, BuildMbr(1040523, 4010000, 1140523, 4850000)) ORDER BY lc_name;",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }
    if ((rows != 22) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select lc_name bad result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -6;
      }
    if (strcmp (results[1], "Ascoli Satriano") != 0)
      {
	  fprintf (stderr, "unexpected result at 1: %s\n", results[1]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -7;
      }
    if (strcmp (results[22], "Zapponeta") != 0)
      {
	  fprintf (stderr, "unexpected result at 22: %s\n", results[22]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -8;
      }
    sqlite3_free_table (results);

    ret = sqlite3_exec (handle, "SELECT CreateMbrCache('Councils', 'geom');",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CreateMbrCache error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }

    rows = 0;
    columns = 0;
    ret =
	sqlite3_get_table (handle,
			   "SELECT lc_name FROM Councils WHERE ROWID IN (SELECT rowid FROM cache_Councils_geom WHERE mbr = FilterMbrWithin(1040523, 4010000, 1140523, 4850000)) ORDER BY lc_name;",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error in Mbr SELECT: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }
    if ((rows != 22) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select lc_name bad cache result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -15;
      }
    if (strcmp (results[1], "Ascoli Satriano") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 1: %s\n", results[1]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -16;
      }
    if (strcmp (results[22], "Zapponeta") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 22: %s\n", results[22]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -17;
      }
    sqlite3_free_table (results);

    rows = 0;
    columns = 0;
    ret =
	sqlite3_get_table (handle,
			   "SELECT lc_name FROM Councils WHERE ROWID IN (SELECT rowid FROM cache_Councils_geom WHERE mbr = FilterMbrContains(997226.750031, 4627372.000018, 997226.750031, 4627372.000018)) ORDER BY lc_name;",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error in Mbr SELECT Contains: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -18;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select lc_name bad cache result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -19;
      }
    if (strcmp (results[1], "Carlantino") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 1: %s\n", results[1]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -20;
      }
    sqlite3_free_table (results);

    rows = 0;
    columns = 0;
    ret =
	sqlite3_get_table (handle,
			   "SELECT lc_name FROM Councils WHERE ROWID IN (SELECT rowid FROM cache_Councils_geom WHERE mbr = FilterMbrIntersects(1040523, 4010000, 1140523, 4850000)) ORDER BY lc_name;",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error in Mbr SELECT: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }
    if ((rows != 35) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select lc_name bad cache result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -22;
      }
    if (strcmp (results[1], "Apricena") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 1: %s\n", results[1]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -23;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (handle,
		      "DELETE FROM Councils WHERE lc_name = 'Zapponeta';", NULL,
		      NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DELETE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }

    rows = 0;
    columns = 0;
    ret =
	sqlite3_get_table (handle,
			   "SELECT lc_name FROM Councils WHERE ROWID IN (SELECT rowid FROM cache_Councils_geom WHERE mbr = FilterMbrWithin(1040523, 4010000, 1140523, 4850000)) ORDER BY lc_name;",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error in Mbr SELECT: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }
    if ((rows != 21) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select lc_name bad cache result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -26;
      }
    if (strcmp (results[1], "Ascoli Satriano") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 1: %s\n", results[1]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -27;
      }
    if (strcmp (results[21], "Vieste") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 21: %s\n", results[21]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -28;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Councils (lc_name, geom) VALUES ('Quairading', GeomFromText('MULTIPOLYGON(((997226.750031 4627372.000018, 997301.750031 4627332.000018, 997402.500031 4627344.000018, 997541.500031 4627326.500018,997226.750031 4627372.000018)))', 23032));",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }

    rows = 0;
    columns = 0;
    ret =
	sqlite3_get_table (handle,
			   "SELECT lc_name FROM Councils WHERE ROWID IN (SELECT rowid FROM cache_Councils_geom WHERE mbr = FilterMbrWithin(1040523, 4010000, 1140523, 4850000)) ORDER BY lc_name;",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error in Mbr SELECT2: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }
    if ((rows != 21) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select lc_name bad cache result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -31;
      }
    if (strcmp (results[1], "Ascoli Satriano") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 1: %s\n", results[1]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -32;
      }
    if (strcmp (results[21], "Vieste") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 21: %s\n", results[21]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -33;
      }
    sqlite3_free_table (results);

    ret =
	sqlite3_exec (handle,
		      "UPDATE Councils SET geom = GeomFromText('MULTIPOLYGON(((987226.750031 4627372.000018, 997301.750031 4627331.000018, 997402.500032 4627344.000018, 997541.500031 4627326.500018, 987226.750031 4627372.000018)))', 23032) WHERE lc_name = \"Quairading\";",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UPDATE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -34;
      }

    rows = 0;
    columns = 0;
    ret =
	sqlite3_get_table (handle,
			   "SELECT lc_name FROM Councils WHERE ROWID IN (SELECT rowid FROM cache_Councils_geom WHERE mbr = FilterMbrWithin(1040523, 4010000, 1140523, 4850000)) ORDER BY lc_name;",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error in Mbr SELECT3: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -35;
      }
    if ((rows != 21) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select lc_name bad cache result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -36;
      }
    if (strcmp (results[1], "Ascoli Satriano") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 1: %s\n", results[1]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -37;
      }
    if (strcmp (results[21], "Vieste") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 21: %s\n", results[21]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -38;
      }
    sqlite3_free_table (results);

    rows = 0;
    columns = 0;
    ret =
	sqlite3_get_table (handle,
			   "SELECT rowid, mbr FROM cache_Councils_geom;",
			   &results, &rows, &columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error in Mbr SELECT: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -39;
      }
    if ((rows != 61) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select lc_name bad cache2 result: %i/%i.\n",
		   rows, columns);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -40;
      }
    if (strcmp (results[2], "1") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 1: %s\n", results[2]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -41;
      }
    if (strcmp (results[12], "6") != 0)
      {
	  fprintf (stderr, "unexpected mbr result at 6: %s\n", results[12]);
	  sqlite3_free_table (results);
	  sqlite3_close (handle);
	  return -42;
      }
    sqlite3_free_table (results);

    ret = sqlite3_exec (handle, "DROP TABLE Councils;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE Councils error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -44;
      }

/* creating and feeding a Point table */
    ret =
	sqlite3_exec (handle, "CREATE TABLE pt (id INTEGER);", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE pt error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -45;
      }
    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('pt', 'g', 4326, 1, 'XY');",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn pt error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -46;
      }
    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('pt', 'g', 4326, 'POINT', 2.5);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn pt error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -47;
      }
    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('pt', 'g', 4326, 'POINT', 'XY', 0.5);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn pt error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -48;
      }
    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('pt', 'g', 4326, 'DUMMY', 'XY');",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn pt error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -49;
      }
    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('pt', 'g', 4326, 'POINT', 'DUMMY');",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn pt error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -50;
      }
    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('pt', 'g', 4326, 'POINT', 2);",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn pt error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -51;
      }
    ret =
	sqlite3_exec (handle, "SELECT CreateMbrCache('pt', 'g');", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CreateMbrCache pt.g error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -52;
      }
    for (pt = 0; pt < 10000; pt++)
      {
	  /* inserting Points */
	  char sql[1024];
	  sprintf (sql,
		   "INSERT INTO pt (id, g) VALUES (%d, GeomFromText('POINT(%1.2f %1.2f)', 4326));",
		   pt, 11.0 + (pt / 10000.0), 43.0 + (pt / 10000.0));
	  ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "INSERT INTO pt error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return -53;
	    }
      }
    for (pt = 5000; pt < 6000; pt++)
      {
	  /* updating Points */
	  char sql[1024];
	  sprintf (sql,
		   "UPDATE pt SET g  = GeomFromText('POINT(%1.2f %1.2f)', 4326) WHERE id = %d;",
		   12.0 + (pt / 10000.0), 42.0 + (pt / 10000.0), pt);
	  ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "UPDATE pt error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return -54;
	    }
      }
    for (pt = 7000; pt < 8000; pt++)
      {
	  /* deleting Points */
	  char sql[1024];
	  sprintf (sql, "DELETE FROM pt WHERE id = %d;", pt);
	  ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
	  if (ret != SQLITE_OK)
	    {
		fprintf (stderr, "UPDATE pt error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return -55;
	    }
      }

    ret = sqlite3_exec (handle, "SELECT CreateMbrCache(1, 'geom');",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "invalid CreateMbrCache error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -56;
      }
    ret = sqlite3_exec (handle, "SELECT CreateMbrCache('Councils', 2);",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "invalid CreateMbrCache error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -57;
      }
    ret = sqlite3_exec (handle, "SELECT FilterMbrWithin('a', 2, 3, 4);",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "invalid FilterMbrWithin error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -58;
      }
    ret = sqlite3_exec (handle, "SELECT FilterMbrWithin(1, 'a', 3, 4);",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "invalid FilterMbrWithin error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -59;
      }
    ret = sqlite3_exec (handle, "SELECT FilterMbrWithin(1, 2, 'a', 4);",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "invalid FilterMbrWithin error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -60;
      }
    ret = sqlite3_exec (handle, "SELECT FilterMbrWithin(1, 2, 3, 'a');",
			NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "invalid FilterMbrWithin error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -61;
      }

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -62;
      }

    spatialite_cleanup_ex (cache);
#endif /* end ICONV conditional */

    return 0;
}
