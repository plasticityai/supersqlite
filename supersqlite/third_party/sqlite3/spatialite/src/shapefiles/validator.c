/*

 validator.c -- implements geometry validation and repair

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

#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <direct.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>
#include <spatialite/debug.h>

#include <spatialite/gaiaaux.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>
#include <spatialite_private.h>

/* 64 bit integer: portable format for printf() */
#if defined(_WIN32) && !defined(__MINGW32__)
#define FRMT64 "%I64d"
#else
#define FRMT64 "%lld"
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#define strcasecmp	_stricmp
#endif

struct validity_report_row
{
    sqlite3_int64 rowid;
    int valid;
    char *warning;
    char *error;
    char *extra;
    struct validity_report_row *next;
};

struct validity_report
{
    struct validity_report_row *first;
    struct validity_report_row *last;
    int n_rows;
    int n_nullgeoms;
    int n_valids;
    int n_invalids;
    int n_warnings;
};

struct sanitize_report_row
{
    sqlite3_int64 rowid;
    int repaired;
    char *warning;
    char *error;
    char *summary;
    struct sanitize_report_row *next;
};

struct sanitize_report
{
    struct sanitize_report_row *first;
    struct sanitize_report_row *last;
    int n_invalids;
    int n_repaired;
    int n_discarded;
    int n_not_repaired;
    int input_type;
    int repaired_type;
    int discarded_type;
};

static int
check_table_column (sqlite3 * sqlite, const char *table, const char *geom,
		    int *gtype, int *srid)
{
/* chacks if a table-column effectively exists */
    char *sql;
    const char *xsql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    int spatial_type = 0;
    int ok = 0;

    *gtype = -1;
    *srid = -2;
/* checking the DB layout */
    xsql = "SELECT CheckSpatialMetadata()";
    ret = sqlite3_get_table (sqlite, xsql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	      spatial_type = atoi (results[(i * columns) + 0]);
      }
    sqlite3_free_table (results);
    if (spatial_type == 1 || spatial_type == 3)
	;
    else
	return 0;

/* checking the table-column */
    if (spatial_type == 1)
	xsql = "SELECT type, coord_dimension, srid FROM geometry_columns ";
    else
	xsql = "SELECT geometry_type, srid FROM geometry_columns ";
    sql = sqlite3_mprintf ("%s WHERE Lower(f_table_name) = Lower(%Q) "
			   "AND Lower(f_geometry_column) = Lower(%Q)", xsql,
			   table, geom);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		ok = 1;
		if (spatial_type == 1)
		  {
		      int ndims;
		      const char *type = results[(i * columns) + 0];
		      const char *dims = results[(i * columns) + 2];
		      if (strcasecmp (dims, "XYZM") == 0)
			  ndims = GAIA_XY_Z_M;
		      else if (strcasecmp (dims, "4") == 0)
			  ndims = GAIA_XY_Z_M;
		      else if (strcasecmp (dims, "XYZ") == 0)
			  ndims = GAIA_XY_Z;
		      else if (strcasecmp (dims, "3") == 0)
			  ndims = GAIA_XY_Z;
		      else if (strcasecmp (dims, "XYM") == 0)
			  ndims = GAIA_XY_M;
		      else
			  ndims = GAIA_XY;
		      if (strcasecmp (type, "POINT") == 0)
			{
			    if (ndims == GAIA_XY_Z_M)
				*gtype = 3001;
			    else if (ndims == GAIA_XY_Z)
				*gtype = 1001;
			    else if (ndims == GAIA_XY_M)
				*gtype = 2001;
			    else
				*gtype = 1;
			}
		      if (strcasecmp (type, "LINESTRING") == 0)
			{
			    if (ndims == GAIA_XY_Z_M)
				*gtype = 3002;
			    else if (ndims == GAIA_XY_Z)
				*gtype = 1002;
			    else if (ndims == GAIA_XY_M)
				*gtype = 2002;
			    else
				*gtype = 2;
			}
		      if (strcasecmp (type, "POLYGON") == 0)
			{
			    if (ndims == GAIA_XY_Z_M)
				*gtype = 3003;
			    else if (ndims == GAIA_XY_Z)
				*gtype = 1003;
			    else if (ndims == GAIA_XY_M)
				*gtype = 2003;
			    else
				*gtype = 3;
			}
		      if (strcasecmp (type, "MULTIPOINT") == 0)
			{
			    if (ndims == GAIA_XY_Z_M)
				*gtype = 3004;
			    else if (ndims == GAIA_XY_Z)
				*gtype = 1004;
			    else if (ndims == GAIA_XY_M)
				*gtype = 2004;
			    else
				*gtype = 4;
			}
		      if (strcasecmp (type, "MULTILINESTRING") == 0)
			{
			    if (ndims == GAIA_XY_Z_M)
				*gtype = 3005;
			    else if (ndims == GAIA_XY_Z)
				*gtype = 1005;
			    else if (ndims == GAIA_XY_M)
				*gtype = 2005;
			    else
				*gtype = 5;
			}
		      if (strcasecmp (type, "MULTIPOLYGON") == 0)
			{
			    if (ndims == GAIA_XY_Z_M)
				*gtype = 3006;
			    else if (ndims == GAIA_XY_Z)
				*gtype = 1006;
			    else if (ndims == GAIA_XY_M)
				*gtype = 2006;
			    else
				*gtype = 6;
			}
		      if (strcasecmp (type, "GEOMETRYCOLLECTION") == 0)
			{
			    if (ndims == GAIA_XY_Z_M)
				*gtype = 3007;
			    else if (ndims == GAIA_XY_Z)
				*gtype = 1007;
			    else if (ndims == GAIA_XY_M)
				*gtype = 2007;
			    else
				*gtype = 7;
			}
		      if (strcasecmp (type, "GEOMETRY") == 0)
			{
			    if (ndims == GAIA_XY_Z_M)
				*gtype = 3000;
			    else if (ndims == GAIA_XY_Z)
				*gtype = 1000;
			    else if (ndims == GAIA_XY_M)
				*gtype = 2000;
			    else
				*gtype = 0;
			}
		      *srid = atoi (results[(i * columns) + 1]);
		  }
		else
		  {
		      *gtype = atoi (results[(i * columns) + 0]);
		      *srid = atoi (results[(i * columns) + 1]);
		  }
	    }
      }
    sqlite3_free_table (results);
    return ok;
}

#ifdef ENABLE_LWGEOM		/* only if LWGEOM is supported */

static struct sanitize_report *
alloc_sanitize_report (void)
{
/* allocating the report container struct */
    struct sanitize_report *p = malloc (sizeof (struct sanitize_report));
    p->first = NULL;
    p->last = NULL;
    p->n_invalids = 0;
    p->n_repaired = 0;
    p->n_discarded = 0;
    p->n_not_repaired = 0;
    p->input_type = -1;
    p->repaired_type = -1;
    p->discarded_type = -1;
    return p;
}

static void
free_sanitize_report (struct sanitize_report *p)
{
/* memory cleanup: freeing the report container struct */
    struct sanitize_report_row *r;
    struct sanitize_report_row *rn;
    r = p->first;
    while (r)
      {
	  rn = r->next;
	  if (r->warning != NULL)
	      free (r->warning);
	  if (r->error != NULL)
	      free (r->error);
	  if (r->summary != NULL)
	      free (r->summary);
	  free (r);
	  r = rn;
      }
    free (p);
}

static void
addMessageToSanitizeReport (struct sanitize_report *report, sqlite3_int64 rowid,
			    int repaired, const char *error,
			    const char *warning, const char *summary)
{
/* adding a message to the report */
    int len;
    struct sanitize_report_row *r =
	malloc (sizeof (struct sanitize_report_row));
    r->rowid = rowid;
    r->repaired = repaired;
    r->error = NULL;
    r->warning = NULL;
    r->summary = NULL;
    r->next = NULL;
    if (error)
      {
	  len = strlen (error);
	  r->error = malloc (len + 1);
	  strcpy (r->error, error);
      }
    if (warning)
      {
	  len = strlen (warning);
	  r->warning = malloc (len + 1);
	  strcpy (r->warning, warning);
      }
    if (summary)
      {
	  len = strlen (summary);
	  r->summary = malloc (len + 1);
	  strcpy (r->summary, summary);
      }
    if (!repaired)
	report->n_not_repaired += 1;
    else
      {
	  if (summary == NULL)
	      report->n_repaired += 1;
	  else
	      report->n_discarded += 1;
      }
    if (report->first == NULL)
	report->first = r;
    if (report->last != NULL)
	report->last->next = r;
    report->last = r;
}

static int
eval_type (int old, int new)
{
/* evaluates the max. common Geometry Type */
    int o_dims;
    int n_dims;
    int r_dims;
    int o_type;
    int n_type;
    int r_type;
    int ret;
    if (old < 0)
      {
	  /* first evaluation */
	  return new;
      }
    switch (old)
      {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
	  o_dims = GAIA_XY;
	  break;
      case 1000:
      case 1001:
      case 1002:
      case 1003:
      case 1004:
      case 1005:
      case 1006:
      case 1007:
	  o_dims = GAIA_XY_Z;
	  break;
      case 2000:
      case 2001:
      case 2002:
      case 2003:
      case 2004:
      case 2005:
      case 2006:
      case 2007:
	  o_dims = GAIA_XY_M;
	  break;
      case 3000:
      case 3001:
      case 3002:
      case 3003:
      case 3004:
      case 3005:
      case 3006:
      case 3007:
	  o_dims = GAIA_XY_Z_M;
	  break;
      default:
	  o_dims = GAIA_XY;
	  break;
      };
    switch (new)
      {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
	  n_dims = GAIA_XY;
	  break;
      case 1000:
      case 1001:
      case 1002:
      case 1003:
      case 1004:
      case 1005:
      case 1006:
      case 1007:
	  n_dims = GAIA_XY_Z;
	  break;
      case 2000:
      case 2001:
      case 2002:
      case 2003:
      case 2004:
      case 2005:
      case 2006:
      case 2007:
	  n_dims = GAIA_XY_M;
	  break;
      case 3000:
      case 3001:
      case 3002:
      case 3003:
      case 3004:
      case 3005:
      case 3006:
      case 3007:
	  n_dims = GAIA_XY_Z_M;
	  break;
      default:
	  n_dims = GAIA_XY;
	  break;
      };
    switch (old)
      {
      case 0:
      case 1000:
      case 2000:
      case 3000:
	  o_type = -1;
	  break;
      case 1:
      case 1001:
      case 2001:
      case 3001:
	  o_type = GAIA_POINT;
	  break;
      case 2:
      case 1002:
      case 2002:
      case 3002:
	  o_type = GAIA_LINESTRING;
	  break;
      case 3:
      case 1003:
      case 2003:
      case 3003:
	  o_type = GAIA_POLYGON;
	  break;
      case 4:
      case 1004:
      case 2004:
      case 3004:
	  o_type = GAIA_MULTIPOINT;
	  break;
      case 5:
      case 1005:
      case 2005:
      case 3005:
	  o_type = GAIA_MULTILINESTRING;
	  break;
      case 6:
      case 1006:
      case 2006:
      case 3006:
	  o_type = GAIA_MULTIPOLYGON;
	  break;
      case 7:
      case 1007:
      case 2007:
      case 3007:
	  o_type = GAIA_GEOMETRYCOLLECTION;
	  break;
      default:
	  o_type = -1;
	  break;
      };
    switch (new)
      {
      case 0:
      case 1000:
      case 2000:
      case 3000:
	  n_type = -1;
	  break;
      case 1:
      case 1001:
      case 2001:
      case 3001:
	  n_type = GAIA_POINT;
	  break;
      case 2:
      case 1002:
      case 2002:
      case 3002:
	  n_type = GAIA_LINESTRING;
	  break;
      case 3:
      case 1003:
      case 2003:
      case 3003:
	  n_type = GAIA_POLYGON;
	  break;
      case 4:
      case 1004:
      case 2004:
      case 3004:
	  n_type = GAIA_MULTIPOINT;
	  break;
      case 5:
      case 1005:
      case 2005:
      case 3005:
	  n_type = GAIA_MULTILINESTRING;
	  break;
      case 6:
      case 1006:
      case 2006:
      case 3006:
	  n_type = GAIA_MULTIPOLYGON;
	  break;
      case 7:
      case 1007:
      case 2007:
      case 3007:
	  n_type = GAIA_GEOMETRYCOLLECTION;
	  break;
      default:
	  n_type = -1;
	  break;
      };
    if (o_type == n_type)
	r_type = n_type;
    else
      {
	  if ((o_type == GAIA_POINT || n_type == GAIA_POINT)
	      && (o_type == GAIA_MULTIPOINT || n_type == GAIA_MULTIPOINT))
	      r_type = GAIA_MULTIPOINT;
	  else if ((o_type == GAIA_LINESTRING || n_type == GAIA_LINESTRING)
		   && (o_type == GAIA_LINESTRING || n_type == GAIA_LINESTRING))
	      r_type = GAIA_MULTILINESTRING;
	  else if ((o_type == GAIA_POLYGON || n_type == GAIA_POLYGON)
		   && (o_type == GAIA_MULTIPOLYGON
		       || n_type == GAIA_MULTIPOLYGON))
	      r_type = GAIA_MULTIPOLYGON;
	  else
	      r_type = GAIA_GEOMETRYCOLLECTION;
      }
    if (o_dims == n_dims)
	r_dims = n_dims;
    else
      {
	  if ((o_dims == GAIA_XY || n_dims == GAIA_XY)
	      && (o_dims == GAIA_XY_Z || n_dims == GAIA_XY_Z))
	      r_dims = GAIA_XY_Z;
	  if ((o_dims == GAIA_XY || n_dims == GAIA_XY)
	      && (o_dims == GAIA_XY_M || n_dims == GAIA_XY_M))
	      r_dims = GAIA_XY_M;
	  if ((o_dims == GAIA_XY || n_dims == GAIA_XY)
	      && (o_dims == GAIA_XY_Z_M || n_dims == GAIA_XY_Z_M))
	      r_dims = GAIA_XY_Z_M;
	  if ((o_dims == GAIA_XY_M || n_dims == GAIA_XY_M)
	      && (o_dims == GAIA_XY_Z || n_dims == GAIA_XY_Z))
	      r_dims = GAIA_XY_Z_M;
	  if ((o_dims == GAIA_XY_M || n_dims == GAIA_XY_M)
	      && (o_dims == GAIA_XY_Z_M || n_dims == GAIA_XY_Z_M))
	      r_dims = GAIA_XY_Z_M;
	  if ((o_dims == GAIA_XY_Z || n_dims == GAIA_XY_Z)
	      && (o_dims == GAIA_XY_Z_M || n_dims == GAIA_XY_Z_M))
	      r_dims = GAIA_XY_Z_M;
      }
    switch (r_type)
      {
      case GAIA_POINT:
	  switch (r_dims)
	    {
	    case GAIA_XY_Z_M:
		ret = 3001;
		break;
	    case GAIA_XY_Z:
		ret = 1001;
		break;
	    case GAIA_XY_M:
		ret = 2001;
		break;
	    case GAIA_XY:
		ret = 1;
		break;
	    };
	  break;
      case GAIA_LINESTRING:
	  switch (r_dims)
	    {
	    case GAIA_XY_Z_M:
		ret = 3002;
		break;
	    case GAIA_XY_Z:
		ret = 1002;
		break;
	    case GAIA_XY_M:
		ret = 2002;
		break;
	    case GAIA_XY:
		ret = 2;
		break;
	    };
	  break;
      case GAIA_POLYGON:
	  switch (r_dims)
	    {
	    case GAIA_XY_Z_M:
		ret = 3003;
		break;
	    case GAIA_XY_Z:
		ret = 1003;
		break;
	    case GAIA_XY_M:
		ret = 2003;
		break;
	    case GAIA_XY:
		ret = 3;
		break;
	    };
	  break;
      case GAIA_MULTIPOINT:
	  switch (r_dims)
	    {
	    case GAIA_XY_Z_M:
		ret = 3004;
		break;
	    case GAIA_XY_Z:
		ret = 1004;
		break;
	    case GAIA_XY_M:
		ret = 2004;
		break;
	    case GAIA_XY:
		ret = 4;
		break;
	    };
	  break;
      case GAIA_MULTILINESTRING:
	  switch (r_dims)
	    {
	    case GAIA_XY_Z_M:
		ret = 3005;
		break;
	    case GAIA_XY_Z:
		ret = 1005;
		break;
	    case GAIA_XY_M:
		ret = 2005;
		break;
	    case GAIA_XY:
		ret = 5;
		break;
	    };
	  break;
      case GAIA_MULTIPOLYGON:
	  switch (r_dims)
	    {
	    case GAIA_XY_Z_M:
		ret = 3006;
		break;
	    case GAIA_XY_Z:
		ret = 1006;
		break;
	    case GAIA_XY_M:
		ret = 2006;
		break;
	    case GAIA_XY:
		ret = 6;
		break;
	    };
	  break;
      default:
	  switch (r_dims)
	    {
	    case GAIA_XY_Z_M:
		ret = 3007;
		break;
	    case GAIA_XY_Z:
		ret = 1007;
		break;
	    case GAIA_XY_M:
		ret = 2007;
		break;
	    case GAIA_XY:
		ret = 7;
		break;
	    };
	  break;
      };
    return ret;
}

