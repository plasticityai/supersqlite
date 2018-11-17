/*

 srs_init.c -- populating the SPATIAL_REF_SYS table

 version 4.3, 2015 June 29

 Author: Sandro Furieri a.furieri@lqt.it

 -----------------------------------------------------------------------------
 
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
 
Portions created by the Initial Developer are Copyright (C) 2008-2015
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

#include <spatialite/sqlite.h>
#include <spatialite/debug.h>

#ifdef _WIN32
#define strcasecmp	_stricmp
#endif /* not WIN32 */

#include <spatialite.h>
#include <spatialite_private.h>

static void
free_epsg_def (struct epsg_defs *ptr)
{
/* memory cleanup - destroying an EPSG def item */
    if (ptr->auth_name)
	free (ptr->auth_name);
    if (ptr->ref_sys_name)
	free (ptr->ref_sys_name);
    if (ptr->proj4text)
	free (ptr->proj4text);
    if (ptr->srs_wkt)
	free (ptr->srs_wkt);
    if (ptr->spheroid)
	free (ptr->spheroid);
    if (ptr->prime_meridian)
	free (ptr->prime_meridian);
    if (ptr->datum)
	free (ptr->datum);
    if (ptr->projection)
	free (ptr->projection);
    if (ptr->unit)
	free (ptr->unit);
    if (ptr->axis_1)
	free (ptr->axis_1);
    if (ptr->orientation_1)
	free (ptr->orientation_1);
    if (ptr->axis_2)
	free (ptr->axis_2);
    if (ptr->orientation_2)
	free (ptr->orientation_2);
    free (ptr);
}

SPATIALITE_PRIVATE struct epsg_defs *
add_epsg_def (int filter_srid, struct epsg_defs **first,
	      struct epsg_defs **last, int srid, const char *auth_name,
	      int auth_srid, const char *ref_sys_name)
{
/* appending an EPSG def to the list */
    int len;
    struct epsg_defs *p;
    if (filter_srid == GAIA_EPSG_NONE)
	return NULL;
    if (filter_srid == GAIA_EPSG_ANY || filter_srid == GAIA_EPSG_WGS84_ONLY)
	;
    else if (srid != filter_srid)
	return NULL;
    p = malloc (sizeof (struct epsg_defs));
    if (!p)
	return NULL;
    p->srid = srid;
    p->auth_name = NULL;
    p->auth_srid = auth_srid;
    p->ref_sys_name = NULL;
    p->proj4text = NULL;
    p->srs_wkt = NULL;
    p->next = NULL;
    if (auth_name)
      {
	  len = strlen (auth_name);
	  if (len > 0)
	    {
		p->auth_name = malloc (len + 1);
		if (p->auth_name == NULL)
		    goto error;
		strcpy (p->auth_name, auth_name);
	    }
      }
    if (ref_sys_name)
      {
	  len = strlen (ref_sys_name);
	  if (len > 0)
	    {
		p->ref_sys_name = malloc (len + 1);
		if (p->ref_sys_name == NULL)
		    goto error;
		strcpy (p->ref_sys_name, ref_sys_name);
	    }
      }
    p->is_geographic = -1;
    p->flipped_axes = -1;
    p->spheroid = NULL;
    p->prime_meridian = NULL;
    p->datum = NULL;
    p->projection = NULL;
    p->unit = NULL;
    p->axis_1 = NULL;
    p->orientation_1 = NULL;
    p->axis_2 = NULL;
    p->orientation_2 = NULL;
    if (*first == NULL)
	*first = p;
    if (*last != NULL)
	(*last)->next = p;
    *last = p;
    return p;
  error:
    free_epsg_def (p);
    return NULL;
}

