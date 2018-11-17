/*

 check_srid_fncts.c -- SpatiaLite Test Case

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

static int
test_srid (sqlite3 * sqlite, int extended)
{
/* testing all SRID functions */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt = NULL;

    sql = "SELECT srid, SridIsGeographic(srid), SridIsProjected(srid), "
	"SridHasFlippedAxes(srid), SridGetSpheroid(srid), SridGetPrimeMeridian(srid), "
	"SridGetDatum(srid), SridGetUnit(srid), SridGetProjection(srid), "
	"SridGetAxis_1_Name(srid), SridGetAxis_1_Orientation(srid), "
	"SridGetAxis_2_Name(srid), SridGetAxis_2_Orientation(srid) "
	"FROM spatial_ref_sys";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "SELECT FROM spatial_ref_sys: \"%s\"\n",
		   sqlite3_errmsg (sqlite));
	  return 0;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		int flag;
		const char *value;
		int srid = sqlite3_column_int (stmt, 0);
		if (srid == 4326)
		  {
		      flag = sqlite3_column_int (stmt, 1);
		      if (flag != 1)
			{
			    fprintf (stderr,
				     "4326: Unexpected IsGeographic result (%d)\n",
				     flag);
			    goto error;
			}
		      flag = sqlite3_column_int (stmt, 2);
		      if (flag != 0)
			{
			    fprintf (stderr,
				     "4326: Unexpected IsProjected result (%d)\n",
				     flag);
			    goto error;
			}
		      flag = sqlite3_column_int (stmt, 3);
		      if (flag != 1)
			{
			    fprintf (stderr,
				     "4326: Unexpected HasFlippedAxes result (%d)\n",
				     flag);
			    goto error;
			}
		      if (extended)
			{
			    value =
				(const char *) sqlite3_column_text (stmt, 4);
			    if (strcasecmp (value, "WGS 84") != 0)
			      {
				  fprintf (stderr,
					   "4326: Unexpected GetSpheroid result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 5);
			    if (strcasecmp (value, "Greenwich") != 0)
			      {
				  fprintf (stderr,
					   "4326: Unexpected GetPrimeMeridian result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 6);
			    if (strcasecmp (value, "WGS_1984") != 0)
			      {
				  fprintf (stderr,
					   "4326: Unexpected GetDatum result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 7);
			    if (strcasecmp (value, "degree") != 0)
			      {
				  fprintf (stderr,
					   "4326: Unexpected GetUnit result (%s)\n",
					   value);
				  goto error;
			      }
			    if (sqlite3_column_type (stmt, 8) == SQLITE_TEXT)
			      {
				  value =
				      (const char *) sqlite3_column_text (stmt,
									  8);
				  if (strcasecmp (value, "none") != 0)
				    {
					fprintf (stderr,
						 "4326: Unexpected GetProjection result (%s)\n",
						 value);
					goto error;
				    }
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 9);
			    if (strcasecmp (value, "Latitude") != 0)
			      {
				  fprintf (stderr,
					   "4326: Unexpected GetAxis_1_Name result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 10);
			    if (strcasecmp (value, "North") != 0)
			      {
				  fprintf (stderr,
					   "4326: Unexpected GetAxis_1_Orientation result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 11);
			    if (strcasecmp (value, "Longitude") != 0)
			      {
				  fprintf (stderr,
					   "4326: Unexpected GetAxis_2_Name result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 12);
			    if (strcasecmp (value, "East") != 0)
			      {
				  fprintf (stderr,
					   "4326: Unexpected GetAxis_2_Orientation result (%s)\n",
					   value);
				  goto error;
			      }
			}
		  }
		if (srid == 32632)
		  {
		      flag = sqlite3_column_int (stmt, 1);
		      if (flag != 0)
			{
			    fprintf (stderr,
				     "32632: Unexpected IsGeographic result (%d)\n",
				     flag);
			    goto error;
			}
		      flag = sqlite3_column_int (stmt, 2);
		      if (flag != 1)
			{
			    fprintf (stderr,
				     "32632: Unexpected IsProjected result (%d)\n",
				     flag);
			    goto error;
			}
		      flag = sqlite3_column_int (stmt, 3);
		      if (flag != 0)
			{
			    fprintf (stderr,
				     "32632: Unexpected HasFlippedAxes result (%d)\n",
				     flag);
			    goto error;
			}
		      if (extended)
			{
			    value =
				(const char *) sqlite3_column_text (stmt, 4);
			    if (strcasecmp (value, "WGS 84") != 0)
			      {
				  fprintf (stderr,
					   "32632: Unexpected GetSpheroid result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 5);
			    if (strcasecmp (value, "Greenwich") != 0)
			      {
				  fprintf (stderr,
					   "32632: Unexpected GetPrimeMeridian result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 6);
			    if (strcasecmp (value, "WGS_1984") != 0)
			      {
				  fprintf (stderr,
					   "32632: Unexpected GetDatum result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 7);
			    if (strcasecmp (value, "metre") != 0)
			      {
				  fprintf (stderr,
					   "32632: Unexpected GetUnit result (%s)\n",
					   value);
				  goto error;
			      }
			    if (sqlite3_column_type (stmt, 8) == SQLITE_TEXT)
			      {
				  value =
				      (const char *) sqlite3_column_text (stmt,
									  8);
				  if (strcasecmp (value, "Transverse_Mercator")
				      != 0)
				    {
					fprintf (stderr,
						 "32632: Unexpected GetProjection result (%s)\n",
						 value);
					goto error;
				    }
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 9);
			    if (strcasecmp (value, "Easting") != 0)
			      {
				  fprintf (stderr,
					   "32632: Unexpected GetAxis_1_Name result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 10);
			    if (strcasecmp (value, "East") != 0)
			      {
				  fprintf (stderr,
					   "32632: Unexpected GetAxis_1_Orientation result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 11);
			    if (strcasecmp (value, "Northing") != 0)
			      {
				  fprintf (stderr,
					   "32632: Unexpected GetAxis_2_Name result (%s)\n",
					   value);
				  goto error;
			      }
			    value =
				(const char *) sqlite3_column_text (stmt, 12);
			    if (strcasecmp (value, "North") != 0)
			      {
				  fprintf (stderr,
					   "32632: Unexpected GetAxis_2_Orientation result (%s)\n",
					   value);
				  goto error;
			      }
			}
		  }
	    }
	  else
	      goto error;
      }
    sqlite3_finalize (stmt);
    return 1;

  error:
    if (stmt != NULL)
	sqlite3_finalize (stmt);
    return 0;
}

int
main (int argc, char *argv[])
{
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
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

/* Step #1: testing via "spatial_ref_sys_aux" */
    ret = test_srid (db_handle, 1);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -3;
      }

    ret =
	sqlite3_exec (db_handle, "DROP VIEW spatial_ref_sys_all", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP VIEW error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -4;
      }
    ret =
	sqlite3_exec (db_handle, "DROP TABLE spatial_ref_sys_aux", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP VIEW error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -4;
      }

/* Step #2: testing via WKT */
    ret = test_srid (db_handle, 1);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -5;
      }

    ret =
	sqlite3_exec (db_handle, "UPDATE spatial_ref_sys SET srtext = ' '",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "UPDATE WKT error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (db_handle);
	  return -6;
      }

/* Step #3: testing via proj4 */
    ret = test_srid (db_handle, 0);
    if (!ret)
      {
	  sqlite3_close (db_handle);
	  return -7;
      }

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
