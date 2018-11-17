/*

 srid_aux.c -- SRID auxiliary functions

 version 4.3.0, 2014 December 4

 Author: Sandro Furieri a.furieri@lqt.it

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
 
Portions created by the Initial Developer are Copyright (C) 2014-2015
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

/*
 
CREDITS:

this module has been partly funded by:
Regione Toscana - Settore Sistema Informativo Territoriale ed Ambientale
(implementing XML support - ISO Metadata and SLD/SE Styles) 

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>
#include <spatialite/debug.h>
#include <spatialite.h>
#include <spatialite_private.h>

#ifdef _WIN32
#define strcasecmp	_stricmp
#endif /* not WIN32 */

static int
parse_proj4 (const char *proj4, const char *key, char **value)
{
/* attempts to find a key=value from within a Proj.4 geodetic string */
    int len;
    char keybuf[1024];
    const char *p = proj4;
    *value = NULL;
    if (proj4 == NULL)
	return 0;
    while (1)
      {
	  /* searching the next key */
	  if (*p == ' ' || *p == '\t')
	    {
		p++;
		continue;
	    }
	  if (*p == '+')
	    {
		/* key begins here */
		const char *v1 = NULL;
		const char *v2 = NULL;
		const char *k = p + 1;
		while (1)
		  {
		      /* searching the key termination */
		      if (*k == ' ' || *k == '=' || *k == '+' || *k == '\0')
			  break;
		      k++;
		  }
		len = k - (p + 1);
		memset (keybuf, '\0', len + 1);
		memcpy (keybuf, p + 1, len);
		if (*k == '=')
		  {
		      v1 = v2 = k + 1;
		      while (1)
			{
			    /* searching the value termination */
			    if (*v2 == ' ' || *v2 == '+' || *v2 == '\0')
				break;
			    v2++;
			}
		  }
		if (strcasecmp (keybuf, key) == 0)
		  {
		      if (v1 != NULL)
			{
			    len = v2 - v1;
			    *value = malloc (len + 1);
			    memset (*value, '\0', len + 1);
			    memcpy (*value, v1, len);
			}
		      return 1;
		  }
		p = k;
		if (v1 != NULL)
		    p = v2;
		continue;

	    }
	  return 0;		/* unexpected char */
      }
}

static int
is_wkt_key (const char *key)
{
/* testing for a WKT keyword */
    if (strcasecmp (key, "PROJCS") == 0)
	return 1;
    if (strcasecmp (key, "GEOGCS") == 0)
	return 1;
    if (strcasecmp (key, "PARAMETER") == 0)
	return 1;
    if (strcasecmp (key, "DATUM") == 0)
	return 1;
    if (strcasecmp (key, "SPHEROID") == 0)
	return 1;
    if (strcasecmp (key, "AUTHORITY") == 0)
	return 1;
    if (strcasecmp (key, "PRIMEM") == 0)
	return 1;
    if (strcasecmp (key, "UNIT") == 0)
	return 1;
    if (strcasecmp (key, "PROJECTION") == 0)
	return 1;
    if (strcasecmp (key, "TOWGS84") == 0)
	return 1;
    if (strcasecmp (key, "AXIS") == 0)
	return 1;
    return 0;
}