SPATIALITE_PRIVATE struct epsg_defs *
add_epsg_def_ex (int filter_srid, struct epsg_defs **first,
		 struct epsg_defs **last, int srid, const char *auth_name,
		 int auth_srid, const char *ref_sys_name, int is_geographic,
		 int flipped_axes, const char *spheroid,
		 const char *prime_meridian, const char *datum,
		 const char *projection, const char *unit, const char *axis_1,
		 const char *orientation_1, const char *axis_2,
		 const char *orientation_2)
{
/* appending an EPSG def to the list */
    int len;
    struct epsg_defs *p;
    if (filter_srid == GAIA_EPSG_NONE)
	return NULL;
    if (filter_srid == GAIA_EPSG_ANY || filter_srid == GAIA_EPSG_WGS84_ONLY)
	;
    else if (srid != filter_srid)
	return NULL;
    p = malloc (sizeof (struct epsg_defs));
    if (!p)
	return NULL;
    p->srid = srid;
    p->auth_name = NULL;
    p->auth_srid = auth_srid;
    p->ref_sys_name = NULL;
    p->proj4text = NULL;
    p->srs_wkt = NULL;
    p->spheroid = NULL;
    p->prime_meridian = NULL;
    p->datum = NULL;
    p->unit = NULL;
    p->axis_1 = NULL;
    p->orientation_1 = NULL;
    p->axis_2 = NULL;
    p->orientation_2 = NULL;
    p->next = NULL;
    if (auth_name)
      {
	  len = strlen (auth_name);
	  if (len > 0)
	    {
		p->auth_name = malloc (len + 1);
		if (p->auth_name == NULL)
		    goto error;
		strcpy (p->auth_name, auth_name);
	    }
      }
    if (ref_sys_name)
      {
	  len = strlen (ref_sys_name);
	  if (len > 0)
	    {
		p->ref_sys_name = malloc (len + 1);
		if (p->ref_sys_name == NULL)
		    goto error;
		strcpy (p->ref_sys_name, ref_sys_name);
	    }
      }
    p->is_geographic = is_geographic;
    p->flipped_axes = flipped_axes;
    if (spheroid)
      {
	  len = strlen (spheroid);
	  p->spheroid = malloc (len + 1);
	  if (p->spheroid == NULL)
	      goto error;
	  strcpy (p->spheroid, spheroid);
      }
    if (prime_meridian)
      {
	  len = strlen (prime_meridian);
	  p->prime_meridian = malloc (len + 1);
	  if (p->prime_meridian == NULL)
	      goto error;
	  strcpy (p->prime_meridian, prime_meridian);
      }
    if (datum)
      {
	  len = strlen (datum);
	  p->datum = malloc (len + 1);
	  if (p->datum == NULL)
	      goto error;
	  strcpy (p->datum, datum);
      }
    if (projection)
      {
	  len = strlen (projection);
	  p->projection = malloc (len + 1);
	  if (p->projection == NULL)
	      goto error;
	  strcpy (p->projection, projection);
      }
    if (unit)
      {
	  len = strlen (unit);
	  p->unit = malloc (len + 1);
	  if (p->unit == NULL)
	      goto error;
	  strcpy (p->unit, unit);
      }
    if (axis_1)
      {
	  len = strlen (axis_1);
	  p->axis_1 = malloc (len + 1);
	  if (p->axis_1 == NULL)
	      goto error;
	  strcpy (p->axis_1, axis_1);
      }
    if (orientation_1)
      {
	  len = strlen (orientation_1);
	  p->orientation_1 = malloc (len + 1);
	  if (p->orientation_1 == NULL)
	      goto error;
	  strcpy (p->orientation_1, orientation_1);
      }
    if (axis_2)
      {
	  len = strlen (axis_2);
	  p->axis_2 = malloc (len + 1);
	  if (p->axis_2 == NULL)
	      goto error;
	  strcpy (p->axis_2, axis_2);
      }
    if (orientation_2)
      {
	  len = strlen (orientation_2);
	  p->orientation_2 = malloc (len + 1);
	  if (p->orientation_2 == NULL)
	      goto error;
	  strcpy (p->orientation_2, orientation_2);
      }
    if (*first == NULL)
	*first = p;
    if (*last != NULL)
	(*last)->next = p;
    *last = p;
    return p;
  error:
    free_epsg_def (p);
    return NULL;
}

SPATIALITE_PRIVATE void
add_proj4text (struct epsg_defs *p, int count, const char *text)
{
/* creating the PROJ4TEXT string */
    int len;
    int olen;
    char *string;
    if (p == NULL || text == NULL)
	return;
    len = strlen (text);
    if (!count)
      {
	  p->proj4text = malloc (len + 1);
	  if (p->proj4text == NULL)
	      return;
	  strcpy (p->proj4text, text);
	  return;
      }
    if (p->proj4text == NULL)
	return;
    olen = strlen (p->proj4text);
    string = malloc (len + olen + 1);
    if (string == NULL)
	return;
    strcpy (string, p->proj4text);
    free (p->proj4text);
    p->proj4text = string;
    strcat (p->proj4text, text);
}