static int
eval_cast_type (int old, int new)
{
/* determining the appropriate Type Casting (if any) */
    int o_type;
    int n_type;
    switch (old)
      {
      case 0:
      case 1000:
      case 2000:
      case 3000:
	  o_type = -1;
	  break;
      case 1:
      case 1001:
      case 2001:
      case 3001:
	  o_type = GAIA_POINT;
	  break;
      case 2:
      case 1002:
      case 2002:
      case 3002:
	  o_type = GAIA_LINESTRING;
	  break;
      case 3:
      case 1003:
      case 2003:
      case 3003:
	  o_type = GAIA_POLYGON;
	  break;
      case 4:
      case 1004:
      case 2004:
      case 3004:
	  o_type = GAIA_MULTIPOINT;
	  break;
      case 5:
      case 1005:
      case 2005:
      case 3005:
	  o_type = GAIA_MULTILINESTRING;
	  break;
      case 6:
      case 1006:
      case 2006:
      case 3006:
	  o_type = GAIA_MULTIPOLYGON;
	  break;
      case 7:
      case 1007:
      case 2007:
      case 3007:
	  o_type = GAIA_GEOMETRYCOLLECTION;
	  break;
      default:
	  o_type = -1;
	  break;
      };
    switch (new)
      {
      case 0:
      case 1000:
      case 2000:
      case 3000:
	  n_type = -1;
	  break;
      case 1:
      case 1001:
      case 2001:
      case 3001:
	  n_type = GAIA_POINT;
	  break;
      case 2:
      case 1002:
      case 2002:
      case 3002:
	  n_type = GAIA_LINESTRING;
	  break;
      case 3:
      case 1003:
      case 2003:
      case 3003:
	  n_type = GAIA_POLYGON;
	  break;
      case 4:
      case 1004:
      case 2004:
      case 3004:
	  n_type = GAIA_MULTIPOINT;
	  break;
      case 5:
      case 1005:
      case 2005:
      case 3005:
	  n_type = GAIA_MULTILINESTRING;
	  break;
      case 6:
      case 1006:
      case 2006:
      case 3006:
	  n_type = GAIA_MULTIPOLYGON;
	  break;
      case 7:
      case 1007:
      case 2007:
      case 3007:
	  n_type = GAIA_GEOMETRYCOLLECTION;
	  break;
      default:
	  n_type = -1;
	  break;
      };
    if (o_type == n_type)
	return -1;
    return n_type;
}

static int
eval_cast_dims (int old, int new)
{
/* determining the appropriate Dimensions Casting (if any) */
    int o_dims;
    int n_dims;
    switch (old)
      {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
	  o_dims = GAIA_XY;
	  break;
      case 1000:
      case 1001:
      case 1002:
      case 1003:
      case 1004:
      case 1005:
      case 1006:
      case 1007:
	  o_dims = GAIA_XY_Z;
	  break;
      case 2000:
      case 2001:
      case 2002:
      case 2003:
      case 2004:
      case 2005:
      case 2006:
      case 2007:
	  o_dims = GAIA_XY_M;
	  break;
      case 3000:
      case 3001:
      case 3002:
      case 3003:
      case 3004:
      case 3005:
      case 3006:
      case 3007:
	  o_dims = GAIA_XY_Z_M;
	  break;
      default:
	  o_dims = GAIA_XY;
	  break;
      };
    switch (new)
      {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
	  n_dims = GAIA_XY;
	  break;
      case 1000:
      case 1001:
      case 1002:
      case 1003:
      case 1004:
      case 1005:
      case 1006:
      case 1007:
	  n_dims = GAIA_XY_Z;
	  break;
      case 2000:
      case 2001:
      case 2002:
      case 2003:
      case 2004:
      case 2005:
      case 2006:
      case 2007:
	  n_dims = GAIA_XY_M;
	  break;
      case 3000:
      case 3001:
      case 3002:
      case 3003:
      case 3004:
      case 3005:
      case 3006:
      case 3007:
	  n_dims = GAIA_XY_Z_M;
	  break;
      default:
	  n_dims = GAIA_XY;
	  break;
      };
    if (o_dims == n_dims)
	return -1;
    return n_dims;
}


static const char *
createDiscardedSummary (gaiaGeomCollPtr geom, char *summary)
{
/* short summary for discarded fragments */
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    pt = geom->FirstPoint;
    while (pt)
      {
	  pts++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  lns++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  pgs++;
	  pg = pg->Next;
      }
    if (pts > 0 && lns == 0 && pgs == 0)
	sprintf (summary, "%d Poin%s", pts, (pts > 1) ? "ts" : "t");
    if (pts == 0 && lns > 0 && pgs == 0)
	sprintf (summary, "%d Linestrin%s", lns, (lns > 1) ? "gs" : "g");
    if (pts == 0 && lns == 0 && pgs > 0)
	sprintf (summary, "%d Polygo%s", pgs, (pgs > 1) ? "ns" : "n");
    if (pts > 0 && lns > 0 && pgs == 0)
	sprintf (summary, "%d Poin%s; %d Linestrin%s", pts,
		 (pts > 1) ? "ts" : "t", lns, (lns > 1) ? "gs" : "s");
    if (pts > 0 && lns == 0 && pgs > 0)
	sprintf (summary, "%d Poin%s; %d Polygo%s", pts, (pts > 1) ? "ts" : "t",
		 pgs, (pgs > 1) ? "ns" : "n");
    if (pts == 0 && lns > 0 && pgs > 0)
	sprintf (summary, "%d Linestrin%s; %d Polygo%s", lns,
		 (lns > 1) ? "gs" : "g", pgs, (pgs > 1) ? "ns" : "n");
    if (pts > 0 && lns > 0 && pgs > 0)
	sprintf (summary, "%d Poin%s; %d Linestrin%s; %d Polygo%s", pts,
		 (pts > 1) ? "ts" : "t", lns, (lns > 1) ? "gs" : "s", pgs,
		 (pgs > 1) ? "ns" : "n");
    return summary;
}