static char *
check_wkt (const char *wkt, const char *key, char axis, char axis_mode)
{
/* parsing a WKT string so to find a given key:value */
    char mux[16][128];
    char *o;
    const char *p = wkt;
    int level = 0;
    int item = 0;
    int quoted = 0;
    int i;
    int axis_no = 0;

    if (wkt == NULL || key == NULL)
	return NULL;

    o = mux[item];
    while (*p != '\0')
      {
	  if (*p == '"')
	    {
		/* quoted string */
		if (quoted)
		  {
		      /* the quoted string ends here */
		      *o = '\0';
		      p++;
		      quoted = 0;
		      continue;
		  }
		else
		  {
		      /* the quoted string starts here */
		      p++;
		      quoted = 1;
		      continue;
		  }
	    }
	  if (quoted)
	    {
		/* within a quoted string */
		*o++ = *p++;
		continue;
	    }
	  if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
	    {
		/* skipping unquoted whitespaces */
		p++;
		continue;
	    }
	  if (*p == ',')
	    {
		*o = '\0';
		item++;
		o = mux[item];
		p++;
		continue;
	    }
	  if (*p == '[')
	    {
		*o = '\0';
		if (is_wkt_key (mux[item]))
		  {
		      char save[128];
		      strcpy (save, mux[item]);
		      item--;
		      if (strcasecmp (key, "AXIS") == 0 && level == 2
			  && item >= 2)
			{
			    int len;
			    char *result;
			    if (strcasecmp (mux[0], key) == 0)
			      {
				  axis_no++;
				  if ((axis_no == 1 && axis == SPLITE_AXIS_1)
				      || (axis_no == 2
					  && axis == SPLITE_AXIS_2))
				    {
					if (axis_mode == SPLITE_AXIS_NAME)
					  {
					      len = strlen (mux[1]);
					      result = malloc (len + 1);
					      strcpy (result, mux[1]);
					      return result;
					  }
					if (axis_mode ==
					    SPLITE_AXIS_ORIENTATION)
					  {
					      len = strlen (mux[2]);
					      result = malloc (len + 1);
					      strcpy (result, mux[2]);
					      return result;
					  }
				    }
			      }
			}
		      if ((strcasecmp (key, "DATUM") == 0
			   || strcasecmp (key, "SPHEROID") == 0
			   || strcasecmp (key, "PRIMEM") == 0) && item >= 1)
			{
			    if (strcasecmp (mux[0], key) == 0)
			      {
				  int len = strlen (mux[1]);
				  char *result = malloc (len + 1);
				  strcpy (result, mux[1]);
				  return result;
			      }
			}
		      if ((strcasecmp (key, "UNIT") == 0
			   || strcasecmp (key, "PROJECTION") == 0) && level == 2
			  && item >= 1)
			{
			    if (strcasecmp (mux[0], key) == 0)
			      {
				  int len = strlen (mux[1]);
				  char *result = malloc (len + 1);
				  strcpy (result, mux[1]);
				  return result;
			      }
			}
		      for (i = 0; i <= item; i++)
			  strcpy (mux[0], save);
		      item = 1;
		      o = mux[1];
		      level++;
		      p++;
		      continue;
		  }
		else
		    for (i = 0; i <= item; i++)
			item = 0;
		o = mux[0];
		level++;
		p++;
		continue;
	    }
	  if (*p == ']')
	    {
		*o = '\0';
		if (strcasecmp (key, "AXIS") == 0 && level == 2 && item >= 2)
		  {
		      int len;
		      char *result;
		      if (strcasecmp (mux[0], key) == 0)
			{
			    axis_no++;
			    if ((axis_no == 1 && axis == SPLITE_AXIS_1)
				|| (axis_no == 2 && axis == SPLITE_AXIS_2))
			      {
				  if (axis_mode == SPLITE_AXIS_NAME)
				    {
					len = strlen (mux[1]);
					result = malloc (len + 1);
					strcpy (result, mux[1]);
					return result;
				    }
				  if (axis_mode == SPLITE_AXIS_ORIENTATION)
				    {
					len = strlen (mux[2]);
					result = malloc (len + 1);
					strcpy (result, mux[2]);
					return result;
				    }
			      }
			}
		  }
		if ((strcasecmp (key, "DATUM") == 0
		     || strcasecmp (key, "SPHEROID") == 0
		     || strcasecmp (key, "PRIMEM") == 0) && item >= 1)
		  {
		      if (strcasecmp (mux[0], key) == 0)
			{
			    int len = strlen (mux[1]);
			    char *result = malloc (len + 1);
			    strcpy (result, mux[1]);
			    return result;
			}
		  }
		if ((strcasecmp (key, "UNIT") == 0
		     || strcasecmp (key, "PROJECTION") == 0) && level == 2
		    && item >= 1)
		  {
		      if (strcasecmp (mux[0], key) == 0)
			{
			    int len = strlen (mux[1]);
			    char *result = malloc (len + 1);
			    strcpy (result, mux[1]);
			    return result;
			}
		  }
		for (i = 0; i <= item; i++)
		    item = 0;
		o = mux[0];
		level--;
		p++;
		continue;
	    }
	  *o++ = *p++;
	  continue;
      }
    return NULL;
}

static int
check_geographic_wkt (const char *wkt, int *geographic)
{
/* testing WKT: is geographic */
    int len;
    char dummy[16];
    const char *p = wkt;
    if (wkt == NULL)
	return 0;
    while (1)
      {
	  /* skipping whitespaces */
	  if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
	    {
		p++;
		continue;
	    }
	  break;
      }
    len = strlen (p);
    if (len < 6)
	return 0;
    memset (dummy, '\0', 7);
    memcpy (dummy, p, 6);
    if (strcasecmp (dummy, "GEOGCS") == 0)
	*geographic = 1;
    else
	*geographic = 0;
    return 1;
}

static int
check_projected_wkt (const char *wkt, int *projected)
{
/* testing WKT: is projected */
    int len;
    char dummy[16];
    const char *p = wkt;
    if (wkt == NULL)
	return 0;
    while (1)
      {
	  /* skipping whitespaces */
	  if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
	    {
		p++;
		continue;
	    }
	  break;
      }
    len = strlen (p);
    if (len < 6)
	return 0;
    memset (dummy, '\0', 7);
    memcpy (dummy, p, 6);
    if (strcasecmp (dummy, "PROJCS") == 0)
	*projected = 1;
    else
	*projected = 0;
    return 1;
}

