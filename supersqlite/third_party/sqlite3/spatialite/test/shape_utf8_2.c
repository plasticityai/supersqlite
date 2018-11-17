/*

 shape_utf8_2.c -- SpatiaLite Test Case

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
Ahmadou Dicko <dicko.ahmadou@gmail.com>

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
#include <unistd.h>
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
    char *kmlname = __FILE__ "test.kml";
    char *geojsonname = __FILE__ "test.geojson";
    char *err_msg = NULL;
    int row_count;
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

    ret =
	load_shapefile (handle, "./shp/taiwan/hystoric", "hystoric", "UTF-8",
			4326, "col1", 1, 0, 1, 0, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr,
		   "load_shapefile() error for shp/taiwan/hystoric: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -3;
      }
    if (row_count != 15)
      {
	  fprintf (stderr, "unexpected row count for shp/taiwan/hystoric: %i\n",
		   row_count);
	  sqlite3_close (handle);
	  return -4;
      }

    ret =
	load_shapefile (handle, "./shp/taiwan/leisure", "leisure", "UTF-8",
			4326, "col1", 1, 0, 1, 0, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr,
		   "load_shapefile() error for shp/taiwan/leisure: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -5;
      }
    if (row_count != 5)
      {
	  fprintf (stderr, "unexpected row count for shp/taiwan/leisure: %i\n",
		   row_count);
	  sqlite3_close (handle);
	  return -6;
      }

    ret = load_shapefile (handle, "./shp/taiwan/route", "route", "UTF-8", 4326,
			  "col1", 1, 0, 1, 0, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "load_shapefile() error for shp/taiwan/route: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -7;
      }
    if (row_count != 4)
      {
	  fprintf (stderr, "unexpected row count for shp/taiwan/route: %i\n",
		   row_count);
	  sqlite3_close (handle);
	  return -8;
      }

#ifndef OMIT_PROJ		/* only if PROJ is supported */
    if (is_kml_constant (handle, "route", "name"))
      {
	  fprintf (stderr, "unexpected result for is_kml_constant (1)\n");
	  return -9;
      }
    if (!is_kml_constant (handle, "route", "foo"))
      {
	  fprintf (stderr, "unexpected result for is_kml_constant (2)\n");
	  return -10;
      }

    ret = dump_kml (handle, "route", "col1", kmlname, NULL, NULL, 10);
    if (!ret)
      {
	  fprintf (stderr, "dump_kml (1) error for shp/taiwan/route: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -11;
      }
    unlink (kmlname);

    ret = dump_kml (handle, "route", "col1", kmlname, "name", NULL, 10);
    if (!ret)
      {
	  fprintf (stderr, "dump_kml (2) error for shp/taiwan/route: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -12;
      }
    unlink (kmlname);

    ret = dump_kml (handle, "route", "col1", kmlname, "theta", NULL, 10);
    if (!ret)
      {
	  fprintf (stderr, "dump_kml (3) error for shp/taiwan/route: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -13;
      }
    unlink (kmlname);

    ret = dump_kml (handle, "route", "col1", kmlname, "name", "sub_type", 10);
    if (!ret)
      {
	  fprintf (stderr, "dump_kml (4) error for shp/taiwan/route: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -14;
      }
    unlink (kmlname);

    ret = dump_kml (handle, "route", "col1", kmlname, "theta", "beta", 10);
    if (!ret)
      {
	  fprintf (stderr, "dump_kml (5) error for shp/taiwan/route: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -15;
      }
    unlink (kmlname);
#endif /* end PROJ conditional */

    ret = dump_geojson (handle, "route", "col1", geojsonname, 10, 5);
    if (!ret)
      {
	  fprintf (stderr, "dump_geojson() error for shp/taiwan/route: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -16;
      }
    unlink (geojsonname);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -17;
      }

    spatialite_cleanup_ex (cache);
#endif /* end ICONV conditional */

    spatialite_shutdown ();
    return 0;
}