static int
change_geometry_type (sqlite3 * sqlite, const char *table, const char *geometry,
		      int cast_type, int cast_dims)
{
/* changing the Geometry Type for the whole table/column */
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    int spatial_type = 0;
    const char *xsql;
    char *sql;
    sqlite3_stmt *stmt;
    char *xtable;
    char *xgeom;
    const char *xtype;
    const char *xdims;

/* checking the DB layout */
    xsql = "SELECT CheckSpatialMetadata()";
    ret = sqlite3_get_table (sqlite, xsql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	      spatial_type = atoi (results[(i * columns) + 0]);
      }
    sqlite3_free_table (results);
    if (spatial_type == 1 || spatial_type == 3)
	;
    else
	return 0;

/* updating the "geometry_columns" row */
    if (spatial_type == 1)
	sql = sqlite3_mprintf ("UPDATE geometry_columns SET type = ?, "
			       "coord_dimension = ? WHERE Lower(f_table_name) = Lower(%Q) AND "
			       "Lower(f_geometry_column) = Lower(%Q)", table,
			       geometry);
    else
	sql = sqlite3_mprintf ("UPDATE geometry_columns SET geometry_type = ?, "
			       "coord_dimension = ? WHERE Lower(f_table_name) = Lower(%Q) AND "
			       "Lower(f_geometry_column) = Lower(%Q)", table,
			       geometry);
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("sanitize_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    if (spatial_type == 1)
      {
	  const char *p_type = "GEOMETRY";
	  const char *p_dims = "XY";
	  switch (cast_type)
	    {
	    case GAIA_POINT:
		p_type = "POINT";
		break;
	    case GAIA_LINESTRING:
		p_type = "LINESTRING";
		break;
	    case GAIA_POLYGON:
		p_type = "POLYGON";
		break;
	    case GAIA_MULTIPOINT:
		p_type = "MULTIPOINT";
		break;
	    case GAIA_MULTILINESTRING:
		p_type = "MULTILINESTRING";
		break;
	    case GAIA_MULTIPOLYGON:
		p_type = "MULTIPOLYGON";
		break;
	    case GAIA_GEOMETRYCOLLECTION:
		p_type = "GEOMETRYCOLLECTION";
		break;
	    };
	  switch (cast_dims)
	    {
	    case GAIA_XY_Z_M:
		p_dims = "XYZM";
		break;
	    case GAIA_XY_Z:
		p_dims = "XYZ";
		break;
	    case GAIA_XY_M:
		p_dims = "ZYM";
		break;
	    };
	  sqlite3_bind_text (stmt, 1, p_type, strlen (p_type), SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 2, p_dims, strlen (p_dims), SQLITE_STATIC);
      }
    else
      {
	  int gtype = 0;
	  int ndims = 2;
	  switch (cast_type)
	    {
	    case GAIA_POINT:
		switch (cast_dims)
		  {
		  case GAIA_XY_Z_M:
		      gtype = 3001;
		      ndims = 4;
		      break;
		  case GAIA_XY_Z:
		      gtype = 1001;
		      ndims = 3;
		      break;
		  case GAIA_XY_M:
		      gtype = 2001;
		      ndims = 3;
		      break;
		  default:
		      gtype = 1;
		      ndims = 2;
		      break;
		  };
		break;
	    case GAIA_LINESTRING:
		switch (cast_dims)
		  {
		  case GAIA_XY_Z_M:
		      gtype = 3002;
		      ndims = 4;
		      break;
		  case GAIA_XY_Z:
		      gtype = 1002;
		      ndims = 3;
		      break;
		  case GAIA_XY_M:
		      gtype = 2002;
		      ndims = 3;
		      break;
		  default:
		      gtype = 2;
		      ndims = 2;
		      break;
		  };
		break;
	    case GAIA_POLYGON:
		switch (cast_dims)
		  {
		  case GAIA_XY_Z_M:
		      gtype = 3003;
		      ndims = 4;
		      break;
		  case GAIA_XY_Z:
		      gtype = 1003;
		      ndims = 3;
		      break;
		  case GAIA_XY_M:
		      gtype = 2003;
		      ndims = 3;
		      break;
		  default:
		      gtype = 3;
		      ndims = 2;
		      break;
		  };
		break;
	    case GAIA_MULTIPOINT:
		switch (cast_dims)
		  {
		  case GAIA_XY_Z_M:
		      gtype = 3004;
		      ndims = 4;
		      break;
		  case GAIA_XY_Z:
		      gtype = 1004;
		      ndims = 3;
		      break;
		  case GAIA_XY_M:
		      gtype = 2004;
		      ndims = 3;
		      break;
		  default:
		      gtype = 4;
		      ndims = 2;
		      break;
		  };
		break;
	    case GAIA_MULTILINESTRING:
		switch (cast_dims)
		  {
		  case GAIA_XY_Z_M:
		      gtype = 3005;
		      ndims = 4;
		      break;
		  case GAIA_XY_Z:
		      gtype = 1005;
		      ndims = 3;
		      break;
		  case GAIA_XY_M:
		      gtype = 2005;
		      ndims = 3;
		      break;
		  default:
		      gtype = 5;
		      ndims = 2;
		      break;
		  };
		break;
	    case GAIA_MULTIPOLYGON:
		switch (cast_dims)
		  {
		  case GAIA_XY_Z_M:
		      gtype = 3006;
		      ndims = 4;
		      break;
		  case GAIA_XY_Z:
		      gtype = 1006;
		      ndims = 3;
		      break;
		  case GAIA_XY_M:
		      gtype = 2006;
		      ndims = 3;
		      break;
		  default:
		      gtype = 6;
		      ndims = 2;
		      break;
		  };
		break;
	    case GAIA_GEOMETRYCOLLECTION:
		switch (cast_dims)
		  {
		  case GAIA_XY_Z_M:
		      gtype = 3007;
		      ndims = 4;
		      break;
		  case GAIA_XY_Z:
		      gtype = 1007;
		      ndims = 3;
		      break;
		  case GAIA_XY_M:
		      gtype = 2007;
		      ndims = 3;
		      break;
		  default:
		      gtype = 7;
		      ndims = 2;
		      break;
		  };
		break;
	    };
	  sqlite3_bind_int (stmt, 1, gtype);
	  sqlite3_bind_int (stmt, 2, ndims);
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("sanitize_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_finalize (stmt);

/* applying  type casting to already inserted Geometries */
    xtable = gaiaDoubleQuotedSql (table);
    xgeom = gaiaDoubleQuotedSql (geometry);
    xtype = "Geometry";
    xdims = "XY";
    switch (cast_type)
      {
      case GAIA_POINT:
	  xtype = "Point";
	  break;
      case GAIA_LINESTRING:
	  xtype = "Linestring";
	  break;
      case GAIA_POLYGON:
	  xtype = "Polygon";
	  break;
      case GAIA_MULTIPOINT:
	  xtype = "MultiPoint";
	  break;
      case GAIA_MULTILINESTRING:
	  xtype = "MultiLinestring";
	  break;
      case GAIA_MULTIPOLYGON:
	  xtype = "MultiPolygon";
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  xtype = "GeometryCollection";
	  break;
      };
    switch (cast_dims)
      {
      case GAIA_XY_Z_M:
	  xdims = "XYZM";
	  break;
      case GAIA_XY_Z:
	  xdims = "XYZ";
	  break;
      case GAIA_XY_M:
	  xdims = "XYM";
	  break;
      };
    sql =
	sqlite3_mprintf
	("UPDATE \"%s\" SET \"%s\" = CastTo%s(CastTo%s(\"%s\"))", xtable, xgeom,
	 xtype, xdims, xgeom);
    free (xtable);
    free (xgeom);
    ret = sqlite3_exec (sqlite, sql, NULL, 0, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("sanitize_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }

    return 1;
}

static void
drop_tmp_table (sqlite3 * sqlite, const char *tmp_table)
{
/* dropping the auxiliary temporary table */
    char *sql;
    char *xtable;
    int ret;

    xtable = gaiaDoubleQuotedSql (tmp_table);
    sql = sqlite3_mprintf ("DROP TABLE \"%s\"", xtable);
    free (xtable);
    ret = sqlite3_exec (sqlite, sql, NULL, 0, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	spatialite_e ("sanitize_geometry_column error: <%s>\n",
		      sqlite3_errmsg (sqlite));
}

static void
update_repaired (sqlite3 * sqlite, const char *table, const char *geometry,
		 const char *tmp_table, int old_type, int new_type)
{
/* updating all repaired geometries */
    char *sql;
    int cast_type = -1;
    int cast_dims = -1;
    int type;
    char *xtmp_table;
    char *xtable;
    char *xgeom;
    const char *casttype;
    const char *castdims;
    int ret;
    int is_error = 0;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt_out;

/* determining the final Geometry Type */
    type = eval_type (old_type, new_type);
    if (type != old_type)
      {
	  /* determining the eventual Castings to be applied */
	  cast_type = eval_cast_type (old_type, new_type);
	  cast_dims = eval_cast_dims (old_type, new_type);
      }

/* preparing the SELECT statement */
    xtmp_table = gaiaDoubleQuotedSql (tmp_table);
    sql = sqlite3_mprintf ("SELECT ref_rowid, repaired_geometry FROM \"%s\" "
			   "WHERE repaired_geometry IS NOT NULL", xtmp_table);
    free (xtmp_table);
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("sanitize_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }

/* preparing the UPDATE statement */
    xtable = gaiaDoubleQuotedSql (table);
    xgeom = gaiaDoubleQuotedSql (geometry);
    switch (new_type)
      {
      case 1:
	  casttype = "Point";
	  castdims = "XY";
	  break;
      case 1001:
	  casttype = "Point";
	  castdims = "XYZ";
	  break;
      case 2001:
	  casttype = "Point";
	  castdims = "XYM";
	  break;
      case 3001:
	  casttype = "Point";
	  castdims = "XYZM";
	  break;
      case 2:
	  casttype = "Linestring";
	  castdims = "XY";
	  break;
      case 1002:
	  casttype = "Linestring";
	  castdims = "XYZ";
	  break;
      case 2002:
	  casttype = "Linestring";
	  castdims = "XYM";
	  break;
      case 3002:
	  casttype = "Linestring";
	  castdims = "XYZM";
	  break;
      case 3:
	  casttype = "Polygon";
	  castdims = "XY";
	  break;
      case 1003:
	  casttype = "Polygon";
	  castdims = "XYZ";
	  break;
      case 2003:
	  casttype = "Polygon";
	  castdims = "XYM";
	  break;
      case 3003:
	  casttype = "Polygon";
	  castdims = "XYZM";
	  break;
      case 4:
	  casttype = "MultiPoint";
	  castdims = "XY";
	  break;
      case 1004:
	  casttype = "MultiPoint";
	  castdims = "XYZ";
	  break;
      case 2004:
	  casttype = "MultiPoint";
	  castdims = "XYM";
	  break;
      case 3004:
	  casttype = "MultiPoint";
	  castdims = "XYZM";
	  break;
      case 5:
	  casttype = "MultiLinestring";
	  castdims = "XY";
	  break;
      case 1005:
	  casttype = "MultiLinestring";
	  castdims = "XYZ";
	  break;
      case 2005:
	  casttype = "MultiLinestring";
	  castdims = "XYM";
	  break;
      case 3005:
	  casttype = "MultiLinestring";
	  castdims = "XYZM";
	  break;
      case 6:
	  casttype = "MultiPolygon";
	  castdims = "XY";
	  break;
      case 1006:
	  casttype = "MultiPolygon";
	  castdims = "XYZ";
	  break;
      case 2006:
	  casttype = "MultiPolygon";
	  castdims = "XYM";
	  break;
      case 3006:
	  casttype = "MultiPolygon";
	  castdims = "XYZM";
	  break;
      case 7:
	  casttype = "GeometryCollection";
	  castdims = "XY";
	  break;
      case 1007:
	  casttype = "GeometryCollection";
	  castdims = "XYZ";
	  break;
      case 2007:
	  casttype = "GeometryCollection";
	  castdims = "XYM";
	  break;
      case 3007:
	  casttype = "GeometryCollection";
	  castdims = "XYZM";
	  break;
      case 0:
	  casttype = "Multi";
	  castdims = "XY";
	  break;
      case 1000:
	  casttype = "Multi";
	  castdims = "XYZ";
	  break;
      case 2000:
	  casttype = "Multi";
	  castdims = "XYM";
	  break;
      case 3000:
	  casttype = "Multi";
	  castdims = "XYZM";
	  break;
      default:
	  casttype = "Multi";
	  castdims = "XY";
	  break;
      };
    sql = sqlite3_mprintf ("UPDATE \"%s\" SET \"%s\" = CastTo%s(CastTo%s(?)) "
			   "WHERE ROWID = ?", xtable, xgeom, casttype,
			   castdims);
    free (xtable);
    free (xgeom);
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_out, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("sanitize_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }

/* starting a Transaction */
    ret = sqlite3_exec (sqlite, "BEGIN", NULL, 0, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("sanitize_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }

    if (cast_type != -1 || cast_dims != -1)
      {
	  /* changing the Geometry Type to the whole table/column */
	  if (!change_geometry_type
	      (sqlite, table, geometry, cast_type, cast_dims))
	      return;
      }

    while (1)
      {
	  /* scrolling the result set */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing one row from the resultset */
		sqlite3_int64 rowid = sqlite3_column_int (stmt, 0);
		if (sqlite3_column_type (stmt, 1) == SQLITE_BLOB)
		  {
		      const unsigned char *blob = sqlite3_column_blob (stmt, 1);
		      int n_bytes = sqlite3_column_bytes (stmt, 1);
		      /* updating the main table */
		      sqlite3_reset (stmt_out);
		      sqlite3_clear_bindings (stmt_out);
		      sqlite3_bind_blob (stmt_out, 1, blob, n_bytes,
					 SQLITE_STATIC);
		      sqlite3_bind_int64 (stmt_out, 2, rowid);
		      ret = sqlite3_step (stmt_out);
		      if (ret == SQLITE_DONE || ret == SQLITE_ROW)
			  ;
		      else
			{
			    is_error = 1;
			    spatialite_e
				("sanitize_geometry_column error: <%s>\n",
				 sqlite3_errmsg (sqlite));
			    break;
			}
		  }
	    }
	  else
	    {
		spatialite_e ("check_geometry_column error: <%s>\n",
			      sqlite3_errmsg (sqlite));
		break;
	    }
      }
    sqlite3_finalize (stmt);
    sqlite3_finalize (stmt_out);

    if (is_error)
      {
	  /* Rollback */
	  ret = sqlite3_exec (sqlite, "ROLLBACK", NULL, 0, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("sanitize_geometry_column error: <%s>\n",
			      sqlite3_errmsg (sqlite));
		goto stop;
	    }
	  goto stop;
      }
    else
      {
	  /* dropping the auxiliary temporary table */
	  drop_tmp_table (sqlite, tmp_table);
	  /* committing the still pending Transaction */
	  ret = sqlite3_exec (sqlite, "COMMIT", NULL, 0, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("sanitize_geometry_column error: <%s>\n",
			      sqlite3_errmsg (sqlite));
		goto stop;
	    }
      }
  stop:
    return;
}

static int
sanitize_geometry_column_common (const void *p_cache, sqlite3 * sqlite,
				 const char *table, const char *geom,
				 const char *tmp_table, const char *report_path,
				 int *n_invalids, int *n_repaired,
				 int *n_discarded, int *n_failures,
				 char **err_msg)
{
#ifdef ENABLE_LWGEOM		/* only if LWGEOM is enabled */

/* attempts to repair invalid Geometries from a Geometry Column */
    char *sql;
    char *xtable;
    char *xgeom;
    char *xtmp_table;
    sqlite3_stmt *stmt;
    sqlite3_stmt *stmt_out;
    int ret;
    int gtype;
    int srid;
    const char *x_type;
    const char *x_dims;
    char num[256];
    FILE *out = NULL;
    const char *p_msg;
    const char *p_summary;
    char summary[1024];
    int is_error = 0;
    time_t v_time;
    struct tm *v_tm;
    const char *day;
    const char *month;
    int len;
    struct sanitize_report_row *p_r;
    struct sanitize_report *report = alloc_sanitize_report ();

    if (err_msg != NULL)
	*err_msg = NULL;

    if (!check_table_column (sqlite, table, geom, &gtype, &srid))
      {
	  spatialite_e ("sanitize_geometry_column error: <%s><%s>\n"
			"Not defined in \"geometry_columns\"", table, geom);
	  if (err_msg != NULL)
	    {
		char *msg =
		    sqlite3_mprintf
		    ("sanitize_geometry_column error: <%s><%s>\n"
		     "Not defined in \"geometry_columns\"", table, geom);
		len = strlen (msg);
		*err_msg = malloc (len + 1);
		strcpy (*err_msg, msg);
		sqlite3_free (msg);
	    }
	  goto stop;
      }

/* decoding Geometry Type and Dimensions */
    switch (gtype)
      {
      case 0:
	  x_type = "GEOMETRY";
	  x_dims = "XY";
	  break;
      case 1000:
	  x_type = "GEOMETRY";
	  x_dims = "XYZ";
	  break;
      case 2000:
	  x_type = "GEOMETRY";
	  x_dims = "XYM";
	  break;
      case 3000:
	  x_type = "GEOMETRY";
	  x_dims = "XYZM";
	  break;
      case 1:
	  x_type = "POINT";
	  x_dims = "XY";
	  break;
      case 1001:
	  x_type = "POINT";
	  x_dims = "XYZ";
	  break;
      case 2001:
	  x_type = "POINT";
	  x_dims = "XYM";
	  break;
      case 3001:
	  x_type = "POINT";
	  x_dims = "XYZM";
	  break;
      case 2:
	  x_type = "LINESTRING";
	  x_dims = "XY";
	  break;
      case 1002:
	  x_type = "LINESTRING";
	  x_dims = "XYZ";
	  break;
      case 2002:
	  x_type = "LINESTRING";
	  x_dims = "XYM";
	  break;
      case 3002:
	  x_type = "LINESTRING";
	  x_dims = "XYZM";
	  break;
      case 3:
	  x_type = "POLYGON";
	  x_dims = "XY";
	  break;
      case 1003:
	  x_type = "POLYGON";
	  x_dims = "XYZ";
	  break;
      case 2003:
	  x_type = "POLYGON";
	  x_dims = "XYM";
	  break;
      case 3003:
	  x_type = "POLYGON";
	  x_dims = "XYZM";
	  break;
      case 4:
	  x_type = "MULTIPOINT";
	  x_dims = "XY";
	  break;
      case 1004:
	  x_type = "MULTIPOINT";
	  x_dims = "XYZ";
	  break;
      case 2004:
	  x_type = "MULTIPOINT";
	  x_dims = "XYM";
	  break;
      case 3004:
	  x_type = "MULTIPOINT";
	  x_dims = "XYZM";
	  break;
      case 5:
	  x_type = "MULTILINESTRING";
	  x_dims = "XY";
	  break;
      case 1005:
	  x_type = "MULTILINESTRING";
	  x_dims = "XYZ";
	  break;
      case 2005:
	  x_type = "MULTILINESTRING";
	  x_dims = "XYM";
	  break;
      case 3005:
	  x_type = "MULTILINESTRING";
	  x_dims = "XYZM";
	  break;
      case 6:
	  x_type = "MULTIPOLYGON";
	  x_dims = "XY";
	  break;
      case 1006:
	  x_type = "MULTIPOLYGON";
	  x_dims = "XYZ";
	  break;
      case 2006:
	  x_type = "MULTIPOLYGON";
	  x_dims = "XYM";
	  break;
      case 3006:
	  x_type = "MULTIPOLYGON";
	  x_dims = "XYZM";
	  break;
      case 7:
	  x_type = "GEOMETRYCOLLECTION";
	  x_dims = "XY";
	  break;
      case 1007:
	  x_type = "GEOMETRYCOLLECTION";
	  x_dims = "XYZ";
	  break;
      case 2007:
	  x_type = "GEOMETRYCOLLECTION";
	  x_dims = "XYM";
	  break;
      case 3007:
	  x_type = "GEOMETRYCOLLECTION";
	  x_dims = "XYZM";
	  break;
      default:
	  x_type = "UNKNOWN";
	  x_dims = "UNKNOWN";
	  break;
      };

    xtable = gaiaDoubleQuotedSql (table);
    xgeom = gaiaDoubleQuotedSql (geom);
    sql = sqlite3_mprintf ("SELECT ROWID, \"%s\" FROM \"%s\"", xgeom, xtable);
    free (xtable);
    free (xgeom);
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("sanitize_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  if (err_msg != NULL)
	    {
		char *msg =
		    sqlite3_mprintf ("sanitize_geometry_column error: <%s>\n",
				     sqlite3_errmsg (sqlite));
		len = strlen (msg);
		*err_msg = malloc (len + 1);
		strcpy (*err_msg, msg);
		sqlite3_free (msg);
	    }
	  goto stop;
      }

/* creating the auxiliary temporary table */
    xtmp_table = gaiaDoubleQuotedSql (tmp_table);
    sql =
	sqlite3_mprintf
	("CREATE TABLE \"%s\" (ref_rowid INTEGER NOT NULL PRIMARY KEY, "
	 "input_geometry BLOB, repaired_geometry BLOB, discarded_geometry BLOB)",
	 xtmp_table);
    free (xtmp_table);
    ret = sqlite3_exec (sqlite, sql, NULL, 0, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("sanitize_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  if (err_msg != NULL)
	    {
		char *msg =
		    sqlite3_mprintf ("sanitize_geometry_column error: <%s>\n",
				     sqlite3_errmsg (sqlite));
		len = strlen (msg);
		*err_msg = malloc (len + 1);
		strcpy (*err_msg, msg);
		sqlite3_free (msg);
	    }
	  goto stop;
      }

/* preparing the INSERT INTO statement */
    xtmp_table = gaiaDoubleQuotedSql (tmp_table);
    sql = sqlite3_mprintf ("INSERT INTO \"%s\" "
			   "(ref_rowid, input_geometry, repaired_geometry, discarded_geometry) "
			   "VALUES (?, ?, ?, ?)", xtmp_table);
    free (xtmp_table);
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_out, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("sanitize_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  if (err_msg != NULL)
	    {
		char *msg =
		    sqlite3_mprintf ("sanitize_geometry_column error: <%s>\n",
				     sqlite3_errmsg (sqlite));
		len = strlen (msg);
		*err_msg = malloc (len + 1);
		strcpy (*err_msg, msg);
		sqlite3_free (msg);
	    }
	  goto stop;
      }

/* starting a Transaction */
    ret = sqlite3_exec (sqlite, "BEGIN", NULL, 0, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("sanitize_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  if (err_msg != NULL)
	    {
		char *msg =
		    sqlite3_mprintf ("sanitize_geometry_column error: <%s>\n",
				     sqlite3_errmsg (sqlite));
		len = strlen (msg);
		*err_msg = malloc (len + 1);
		strcpy (*err_msg, msg);
		sqlite3_free (msg);
	    }
	  goto stop;
      }

/* opening the HTML report */
    out = fopen (report_path, "wb");
    if (out == NULL)
	goto stop;

    while (1)
      {
	  /* scrolling the result set */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing one row from the resultset */
		gaiaGeomCollPtr geom = NULL;
		sqlite3_int64 rowid = sqlite3_column_int (stmt, 0);
		if (sqlite3_column_type (stmt, 1) == SQLITE_BLOB)
		  {
		      const unsigned char *blob = sqlite3_column_blob (stmt, 1);
		      int n_bytes = sqlite3_column_bytes (stmt, 1);
		      geom = gaiaFromSpatiaLiteBlobWkb (blob, n_bytes);
		  }
		if (geom)
		  {
		      /* checking a geometry for validity */
		      int valret;
		      if (p_cache != NULL)
			{
			    gaiaResetGeosMsg_r (p_cache);
			    valret = gaiaIsValid_r (p_cache, geom);
			}
		      else
			{
			    gaiaResetGeosMsg ();
			    valret = gaiaIsValid (geom);
			}
		      if (!valret)
			{
			    unsigned char *blob_geom;
			    int blob_sz_geom;
			    unsigned char *blob_saned;
			    int blob_sz_saned;
			    unsigned char *blob_dscrd;
			    int blob_sz_dscrd;
			    const char *error;
			    const char *warning;
			    gaiaGeomCollPtr repaired;
			    gaiaGeomCollPtr discarded;
			    report->n_invalids += 1;
			    gaiaResetLwGeomMsg ();
			    repaired = gaiaMakeValid (geom);
			    discarded = gaiaMakeValidDiscarded (geom);
			    error = gaiaGetLwGeomErrorMsg ();
			    warning = gaiaGetLwGeomWarningMsg ();
			    if (discarded == NULL)
				p_summary = NULL;
			    else
				p_summary =
				    createDiscardedSummary (discarded, summary);
			    addMessageToSanitizeReport (report, rowid,
							(repaired ==
							 NULL) ? 0 : 1, error,
							warning, p_summary);
			    /* inserting into the auxiliary temporary table */
			    sqlite3_reset (stmt_out);
			    sqlite3_clear_bindings (stmt_out);
			    sqlite3_bind_int64 (stmt_out, 1, rowid);
			    if (!repaired)
			      {
				  report->input_type =
				      eval_type (report->input_type,
						 gaiaGeometryType (geom));
				  gaiaToSpatiaLiteBlobWkb (geom, &blob_geom,
							   &blob_sz_geom);
				  sqlite3_bind_blob (stmt_out, 2, blob_geom,
						     blob_sz_geom, free);
				  sqlite3_bind_null (stmt_out, 3);
			      }
			    else
			      {
				  report->repaired_type =
				      eval_type (report->repaired_type,
						 gaiaGeometryType (repaired));
				  sqlite3_bind_null (stmt_out, 2);
				  gaiaToSpatiaLiteBlobWkb (repaired,
							   &blob_saned,
							   &blob_sz_saned);
				  sqlite3_bind_blob (stmt_out, 3, blob_saned,
						     blob_sz_saned, free);
			      }
			    if (discarded)
			      {
				  report->discarded_type =
				      eval_type (report->discarded_type,
						 gaiaGeometryType (discarded));
				  gaiaToSpatiaLiteBlobWkb (discarded,
							   &blob_dscrd,
							   &blob_sz_dscrd);
				  sqlite3_bind_blob (stmt_out, 4, blob_dscrd,
						     blob_sz_dscrd, free);
			      }
			    else
				sqlite3_bind_null (stmt_out, 4);
			    /* inserting into the auxiliary temporary table */
			    ret = sqlite3_step (stmt_out);
			    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
				;
			    else
			      {
				  is_error = 1;
				  spatialite_e
				      ("sanitize_geometry_column error: <%s>\n",
				       sqlite3_errmsg (sqlite));
				  if (err_msg != NULL)
				    {
					char *msg =
					    sqlite3_mprintf
					    ("sanitize_geometry_column error: <%s>\n",
					     sqlite3_errmsg (sqlite));
					len = strlen (msg);
					*err_msg = malloc (len + 1);
					strcpy (*err_msg, msg);
					sqlite3_free (msg);
				    }
				  break;
			      }
			    if (repaired)
				gaiaFreeGeomColl (repaired);
			    if (discarded)
				gaiaFreeGeomColl (discarded);
			}
		      gaiaFreeGeomColl (geom);
		  }
	    }
	  else
	    {
		spatialite_e ("check_geometry_column error: <%s>\n",
			      sqlite3_errmsg (sqlite));
		if (err_msg != NULL)
		  {
		      char *msg =
			  sqlite3_mprintf
			  ("sanitize_geometry_column error: <%s>\n",
			   sqlite3_errmsg (sqlite));
		      len = strlen (msg);
		      *err_msg = malloc (len + 1);
		      strcpy (*err_msg, msg);
		      sqlite3_free (msg);
		  }
		break;
	    }
      }
    sqlite3_finalize (stmt);
    sqlite3_finalize (stmt_out);

    if (is_error)
      {
	  /* Rollback */
	  ret = sqlite3_exec (sqlite, "ROLLBACK", NULL, 0, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("sanitize_geometry_column error: <%s>\n",
			      sqlite3_errmsg (sqlite));
		if (err_msg != NULL)
		  {
		      char *msg =
			  sqlite3_mprintf
			  ("sanitize_geometry_column error: <%s>\n",
			   sqlite3_errmsg (sqlite));
		      len = strlen (msg);
		      *err_msg = malloc (len + 1);
		      strcpy (*err_msg, msg);
		      sqlite3_free (msg);
		  }
		goto stop;
	    }
	  goto stop;
      }
    else
      {
	  /* committing the still pending Transaction */
	  ret = sqlite3_exec (sqlite, "COMMIT", NULL, 0, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("sanitize_geometry_column error: <%s>\n",
			      sqlite3_errmsg (sqlite));
		if (err_msg != NULL)
		  {
		      char *msg =
			  sqlite3_mprintf
			  ("sanitize_geometry_column error: <%s>\n",
			   sqlite3_errmsg (sqlite));
		      len = strlen (msg);
		      *err_msg = malloc (len + 1);
		      strcpy (*err_msg, msg);
		      sqlite3_free (msg);
		  }
		goto stop;
	    }
      }

/* generating the HTML header */
    fprintf (out,
	     "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
    fprintf (out, "<html>\n\t<head>\n");
    fprintf (out,
	     "\t\t<meta content=\"text/html; charset=UTF-8\" http-equiv=\"content-type\">\n");
    fprintf (out,
	     "\t\t<title>SpatiaLite Repair Geometries - Table: %s</title>\n",
	     table);
    fprintf (out, "\t\t<style type=\"text/css\">\n");
    fprintf (out, "\t\t\th1 {color:red;}\n");
    fprintf (out, "\t\t\th2 {color:navy;}\n");
    fprintf (out, "\t\t\ttd.title {background-color:silver;}\n");
    fprintf (out, "\t\t\ttd.ok {background-color:#00ff00;}\n");
    fprintf (out, "\t\t\ttd.err {background-color:#ff0000;}\n");
    fprintf (out, "\t\t\ttd.wng {background-color:#ffff00;}\n");
    fprintf (out, "\t\t</style>\n");
    fprintf (out, "\t</head>\n\t<body bgcolor=\"#f8fff8\">\n");
    time (&v_time);
    v_tm = localtime (&v_time);
    switch (v_tm->tm_wday)
      {
      case 0:
	  day = "Sun";
	  break;
      case 1:
	  day = "Mon";
	  break;
      case 2:
	  day = "Tue";
	  break;
      case 3:
	  day = "Wed";
	  break;
      case 4:
	  day = "Thu";
	  break;
      case 5:
	  day = "Fri";
	  break;
      case 6:
	  day = "Sat";
	  break;
      };
    switch (v_tm->tm_mon)
      {
      case 0:
	  month = "Jan";
	  break;
      case 1:
	  month = "Feb";
	  break;
      case 2:
	  month = "Mar";
	  break;
      case 3:
	  month = "Apr";
	  break;
      case 4:
	  month = "May";
	  break;
      case 5:
	  month = "Jun";
	  break;
      case 6:
	  month = "Jul";
	  break;
      case 7:
	  month = "Aug";
	  break;
      case 8:
	  month = "Sep";
	  break;
      case 9:
	  month = "Oct";
	  break;
      case 10:
	  month = "Nov";
	  break;
      case 11:
	  month = "Dec";
	  break;
      };
    fprintf (out, "\t\t%d-%s-%d, %s [%02d:%02d:%02d]<br>\n",
	     v_tm->tm_year + 1900, month, v_tm->tm_mday, day, v_tm->tm_hour,
	     v_tm->tm_min, v_tm->tm_sec);
    fprintf (out, "\t\t<h1>SpatiaLite Repair Geometries</h1>\n");

/* generating the summary sub-report */
    fprintf (out, "\t\t<h2>Summary</h2>\n");
    fprintf (out,
	     "\t\t<table cellspacing=\"4\" callpadding=\"4\" border=\"1\">\n");
    fprintf (out,
	     "\t\t\t<tr><td class=\"title\" colspan=\"2\" align=\"center\">Table / Layer</td></tr>\n");
    fprintf (out, "\t\t\t<tr><td>Table</td><td>%s</td></tr>\n", table);
    fprintf (out, "\t\t\t<tr><td>Geometry Column</td><td>%s</td></tr>\n", geom);
    fprintf (out, "\t\t\t<tr><td>Geometry Type</td><td>%s</td></tr>\n", x_type);
    fprintf (out, "\t\t\t<tr><td>Dimensions</td><td>%s</td></tr>\n", x_dims);
    fprintf (out, "\t\t\t<tr><td>SRID</td><td align=\"right\">%d</td></tr>\n",
	     srid);
    fprintf (out,
	     "\t\t\t<tr><td class=\"title\" colspan=\"2\" align=\"center\">Statistics</td></tr>\n");
    fprintf (out,
	     "\t\t\t<tr><td>Invalid Geometries</td><td align=\"right\">%d</td></tr>\n",
	     report->n_invalids);
    if (report->n_repaired > 0)
	fprintf (out,
		 "\t\t\t<tr><td>Repaired Geometries (fully recovered)</td><td class=\"ok\" align=\"right\">%d</td></tr>\n",
		 report->n_repaired);
    else
	fprintf (out,
		 "\t\t\t<tr><td>Repaired Geometries (fully recovered)</td><td align=\"right\">NONE</td></tr>\n");
    if (report->n_discarded > 0)
	fprintf (out,
		 "\t\t\t<tr><td>Repaired Geometries (by discarding fragments)</td><td class=\"wng\" align=\"right\">%d</td></tr>\n",
		 report->n_discarded);
    else
	fprintf (out,
		 "\t\t\t<tr><td>Repaired Geometries (by discarding fragments)</td><td align=\"right\">NONE</td></tr>\n");
    if (report->n_not_repaired > 0)
	fprintf (out,
		 "\t\t\t<tr><td>Failures (Not Repaired Geometries)</td><td class=\"err\" align=\"right\">%d</td></tr>\n",
		 report->n_not_repaired);
    else
	fprintf (out,
		 "\t\t\t<tr><td>Failures (Not Repaired Geometries)</td><td align=\"right\">NONE</td></tr>\n");
    fprintf (out,
	     "\t\t\t<tr><td class=\"title\" colspan=\"2\" align=\"center\">Suggested Action</td></tr>\n");
    if (report->n_not_repaired > 0)
      {
	  fprintf (out,
		   "\t\t\t<tr><td colspan=\"2\" class=\"err\">This layer contains severly damaged Geometries (beyond any possible repair);<br>");
	  fprintf (out,
		   "please manually check the <b>%s</b> table.</td></tr>\n",
		   tmp_table);
      }
    else if (report->n_discarded > 0)
      {
	  fprintf (out,
		   "\t\t\t<tr><td colspan=\"2\" class=\"wng\">This layer contains valid but dubious Geometries;<br>");
	  fprintf (out, "a repair action is suggested.</td></tr>\n");
      }
    else if (report->n_repaired > 0)
      {
	  fprintf (out,
		   "\t\t\t<tr><td colspan=\"2\" class=\"ok\">This layer has been successfully repaired and is now completely valid;<br>");
	  fprintf (out,
		   "doesn't require any further corrective action.</td></tr>\n");
      }
    else
      {
	  fprintf (out,
		   "\t\t\t<tr><td colspan=\"2\" class=\"ok\">This layer already was completely valid;<br>");
	  fprintf (out, "no corrective action was applied.</td></tr>\n");
      }
    fprintf (out, "\t\t</table>\n");

    if (report->n_not_repaired != 0)
      {
	  /* generating the NotRepaired sub-report */
	  fprintf (out, "\t\t\t<hr>\n\t\t<h2>Not Repaired Geometries</h2>\n");
	  fprintf (out,
		   "\t\t<table cellspacing=\"4\" callpadding=\"4\" border=\"1\">\n");
	  fprintf (out,
		   "\t\t\t<tr><td class=\"title\" align=\"center\">ROWID</td>");
	  fprintf (out, "<td class=\"title\" align=\"center\">Status</td>");
	  fprintf (out,
		   "<td class=\"title\" align=\"center\">Error Message</td></tr>\n");
	  p_r = report->first;
	  while (p_r)
	    {
		if (p_r->repaired != 0)
		  {
		      p_r = p_r->next;
		      continue;
		  }
		sprintf (num, FRMT64, p_r->rowid);
		p_msg = "Unknown cause";
		if (p_r->error)
		    p_msg = p_r->error;
		else if (p_r->warning)
		    p_msg = p_r->warning;
		fprintf (out,
			 "\t\t\t<tr><td align=\"right\">%s</td><td class=\"err\">Not Repaired (beyond possible repair)</td><td>%s</td></tr>\n",
			 num, p_msg);
		p_r = p_r->next;
	    }
	  fprintf (out, "\t\t</table>\n");
      }

    if (report->n_discarded != 0)
      {
	  /* generating the DiscardedFragments sub-report */
	  fprintf (out,
		   "\t\t\t<hr>\n\t\t<h2>Repaired Geometries (by discarding fragments)</h2>\n");
	  fprintf (out,
		   "\t\t<table cellspacing=\"4\" callpadding=\"4\" border=\"1\">\n");
	  fprintf (out,
		   "\t\t\t<tr><td class=\"title\" align=\"center\">ROWID</td>");
	  fprintf (out, "<td class=\"title\" align=\"center\">Status</td>");
	  fprintf (out,
		   "<td class=\"title\" align=\"center\">Discarded fragments summary</td></tr>\n");
	  p_r = report->first;
	  while (p_r)
	    {
		if (p_r->summary == NULL)
		  {
		      p_r = p_r->next;
		      continue;
		  }
		sprintf (num, FRMT64, p_r->rowid);
		fprintf (out,
			 "\t\t\t<tr><td align=\"right\">%s</td><td class=\"wng\">Repaired by discarding fragments</td><td>%s</td></tr>\n",
			 num, p_r->summary);
		p_r = p_r->next;
	    }
	  fprintf (out, "\t\t</table>\n");
      }

/* generating the HTML footer */
    fprintf (out, "\t</body>\n</html>\n");

    fclose (out);

/* post-processing actions */
    if (report->n_invalids == 0)
      {
	  /* we can immediately drop the tmp-table for sure */
	  drop_tmp_table (sqlite, tmp_table);
      }
    if (report->n_repaired > 0 && report->n_discarded == 0
	&& report->n_not_repaired == 0)
      {
	  /* we can safely update all repaired geometries, then dropping the tmp_table */
	  update_repaired (sqlite, table, geom, tmp_table, gtype,
			   report->repaired_type);
      }

    if (n_invalids != NULL)
	*n_invalids = report->n_invalids;
    if (n_repaired != NULL)
	*n_repaired = report->n_repaired;
    if (n_discarded != NULL)
	*n_discarded = report->n_discarded;
    if (n_failures != NULL)
	*n_failures = report->n_not_repaired;
    free_sanitize_report (report);
    return 1;
  stop:
    free_sanitize_report (report);
    if (out)
	fclose (out);
	
#endif /* end LWGEOM conditional */
    
    return 0;
}

SPATIALITE_DECLARE int
sanitize_geometry_column (sqlite3 * sqlite, const char *table, const char *geom,
			  const char *tmp_table, const char *report_path,
			  int *n_invalids, int *n_repaired, int *n_discarded,
			  int *n_failures, char **err_msg)
{
    return sanitize_geometry_column_common (NULL, sqlite, table, geom,
					    tmp_table, report_path, n_invalids,
					    n_repaired, n_discarded, n_failures,
					    err_msg);
}

SPATIALITE_DECLARE int
sanitize_geometry_column_r (const void *p_cache, sqlite3 * sqlite,
			    const char *table, const char *geom,
			    const char *tmp_table, const char *report_path,
			    int *n_invalids, int *n_repaired, int *n_discarded,
			    int *n_failures, char **err_msg)
{
    return sanitize_geometry_column_common (p_cache, sqlite, table, geom,
					    tmp_table, report_path, n_invalids,
					    n_repaired, n_discarded, n_failures,
					    err_msg);
}

static int
sanitize_all_geometry_columns_common (const void *p_cache, sqlite3 * sqlite,
				      const char *tmp_prefix,
				      const char *output_dir,
				      int *x_not_repaired, char **err_msg)
{
/* attempts to repair invalid Geometries from all Geometry Columns */
    const char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    int n_invalids;
    int n_repaired;
    int n_discarded;
    int n_failures;
    char *report;
    char *tmp_table;
    FILE *out = NULL;
    time_t v_time;
    struct tm *v_tm;
    const char *day;
    const char *month;
    int sum_not_repaired = 0;

    if (err_msg != NULL)
	*err_msg = NULL;

/* attempting to create the output directory */
#if defined(_WIN32)
    _mkdir (output_dir);
#else
    mkdir (output_dir, 0777);
#endif

/* opening the HTML report */
    report = sqlite3_mprintf ("%s/index.html", output_dir);
    out = fopen (report, "wb");
    sqlite3_free (report);
    if (out == NULL)
	goto stop;

/* generating the HTML header */
    fprintf (out,
	     "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
    fprintf (out, "<html>\n\t<head>\n");
    fprintf (out,
	     "\t\t<meta content=\"text/html; charset=UTF-8\" http-equiv=\"content-type\">\n");
    fprintf (out,
	     "\t\t<title>SpatiaLite Repair Geometries - All Tables</title>\n");
    fprintf (out, "\t\t<style type=\"text/css\">\n");
    fprintf (out, "\t\t\th1 {color:navy;}\n");
    fprintf (out, "\t\t\ttd.title {background-color:silver;}\n");
    fprintf (out, "\t\t\ttd.ok {background-color:#00ff00;}\n");
    fprintf (out, "\t\t\ttd.err {background-color:#ff0000;}\n");
    fprintf (out, "\t\t\ttd.wng {background-color:#ffff00;}\n");
    fprintf (out, "\t\t\ttd.nil {background-color:#ffffff;}\n");
    fprintf (out, "\t\t</style>\n");
    fprintf (out, "\t</head>\n\t<body bgcolor=\"#f8fff8\">\n");

    time (&v_time);
    v_tm = localtime (&v_time);
    switch (v_tm->tm_wday)
      {
      case 0:
	  day = "Sun";
	  break;
      case 1:
	  day = "Mon";
	  break;
      case 2:
	  day = "Tue";
	  break;
      case 3:
	  day = "Wed";
	  break;
      case 4:
	  day = "Thu";
	  break;
      case 5:
	  day = "Fri";
	  break;
      case 6:
	  day = "Sat";
	  break;
      };
    switch (v_tm->tm_mon)
      {
      case 0:
	  month = "Jan";
	  break;
      case 1:
	  month = "Feb";
	  break;
      case 2:
	  month = "Mar";
	  break;
      case 3:
	  month = "Apr";
	  break;
      case 4:
	  month = "May";
	  break;
      case 5:
	  month = "Jun";
	  break;
      case 6:
	  month = "Jul";
	  break;
      case 7:
	  month = "Aug";
	  break;
      case 8:
	  month = "Sep";
	  break;
      case 9:
	  month = "Oct";
	  break;
      case 10:
	  month = "Nov";
	  break;
      case 11:
	  month = "Dec";
	  break;
      };
    fprintf (out, "\t\t%d-%s-%d, %s [%02d:%02d:%02d]<br>\n",
	     v_tm->tm_year + 1900, month, v_tm->tm_mday, day, v_tm->tm_hour,
	     v_tm->tm_min, v_tm->tm_sec);
    fprintf (out, "\t\t<h1>SpatiaLite Repair Geometries</h1>\n");

/* table header */
    fprintf (out,
	     "\t\t<table cellspacing=\"4\" callpadding=\"4\" border=\"1\">\n");
    fprintf (out,
	     "\t\t\t<tr><td class=\"title\" align=\"center\">Show Details</td>");
    fprintf (out, "<td class=\"title\" align=\"center\">Table</td>");
    fprintf (out, "<td class=\"title\" align=\"center\">Geometry</td>");
    fprintf (out,
	     "<td class=\"title\" align=\"center\">Invalid Geometries</td>");
    fprintf (out,
	     "<td class=\"title\" align=\"center\">Repaired Geometries</td>");
    fprintf (out,
	     "<td class=\"title\" align=\"center\">Repaired Geometries<br>(by Discarding Fragments)</td>");
    fprintf (out,
	     "<td class=\"title\" align=\"center\">Repair Failures<br>(beyond possible repair)</td>");
    fprintf (out,
	     "<td class=\"title\" align=\"center\">Suggested Action</td></tr>\n");

    sql = "SELECT f_table_name, f_geometry_column FROM geometry_columns";
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	goto stop;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		const char *cls_1;
		const char *cls_2;
		const char *cls_msg;
		const char *p_msg;
		const char *table = results[(i * columns) + 0];
		const char *geom = results[(i * columns) + 1];
		report = sqlite3_mprintf ("%s/lyr_%04d.html", output_dir, i);
		tmp_table =
		    sqlite3_mprintf ("%s%s_%s", tmp_prefix, table, geom);
		if (p_cache != NULL)
		    ret =
			sanitize_geometry_column_r (p_cache, sqlite, table,
						    geom, tmp_table, report,
						    &n_invalids, &n_repaired,
						    &n_discarded, &n_failures,
						    err_msg);
		else
		    ret =
			sanitize_geometry_column (sqlite, table, geom,
						  tmp_table, report,
						  &n_invalids, &n_repaired,
						  &n_discarded, &n_failures,
						  err_msg);
		sqlite3_free (report);
		sqlite3_free (tmp_table);
		fprintf (out,
			 "\t\t\t<tr><td align=\"center\"><a href=\"./lyr_%04d.html\">show</a></td>",
			 i);
		fprintf (out, "<td>%s</td><td>%s</td>", table, geom);
		sum_not_repaired += n_failures;
		if (n_invalids == 0)
		  {
		      p_msg = "NONE: this layer was already fully valid";
		      cls_msg = "nil";
		  }
		else if (n_discarded == 0 && n_failures == 0)
		  {
		      p_msg =
			  "NONE: this layer has been successfully sanitized and is now fully valid";
		      cls_msg = "ok";
		  }
		else if (n_discarded == 0 && n_failures > 0)
		  {
		      p_msg = "Please check all discarded fragments";
		      cls_msg = "wng";
		  }
		else
		  {
		      p_msg =
			  "Manually adjust all Geometries beyond possible repair, then retry";
		      cls_msg = "err";
		  }
		if (n_repaired == 0)
		    cls_1 = "nil";
		else
		    cls_1 = "ok";
		fprintf (out,
			 "<td align=\"right\">%d</td><td class=\"%s\" align=\"right\">%d</td>",
			 n_invalids, cls_1, n_repaired);
		if (n_discarded == 0)
		    cls_1 = "nil";
		else
		    cls_1 = "wng";
		if (n_failures == 0)
		    cls_2 = "nil";
		else
		    cls_2 = "err";
		fprintf (out,
			 "<td class=\"%s\" align=\"right\">%d</td><td class=\"%s\" align=\"right\">%d</td>",
			 cls_1, n_discarded, cls_2, n_failures);
		fprintf (out, "<td class=\"%s\">%s</td></tr>\n", cls_msg,
			 p_msg);
	    }
      }
    sqlite3_free_table (results);

/* generating the HTML footer */
    fprintf (out, "\t\t</table>\n\t</body>\n</html>\n");

    fclose (out);
    if (x_not_repaired != NULL)
	*x_not_repaired = sum_not_repaired;
    return 1;
  stop:
    return 0;
}

SPATIALITE_DECLARE int
sanitize_all_geometry_columns (sqlite3 * sqlite,
			       const char *tmp_prefix,
			       const char *output_dir, int *x_not_repaired,
			       char **err_msg)
{
    return sanitize_all_geometry_columns_common (NULL, sqlite, tmp_prefix,
						 output_dir, x_not_repaired,
						 err_msg);
}

SPATIALITE_DECLARE int
sanitize_all_geometry_columns_r (const void *p_cache, sqlite3 * sqlite,
				 const char *tmp_prefix,
				 const char *output_dir, int *x_not_repaired,
				 char **err_msg)
{
    return sanitize_all_geometry_columns_common (p_cache, sqlite, tmp_prefix,
						 output_dir, x_not_repaired,
						 err_msg);
}

#else /* LIBXML2 isn't enabled */

SPATIALITE_DECLARE int
sanitize_all_geometry_columns (sqlite3 * sqlite,
			       const char *tmp_prefix,
			       const char *output_dir, int *x_not_repaired,
			       char **err_msg)
{
/* LWGEOM isn't enabled: always returning an error */
    int len;
    const char *msg = "Sorry ... libspatialite was built disabling LWGEOM\n"
	"and is thus unable to support MakeValid";

/* silencing stupid compiler warnings */
    if (sqlite == NULL || tmp_prefix == NULL || output_dir == NULL
	|| x_not_repaired == NULL)
	tmp_prefix = NULL;

    if (err_msg == NULL)
	return 0;
    len = strlen (msg);
    *err_msg = malloc (len + 1);
    strcpy (*err_msg, msg);
    return 0;
}

SPATIALITE_DECLARE int
sanitize_all_geometry_columns_r (const void *p_cache, sqlite3 * sqlite,
				 const char *tmp_prefix,
				 const char *output_dir, int *x_not_repaired,
				 char **err_msg)
{
/* LWGEOM isn't enabled: always returning an error */
    int len;
    const char *msg = "Sorry ... libspatialite was built disabling LWGEOM\n"
	"and is thus unable to support MakeValid";

/* silencing stupid compiler warnings */
    if (p_cache == NULL || sqlite == NULL || tmp_prefix == NULL
	|| output_dir == NULL || x_not_repaired == NULL)
	tmp_prefix = NULL;

    if (err_msg == NULL)
	return 0;
    len = strlen (msg);
    *err_msg = malloc (len + 1);
    strcpy (*err_msg, msg);
    return 0;
}

SPATIALITE_DECLARE int
sanitize_geometry_column (sqlite3 * sqlite, const char *table, const char *geom,
			  const char *tmp_table, const char *report_path,
			  int *n_invalids, int *n_repaired, int *n_discarded,
			  int *n_failures, char **err_msg)
{
/* LWGEOM isn't enabled: always returning an error */
    int len;
    const char *msg = "Sorry ... libspatialite was built disabling LWGEOM\n"
	"and is thus unable to support MakeValid";

/* silencing stupid compiler warnings */
    if (sqlite == NULL || table == NULL || geom == NULL || tmp_table == NULL
	|| report_path == NULL || n_invalids == NULL || n_repaired == NULL
	|| n_discarded == NULL || n_failures == NULL)
	table = NULL;


    if (err_msg == NULL)
	return 0;
    len = strlen (msg);
    *err_msg = malloc (len + 1);
    strcpy (*err_msg, msg);
    return 0;
}

SPATIALITE_DECLARE int
sanitize_geometry_column_r (const void *p_cache, sqlite3 * sqlite,
			    const char *table, const char *geom,
			    const char *tmp_table, const char *report_path,
			    int *n_invalids, int *n_repaired, int *n_discarded,
			    int *n_failures, char **err_msg)
{
/* LWGEOM isn't enabled: always returning an error */
    int len;
    const char *msg = "Sorry ... libspatialite was built disabling LWGEOM\n"
	"and is thus unable to support MakeValid";

/* silencing stupid compiler warnings */
    if (p_cache == NULL || sqlite == NULL || table == NULL || geom == NULL
	|| tmp_table == NULL || report_path == NULL || n_invalids == NULL
	|| n_repaired == NULL || n_discarded == NULL || n_failures == NULL)
	table = NULL;


    if (err_msg == NULL)
	return 0;
    len = strlen (msg);
    *err_msg = malloc (len + 1);
    strcpy (*err_msg, msg);
    return 0;
}

#endif /* end LWGEOM conditionals */


#ifndef OMIT_GEOS		/* only if GEOS is supported */

static struct validity_report *
alloc_validity_report (void)
{
/* allocating the report container struct */
    struct validity_report *p = malloc (sizeof (struct validity_report));
    p->first = NULL;
    p->last = NULL;
    p->n_rows = 0;
    p->n_nullgeoms = 0;
    p->n_valids = 0;
    p->n_invalids = 0;
    p->n_warnings = 0;
    return p;
}

static void
free_validity_report (struct validity_report *p)
{
/* memory cleanup: freeing the report container struct */
    struct validity_report_row *r;
    struct validity_report_row *rn;
    r = p->first;
    while (r)
      {
	  rn = r->next;
	  if (r->warning != NULL)
	      free (r->warning);
	  if (r->error != NULL)
	      free (r->error);
	  if (r->extra != NULL)
	      free (r->extra);
	  free (r);
	  r = rn;
      }
    free (p);
}

static void
addMessageToValidityReport (struct validity_report *report, sqlite3_int64 rowid,
			    int valid, const char *error, const char *warning,
			    const char *extra)
{
/* adding a message to the report */
    int len;
    struct validity_report_row *r =
	malloc (sizeof (struct validity_report_row));
    r->rowid = rowid;
    r->valid = valid;
    r->error = NULL;
    r->warning = NULL;
    r->extra = NULL;
    r->next = NULL;
    if (error)
      {
	  len = strlen (error);
	  r->error = malloc (len + 1);
	  strcpy (r->error, error);
      }
    if (warning)
      {
	  len = strlen (warning);
	  r->warning = malloc (len + 1);
	  strcpy (r->warning, warning);
      }
    if (extra)
      {
	  len = strlen (extra);
	  r->extra = malloc (len + 1);
	  strcpy (r->extra, extra);
      }
    if (!valid)
	report->n_invalids += 1;
    else
	report->n_warnings += 1;
    if (report->first == NULL)
	report->first = r;
    if (report->last != NULL)
	report->last->next = r;
    report->last = r;
}

static int
check_geometry_column_common (const void *p_cache, sqlite3 * sqlite,
			      const char *table, const char *geom,
			      const char *report_path, int *n_rows,
			      int *n_invalids, char **err_msg)
{
/* checks a Geometry Column for validity */
    char *sql;
    char *xtable;
    char *xgeom;
    sqlite3_stmt *stmt;
    int ret;
    int gtype;
    int srid;
    const char *x_type;
    const char *x_dims;
    char num[256];
    time_t v_time;
    struct tm *v_tm;
    const char *day;
    const char *month;
    FILE *out = NULL;
    const char *p_msg;
    int len;
    struct validity_report_row *p_r;
    struct validity_report *report = alloc_validity_report ();

    if (err_msg != NULL)
	*err_msg = NULL;

    if (!check_table_column (sqlite, table, geom, &gtype, &srid))
      {
	  spatialite_e ("check_geometry_column error: <%s>\n"
			"Not defined in \"geometry_columns\"", table);
	  if (err_msg != NULL)
	    {
		char *msg =
		    sqlite3_mprintf ("check_geometry_column error: <%s><%s>\n"
				     "Not defined in \"geometry_columns\"",
				     table, geom);
		len = strlen (msg);
		*err_msg = malloc (len + 1);
		strcpy (*err_msg, msg);
		sqlite3_free (msg);
	    }
	  goto stop;
      }

/* decoding Geometry Type and Dimensions */
    switch (gtype)
      {
      case 0:
	  x_type = "GEOMETRY";
	  x_dims = "XY";
	  break;
      case 1000:
	  x_type = "GEOMETRY";
	  x_dims = "XYZ";
	  break;
      case 2000:
	  x_type = "GEOMETRY";
	  x_dims = "XYM";
	  break;
      case 3000:
	  x_type = "GEOMETRY";
	  x_dims = "XYZM";
	  break;
      case 1:
	  x_type = "POINT";
	  x_dims = "XY";
	  break;
      case 1001:
	  x_type = "POINT";
	  x_dims = "XYZ";
	  break;
      case 2001:
	  x_type = "POINT";
	  x_dims = "XYM";
	  break;
      case 3001:
	  x_type = "POINT";
	  x_dims = "XYZM";
	  break;
      case 2:
	  x_type = "LINESTRING";
	  x_dims = "XY";
	  break;
      case 1002:
	  x_type = "LINESTRING";
	  x_dims = "XYZ";
	  break;
      case 2002:
	  x_type = "LINESTRING";
	  x_dims = "XYM";
	  break;
      case 3002:
	  x_type = "LINESTRING";
	  x_dims = "XYZM";
	  break;
      case 3:
	  x_type = "POLYGON";
	  x_dims = "XY";
	  break;
      case 1003:
	  x_type = "POLYGON";
	  x_dims = "XYZ";
	  break;
      case 2003:
	  x_type = "POLYGON";
	  x_dims = "XYM";
	  break;
      case 3003:
	  x_type = "POLYGON";
	  x_dims = "XYZM";
	  break;
      case 4:
	  x_type = "MULTIPOINT";
	  x_dims = "XY";
	  break;
      case 1004:
	  x_type = "MULTIPOINT";
	  x_dims = "XYZ";
	  break;
      case 2004:
	  x_type = "MULTIPOINT";
	  x_dims = "XYM";
	  break;
      case 3004:
	  x_type = "MULTIPOINT";
	  x_dims = "XYZM";
	  break;
      case 5:
	  x_type = "MULTILINESTRING";
	  x_dims = "XY";
	  break;
      case 1005:
	  x_type = "MULTILINESTRING";
	  x_dims = "XYZ";
	  break;
      case 2005:
	  x_type = "MULTILINESTRING";
	  x_dims = "XYM";
	  break;
      case 3005:
	  x_type = "MULTILINESTRING";
	  x_dims = "XYZM";
	  break;
      case 6:
	  x_type = "MULTIPOLYGON";
	  x_dims = "XY";
	  break;
      case 1006:
	  x_type = "MULTIPOLYGON";
	  x_dims = "XYZ";
	  break;
      case 2006:
	  x_type = "MULTIPOLYGON";
	  x_dims = "XYM";
	  break;
      case 3006:
	  x_type = "MULTIPOLYGON";
	  x_dims = "XYZM";
	  break;
      case 7:
	  x_type = "GEOMETRYCOLLECTION";
	  x_dims = "XY";
	  break;
      case 1007:
	  x_type = "GEOMETRYCOLLECTION";
	  x_dims = "XYZ";
	  break;
      case 2007:
	  x_type = "GEOMETRYCOLLECTION";
	  x_dims = "XYM";
	  break;
      case 3007:
	  x_type = "GEOMETRYCOLLECTION";
	  x_dims = "XYZM";
	  break;
      default:
	  x_type = "UNKNOWN";
	  x_dims = "UNKNOWN";
	  break;
      };

    xtable = gaiaDoubleQuotedSql (table);
    xgeom = gaiaDoubleQuotedSql (geom);
    sql = sqlite3_mprintf ("SELECT ROWID, \"%s\" FROM \"%s\"", xgeom, xtable);
    free (xtable);
    free (xgeom);
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check_geometry_column error: <%s>\n",
			sqlite3_errmsg (sqlite));
	  if (err_msg != NULL)
	    {
		char *msg =
		    sqlite3_mprintf ("check_geometry_column error: <%s>\n",
				     sqlite3_errmsg (sqlite));
		len = strlen (msg);
		*err_msg = malloc (len + 1);
		strcpy (*err_msg, msg);
		sqlite3_free (msg);
	    }
	  goto stop;
      }

/* opening the HTML report */
    out = fopen (report_path, "wb");
    if (out == NULL)
	goto stop;

    while (1)
      {
	  /* scrolling the result set */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing one row from the resultset */
		gaiaGeomCollPtr geom = NULL;
		sqlite3_int64 rowid = sqlite3_column_int (stmt, 0);
		report->n_rows += 1;
		if (sqlite3_column_type (stmt, 1) == SQLITE_BLOB)
		  {
		      const unsigned char *blob = sqlite3_column_blob (stmt, 1);
		      int n_bytes = sqlite3_column_bytes (stmt, 1);
		      geom = gaiaFromSpatiaLiteBlobWkb (blob, n_bytes);
		  }
		if (geom)
		  {
		      /* checking a geometry for validity */
		      int valid;
		      const char *error;
		      const char *warning;
		      const char *extra;
		      if (p_cache != NULL)
			{
			    gaiaResetGeosMsg_r (p_cache);
			    valid = gaiaIsValid_r (p_cache, geom);
			    error = gaiaGetGeosErrorMsg_r (p_cache);
			    warning = gaiaGetGeosWarningMsg_r (p_cache);
			    extra = gaiaGetGeosAuxErrorMsg_r (p_cache);
			}
		      else
			{
			    gaiaResetGeosMsg ();
			    valid = gaiaIsValid (geom);
			    error = gaiaGetGeosErrorMsg ();
			    warning = gaiaGetGeosWarningMsg ();
			    extra = gaiaGetGeosAuxErrorMsg ();
			}
		      if (!valid || error || warning)
			  addMessageToValidityReport (report, rowid, valid,
						      error, warning, extra);
		      else
			  report->n_valids += 1;
		      gaiaFreeGeomColl (geom);
		  }
		else
		    report->n_nullgeoms += 1;
	    }
	  else
	    {
		spatialite_e ("check_geometry_column error: <%s>\n",
			      sqlite3_errmsg (sqlite));
		if (err_msg != NULL)
		  {
		      char *msg =
			  sqlite3_mprintf
			  ("check_geometry_column error: <%s>\n",
			   sqlite3_errmsg (sqlite));
		      len = strlen (msg);
		      *err_msg = malloc (len + 1);
		      strcpy (*err_msg, msg);
		      sqlite3_free (msg);
		  }
		break;
	    }
      }
    sqlite3_finalize (stmt);

/* generating the HTML header */
    fprintf (out,
	     "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
    fprintf (out, "<html>\n\t<head>\n");
    fprintf (out,
	     "\t\t<meta content=\"text/html; charset=UTF-8\" http-equiv=\"content-type\">\n");
    fprintf (out, "\t\t<title>SpatiaLite Validity Check - Table: %s</title>\n",
	     table);
    fprintf (out, "\t\t<style type=\"text/css\">\n");
    fprintf (out, "\t\t\th1 {color:red;}\n");
    fprintf (out, "\t\t\th2 {color:navy;}\n");
    fprintf (out, "\t\t\ttd.title {background-color:silver;}\n");
    fprintf (out, "\t\t\ttd.ok {background-color:#00ff00;}\n");
    fprintf (out, "\t\t\ttd.err {background-color:#ff0000;}\n");
    fprintf (out, "\t\t\ttd.wng {background-color:#ffff00;}\n");
    fprintf (out, "\t\t</style>\n");
    fprintf (out, "\t</head>\n\t<body bgcolor=\"#f8fff8\">\n");

    time (&v_time);
    v_tm = localtime (&v_time);
    switch (v_tm->tm_wday)
      {
      case 0:
	  day = "Sun";
	  break;
      case 1:
	  day = "Mon";
	  break;
      case 2:
	  day = "Tue";
	  break;
      case 3:
	  day = "Wed";
	  break;
      case 4:
	  day = "Thu";
	  break;
      case 5:
	  day = "Fri";
	  break;
      case 6:
	  day = "Sat";
	  break;
      };
    switch (v_tm->tm_mon)
      {
      case 0:
	  month = "Jan";
	  break;
      case 1:
	  month = "Feb";
	  break;
      case 2:
	  month = "Mar";
	  break;
      case 3:
	  month = "Apr";
	  break;
      case 4:
	  month = "May";
	  break;
      case 5:
	  month = "Jun";
	  break;
      case 6:
	  month = "Jul";
	  break;
      case 7:
	  month = "Aug";
	  break;
      case 8:
	  month = "Sep";
	  break;
      case 9:
	  month = "Oct";
	  break;
      case 10:
	  month = "Nov";
	  break;
      case 11:
	  month = "Dec";
	  break;
      };
    fprintf (out, "\t\t%d-%s-%d, %s [%02d:%02d:%02d]<br>\n",
	     v_tm->tm_year + 1900, month, v_tm->tm_mday, day, v_tm->tm_hour,
	     v_tm->tm_min, v_tm->tm_sec);
    fprintf (out, "\t\t<h1>SpatiaLite Validity Check</h1>\n");

/* generating the summary sub-report */
    fprintf (out, "\t\t<h2>Summary</h2>\n");
    fprintf (out,
	     "\t\t<table cellspacing=\"4\" callpadding=\"4\" border=\"1\">\n");
    fprintf (out,
	     "\t\t\t<tr><td class=\"title\" colspan=\"2\" align=\"center\">Table / Layer</td></tr>\n");
    fprintf (out, "\t\t\t<tr><td>Table</td><td>%s</td></tr>\n", table);
    fprintf (out, "\t\t\t<tr><td>Geometry Column</td><td>%s</td></tr>\n", geom);
    fprintf (out, "\t\t\t<tr><td>Geometry Type</td><td>%s</td></tr>\n", x_type);
    fprintf (out, "\t\t\t<tr><td>Dimensions</td><td>%s</td></tr>\n", x_dims);
    fprintf (out, "\t\t\t<tr><td>SRID</td><td align=\"right\">%d</td></tr>\n",
	     srid);
    fprintf (out,
	     "\t\t\t<tr><td class=\"title\" colspan=\"2\" align=\"center\">Statistics</td></tr>\n");
    fprintf (out,
	     "\t\t\t<tr><td>Total Rows</td><td align=\"right\">%d</td></tr>\n",
	     report->n_rows);
    if (report->n_nullgeoms > 0)
	fprintf (out,
		 "\t\t\t<tr><td>NULL Geometries</td><td class=\"wng\" align=\"right\">%d</td></tr>\n",
		 report->n_nullgeoms);
    else
	fprintf (out,
		 "\t\t\t<tr><td>NULL Geometries</td><td align=\"right\">NONE</td></tr>\n");
    fprintf (out,
	     "\t\t\t<tr><td>Valid Geometries (full valid)</td><td class=\"ok\" align=\"right\">%d</td></tr>\n",
	     report->n_valids);
    if (report->n_warnings > 0)
	fprintf (out,
		 "\t\t\t<tr><td>Valid Geometries (minor issues)</td><td class=\"wng\" align=\"right\">%d</td></tr>\n",
		 report->n_warnings);
    else
	fprintf (out,
		 "\t\t\t<tr><td>Valid Geometries (minor issues)</td><td align=\"right\">NONE</td></tr>\n");
    if (report->n_invalids > 0)
	fprintf (out,
		 "\t\t\t<tr><td>Invalid Geometries</td><td class=\"err\" align=\"right\">%d</td></tr>\n",
		 report->n_invalids);
    else
	fprintf (out,
		 "\t\t\t<tr><td>Invalid Geometries</td><td align=\"right\">NONE</td></tr>\n");
    fprintf (out,
	     "\t\t\t<tr><td class=\"title\" colspan=\"2\" align=\"center\">Suggested Action</td></tr>\n");
    if (report->n_invalids > 0)
      {
	  fprintf (out,
		   "\t\t\t<tr><td colspan=\"2\" class=\"err\">This layer contains invalid Geometries;<br>");
	  fprintf (out, "a repair action is urgently required.</td></tr>\n");
      }
    else if (report->n_warnings > 0)
      {
	  fprintf (out,
		   "\t\t\t<tr><td colspan=\"2\" class=\"wng\">This layer contains valid but dubious Geometries;<br>");
	  fprintf (out, "a repair action is suggested.</td></tr>\n");
      }
    else
      {
	  fprintf (out,
		   "\t\t\t<tr><td colspan=\"2\" class=\"ok\">This layer is perfectly valid;<br>");
	  fprintf (out, "doesn't require any corrective action.</td></tr>\n");
      }
    fprintf (out, "\t\t</table>\n");

    if (report->n_invalids > 0)
      {
	  /* generating the errors sub-report */
	  fprintf (out, "\t\t\t<hr>\n\t\t<h2>Invalid Geometries List</h2>\n");
	  fprintf (out,
		   "\t\t<table cellspacing=\"4\" callpadding=\"4\" border=\"1\">\n");
	  fprintf (out,
		   "\t\t\t<tr><td class=\"title\" align=\"center\">ROWID</td>");
	  fprintf (out,
		   "<td class=\"title\" align=\"center\">Error Cause</td></tr>\n");
	  p_r = report->first;
	  while (p_r)
	    {
		if (p_r->valid != 0)
		  {
		      p_r = p_r->next;
		      continue;
		  }
		sprintf (num, FRMT64, p_r->rowid);
		p_msg = "Unknown cause";
		if (p_r->error)
		    p_msg = p_r->error;
		else if (p_r->warning)
		    p_msg = p_r->warning;
		else if (p_r->extra)
		    p_msg = p_r->extra;
		fprintf (out,
			 "\t\t\t<tr><td align=\"right\">%s</td><td>%s</td></tr>\n",
			 num, p_msg);
		p_r = p_r->next;
	    }
	  fprintf (out, "\t\t</table>\n");
      }

    if (report->n_warnings > 0)
      {
	  /* generating the warnings sub-report */
	  fprintf (out,
		   "\t\t\t<hr>\n\t\t<h2>Valid Geometries List (anyway presenting some minor issue)</h2>\n");
	  fprintf (out,
		   "\t\t<table cellspacing=\"4\" callpadding=\"4\" border=\"1\">\n");
	  fprintf (out,
		   "\t\t\t<tr><td class=\"title\" align=\"center\">ROWID</td>");
	  fprintf (out,
		   "<td class=\"title\" align=\"center\">Warning Cause</td></tr>\n");
	  p_r = report->first;
	  while (p_r)
	    {
		if (p_r->valid != 1)
		  {
		      p_r = p_r->next;
		      continue;
		  }
		sprintf (num, FRMT64, p_r->rowid);
		p_msg = "Unknown cause";
		if (p_r->error)
		    p_msg = p_r->error;
		else if (p_r->warning)
		    p_msg = p_r->warning;
		fprintf (out,
			 "\t\t\t<tr><td align=\"right\">%s</td><td>%s</td></tr>\n",
			 num, p_msg);
		p_r = p_r->next;
	    }
	  fprintf (out, "\t\t</table>\n");
      }

/* generating the HTML footer */
    fprintf (out, "\t</body>\n</html>\n");

    fclose (out);
    if (n_rows != NULL)
	*n_rows = report->n_rows;
    if (n_invalids != NULL)
	*n_invalids = report->n_invalids;
    free_validity_report (report);
    return 1;
  stop:
    free_validity_report (report);
    if (out)
	fclose (out);
    return 0;
}

SPATIALITE_DECLARE int
check_geometry_column (sqlite3 * sqlite, const char *table, const char *geom,
		       const char *report_path, int *n_rows, int *n_invalids,
		       char **err_msg)
{
    return check_geometry_column_common (NULL, sqlite, table, geom, report_path,
					 n_rows, n_invalids, err_msg);
}

SPATIALITE_DECLARE int
check_geometry_column_r (const void *p_cache, sqlite3 * sqlite,
			 const char *table, const char *geom,
			 const char *report_path, int *n_rows, int *n_invalids,
			 char **err_msg)
{
    return check_geometry_column_common (p_cache, sqlite, table, geom,
					 report_path, n_rows, n_invalids,
					 err_msg);
}

static int
check_all_geometry_columns_common (const void *p_cache, sqlite3 * sqlite,
				   const char *output_dir, int *x_invalids,
				   char **err_msg)
{
/* checks all Geometry Columns for validity */
    const char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    int n_rows;
    int n_invalids;
    char *report;
    FILE *out = NULL;
    time_t v_time;
    struct tm *v_tm;
    const char *day;
    const char *month;
    int sum_invalids = 0;

/* attempting to create the output directory */
#if defined(_WIN32)
    _mkdir (output_dir);
#else
    mkdir (output_dir, 0777);
#endif

    if (err_msg != NULL)
	*err_msg = NULL;

/* opening the HTML report */
    report = sqlite3_mprintf ("%s/index.html", output_dir);
    out = fopen (report, "wb");
    sqlite3_free (report);
    if (out == NULL)
	goto stop;

/* generating the HTML header */
    fprintf (out,
	     "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
    fprintf (out, "<html>\n\t<head>\n");
    fprintf (out,
	     "\t\t<meta content=\"text/html; charset=UTF-8\" http-equiv=\"content-type\">\n");
    fprintf (out,
	     "\t\t<title>SpatiaLite Validity Check - All Tables</title>\n");
    fprintf (out, "\t\t<style type=\"text/css\">\n");
    fprintf (out, "\t\t\th1 {color:navy;}\n");
    fprintf (out, "\t\t\ttd.title {background-color:silver;}\n");
    fprintf (out, "\t\t\ttd.ok {background-color:#00ff00;}\n");
    fprintf (out, "\t\t\ttd.err {background-color:#ff0000;}\n");
    fprintf (out, "\t\t</style>\n");
    fprintf (out, "\t</head>\n\t<body bgcolor=\"#f8fff8\">\n");

    time (&v_time);
    v_tm = localtime (&v_time);
    switch (v_tm->tm_wday)
      {
      case 0:
	  day = "Sun";
	  break;
      case 1:
	  day = "Mon";
	  break;
      case 2:
	  day = "Tue";
	  break;
      case 3:
	  day = "Wed";
	  break;
      case 4:
	  day = "Thu";
	  break;
      case 5:
	  day = "Fri";
	  break;
      case 6:
	  day = "Sat";
	  break;
      };
    switch (v_tm->tm_mon)
      {
      case 0:
	  month = "Jan";
	  break;
      case 1:
	  month = "Feb";
	  break;
      case 2:
	  month = "Mar";
	  break;
      case 3:
	  month = "Apr";
	  break;
      case 4:
	  month = "May";
	  break;
      case 5:
	  month = "Jun";
	  break;
      case 6:
	  month = "Jul";
	  break;
      case 7:
	  month = "Aug";
	  break;
      case 8:
	  month = "Sep";
	  break;
      case 9:
	  month = "Oct";
	  break;
      case 10:
	  month = "Nov";
	  break;
      case 11:
	  month = "Dec";
	  break;
      };
    fprintf (out, "\t\t%d-%s-%d, %s [%02d:%02d:%02d]<br>\n",
	     v_tm->tm_year + 1900, month, v_tm->tm_mday, day, v_tm->tm_hour,
	     v_tm->tm_min, v_tm->tm_sec);
    fprintf (out, "\t\t<h1>SpatiaLite Validity Check</h1>\n");

/* table header */
    fprintf (out,
	     "\t\t<table cellspacing=\"4\" callpadding=\"4\" border=\"1\">\n");
    fprintf (out,
	     "\t\t\t<tr><td class=\"title\" align=\"center\">Show Details</td>");
    fprintf (out, "<td class=\"title\" align=\"center\">Table</td>");
    fprintf (out, "<td class=\"title\" align=\"center\">Geometry</td>");
    fprintf (out, "<td class=\"title\" align=\"center\">Total Rows</td>");
    fprintf (out,
	     "<td class=\"title\" align=\"center\">Invalid Geometries</td>");
    fprintf (out,
	     "<td class=\"title\" align=\"center\">Suggested Action</td></tr>\n");

    sql = "SELECT f_table_name, f_geometry_column FROM geometry_columns";
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	goto stop;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		const char *table = results[(i * columns) + 0];
		const char *geom = results[(i * columns) + 1];
		report = sqlite3_mprintf ("%s/lyr_%04d.html", output_dir, i);
		if (p_cache != NULL)
		    ret =
			check_geometry_column_r (p_cache, sqlite, table, geom,
						 report, &n_rows, &n_invalids,
						 err_msg);
		else
		    ret =
			check_geometry_column (sqlite, table, geom, report,
					       &n_rows, &n_invalids, err_msg);
		sqlite3_free (report);
		fprintf (out,
			 "\t\t\t<tr><td align=\"center\"><a href=\"./lyr_%04d.html\">show</a></td>",
			 i);
		fprintf (out, "<td>%s</td><td>%s</td>", table, geom);
		sum_invalids += n_invalids;
		if (n_invalids == 0)
		  {
		      fprintf (out,
			       "<td align=\"right\">%d</td><td class=\"ok\" align=\"right\">%d</td>",
			       n_rows, n_invalids);
		      fprintf (out,
			       "<td class=\"ok\">NONE: this layer is fully valid</td></tr>\n");
		  }
		else
		  {
		      fprintf (out,
			       "<td align=\"right\">%d</td><td class=\"err\" align=\"right\">%d</td>",
			       n_rows, n_invalids);
		      fprintf (out,
			       "<td class=\"err\">Repairing this layer is urgently required</td></tr>\n");
		  }
	    }
      }
    sqlite3_free_table (results);

/* generating the HTML footer */
    fprintf (out, "\t\t</table>\n\t</body>\n</html>\n");

    fclose (out);
    if (x_invalids != NULL)
	*x_invalids = sum_invalids;
    return 1;
  stop:
    return 0;
}

SPATIALITE_DECLARE int
check_all_geometry_columns (sqlite3 * sqlite,
			    const char *output_dir, int *x_invalids,
			    char **err_msg)
{
    return check_all_geometry_columns_common (NULL, sqlite, output_dir,
					      x_invalids, err_msg);
}

SPATIALITE_DECLARE int
check_all_geometry_columns_r (const void *p_cache, sqlite3 * sqlite,
			      const char *output_dir, int *x_invalids,
			      char **err_msg)
{
    return check_all_geometry_columns_common (p_cache, sqlite, output_dir,
					      x_invalids, err_msg);
}

#else

SPATIALITE_DECLARE int
check_all_geometry_columns (sqlite3 * sqlite,
			    const char *output_dir, int *x_invalids,
			    char **err_msg)
{
/* GEOS isn't enabled: always returning an error */
    int len;
    const char *msg = "Sorry ... libspatialite was built disabling LWGEOM\n"
	"and is thus unable to support IsValid";
/* silencing stupid compiler warnings */
    if (sqlite == NULL || output_dir == NULL || x_invalids == NULL)
	output_dir = NULL;

    if (err_msg == NULL)
	return 0;
    len = strlen (msg);
    *err_msg = malloc (len + 1);
    strcpy (*err_msg, msg);
    return 0;
}

SPATIALITE_DECLARE int
check_all_geometry_columns_r (const void *p_cache, sqlite3 * sqlite,
			      const char *output_dir, int *x_invalids,
			      char **err_msg)
{
/* GEOS isn't enabled: always returning an error */
    int len;
    const char *msg = "Sorry ... libspatialite was built disabling LWGEOM\n"
	"and is thus unable to support IsValid";
/* silencing stupid compiler warnings */
    if (p_cache == NULL || sqlite == NULL || output_dir == NULL
	|| x_invalids == NULL)
	output_dir = NULL;

    if (err_msg == NULL)
	return 0;
    len = strlen (msg);
    *err_msg = malloc (len + 1);
    strcpy (*err_msg, msg);
    return 0;
}

SPATIALITE_DECLARE int
check_geometry_column (sqlite3 * sqlite,
		       const char *table,
		       const char *geom,
		       const char *report_path,
		       int *n_rows, int *n_invalids, char **err_msg)
{
/* GEOS isn't enabled: always returning an error */
    int len;
    const char *msg = "Sorry ... libspatialite was built disabling GEOS\n"
	"and is thus unable to support IsValid";

/* silencing stupid compiler warnings */
    if (sqlite == NULL || table == NULL || geom == NULL ||
	report_path == NULL || n_rows == NULL || n_invalids == NULL)
	table = NULL;

    if (err_msg == NULL)
	return 0;
    len = strlen (msg);
    *err_msg = malloc (len + 1);
    strcpy (*err_msg, msg);
    return 0;
}

SPATIALITE_DECLARE int
check_geometry_column_r (const void *p_cache, sqlite3 * sqlite,
			 const char *table,
			 const char *geom,
			 const char *report_path,
			 int *n_rows, int *n_invalids, char **err_msg)
{
/* GEOS isn't enabled: always returning an error */
    int len;
    const char *msg = "Sorry ... libspatialite was built disabling GEOS\n"
	"and is thus unable to support IsValid";

/* silencing stupid compiler warnings */
    if (p_cache == NULL || sqlite == NULL || table == NULL || geom == NULL ||
	report_path == NULL || n_rows == NULL || n_invalids == NULL)
	table = NULL;

    if (err_msg == NULL)
	return 0;
    len = strlen (msg);
    *err_msg = malloc (len + 1);
    strcpy (*err_msg, msg);
    return 0;
}

#endif /* end GEOS conditionals */