SPATIALITE_DECLARE int
srid_is_geographic (sqlite3 * sqlite, int srid, int *geographic)
{
/* 
/ checks a SRID definition from the "spatial_ref_sys" table
/  determining if it is of the geographic type
*/
    int ok = 0;
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *sql;

/* step #1: testing SPATIAL_REF_SYS_AUX */
    sql = "SELECT is_geographic FROM spatial_ref_sys_aux WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_INTEGER)
			{
			    int is_geographic = sqlite3_column_int (stmt, 0);
			    if (is_geographic)
				*geographic = 1;
			    else
				*geographic = 0;
			    ok = 1;
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (ok)
	      return 1;
      }

/* step #2: testing WKT */
    sql = "SELECT srtext FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *wkt =
				(const char *) sqlite3_column_text (stmt, 0);
			    if (check_geographic_wkt (wkt, geographic))
				ok = 1;
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (ok)
	      return 1;
      }

/* step #3: testing Proj.4 */
    sql = "SELECT proj4text FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *proj4 =
				(const char *) sqlite3_column_text (stmt, 0);
			    char *str = NULL;
			    if (parse_proj4 (proj4, "proj", &str))
			      {
				  if (strcasecmp (str, "latlong") == 0
				      || strcasecmp (str, "longlat") == 0)
				      *geographic = 1;
				  else
				      *geographic = 0;
				  ok = 1;
			      }
			    if (str)
				free (str);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (ok)
	      return 1;
      };
    return 0;
}

SPATIALITE_DECLARE int
srid_is_projected (sqlite3 * sqlite, int srid, int *projected)
{
/* 
/ checks a SRID definition from the "spatial_ref_sys" table
/  determining if it is of the projected type
*/
    int ok = 0;
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *sql;

/* step #1: testing SPATIAL_REF_SYS_AUX */
    sql = "SELECT is_geographic FROM spatial_ref_sys_aux WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_INTEGER)
			{
			    int is_geographic = sqlite3_column_int (stmt, 0);
			    if (is_geographic)
				*projected = 0;
			    else
				*projected = 1;
			    ok = 1;
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (ok)
	      return 1;
      }

/* step #2: testing WKT */
    sql = "SELECT srtext FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *wkt =
				(const char *) sqlite3_column_text (stmt, 0);
			    if (check_projected_wkt (wkt, projected))
				ok = 1;
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (ok)
	      return 1;
      }

/* step #3: testing Proj.4 */
    sql = "SELECT proj4text FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *proj4 =
				(const char *) sqlite3_column_text (stmt, 0);
			    char *str = NULL;
			    if (parse_proj4 (proj4, "proj", &str))
			      {
				  if (strcasecmp (str, "latlong") == 0
				      || strcasecmp (str, "longlat") == 0)
				      *projected = 0;
				  else
				      *projected = 1;
				  ok = 1;
			      }
			    if (str)
				free (str);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (ok)
	      return 1;
      }
    return 0;
}

SPATIALITE_DECLARE int
srid_has_flipped_axes (sqlite3 * sqlite, int srid, int *flipped)
{
/* 
/ checks a SRS WKT definition from the "spatial_ref_sys" table
/  determining if the axes order is X-Y or Y-X
*/
    int ok = 0;
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *sql;
    int geographic;
    char *axis_1_name = NULL;
    char *axis_1_orientation = NULL;
    char *axis_2_name = NULL;
    char *axis_2_orientation = NULL;
    int ok_from_wkt = 0;

/* step #1: testing SPATIAL_REF_SYS_AUX */
    sql = "SELECT has_flipped_axes FROM spatial_ref_sys_aux WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_INTEGER)
			{
			    int has_flipped_axes = sqlite3_column_int (stmt, 0);
			    if (has_flipped_axes)
				*flipped = 1;
			    else
				*flipped = 0;
			    ok = 1;
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (ok)
	      return 1;
      }

/* step #2: testing WKT */
    axis_1_name = srid_get_axis (sqlite, srid, SPLITE_AXIS_1, SPLITE_AXIS_NAME);
    axis_1_orientation =
	srid_get_axis (sqlite, srid, SPLITE_AXIS_1, SPLITE_AXIS_ORIENTATION);
    axis_2_name = srid_get_axis (sqlite, srid, SPLITE_AXIS_2, SPLITE_AXIS_NAME);
    axis_2_orientation =
	srid_get_axis (sqlite, srid, SPLITE_AXIS_2, SPLITE_AXIS_ORIENTATION);
    if (axis_1_name != NULL && axis_1_orientation != NULL && axis_2_name != NULL
	&& axis_2_orientation != NULL)
      {
	  if ((strcasecmp (axis_1_orientation, "NORTH") == 0
	       || strcasecmp (axis_1_orientation, "SOUTH") == 0)
	      && (strcasecmp (axis_2_orientation, "EAST") == 0
		  || strcasecmp (axis_2_orientation, "WEST") == 0))
	      *flipped = 1;
	  else
	      *flipped = 0;
	  ok_from_wkt = 1;
      }
    if (axis_1_name)
	free (axis_1_name);
    if (axis_1_orientation)
	free (axis_1_orientation);
    if (axis_2_name)
	free (axis_2_name);
    if (axis_2_orientation)
	free (axis_2_orientation);
    if (ok_from_wkt)
	return 1;

/* step #3: just assuming that every Geographic SRS has flipped axes */
    ret = srid_is_geographic (sqlite, srid, &geographic);
    if (ret)
      {
	  if (geographic)
	      *flipped = 1;
	  else
	      *flipped = 0;
	  return 1;
      }
    return 0;
}

