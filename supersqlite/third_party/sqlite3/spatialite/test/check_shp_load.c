/*

 check_shp_load.c -- SpatiaLite Test Case

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

static int
do_test (sqlite3 * handle, const void *p_cache)
{
    int ret;
    char *err_msg = NULL;
    int row_count;

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

    ret = load_shapefile (handle, "./shapetest1", "test1", "UTF-8", 4326,
			  "col1", 1, 0, 1, 0, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "load_shapefile() error: %s\n", err_msg);
	  sqlite3_close (handle);
	  return -3;
      }

#ifdef ENABLE_LWGEOM		/* only if LWGEOM is supported */

    if (p_cache == NULL)
	ret =
	    check_geometry_column (handle, "test1", "geometry", "./report.html",
				   NULL, NULL, NULL);
    else
	ret =
	    check_geometry_column_r (p_cache, handle, "test1", "geometry",
				     "./report.html", NULL, NULL, NULL);
    if (ret)
      {
	  fprintf (stderr, "check_geometry_column() error\n");
	  sqlite3_close (handle);
	  return -4;
      }

    if (p_cache == NULL)
	ret =
	    sanitize_geometry_column (handle, "test1", "geometry", "tmp_test1",
				      "./report.html", NULL, NULL, NULL, NULL,
				      NULL);
    else
	ret =
	    sanitize_geometry_column_r (p_cache, handle, "test1", "geometry",
					"tmp_test1", "./report.html", NULL,
					NULL, NULL, NULL, NULL);
    if (ret)
      {
	  fprintf (stderr, "sanitize_geometry_column() error\n");
	  sqlite3_close (handle);
	  return -5;
      }

    if (p_cache == NULL)
	ret =
	    check_geometry_column (handle, "test1", "col1", "./report.html",
				   NULL, NULL, NULL);
    else
	ret =
	    check_geometry_column_r (p_cache, handle, "test1", "col1",
				     "./report.html", NULL, NULL, NULL);
    if (!ret)
      {
	  fprintf (stderr, "check_geometry_column() error\n");
	  sqlite3_close (handle);
	  return -6;
      }

    if (p_cache == NULL)
	ret =
	    sanitize_geometry_column (handle, "test1", "col1", "tmp_test1",
				      "./report.html", NULL, NULL, NULL, NULL,
				      NULL);
    else
	ret =
	    sanitize_geometry_column_r (p_cache, handle, "test1", "col1",
					"tmp_test1", "./report.html", NULL,
					NULL, NULL, NULL, NULL);
    if (!ret)
      {
	  fprintf (stderr, "sanitize_geometry_column() error\n");
	  sqlite3_close (handle);
	  return -7;
      }

#endif /* end LWGEOM conditionals */
    return 0;
}

int
main (int argc, char *argv[])
{
#ifndef OMIT_ICONV		/* only if ICONV is supported */
    int ret;
    sqlite3 *handle;
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

    ret = do_test (handle, cache);
    if (ret != 0)
	return ret;

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -8;
      }

    spatialite_cleanup_ex (cache);

/* testing again in legacy mode */
    spatialite_init (0);
    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory database: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -9;
      }

    ret = do_test (handle, NULL);
    if (ret != 0)
	return ret;

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -10;
      }

    spatialite_cleanup ();
#endif /* end ICONV conditional */

    spatialite_shutdown ();
    return 0;
}