SPATIALITE_PRIVATE void
add_srs_wkt (struct epsg_defs *p, int count, const char *text)
{
/* creating the SRS_WKT string */
    int len;
    int olen;
    char *string;
    if (p == NULL || text == NULL)
	return;
    len = strlen (text);
    if (!count)
      {
	  p->srs_wkt = malloc (len + 1);
	  if (p->srs_wkt == NULL)
	      return;
	  strcpy (p->srs_wkt, text);
	  return;
      }
    if (p->srs_wkt == NULL)
	return;
    olen = strlen (p->srs_wkt);
    string = malloc (len + olen + 1);
    if (string == NULL)
	return;
    strcpy (string, p->srs_wkt);
    free (p->srs_wkt);
    p->srs_wkt = string;
    strcat (p->srs_wkt, text);
}

SPATIALITE_PRIVATE void
free_epsg (struct epsg_defs *first)
{
/* memory cleanup - destroying the EPSG list */
    struct epsg_defs *p = first;
    struct epsg_defs *pn;
    while (p)
      {
	  pn = p->next;
	  free_epsg_def (p);
	  p = pn;
      }
}

static void
create_spatial_ref_sys_aux (sqlite3 * handle)
{
/* just in case, we'll create the SPATIAL_REF_SYS_AUX table */
    const char *sql = "CREATE TABLE IF NOT EXISTS spatial_ref_sys_aux (\n"
	"\tsrid INTEGER NOT NULL PRIMARY KEY,\n"
	"\tis_geographic INTEGER,\n"
	"\thas_flipped_axes INTEGER,\n"
	"\tspheroid TEXT,\n"
	"\tprime_meridian TEXT,\n"
	"\tdatum TEXT,\n"
	"\tprojection TEXT,\n"
	"\tunit TEXT,\n"
	"\taxis_1_name TEXT,\n"
	"\taxis_1_orientation TEXT,\n"
	"\taxis_2_name TEXT,\n"
	"\taxis_2_orientation TEXT,\n"
	"\tCONSTRAINT fk_sprefsys FOREIGN KEY (srid) "
	"\tREFERENCES spatial_ref_sys (srid))";
    sqlite3_exec (handle, sql, NULL, NULL, NULL);
/* creating the SPATIAL_REF_SYS_ALL view */
    sql = "CREATE VIEW IF NOT EXISTS spatial_ref_sys_all AS\n"
	"SELECT a.srid AS srid, a.auth_name AS auth_name, "
	"a.auth_srid AS auth_srid, a.ref_sys_name AS ref_sys_name,\n"
	"b.is_geographic AS is_geographic, "
	"b.has_flipped_axes AS has_flipped_axes, "
	"b.spheroid AS spheroid, b.prime_meridian AS prime_meridian, "
	"b.datum AS datum, b.projection AS projection, b.unit AS unit,\n"
	"b.axis_1_name AS axis_1_name, "
	"b.axis_1_orientation AS axis_1_orientation,\n"
	"b.axis_2_name AS axis_2_name, "
	"b.axis_2_orientation AS axis_2_orientation,\n"
	"a.proj4text AS proj4text, a.srtext AS srtext\n"
	"FROM spatial_ref_sys AS a\n"
	"LEFT JOIN spatial_ref_sys_aux AS b ON (a.srid = b.srid)";
    sqlite3_exec (handle, sql, NULL, NULL, NULL);
}

static int
populate_spatial_ref_sys (sqlite3 * handle, int mode)
{
/* populating the EPSG dataset into the SPATIAL_REF_SYS table */
    struct epsg_defs *first = NULL;
    struct epsg_defs *last = NULL;
    struct epsg_defs *p;
    char sql[1024];
    int ret;
    sqlite3_stmt *stmt = NULL;
    sqlite3_stmt *stmt_aux = NULL;
    int ok_aux;

/* initializing the EPSG defs list */
    initialize_epsg (mode, &first, &last);

    create_spatial_ref_sys_aux (handle);
/* preparing the SQL parameterized statement (main) */
    strcpy (sql, "INSERT INTO spatial_ref_sys ");
    strcat (sql,
	    "(srid, auth_name, auth_srid, ref_sys_name, proj4text, srtext) ");
    strcat (sql, "VALUES (?, ?, ?, ?, ?, ?)");
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("%s\n", sqlite3_errmsg (handle));
	  goto error;
      }