SPATIALITE_DECLARE char *
srid_get_spheroid (sqlite3 * sqlite, int srid)
{
/* 
/ checks a SRID definition from the "spatial_ref_sys" table
/  then returning the corresponding Spheroid name
*/
    char *result = NULL;
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *sql;

/* step #1: testing SPATIAL_REF_SYS_AUX */
    sql = "SELECT spheroid FROM spatial_ref_sys_aux WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    int len;
			    const char *spheroid =
				(const char *) sqlite3_column_text (stmt, 0);
			    len = strlen (spheroid);
			    result = malloc (len + 1);
			    strcpy (result, spheroid);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #2: testing WKT */
    sql = "SELECT srtext FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *wkt =
				(const char *) sqlite3_column_text (stmt, 0);
			    result = check_wkt (wkt, "SPHEROID", 0, 0);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #3: testing Proj.4 */
    sql = "SELECT proj4text FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *proj4 =
				(const char *) sqlite3_column_text (stmt, 0);
			    char *str = NULL;
			    if (parse_proj4 (proj4, "ellps", &str))
			      {
				  if (strcasecmp (str, "clrk80") == 0)
				    {
					result = malloc (18);
					strcpy (result, "Clarke 1880 (RGS)");
				    }
				  else if (strcasecmp (str, "clrk66") == 0)
				    {
					result = malloc (12);
					strcpy (result, "Clarke 1866");
				    }
				  else if (strcasecmp (str, "GRS80") == 0)
				    {
					result = malloc (9);
					strcpy (result, "GRS 1980");
				    }
				  else if (strcasecmp (str, "WGS84") == 0)
				    {
					result = malloc (7);
					strcpy (result, "WGS 84");
				    }
				  else if (strcasecmp (str, "krass") == 0)
				    {
					result = malloc (16);
					strcpy (result, "Krassowsky 1940");
				    }
				  else if (strcasecmp (str, "intl") == 0)
				    {
					result = malloc (19);
					strcpy (result, "International 1924");
				    }
				  else if (strcasecmp (str, "bess_nam") == 0)
				    {
					result = malloc (21);
					strcpy (result, "Bessel Namibia (GLM)");
				    }
				  else if (strcasecmp (str, "bessel") == 0)
				    {
					result = malloc (12);
					strcpy (result, "Bessel 1841");
				    }
				  else if (strcasecmp (str, "aust_SA") == 0)
				    {
					result = malloc (29);
					strcpy (result,
						"Australian National Spheroid");
				    }
				  else if (strcasecmp (str, "WGS72") == 0)
				    {
					result = malloc (9);
					strcpy (result, "WGS_1972");
				    }
				  else if (strcasecmp (str, "GRS67") == 0)
				    {
					result = malloc (9);
					strcpy (result, "GRS 1967");
				    }
				  else if (strcasecmp (str, "WGS66") == 0)
				    {
					result = malloc (7);
					strcpy (result, "NWL 9D");
				    }
				  else if (strcasecmp (str, "helmert") == 0)
				    {
					result = malloc (13);
					strcpy (result, "Helmert 1906");
				    }
				  else if (strcasecmp (str, "airy") == 0)
				    {
					result = malloc (10);
					strcpy (result, "Airy 1830");
				    }
				  else if (strcasecmp (str, "mod_airy") == 0)
				    {
					result = malloc (19);
					strcpy (result, "Airy Modified 1849");
				    }
				  else if (strcasecmp (str, "evrstSS") == 0)
				    {
					result = malloc (31);
					strcpy (result,
						"Everest 1830 (1967 Definition)");
				    }
			      }
			    if (str)
				free (str);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }
    return NULL;
}

SPATIALITE_DECLARE char *
srid_get_prime_meridian (sqlite3 * sqlite, int srid)
{
/* 
/ checks a SRID definition from the "spatial_ref_sys" table
/  then returning the corresponding Prime Meridian name
*/
    char *result = NULL;
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *sql;

/* step #1: testing SPATIAL_REF_SYS_AUX */
    sql = "SELECT prime_meridian FROM spatial_ref_sys_aux WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    int len;
			    const char *prime_meridian =
				(const char *) sqlite3_column_text (stmt, 0);
			    len = strlen (prime_meridian);
			    result = malloc (len + 1);
			    strcpy (result, prime_meridian);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #2: testing WKT */
    sql = "SELECT srtext FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *wkt =
				(const char *) sqlite3_column_text (stmt, 0);
			    result = check_wkt (wkt, "PRIMEM", 0, 0);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #3: testing Proj.4 */
    sql = "SELECT proj4text FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *proj4 =
				(const char *) sqlite3_column_text (stmt, 0);
			    char *str = NULL;
			    if (parse_proj4 (proj4, "pm", &str))
			      {
				  if (strcasecmp (str, "jakarta") == 0)
				    {
					result = malloc (8);
					strcpy (result, "Jakarta");
				    }
				  else if (strcasecmp (str, "brussels") == 0)
				    {
					result = malloc (9);
					strcpy (result, "Brussels");
				    }
				  else if (strcasecmp (str, "rome") == 0)
				    {
					result = malloc (5);
					strcpy (result, "Rome");
				    }
				  else if (strcasecmp (str, "madrid") == 0)
				    {
					result = malloc (7);
					strcpy (result, "Madrid");
				    }
				  else if (strcasecmp (str, "ferro") == 0)
				    {
					result = malloc (6);
					strcpy (result, "Ferro");
				    }
				  else if (strcasecmp (str, "bern") == 0)
				    {
					result = malloc (5);
					strcpy (result, "Bern");
				    }
				  else if (strcasecmp (str, "bogota") == 0)
				    {
					result = malloc (7);
					strcpy (result, "Bogota");
				    }
				  else if (strcasecmp (str, "lisbon") == 0)
				    {
					result = malloc (7);
					strcpy (result, "Lisbon");
				    }
				  else if (strcasecmp (str, "paris") == 0)
				    {
					result = malloc (6);
					strcpy (result, "Paris");
				    }
				  else if (strcasecmp (str, "stockholm") == 0)
				    {
					result = malloc (10);
					strcpy (result, "Stockholm");
				    }
				  else if (strcasecmp (str, "athens") == 0)
				    {
					result = malloc (7);
					strcpy (result, "Athens");
				    }
				  else if (strcasecmp (str, "oslo") == 0)
				    {
					result = malloc (5);
					strcpy (result, "Oslo");
				    }
				  else if (strcasecmp (str, "2.337208333333333")
					   == 0)
				    {
					result = malloc (10);
					strcpy (result, "Paris RGS");
				    }
			      }
			    if (str)
				free (str);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }
    return NULL;
}