/* preparing the SQL parameterized statement (aux) */
    strcpy (sql, "INSERT INTO spatial_ref_sys_aux ");
    strcat (sql,
	    "(srid, is_geographic, has_flipped_axes, spheroid, prime_meridian, ");
    strcat (sql, "datum, projection, unit, axis_1_name, axis_1_orientation, ");
    strcat (sql, "axis_2_name, axis_2_orientation) ");
    strcat (sql, "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt_aux, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("%s\n", sqlite3_errmsg (handle));
	  goto error;
      }

    p = first;
    while (p)
      {
	  if (p->auth_name == NULL)
	      break;
	  /* inserting into SPATIAL_REF_SYS */
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, p->srid);
	  sqlite3_bind_text (stmt, 2, p->auth_name, strlen (p->auth_name),
			     SQLITE_STATIC);
	  sqlite3_bind_int (stmt, 3, p->auth_srid);
	  sqlite3_bind_text (stmt, 4, p->ref_sys_name, strlen (p->ref_sys_name),
			     SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 5, p->proj4text, strlen (p->proj4text),
			     SQLITE_STATIC);
	  if (strlen (p->srs_wkt) == 0)
	      sqlite3_bind_text (stmt, 6, "Undefined", 9, SQLITE_STATIC);
	  else
	      sqlite3_bind_text (stmt, 6, p->srs_wkt, strlen (p->srs_wkt),
				 SQLITE_STATIC);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("%s\n", sqlite3_errmsg (handle));
		goto error;
	    }

	  /* inserting into SPATIAL_REF_SYS_AUX */
	  ok_aux = 0;
	  sqlite3_reset (stmt_aux);
	  sqlite3_clear_bindings (stmt_aux);
	  sqlite3_bind_int (stmt_aux, 1, p->srid);
	  if (p->is_geographic < 0)
	      sqlite3_bind_null (stmt_aux, 2);
	  else
	    {
		sqlite3_bind_int (stmt_aux, 2, p->is_geographic);
		ok_aux = 1;
	    }
	  if (p->flipped_axes < 0)
	      sqlite3_bind_null (stmt_aux, 3);
	  else
	    {
		sqlite3_bind_int (stmt_aux, 3, p->flipped_axes);
		ok_aux = 1;
	    }
	  if (p->spheroid == NULL)
	      sqlite3_bind_null (stmt_aux, 4);
	  else
	    {
		sqlite3_bind_text (stmt_aux, 4, p->spheroid,
				   strlen (p->spheroid), SQLITE_STATIC);
		ok_aux = 1;
	    }
	  if (p->prime_meridian == NULL)
	      sqlite3_bind_null (stmt_aux, 5);
	  else
	    {
		sqlite3_bind_text (stmt_aux, 5, p->prime_meridian,
				   strlen (p->prime_meridian), SQLITE_STATIC);
		ok_aux = 1;
	    }
	  if (p->datum == NULL)
	      sqlite3_bind_null (stmt_aux, 6);
	  else
	    {
		sqlite3_bind_text (stmt_aux, 6, p->datum, strlen (p->datum),
				   SQLITE_STATIC);
		ok_aux = 1;
	    }
	  if (p->projection == NULL)
	      sqlite3_bind_null (stmt_aux, 7);
	  else
	    {
		sqlite3_bind_text (stmt_aux, 7, p->projection,
				   strlen (p->projection), SQLITE_STATIC);
		ok_aux = 1;
	    }
	  if (p->unit == NULL)
	      sqlite3_bind_null (stmt_aux, 8);
	  else
	    {
		sqlite3_bind_text (stmt_aux, 8, p->unit, strlen (p->unit),
				   SQLITE_STATIC);
		ok_aux = 1;
	    }
	  if (p->axis_1 == NULL)
	      sqlite3_bind_null (stmt_aux, 9);
	  else
	    {
		sqlite3_bind_text (stmt_aux, 9, p->axis_1, strlen (p->axis_1),
				   SQLITE_STATIC);
		ok_aux = 1;
	    }
	  if (p->orientation_1 == NULL)
	      sqlite3_bind_null (stmt_aux, 10);
	  else
	    {
		sqlite3_bind_text (stmt_aux, 10, p->orientation_1,
				   strlen (p->orientation_1), SQLITE_STATIC);
		ok_aux = 1;
	    }
	  if (p->axis_2 == NULL)
	      sqlite3_bind_null (stmt_aux, 11);
	  else
	    {
		sqlite3_bind_text (stmt_aux, 11, p->axis_2, strlen (p->axis_2),
				   SQLITE_STATIC);
		ok_aux = 1;
	    }
	  if (p->orientation_2 == NULL)
	      sqlite3_bind_null (stmt_aux, 12);
	  else
	    {
		sqlite3_bind_text (stmt_aux, 12, p->orientation_2,
				   strlen (p->orientation_2), SQLITE_STATIC);
		ok_aux = 1;
	    }
	  if (ok_aux)
	    {
		ret = sqlite3_step (stmt_aux);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      spatialite_e ("%s\n", sqlite3_errmsg (handle));
		      goto error;
		  }
	    }
	  p = p->next;
      }
    sqlite3_finalize (stmt);
    sqlite3_finalize (stmt_aux);