SPATIALITE_DECLARE char *
srid_get_projection (sqlite3 * sqlite, int srid)
{
/* 
/ checks a SRID definition from the "spatial_ref_sys" table
/  then returning the corresponding Projection name
*/
    char *result = NULL;
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *sql;

/* step #1: testing SPATIAL_REF_SYS_AUX */
    sql = "SELECT projection FROM spatial_ref_sys_aux WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    int len;
			    const char *projection =
				(const char *) sqlite3_column_text (stmt, 0);
			    len = strlen (projection);
			    result = malloc (len + 1);
			    strcpy (result, projection);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #2: testing WKT */
    sql = "SELECT srtext FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *wkt =
				(const char *) sqlite3_column_text (stmt, 0);
			    result = check_wkt (wkt, "PROJECTION", 0, 0);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #3: testing Proj.4 */
    sql = "SELECT proj4text FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *proj4 =
				(const char *) sqlite3_column_text (stmt, 0);
			    char *str = NULL;
			    if (parse_proj4 (proj4, "proj", &str))
			      {
				  if (strcasecmp (str, "tmerc") == 0)
				    {
					result = malloc (20);
					strcpy (result, "Transverse_Mercator");
				    }
				  else if (strcasecmp (str, "utm") == 0)
				    {
					result = malloc (20);
					strcpy (result, "Transverse_Mercator");
				    }
				  else if (strcasecmp (str, "merc") == 0)
				    {
					result = malloc (13);
					strcpy (result, "Mercator_1SP");
				    }
				  else if (strcasecmp (str, "stere") == 0)
				    {
					result = malloc (20);
					strcpy (result, "Polar_Stereographic");
				    }
				  else if (strcasecmp (str, "sterea") == 0)
				    {
					result = malloc (22);
					strcpy (result,
						"Oblique_Stereographic");
				    }
				  else if (strcasecmp (str, "somerc") == 0)
				    {
					result = malloc (39);
					strcpy (result,
						"Hotine_Oblique_Mercator_Azimuth_Center");
				    }
				  else if (strcasecmp (str, "omerc") == 0)
				    {
					result = malloc (39);
					strcpy (result,
						"Hotine_Oblique_Mercator_Azimuth_Center");
				    }
				  else if (strcasecmp (str, "krovak") == 0)
				    {
					result = malloc (7);
					strcpy (result, "Krovak");
				    }
				  else if (strcasecmp (str, "cass") == 0)
				    {
					result = malloc (16);
					strcpy (result, "Cassini_Soldner");
				    }
				  else if (strcasecmp (str, "lcc") == 0)
				    {
					result = malloc (28);
					strcpy (result,
						"Lambert_Conformal_Conic_1SP");
				    }
				  else if (strcasecmp (str, "lea") == 0)
				    {
					result = malloc (29);
					strcpy (result,
						"Lambert_Azimuthal_Equal_Area");
				    }
				  else if (strcasecmp (str, "laea") == 0)
				    {
					result = malloc (29);
					strcpy (result,
						"Lambert_Azimuthal_Equal_Area");
				    }
				  else if (strcasecmp (str, "aea") == 0)
				    {
					result = malloc (24);
					strcpy (result,
						"Albers_Conic_Equal_Area");
				    }
				  else if (strcasecmp (str, "cea") == 0)
				    {
					result = malloc (23);
					strcpy (result,
						"Cylindrical_Equal_Area");
				    }
				  else if (strcasecmp (str, "eqc") == 0)
				    {
					result = malloc (16);
					strcpy (result, "Equirectangular");
				    }
				  else if (strcasecmp (str, "poly") == 0)
				    {
					result = malloc (10);
					strcpy (result, "Polyconic");
				    }
				  else if (strcasecmp (str, "nzmg") == 0)
				    {
					result = malloc (21);
					strcpy (result, "New_Zealand_Map_Grid");
				    }
				  else if (strcasecmp (str, "longlat") == 0)
				    {
					result = malloc (5);
					strcpy (result, "none");
				    }
			      }
			    if (str)
				free (str);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }
    return NULL;
}

SPATIALITE_DECLARE char *
srid_get_datum (sqlite3 * sqlite, int srid)
{
/* 
/ checks a SRID definition from the "spatial_ref_sys" table
/  then returning the corresponding Unit name
*/
    char *result = NULL;
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *sql;

/* step #1: testing SPATIAL_REF_SYS_AUX */
    sql = "SELECT datum FROM spatial_ref_sys_aux WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    int len;
			    const char *datum =
				(const char *) sqlite3_column_text (stmt, 0);
			    len = strlen (datum);
			    result = malloc (len + 1);
			    strcpy (result, datum);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #2: testing WKT */
    sql = "SELECT srtext FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *wkt =
				(const char *) sqlite3_column_text (stmt, 0);
			    result = check_wkt (wkt, "DATUM", 0, 0);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #3: testing Proj.4 */
    sql = "SELECT proj4text FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *proj4 =
				(const char *) sqlite3_column_text (stmt, 0);
			    char *str = NULL;
			    if (parse_proj4 (proj4, "datum", &str))
			      {
				  if (strcasecmp (str, "NAD27") == 0)
				    {
					result = malloc (26);
					strcpy (result,
						"North_American_Datum_1927");
				    }
				  else if (strcasecmp (str, "NAD83") == 0)
				    {
					result = malloc (26);
					strcpy (result,
						"North_American_Datum_1983");
				    }
				  else if (strcasecmp (str, "WGS84") == 0)
				    {
					result = malloc (9);
					strcpy (result, "WGS_1984");
				    }
				  else if (strcasecmp (str, "potsdam") == 0)
				    {
					result = malloc (28);
					strcpy (result,
						"Deutsches_Hauptdreiecksnetz");
				    }
				  else if (strcasecmp (str, "hermannskogel") ==
					   0)
				    {
					result = malloc (32);
					strcpy (result,
						"Militar_Geographische_Institute");
				    }
				  else if (strcasecmp (str, "nzgd49") == 0)
				    {
					result = malloc (32);
					strcpy (result,
						"New_Zealand_Geodetic_Datum_1949");
				    }
				  else if (strcasecmp (str, "carthage") == 0)
				    {
					result = malloc (9);
					strcpy (result, "Carthage");
				    }
				  else if (strcasecmp (str, "GGRS87") == 0)
				    {
					result = malloc (37);
					strcpy (result,
						"Greek_Geodetic_Reference_System_1987");
				    }
				  else if (strcasecmp (str, "ire65") == 0)
				    {
					result = malloc (5);
					strcpy (result, "TM65");
				    }
				  else if (strcasecmp (str, "OSGB36") == 0)
				    {
					result = malloc (10);
					strcpy (result, "OSGB_1936");
				    }
			      }
			    if (str)
				free (str);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }
    return NULL;
}

SPATIALITE_DECLARE char *
srid_get_unit (sqlite3 * sqlite, int srid)
{
/* 
/ checks a SRID definition from the "spatial_ref_sys" table
/  then returning the corresponding Unit name
*/
    char *result = NULL;
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *sql;

/* step #1: testing SPATIAL_REF_SYS_AUX */
    sql = "SELECT unit FROM spatial_ref_sys_aux WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    int len;
			    const char *unit =
				(const char *) sqlite3_column_text (stmt, 0);
			    len = strlen (unit);
			    result = malloc (len + 1);
			    strcpy (result, unit);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #2: testing WKT */
    sql = "SELECT srtext FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *wkt =
				(const char *) sqlite3_column_text (stmt, 0);
			    result = check_wkt (wkt, "UNIT", 0, 0);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #3: testing Proj.4 */
    sql = "SELECT proj4text FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *proj4 =
				(const char *) sqlite3_column_text (stmt, 0);
			    char *str = NULL;
			    if (parse_proj4 (proj4, "units", &str))
			      {
				  if (strcasecmp (str, "m") == 0)
				    {
					result = malloc (6);
					strcpy (result, "metre");
				    }
				  else if (strcasecmp (str, "us-ft") == 0)
				    {
					result = malloc (16);
					strcpy (result, "US survery foot");
				    }
				  else if (strcasecmp (str, "ft") == 0)
				    {
					result = malloc (5);
					strcpy (result, "foot");
				    }
			      }
			    if (str)
				free (str);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }
    return NULL;
}

SPATIALITE_DECLARE char *
srid_get_axis (sqlite3 * sqlite, int srid, char axis, char mode)
{
/* 
/ checks a SRID definition from the "spatial_ref_sys" table
/  then returning the corresponding Axis name or orientation
*/
    char *result = NULL;
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *sql;

    if ((axis == SPLITE_AXIS_1 || axis == SPLITE_AXIS_2)
	&& (mode == SPLITE_AXIS_NAME || mode == SPLITE_AXIS_ORIENTATION))
	;
    else
	return NULL;

/* step #1: testing SPATIAL_REF_SYS_AUX */
    if (axis == SPLITE_AXIS_1 && mode == SPLITE_AXIS_NAME)
	sql = "SELECT axis_1_name FROM spatial_ref_sys_aux WHERE srid = ?";
    if (axis == SPLITE_AXIS_1 && mode == SPLITE_AXIS_ORIENTATION)
	sql =
	    "SELECT axis_1_orientation FROM spatial_ref_sys_aux WHERE srid = ?";
    if (axis == SPLITE_AXIS_2 && mode == SPLITE_AXIS_NAME)
	sql = "SELECT axis_2_name FROM spatial_ref_sys_aux WHERE srid = ?";
    if (axis == SPLITE_AXIS_2 && mode == SPLITE_AXIS_ORIENTATION)
	sql =
	    "SELECT axis_2_orientation FROM spatial_ref_sys_aux WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    int len;
			    const char *unit =
				(const char *) sqlite3_column_text (stmt, 0);
			    len = strlen (unit);
			    result = malloc (len + 1);
			    strcpy (result, unit);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }

/* step #2: testing WKT */
    sql = "SELECT srtext FROM spatial_ref_sys WHERE srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret == SQLITE_OK)
      {
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int (stmt, 1, srid);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      if (sqlite3_column_type (stmt, 0) == SQLITE_TEXT)
			{
			    const char *wkt =
				(const char *) sqlite3_column_text (stmt, 0);
			    result = check_wkt (wkt, "AXIS", axis, mode);
			}
		  }
	    }
	  sqlite3_finalize (stmt);
	  stmt = NULL;
	  if (result != NULL)
	      return result;
      }
    return NULL;
}