/* freeing the EPSG defs list */
    free_epsg (first);
    return 1;

  error:
    if (stmt)
	sqlite3_finalize (stmt);
    if (stmt_aux)
	sqlite3_finalize (stmt_aux);
/* freeing the EPSG defs list */
    free_epsg (first);

    return 0;
}

SPATIALITE_PRIVATE int
exists_spatial_ref_sys (void *p_sqlite)
{
/* checking if the SPATIAL_REF_SYS table exists */
    int ret;
    int ok = 0;
    char sql[1024];
    char **results;
    int n_rows;
    int n_columns;
    char *err_msg = NULL;

    sqlite3 *handle = (sqlite3 *) p_sqlite;

    strcpy (sql,
	    "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE 'spatial_ref_sys'");
    ret =
	sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
/* some error occurred */
	  spatialite_e ("XX %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    if (n_rows > 0)
	ok = 1;
    sqlite3_free_table (results);
    return ok;
}

static int
check_spatial_ref_sys (sqlite3 * handle)
{
/* checking if the SPATIAL_REF_SYS table has an appropriate layout */
    int ret;
    int i;
    const char *name;
    char sql[1024];
    char **results;
    int n_rows;
    int n_columns;
    char *err_msg = NULL;
    int rs_srid = 0;
    int auth_name = 0;
    int auth_srid = 0;
    int ref_sys_name = 0;
    int proj4text = 0;
    int srtext = 0;

    strcpy (sql, "PRAGMA table_info(spatial_ref_sys)");
    ret =
	sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
/* some error occurred */
	  spatialite_e ("%s\n", err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    if (n_rows > 0)
      {
	  for (i = 1; i <= n_rows; i++)
	    {
		name = results[(i * n_columns) + 1];
		if (strcasecmp (name, "srid") == 0)
		    rs_srid = 1;
		if (strcasecmp (name, "auth_name") == 0)
		    auth_name = 1;
		if (strcasecmp (name, "auth_srid") == 0)
		    auth_srid = 1;
		if (strcasecmp (name, "ref_sys_name") == 0)
		    ref_sys_name = 1;
		if (strcasecmp (name, "proj4text") == 0)
		    proj4text = 1;
		if (strcasecmp (name, "srtext") == 0)
		    srtext = 1;
	    }
      }
    sqlite3_free_table (results);
    if (rs_srid && auth_name && auth_srid && ref_sys_name && proj4text
	&& srtext)
	ret = 1;
    else
	ret = 0;
    return ret;
}

static int
spatial_ref_sys_count (sqlite3 * handle)
{
/* checking if the SPATIAL_REF_SYS table is empty */
    int ret;
    int i;
    int count = 0;
    char sql[1024];
    char **results;
    int n_rows;
    int n_columns;
    char *err_msg = NULL;

    strcpy (sql, "SELECT Count(*) FROM spatial_ref_sys");
    ret =
	sqlite3_get_table (handle, sql, &results, &n_rows, &n_columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
/* some error occurred */
	  spatialite_e ("%s\n", err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    if (n_rows > 0)
      {
	  for (i = 1; i <= n_rows; i++)
	    {
		count = atoi (results[(i * n_columns) + 0]);
	    }
      }
    sqlite3_free_table (results);
    return count;
}

SPATIALITE_DECLARE int
spatial_ref_sys_init (sqlite3 * handle, int verbose)
{
/* 
/ deprecated function 
/ [still supported simply not to break API-level back-compatibility] 
*/
    return spatial_ref_sys_init2 (handle, GAIA_EPSG_ANY, verbose);
}

SPATIALITE_DECLARE int
spatial_ref_sys_init2 (sqlite3 * handle, int mode, int verbose)
{
/* populating the EPSG dataset into the SPATIAL_REF_SYS table */
    if (!exists_spatial_ref_sys (handle))
      {
	  if (verbose)
	      spatialite_e ("the SPATIAL_REF_SYS table doesn't exists\n");
	  return 0;
      }
    if (!check_spatial_ref_sys (handle))
      {
	  if (verbose)
	      spatialite_e
		  ("the SPATIAL_REF_SYS table has an unsupported layout\n");
	  return 0;
      }
    if (spatial_ref_sys_count (handle))
      {
	  if (verbose)
	      spatialite_e
		  ("the SPATIAL_REF_SYS table already contains some row(s)\n");
	  return 0;
      }
    if (mode == GAIA_EPSG_ANY || mode == GAIA_EPSG_NONE
	|| mode == GAIA_EPSG_WGS84_ONLY)
	;
    else
	mode = GAIA_EPSG_ANY;
    if (populate_spatial_ref_sys (handle, mode))
      {
	  if (verbose && mode != GAIA_EPSG_NONE)
	      spatialite_e
		  ("OK: the SPATIAL_REF_SYS table was successfully populated\n");
	  return 1;
      }
    return 0;
}

SPATIALITE_DECLARE int
insert_epsg_srid (sqlite3 * handle, int srid)
{
/* inserting a single EPSG definition into the SPATIAL_REF_SYS table */
    struct epsg_defs *first = NULL;
    struct epsg_defs *last = NULL;
    char sql[1024];
    int ret;
    int error = 0;
    sqlite3_stmt *stmt = NULL;
    sqlite3_stmt *stmt_aux = NULL;
    int ok_aux;

    if (!exists_spatial_ref_sys (handle))
      {
	  spatialite_e ("the SPATIAL_REF_SYS table doesn't exists\n");
	  return 0;
      }
    if (!check_spatial_ref_sys (handle))
      {
	  spatialite_e
	      ("the SPATIAL_REF_SYS table has an unsupported layout\n");
	  return 0;
      }

/* initializing the EPSG defs list */
    initialize_epsg (srid, &first, &last);
    if (first == NULL)
      {
	  spatialite_e ("SRID=%d isn't defined in the EPSG inlined dataset\n",
			srid);
	  return 0;
      }

    create_spatial_ref_sys_aux (handle);
/* preparing the SQL parameterized statement */
    strcpy (sql, "INSERT INTO spatial_ref_sys ");
    strcat (sql,
	    "(srid, auth_name, auth_srid, ref_sys_name, proj4text, srtext) ");
    strcat (sql, "VALUES (?, ?, ?, ?, ?, ?)");
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("%s\n", sqlite3_errmsg (handle));
	  error = 1;
	  goto stop;
      }
/* preparing the SQL parameterized statement (aux) */
    strcpy (sql, "INSERT INTO spatial_ref_sys_aux ");
    strcat (sql,
	    "(srid, is_geographic, has_flipped_axes, spheroid, prime_meridian, ");
    strcat (sql, "datum, projection, unit, axis_1_name, axis_1_orientation, ");
    strcat (sql, "axis_2_name, axis_2_orientation) ");
    strcat (sql, "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt_aux, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("%s\n", sqlite3_errmsg (handle));
	  error = 1;
	  goto stop;
      }

    /* inserting into SPATIAL_REF_SYS_AUX */
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int (stmt, 1, first->srid);
    sqlite3_bind_text (stmt, 2, first->auth_name, strlen (first->auth_name),
		       SQLITE_STATIC);
    sqlite3_bind_int (stmt, 3, first->auth_srid);
    sqlite3_bind_text (stmt, 4, first->ref_sys_name,
		       strlen (first->ref_sys_name), SQLITE_STATIC);
    sqlite3_bind_text (stmt, 5, first->proj4text, strlen (first->proj4text),
		       SQLITE_STATIC);
    if (strlen (first->srs_wkt) == 0)
	sqlite3_bind_text (stmt, 6, "Undefined", 9, SQLITE_STATIC);
    else
	sqlite3_bind_text (stmt, 6, first->srs_wkt, strlen (first->srs_wkt),
			   SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("%s\n", sqlite3_errmsg (handle));
	  error = 1;
	  goto stop;
      }

    /* inserting into SPATIAL_REF_SYS_AUX */
    ok_aux = 0;
    sqlite3_reset (stmt_aux);
    sqlite3_clear_bindings (stmt_aux);
    sqlite3_bind_int (stmt_aux, 1, first->srid);
    if (first->is_geographic < 0)
	sqlite3_bind_null (stmt_aux, 2);
    else
      {
	  sqlite3_bind_int (stmt_aux, 2, first->is_geographic);
	  ok_aux = 1;
      }
    if (first->flipped_axes < 0)
	sqlite3_bind_null (stmt_aux, 3);
    else
      {
	  sqlite3_bind_int (stmt_aux, 3, first->flipped_axes);
	  ok_aux = 1;
      }
    if (first->spheroid == NULL)
	sqlite3_bind_null (stmt_aux, 4);
    else
      {
	  sqlite3_bind_text (stmt_aux, 4, first->spheroid,
			     strlen (first->spheroid), SQLITE_STATIC);
	  ok_aux = 1;
      }
    if (first->prime_meridian == NULL)
	sqlite3_bind_null (stmt_aux, 5);
    else
      {
	  sqlite3_bind_text (stmt_aux, 5, first->prime_meridian,
			     strlen (first->prime_meridian), SQLITE_STATIC);
	  ok_aux = 1;
      }
    if (first->datum == NULL)
	sqlite3_bind_null (stmt_aux, 6);
    else
      {
	  sqlite3_bind_text (stmt_aux, 6, first->datum, strlen (first->datum),
			     SQLITE_STATIC);
	  ok_aux = 1;
      }
    if (first->projection == NULL)
	sqlite3_bind_null (stmt_aux, 7);
    else
      {
	  sqlite3_bind_text (stmt_aux, 7, first->projection,
			     strlen (first->projection), SQLITE_STATIC);
	  ok_aux = 1;
      }
    if (first->unit == NULL)
	sqlite3_bind_null (stmt_aux, 8);
    else
      {
	  sqlite3_bind_text (stmt_aux, 8, first->unit, strlen (first->unit),
			     SQLITE_STATIC);
	  ok_aux = 1;
      }
    if (first->axis_1 == NULL)
	sqlite3_bind_null (stmt_aux, 9);
    else
      {
	  sqlite3_bind_text (stmt_aux, 9, first->axis_1, strlen (first->axis_1),
			     SQLITE_STATIC);
	  ok_aux = 1;
      }
    if (first->orientation_1 == NULL)
	sqlite3_bind_null (stmt_aux, 10);
    else
      {
	  sqlite3_bind_text (stmt_aux, 10, first->orientation_1,
			     strlen (first->orientation_1), SQLITE_STATIC);
	  ok_aux = 1;
      }
    if (first->axis_2 == NULL)
	sqlite3_bind_null (stmt_aux, 11);
    else
      {
	  sqlite3_bind_text (stmt_aux, 11, first->axis_2,
			     strlen (first->axis_2), SQLITE_STATIC);
	  ok_aux = 1;
      }
    if (first->orientation_2 == NULL)
	sqlite3_bind_null (stmt_aux, 11);
    else
      {
	  sqlite3_bind_text (stmt_aux, 11, first->orientation_2,
			     strlen (first->orientation_2), SQLITE_STATIC);
	  ok_aux = 1;
      }
    if (ok_aux)
      {
	  ret = sqlite3_step (stmt_aux);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("%s\n", sqlite3_errmsg (handle));
		goto stop;
	    }
      }
  stop:
    if (stmt != NULL)
	sqlite3_finalize (stmt);
    if (stmt_aux != NULL)
	sqlite3_finalize (stmt_aux);

/* freeing the EPSG defs list */
    free_epsg (first);
    if (error)
	return 0;
    return 1;
}