static void
getProjParamsFromSpatialReferenceSystemTable (sqlite3 * sqlite, int srid,
					      char **proj_params)
{
/* retrives the PROJ params from SPATIAL_SYS_REF table, if possible */
    char *sql;
    char **results;
    int rows;
    int columns;
    int i;
    int ret;
    int len;
    const char *proj4text;
    char *errMsg = NULL;
    *proj_params = NULL;
    sql =
	sqlite3_mprintf
	("SELECT proj4text FROM spatial_ref_sys WHERE srid = %d", srid);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, &errMsg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unknown SRID: %d\t<%s>\n", srid, errMsg);
	  sqlite3_free (errMsg);
	  return;
      }
    for (i = 1; i <= rows; i++)
      {
	  proj4text = results[(i * columns)];
	  if (proj4text != NULL)
	    {
		len = strlen (proj4text);
		*proj_params = malloc (len + 1);
		strcpy (*proj_params, proj4text);
	    }
      }
    if (*proj_params == NULL)
      {
	  spatialite_e ("unknown SRID: %d\n", srid);
      }
    sqlite3_free_table (results);
}

static int
coordinates_system_matches (struct epsg_defs *iter, const char *organization,
			    const int organization_coordsys_id)
{
    if (organization == NULL)
      {
	  /* we didn't have a good lookup on the target database, so use best-efforts lookup */
	  return ((iter->srid == organization_coordsys_id)
		  && (iter->proj4text != NULL));
      }
    else
      {
	  return ((strcasecmp (iter->auth_name, organization) == 0)
		  && (iter->auth_srid == organization_coordsys_id)
		  && (iter->proj4text != NULL));
      }
}

static void
getProjParamsFromGeopackageTable (sqlite3 * sqlite, int srid,
				  char **proj_params)
{
    char *sql;
    char **results;
    int rows;
    int columns;
    int ret;
    int len;
    char *errMsg = NULL;
    struct epsg_defs *first = NULL;
    struct epsg_defs *last = NULL;
    struct epsg_defs *iter = NULL;
    const char *organization = NULL;
    int organization_coordsys_id = -1;

    *proj_params = NULL;

    sql =
	sqlite3_mprintf
	("SELECT organization, organization_coordsys_id FROM gpkg_spatial_ref_sys WHERE srs_id = %d",
	 srid);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, &errMsg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unknown SRID: %d\t<%s>\n", srid, errMsg);
	  sqlite3_free (errMsg);
	  return;
      }
    if (rows == 0)
      {
	  printf
	      ("unknown SRID: %d\t(not in local database, ignoring authority and using best efforts...)\n",
	       srid);
	  organization_coordsys_id = srid;
      }
    else if (rows == 1)
      {
	  /* there are 'columns' entries in the header row (result indexes 0 to columns - 1), and our data is the next row */
	  organization = results[columns];
	  errno = 0;
	  organization_coordsys_id = strtol (results[columns + 1], NULL, 10);
	  if ((errno != 0) || (organization_coordsys_id == 0))
	    {
		spatialite_e ("Invalid organization_coordsys_id format: %s\n",
			      results[columns + 1]);
		sqlite3_free_table (results);
		return;
	    }
      }
    else if (rows > 1)
      {
	  spatialite_e
	      ("invalid or corrupt gpkg_spatial_ref_sys table - duplicate entries for : %d\n",
	       srid);
	  sqlite3_free_table (results);
	  return;
      }

    if (organization == NULL)
      {
	  /* best-effort mode */
	  initialize_epsg (srid, &first, &last);
      }
    else
      {
	  initialize_epsg (GAIA_EPSG_ANY, &first, &last);
      }
    iter = first;
    while (iter)
      {
	  if (coordinates_system_matches
	      (iter, organization, organization_coordsys_id))
	    {
		len = strlen (iter->proj4text);
		*proj_params = malloc (len + 1);
		strcpy (*proj_params, iter->proj4text);
		free_epsg (first);
		sqlite3_free_table (results);
		return;
	    }
	  iter = iter->next;
      }
    /* if we get here, we didn't find a match */
    free_epsg (first);
    sqlite3_free_table (results);
    spatialite_e ("unknown SRID: %d\n", srid);
}

SPATIALITE_PRIVATE void
getProjParamsEx (void *p_sqlite, int srid, char **proj_params,
		 int gpkg_amphibious_mode)
{
/* retrives the PROJ params - generic interface */sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    *proj_params = NULL;

/* searching within "vanilla" spatial_reference_sys */
    getProjParamsFromSpatialReferenceSystemTable (sqlite, srid, proj_params);
    if (*proj_params != NULL)
	return;

/* last opportunity: search within GPKG srs */
    getProjParamsFromGeopackageTable (sqlite, srid, proj_params);
}

SPATIALITE_PRIVATE void
getProjParams (void *p_sqlite, int srid, char **proj_params)
{
/* 
* retrives the PROJ params from SPATIAL_SYS_REF table, if possible 
* convenience method - disabling GPKG amphibious mode
*/
    getProjParamsEx (p_sqlite, srid, proj_params, 0);
}
