/*

 gg_lwgeom.c -- Gaia LWGEOM support
    
 version 4.3, 2015 June 29

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
 
Portions created by the Initial Developer are Copyright (C) 2012-2015
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

this module (wrapping liblwgeom APIs) has been entierely funded by:
Regione Toscana - Settore Sistema Informativo Territoriale ed Ambientale

*/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite_private.h>
#include <spatialite/sqlite.h>
#include <spatialite.h>
#include <spatialite/debug.h>

#include <spatialite/gaiageo.h>

#ifdef ENABLE_LWGEOM		/* enabling LWGEOM support */

#include <liblwgeom.h>

/* GLOBAL variables */
char *gaia_lwgeom_error_msg = NULL;
char *gaia_lwgeom_warning_msg = NULL;

const char splitelwgeomversion[] = LIBLWGEOM_VERSION;

SPATIALITE_PRIVATE const char *
splite_lwgeom_version (void)
{
    return splitelwgeomversion;
}

static void
lwgaia_noticereporter (const char *fmt, va_list ap)
{
    char *msg;
    if (!lw_vasprintf (&msg, fmt, ap))
      {
	  va_end (ap);
	  return;
      }
    spatialite_e ("LWGEOM notice: %s\n", msg);
    gaiaSetLwGeomWarningMsg (msg);
    free (msg);
}

static void
lwgaia_errorreporter (const char *fmt, va_list ap)
{
    char *msg;
    if (!lw_vasprintf (&msg, fmt, ap))
      {
	  va_end (ap);
	  return;
      }
    spatialite_e ("LWGEOM error: %s\n", msg);
    gaiaSetLwGeomErrorMsg (msg);
    free (msg);
}

#ifndef POSTGIS_2_1
/* liblwgeom initializion function: required by PostGIS 2.0.x */
void
lwgeom_init_allocators (void)
{
/* Set up liblwgeom to run in stand-alone mode using the
* usual system memory handling functions. */
    lwalloc_var = default_allocator;
    lwrealloc_var = default_reallocator;
    lwfree_var = default_freeor;
    lwnotice_var = lwgaia_noticereporter;
    lwerror_var = lwgaia_errorreporter;
}
#else
/* liblwgeom initialization function: required by PostGIS 2.1.x */
SPATIALITE_PRIVATE void
splite_lwgeom_init (void)
{
    lwgeom_set_handlers (NULL, NULL, NULL, lwgaia_errorreporter,
			 lwgaia_noticereporter);
}
#endif

GAIAGEO_DECLARE void
gaiaResetLwGeomMsg ()
{
/* resets the LWGEOM error and warning messages */
    if (gaia_lwgeom_error_msg != NULL)
	free (gaia_lwgeom_error_msg);
    if (gaia_lwgeom_warning_msg != NULL)
	free (gaia_lwgeom_warning_msg);
    gaia_lwgeom_error_msg = NULL;
    gaia_lwgeom_warning_msg = NULL;
}

GAIAGEO_DECLARE const char *
gaiaGetLwGeomErrorMsg ()
{
/* setting the latest LWGEOM error message */
    return gaia_lwgeom_error_msg;
}

GAIAGEO_DECLARE const char *
gaiaGetLwGeomWarningMsg ()
{
/* return the latest LWGEOM error message */
    return gaia_lwgeom_warning_msg;
}

GAIAGEO_DECLARE void
gaiaSetLwGeomErrorMsg (const char *msg)
{
/* setting the latest LWGEOM error message */
    int len;
    if (gaia_lwgeom_error_msg != NULL)
	free (gaia_lwgeom_error_msg);
    gaia_lwgeom_error_msg = NULL;
    if (msg == NULL)
	return;
    len = strlen (msg);
    gaia_lwgeom_error_msg = malloc (len + 1);
    strcpy (gaia_lwgeom_error_msg, msg);
}

GAIAGEO_DECLARE void
gaiaSetLwGeomWarningMsg (const char *msg)
{
/* return the latest LWGEOM error message */
    int len;
    if (gaia_lwgeom_warning_msg != NULL)
	free (gaia_lwgeom_warning_msg);
    gaia_lwgeom_warning_msg = NULL;
    if (msg == NULL)
	return;
    len = strlen (msg);
    gaia_lwgeom_warning_msg = malloc (len + 1);
    strcpy (gaia_lwgeom_warning_msg, msg);
}

static int
check_unclosed_ring (gaiaRingPtr rng)
{
/* checks if a Ring is closed or not */
    double x0;
    double y0;
    double z0 = 0.0;
    double m0 = 0.0;
    double x1;
    double y1;
    double z1 = 0.0;
    double m1 = 0.0;
    int last = rng->Points - 1;
    if (rng->DimensionModel == GAIA_XY_Z)
      {
	  gaiaGetPointXYZ (rng->Coords, 0, &x0, &y0, &z0);
      }
    else if (rng->DimensionModel == GAIA_XY_M)
      {
	  gaiaGetPointXYM (rng->Coords, 0, &x0, &y0, &m0);
      }
    else if (rng->DimensionModel == GAIA_XY_Z_M)
      {
	  gaiaGetPointXYZM (rng->Coords, 0, &x0, &y0, &z0, &m0);
      }
    else
      {
	  gaiaGetPoint (rng->Coords, 0, &x0, &y0);
      }
    if (rng->DimensionModel == GAIA_XY_Z)
      {
	  gaiaGetPointXYZ (rng->Coords, last, &x1, &y1, &z1);
      }
    else if (rng->DimensionModel == GAIA_XY_M)
      {
	  gaiaGetPointXYM (rng->Coords, last, &x1, &y1, &m1);
      }
    else if (rng->DimensionModel == GAIA_XY_Z_M)
      {
	  gaiaGetPointXYZM (rng->Coords, last, &x1, &y1, &z1, &m1);
      }
    else
      {
	  gaiaGetPoint (rng->Coords, last, &x1, &y1);
      }
    if (x0 == x1 && y0 == y1 && z0 == z1 && m0 == m1)
	return 0;
    return 1;
}

static LWGEOM *
toLWGeom (const gaiaGeomCollPtr gaia)
{
/* converting a GAIA Geometry into a LWGEOM Geometry */
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    int has_z;
    int has_m;
    int ngeoms;
    int numg;
    int ib;
    int iv;
    int type;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double m = 0.0;
    int close_ring;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    POINTARRAY *pa;
    POINTARRAY **ppaa;
    POINT4D point;
    LWGEOM **geoms;

    if (!gaia)
	return NULL;
    pt = gaia->FirstPoint;
    while (pt)
      {
	  /* counting how many POINTs are there */
	  pts++;
	  pt = pt->Next;
      }
    ln = gaia->FirstLinestring;
    while (ln)
      {
	  /* counting how many LINESTRINGs are there */
	  lns++;
	  ln = ln->Next;
      }
    pg = gaia->FirstPolygon;
    while (pg)
      {
	  /* counting how many POLYGONs are there */
	  pgs++;
	  pg = pg->Next;
      }
    if (pts == 0 && lns == 0 && pgs == 0)
	return NULL;

    if (pts == 1 && lns == 0 && pgs == 0)
      {
	  /* single Point */
	  pt = gaia->FirstPoint;
	  has_z = 0;
	  has_m = 0;
	  if (gaia->DimensionModel == GAIA_XY_Z
	      || gaia->DimensionModel == GAIA_XY_Z_M)
	      has_z = 1;
	  if (gaia->DimensionModel == GAIA_XY_M
	      || gaia->DimensionModel == GAIA_XY_Z_M)
	      has_m = 1;
	  pa = ptarray_construct (has_z, has_m, 1);
	  point.x = pt->X;
	  point.y = pt->Y;
	  if (has_z)
	      point.z = pt->Z;
	  if (has_m)
	      point.m = pt->M;
	  ptarray_set_point4d (pa, 0, &point);
	  return (LWGEOM *) lwpoint_construct (gaia->Srid, NULL, pa);
      }
    else if (pts == 0 && lns == 1 && pgs == 0)
      {
	  /* single Linestring */
	  ln = gaia->FirstLinestring;
	  has_z = 0;
	  has_m = 0;
	  if (gaia->DimensionModel == GAIA_XY_Z
	      || gaia->DimensionModel == GAIA_XY_Z_M)
	      has_z = 1;
	  if (gaia->DimensionModel == GAIA_XY_M
	      || gaia->DimensionModel == GAIA_XY_Z_M)
	      has_m = 1;
	  pa = ptarray_construct (has_z, has_m, ln->Points);
	  for (iv = 0; iv < ln->Points; iv++)
	    {
		/* copying vertices */
		if (gaia->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ln->Coords, iv, &x, &y, &z);
		  }
		else if (gaia->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ln->Coords, iv, &x, &y, &m);
		  }
		else if (gaia->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ln->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ln->Coords, iv, &x, &y);
		  }
		point.x = x;
		point.y = y;
		if (has_z)
		    point.z = z;
		if (has_m)
		    point.m = m;
		ptarray_set_point4d (pa, iv, &point);
	    }
	  return (LWGEOM *) lwline_construct (gaia->Srid, NULL, pa);
      }
    else if (pts == 0 && lns == 0 && pgs == 1)
      {
	  /* single Polygon */
	  pg = gaia->FirstPolygon;
	  has_z = 0;
	  has_m = 0;
	  if (gaia->DimensionModel == GAIA_XY_Z
	      || gaia->DimensionModel == GAIA_XY_Z_M)
	      has_z = 1;
	  if (gaia->DimensionModel == GAIA_XY_M
	      || gaia->DimensionModel == GAIA_XY_Z_M)
	      has_m = 1;
	  ngeoms = pg->NumInteriors;
	  ppaa = lwalloc (sizeof (POINTARRAY *) * (ngeoms + 1));
	  rng = pg->Exterior;
	  close_ring = check_unclosed_ring (rng);
	  if (close_ring)
	      ppaa[0] = ptarray_construct (has_z, has_m, rng->Points + 1);
	  else
	      ppaa[0] = ptarray_construct (has_z, has_m, rng->Points);
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		/* copying vertices - Exterior Ring */
		if (gaia->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		  }
		else if (gaia->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		  }
		else if (gaia->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		  }
		point.x = x;
		point.y = y;
		if (has_z)
		    point.z = z;
		if (has_m)
		    point.m = m;
		ptarray_set_point4d (ppaa[0], iv, &point);
	    }
	  if (close_ring)
	    {
		/* making an unclosed ring to be closed */
		if (gaia->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (rng->Coords, 0, &x, &y, &z);
		  }
		else if (gaia->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (rng->Coords, 0, &x, &y, &m);
		  }
		else if (gaia->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (rng->Coords, 0, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (rng->Coords, 0, &x, &y);
		  }
		point.x = x;
		point.y = y;
		if (has_z)
		    point.z = z;
		if (has_m)
		    point.m = m;
		ptarray_set_point4d (ppaa[0], rng->Points, &point);
	    }
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		/* copying vertices - Interior Rings */
		rng = pg->Interiors + ib;
		close_ring = check_unclosed_ring (rng);
		if (close_ring)
		    ppaa[1 + ib] =
			ptarray_construct (has_z, has_m, rng->Points + 1);
		else
		    ppaa[1 + ib] =
			ptarray_construct (has_z, has_m, rng->Points);
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      if (gaia->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
			}
		      else if (gaia->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
			}
		      else if (gaia->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (rng->Coords, iv, &x, &y);
			}
		      point.x = x;
		      point.y = y;
		      if (has_z)
			  point.z = z;
		      if (has_m)
			  point.m = m;
		      ptarray_set_point4d (ppaa[1 + ib], iv, &point);
		  }
		if (close_ring)
		  {
		      /* making an unclosed ring to be closed */
		      if (gaia->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (rng->Coords, 0, &x, &y, &z);
			}
		      else if (gaia->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (rng->Coords, 0, &x, &y, &m);
			}
		      else if (gaia->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (rng->Coords, 0, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (rng->Coords, 0, &x, &y);
			}
		      point.x = x;
		      point.y = y;
		      if (has_z)
			  point.z = z;
		      if (has_m)
			  point.m = m;
		      ptarray_set_point4d (ppaa[1 + ib], rng->Points, &point);
		  }
	    }
	  return (LWGEOM *) lwpoly_construct (gaia->Srid, NULL, ngeoms + 1,
					      ppaa);
      }
    else
      {
	  /* some Collection */
	  switch (gaia->DeclaredType)
	    {
	    case GAIA_POINT:
		type = POINTTYPE;
		break;
	    case GAIA_LINESTRING:
		type = LINETYPE;
		break;
	    case GAIA_POLYGON:
		type = POLYGONTYPE;
		break;
	    case GAIA_MULTIPOINT:
		type = MULTIPOINTTYPE;
		break;
	    case GAIA_MULTILINESTRING:
		type = MULTILINETYPE;
		break;
	    case GAIA_MULTIPOLYGON:
		type = MULTIPOLYGONTYPE;
		break;
	    case GAIA_GEOMETRYCOLLECTION:
		type = COLLECTIONTYPE;
		break;
	    default:
		if (lns == 0 && pgs == 0)
		    type = MULTIPOINTTYPE;
		else if (pts == 0 && pgs == 0)
		    type = MULTILINETYPE;
		else if (pts == 0 && lns == 0)
		    type = MULTIPOLYGONTYPE;
		else
		    type = COLLECTIONTYPE;
		break;
	    };
	  numg = pts + lns + pgs;
	  geoms = lwalloc (sizeof (LWGEOM *) * numg);

	  numg = 0;
	  pt = gaia->FirstPoint;
	  while (pt)
	    {
		/* copying POINTs */
		has_z = 0;
		has_m = 0;
		if (gaia->DimensionModel == GAIA_XY_Z
		    || gaia->DimensionModel == GAIA_XY_Z_M)
		    has_z = 1;
		if (gaia->DimensionModel == GAIA_XY_M
		    || gaia->DimensionModel == GAIA_XY_Z_M)
		    has_m = 1;
		pa = ptarray_construct (has_z, has_m, 1);
		point.x = pt->X;
		point.y = pt->Y;
		if (has_z)
		    point.z = pt->Z;
		if (has_m)
		    point.m = pt->M;
		ptarray_set_point4d (pa, 0, &point);
		geoms[numg++] =
		    (LWGEOM *) lwpoint_construct (gaia->Srid, NULL, pa);
		pt = pt->Next;
	    }
	  ln = gaia->FirstLinestring;
	  while (ln)
	    {
		/* copying LINESTRINGs */
		has_z = 0;
		has_m = 0;
		if (gaia->DimensionModel == GAIA_XY_Z
		    || gaia->DimensionModel == GAIA_XY_Z_M)
		    has_z = 1;
		if (gaia->DimensionModel == GAIA_XY_M
		    || gaia->DimensionModel == GAIA_XY_Z_M)
		    has_m = 1;
		pa = ptarray_construct (has_z, has_m, ln->Points);
		for (iv = 0; iv < ln->Points; iv++)
		  {
		      /* copying vertices */
		      if (gaia->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (ln->Coords, iv, &x, &y, &z);
			}
		      else if (gaia->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ln->Coords, iv, &x, &y, &m);
			}
		      else if (gaia->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (ln->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ln->Coords, iv, &x, &y);
			}
		      point.x = x;
		      point.y = y;
		      if (has_z)
			  point.z = z;
		      if (has_m)
			  point.m = m;
		      ptarray_set_point4d (pa, iv, &point);
		  }
		geoms[numg++] =
		    (LWGEOM *) lwline_construct (gaia->Srid, NULL, pa);
		ln = ln->Next;
	    }
	  pg = gaia->FirstPolygon;
	  while (pg)
	    {
		/* copying POLYGONs */
		has_z = 0;
		has_m = 0;
		if (gaia->DimensionModel == GAIA_XY_Z
		    || gaia->DimensionModel == GAIA_XY_Z_M)
		    has_z = 1;
		if (gaia->DimensionModel == GAIA_XY_M
		    || gaia->DimensionModel == GAIA_XY_Z_M)
		    has_m = 1;
		ngeoms = pg->NumInteriors;
		ppaa = lwalloc (sizeof (POINTARRAY *) * (ngeoms + 1));
		rng = pg->Exterior;
		close_ring = check_unclosed_ring (rng);
		if (close_ring)
		    ppaa[0] = ptarray_construct (has_z, has_m, rng->Points + 1);
		else
		    ppaa[0] = ptarray_construct (has_z, has_m, rng->Points);
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      /* copying vertices - Exterior Ring */
		      if (gaia->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
			}
		      else if (gaia->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
			}
		      else if (gaia->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (rng->Coords, iv, &x, &y);
			}
		      point.x = x;
		      point.y = y;
		      if (has_z)
			  point.z = z;
		      if (has_m)
			  point.m = m;
		      ptarray_set_point4d (ppaa[0], iv, &point);
		  }
		if (close_ring)
		  {
		      /* making an unclosed ring to be closed */
		      if (gaia->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (rng->Coords, 0, &x, &y, &z);
			}
		      else if (gaia->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (rng->Coords, 0, &x, &y, &m);
			}
		      else if (gaia->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (rng->Coords, 0, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (rng->Coords, 0, &x, &y);
			}
		      point.x = x;
		      point.y = y;
		      if (has_z)
			  point.z = z;
		      if (has_m)
			  point.m = m;
		      ptarray_set_point4d (ppaa[0], rng->Points, &point);
		  }
		for (ib = 0; ib < pg->NumInteriors; ib++)
		  {
		      /* copying vertices - Interior Rings */
		      rng = pg->Interiors + ib;
		      close_ring = check_unclosed_ring (rng);
		      if (close_ring)
			  ppaa[1 + ib] =
			      ptarray_construct (has_z, has_m, rng->Points + 1);
		      else
			  ppaa[1 + ib] =
			      ptarray_construct (has_z, has_m, rng->Points);
		      for (iv = 0; iv < rng->Points; iv++)
			{
			    if (gaia->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
			      }
			    else if (gaia->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
			      }
			    else if (gaia->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (rng->Coords, iv, &x, &y,
						    &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (rng->Coords, iv, &x, &y);
			      }
			    point.x = x;
			    point.y = y;
			    if (has_z)
				point.z = z;
			    if (has_m)
				point.m = m;
			    ptarray_set_point4d (ppaa[1 + ib], iv, &point);
			}
		      if (close_ring)
			{
			    /* making an unclosed ring to be closed */
			    if (gaia->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (rng->Coords, 0, &x, &y, &z);
			      }
			    else if (gaia->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (rng->Coords, 0, &x, &y, &m);
			      }
			    else if (gaia->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (rng->Coords, 0, &x, &y, &z,
						    &m);
			      }
			    else
			      {
				  gaiaGetPoint (rng->Coords, 0, &x, &y);
			      }
			    point.x = x;
			    point.y = y;
			    if (has_z)
				point.z = z;
			    if (has_m)
				point.m = m;
			    ptarray_set_point4d (ppaa[1 + ib], rng->Points,
						 &point);
			}
		  }
		geoms[numg++] =
		    (LWGEOM *) lwpoly_construct (gaia->Srid, NULL, ngeoms + 1,
						 ppaa);
		pg = pg->Next;
	    }
	  return (LWGEOM *) lwcollection_construct (type, gaia->Srid, NULL,
						    numg, geoms);
      }
    return NULL;
}

static gaiaGeomCollPtr
fromLWGeomIncremental (gaiaGeomCollPtr gaia, const LWGEOM * lwgeom)
{
/* converting a LWGEOM Geometry into a GAIA Geometry */
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    int dimension_model = gaia->DimensionModel;
    int declared_type = gaia->DeclaredType;
    LWGEOM *lwg2 = NULL;
    LWPOINT *lwp = NULL;
    LWLINE *lwl = NULL;
    LWPOLY *lwpoly = NULL;
    LWCOLLECTION *lwc = NULL;
    POINTARRAY *pa;
    POINT4D pt4d;
    int has_z;
    int has_m;
    int iv;
    int ib;
    int ngeoms;
    int ng;
    double x;
    double y;
    double z;
    double m;

    if (lwgeom == NULL)
	return NULL;
    if (lwgeom_is_empty (lwgeom))
	return NULL;

    switch (lwgeom->type)
      {
      case POINTTYPE:
	  lwp = (LWPOINT *) lwgeom;
	  has_z = 0;
	  has_m = 0;
	  pa = lwp->point;
	  if (FLAGS_GET_Z (pa->flags))
	      has_z = 1;
	  if (FLAGS_GET_M (pa->flags))
	      has_m = 1;
	  getPoint4d_p (pa, 0, &pt4d);
	  x = pt4d.x;
	  y = pt4d.y;
	  if (has_z)
	      z = pt4d.z;
	  else
	      z = 0.0;
	  if (has_m)
	      m = pt4d.m;
	  else
	      m = 0.0;
	  if (dimension_model == GAIA_XY_Z)
	      gaiaAddPointToGeomCollXYZ (gaia, x, y, z);
	  else if (dimension_model == GAIA_XY_M)
	      gaiaAddPointToGeomCollXYM (gaia, x, y, m);
	  else if (dimension_model == GAIA_XY_Z_M)
	      gaiaAddPointToGeomCollXYZM (gaia, x, y, z, m);
	  else
	      gaiaAddPointToGeomColl (gaia, x, y);
	  if (declared_type == GAIA_MULTIPOINT)
	      gaia->DeclaredType = GAIA_MULTIPOINT;
	  else if (declared_type == GAIA_GEOMETRYCOLLECTION)
	      gaia->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  else
	      gaia->DeclaredType = GAIA_POINT;
	  break;
      case LINETYPE:
	  lwl = (LWLINE *) lwgeom;
	  has_z = 0;
	  has_m = 0;
	  pa = lwl->points;
	  if (FLAGS_GET_Z (pa->flags))
	      has_z = 1;
	  if (FLAGS_GET_M (pa->flags))
	      has_m = 1;
	  ln = gaiaAddLinestringToGeomColl (gaia, pa->npoints);
	  for (iv = 0; iv < pa->npoints; iv++)
	    {
		/* copying LINESTRING vertices */
		getPoint4d_p (pa, iv, &pt4d);
		x = pt4d.x;
		y = pt4d.y;
		if (has_z)
		    z = pt4d.z;
		else
		    z = 0.0;
		if (has_m)
		    m = pt4d.m;
		else
		    m = 0.0;
		if (dimension_model == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, iv, x, y, z);
		  }
		else if (dimension_model == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, iv, x, y, m);
		  }
		else if (dimension_model == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, iv, x, y);
		  }
	    }
	  if (declared_type == GAIA_MULTILINESTRING)
	      gaia->DeclaredType = GAIA_MULTILINESTRING;
	  else if (declared_type == GAIA_GEOMETRYCOLLECTION)
	      gaia->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  else
	      gaia->DeclaredType = GAIA_LINESTRING;
	  break;
      case POLYGONTYPE:
	  lwpoly = (LWPOLY *) lwgeom;
	  has_z = 0;
	  has_m = 0;
	  pa = lwpoly->rings[0];
	  if (FLAGS_GET_Z (pa->flags))
	      has_z = 1;
	  if (FLAGS_GET_M (pa->flags))
	      has_m = 1;
	  pg = gaiaAddPolygonToGeomColl (gaia, pa->npoints, lwpoly->nrings - 1);
	  rng = pg->Exterior;
	  for (iv = 0; iv < pa->npoints; iv++)
	    {
		/* copying Exterion Ring vertices */
		getPoint4d_p (pa, iv, &pt4d);
		x = pt4d.x;
		y = pt4d.y;
		if (has_z)
		    z = pt4d.z;
		else
		    z = 0.0;
		if (has_m)
		    m = pt4d.m;
		else
		    m = 0.0;
		if (dimension_model == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (rng->Coords, iv, x, y, z);
		  }
		else if (dimension_model == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (rng->Coords, iv, x, y, m);
		  }
		else if (dimension_model == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (rng->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (rng->Coords, iv, x, y);
		  }
	    }
	  for (ib = 1; ib < lwpoly->nrings; ib++)
	    {
		has_z = 0;
		has_m = 0;
		pa = lwpoly->rings[ib];
		if (FLAGS_GET_Z (pa->flags))
		    has_z = 1;
		if (FLAGS_GET_M (pa->flags))
		    has_m = 1;
		rng = gaiaAddInteriorRing (pg, ib - 1, pa->npoints);
		for (iv = 0; iv < pa->npoints; iv++)
		  {
		      /* copying Exterion Ring vertices */
		      getPoint4d_p (pa, iv, &pt4d);
		      x = pt4d.x;
		      y = pt4d.y;
		      if (has_z)
			  z = pt4d.z;
		      else
			  z = 0.0;
		      if (has_m)
			  m = pt4d.m;
		      else
			  m = 0.0;
		      if (dimension_model == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (rng->Coords, iv, x, y, z);
			}
		      else if (dimension_model == GAIA_XY_M)
			{
			    gaiaSetPointXYM (rng->Coords, iv, x, y, m);
			}
		      else if (dimension_model == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (rng->Coords, iv, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (rng->Coords, iv, x, y);
			}
		  }
	    }
	  if (declared_type == GAIA_MULTIPOLYGON)
	      gaia->DeclaredType = GAIA_MULTIPOLYGON;
	  else if (declared_type == GAIA_GEOMETRYCOLLECTION)
	      gaia->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  else
	      gaia->DeclaredType = GAIA_POLYGON;
	  break;
      case MULTIPOINTTYPE:
      case MULTILINETYPE:
      case MULTIPOLYGONTYPE:
      case COLLECTIONTYPE:
	  if (lwgeom->type == MULTIPOINTTYPE)
	    {
		if (declared_type == GAIA_GEOMETRYCOLLECTION)
		    gaia->DeclaredType = GAIA_GEOMETRYCOLLECTION;
		else
		    gaia->DeclaredType = GAIA_MULTIPOINT;
	    }
	  else if (lwgeom->type == MULTILINETYPE)
	    {
		if (declared_type == GAIA_GEOMETRYCOLLECTION)
		    gaia->DeclaredType = GAIA_GEOMETRYCOLLECTION;
		else
		    gaia->DeclaredType = GAIA_MULTILINESTRING;
	    }
	  else if (lwgeom->type == MULTIPOLYGONTYPE)
	    {
		if (declared_type == GAIA_GEOMETRYCOLLECTION)
		    gaia->DeclaredType = GAIA_GEOMETRYCOLLECTION;
		else
		    gaia->DeclaredType = GAIA_MULTIPOLYGON;
	    }
	  else
	      gaia->DeclaredType = GAIA_GEOMETRYCOLLECTION;

	  lwc = (LWCOLLECTION *) lwgeom;
	  ngeoms = lwc->ngeoms;
	  if (ngeoms == 0)
	    {
		gaiaFreeGeomColl (gaia);
		gaia = NULL;
		break;
	    }
	  for (ng = 0; ng < ngeoms; ++ng)
	    {
		/* looping on elementary geometries */
		lwg2 = lwc->geoms[ng];
		switch (lwg2->type)
		  {
		  case POINTTYPE:
		      lwp = (LWPOINT *) lwg2;
		      has_z = 0;
		      has_m = 0;
		      pa = lwp->point;
		      if (FLAGS_GET_Z (pa->flags))
			  has_z = 1;
		      if (FLAGS_GET_M (pa->flags))
			  has_m = 1;
		      getPoint4d_p (pa, 0, &pt4d);
		      x = pt4d.x;
		      y = pt4d.y;
		      if (has_z)
			  z = pt4d.z;
		      else
			  z = 0.0;
		      if (has_m)
			  m = pt4d.m;
		      else
			  m = 0.0;
		      if (dimension_model == GAIA_XY_Z)
			  gaiaAddPointToGeomCollXYZ (gaia, x, y, z);
		      else if (dimension_model == GAIA_XY_M)
			  gaiaAddPointToGeomCollXYM (gaia, x, y, m);
		      else if (dimension_model == GAIA_XY_Z_M)
			  gaiaAddPointToGeomCollXYZM (gaia, x, y, z, m);
		      else
			  gaiaAddPointToGeomColl (gaia, x, y);
		      break;
		  case LINETYPE:
		      lwl = (LWLINE *) lwg2;
		      has_z = 0;
		      has_m = 0;
		      pa = lwl->points;
		      if (FLAGS_GET_Z (pa->flags))
			  has_z = 1;
		      if (FLAGS_GET_M (pa->flags))
			  has_m = 1;
		      ln = gaiaAddLinestringToGeomColl (gaia, pa->npoints);
		      for (iv = 0; iv < pa->npoints; iv++)
			{
			    /* copying LINESTRING vertices */
			    getPoint4d_p (pa, iv, &pt4d);
			    x = pt4d.x;
			    y = pt4d.y;
			    if (has_z)
				z = pt4d.z;
			    else
				z = 0.0;
			    if (has_m)
				m = pt4d.m;
			    else
				m = 0.0;
			    if (dimension_model == GAIA_XY_Z)
			      {
				  gaiaSetPointXYZ (ln->Coords, iv, x, y, z);
			      }
			    else if (dimension_model == GAIA_XY_M)
			      {
				  gaiaSetPointXYM (ln->Coords, iv, x, y, m);
			      }
			    else if (dimension_model == GAIA_XY_Z_M)
			      {
				  gaiaSetPointXYZM (ln->Coords, iv, x, y, z, m);
			      }
			    else
			      {
				  gaiaSetPoint (ln->Coords, iv, x, y);
			      }
			}
		      break;
		  case POLYGONTYPE:
		      lwpoly = (LWPOLY *) lwg2;
		      has_z = 0;
		      has_m = 0;
		      pa = lwpoly->rings[0];
		      if (FLAGS_GET_Z (pa->flags))
			  has_z = 1;
		      if (FLAGS_GET_M (pa->flags))
			  has_m = 1;
		      pg = gaiaAddPolygonToGeomColl (gaia, pa->npoints,
						     lwpoly->nrings - 1);
		      rng = pg->Exterior;
		      for (iv = 0; iv < pa->npoints; iv++)
			{
			    /* copying Exterion Ring vertices */
			    getPoint4d_p (pa, iv, &pt4d);
			    x = pt4d.x;
			    y = pt4d.y;
			    if (has_z)
				z = pt4d.z;
			    else
				z = 0.0;
			    if (has_m)
				m = pt4d.m;
			    else
				m = 0.0;
			    if (dimension_model == GAIA_XY_Z)
			      {
				  gaiaSetPointXYZ (rng->Coords, iv, x, y, z);
			      }
			    else if (dimension_model == GAIA_XY_M)
			      {
				  gaiaSetPointXYM (rng->Coords, iv, x, y, m);
			      }
			    else if (dimension_model == GAIA_XY_Z_M)
			      {
				  gaiaSetPointXYZM (rng->Coords, iv, x, y, z,
						    m);
			      }
			    else
			      {
				  gaiaSetPoint (rng->Coords, iv, x, y);
			      }
			}
		      for (ib = 1; ib < lwpoly->nrings; ib++)
			{
			    has_z = 0;
			    has_m = 0;
			    pa = lwpoly->rings[ib];
			    if (FLAGS_GET_Z (pa->flags))
				has_z = 1;
			    if (FLAGS_GET_M (pa->flags))
				has_m = 1;
			    rng = gaiaAddInteriorRing (pg, ib - 1, pa->npoints);
			    for (iv = 0; iv < pa->npoints; iv++)
			      {
				  /* copying Exterion Ring vertices */
				  getPoint4d_p (pa, iv, &pt4d);
				  x = pt4d.x;
				  y = pt4d.y;
				  if (has_z)
				      z = pt4d.z;
				  else
				      z = 0.0;
				  if (has_m)
				      m = pt4d.m;
				  else
				      m = 0.0;
				  if (dimension_model == GAIA_XY_Z)
				    {
					gaiaSetPointXYZ (rng->Coords, iv, x,
							 y, z);
				    }
				  else if (dimension_model == GAIA_XY_M)
				    {
					gaiaSetPointXYM (rng->Coords, iv, x,
							 y, m);
				    }
				  else if (dimension_model == GAIA_XY_Z_M)
				    {
					gaiaSetPointXYZM (rng->Coords, iv, x,
							  y, z, m);
				    }
				  else
				    {
					gaiaSetPoint (rng->Coords, iv, x, y);
				    }
			      }
			}
		      break;
		  };
	    }
	  break;
      default:
	  gaiaFreeGeomColl (gaia);
	  gaia = NULL;
	  break;
      };

    return gaia;
}

static gaiaGeomCollPtr
fromLWGeom (const LWGEOM * lwgeom, const int dimension_model,
	    const int declared_type)
{
/* converting a LWGEOM Geometry into a GAIA Geometry */
    gaiaGeomCollPtr gaia = NULL;

    if (lwgeom == NULL)
	return NULL;
    if (lwgeom_is_empty (lwgeom))
	return NULL;

    if (dimension_model == GAIA_XY_Z)
	gaia = gaiaAllocGeomCollXYZ ();
    else if (dimension_model == GAIA_XY_M)
	gaia = gaiaAllocGeomCollXYM ();
    else if (dimension_model == GAIA_XY_Z_M)
	gaia = gaiaAllocGeomCollXYZM ();
    else
	gaia = gaiaAllocGeomColl ();
    gaia->DeclaredType = declared_type;
    fromLWGeomIncremental (gaia, lwgeom);

    return gaia;
}

static int
check_valid_type (const LWGEOM * lwgeom, int declared_type)
{
/* checking if the geometry type is a valid one */
    int ret = 0;
    switch (lwgeom->type)
      {
      case POINTTYPE:
      case MULTIPOINTTYPE:
	  if (declared_type == GAIA_POINT || declared_type == GAIA_POINTZ
	      || declared_type == GAIA_POINTM || declared_type == GAIA_POINTZM)
	      ret = 1;
	  if (declared_type == GAIA_MULTIPOINT
	      || declared_type == GAIA_MULTIPOINTZ
	      || declared_type == GAIA_MULTIPOINTM
	      || declared_type == GAIA_MULTIPOINTZM)
	      ret = 1;
	  break;
      case LINETYPE:
      case MULTILINETYPE:
	  if (declared_type == GAIA_LINESTRING
	      || declared_type == GAIA_LINESTRINGZ
	      || declared_type == GAIA_LINESTRINGM
	      || declared_type == GAIA_LINESTRINGZM)
	      ret = 1;
	  if (declared_type == GAIA_MULTILINESTRING
	      || declared_type == GAIA_MULTILINESTRINGZ
	      || declared_type == GAIA_MULTILINESTRINGM
	      || declared_type == GAIA_MULTILINESTRINGZM)
	      ret = 1;
	  break;
      case POLYGONTYPE:
      case MULTIPOLYGONTYPE:
	  if (declared_type == GAIA_POLYGON || declared_type == GAIA_POLYGONZ
	      || declared_type == GAIA_POLYGONM
	      || declared_type == GAIA_POLYGONZM)
	      ret = 1;
	  if (declared_type == GAIA_MULTIPOLYGON
	      || declared_type == GAIA_MULTIPOLYGONZ
	      || declared_type == GAIA_MULTIPOLYGONM
	      || declared_type == GAIA_MULTIPOLYGONZM)
	      ret = 1;
	  break;
      case COLLECTIONTYPE:
	  if (declared_type == GAIA_GEOMETRYCOLLECTION
	      || declared_type == GAIA_GEOMETRYCOLLECTIONZ
	      || declared_type == GAIA_GEOMETRYCOLLECTIONM
	      || declared_type == GAIA_GEOMETRYCOLLECTIONZM)
	      ret = 1;
	  break;
      };
    return ret;
}

static gaiaGeomCollPtr
fromLWGeomValidated (const LWGEOM * lwgeom, const int dimension_model,
		     const int declared_type)
{
/* 
/ converting a LWGEOM Geometry into a GAIA Geometry 
/ first collection - validated items
*/
    gaiaGeomCollPtr gaia = NULL;
    LWGEOM *lwg2 = NULL;
    LWCOLLECTION *lwc = NULL;
    int ngeoms;

    if (lwgeom == NULL)
	return NULL;
    if (lwgeom_is_empty (lwgeom))
	return NULL;

    switch (lwgeom->type)
      {
      case COLLECTIONTYPE:
	  lwc = (LWCOLLECTION *) lwgeom;
	  ngeoms = lwc->ngeoms;
	  if (ngeoms <= 2)
	    {
		lwg2 = lwc->geoms[0];
		if (check_valid_type (lwg2, declared_type))
		    gaia = fromLWGeom (lwg2, dimension_model, declared_type);
	    }
	  break;
      default:
	  if (check_valid_type (lwgeom, declared_type))
	      gaia = fromLWGeom (lwgeom, dimension_model, declared_type);
	  if (gaia == NULL)
	    {
		/* Andrea Peri: 2013-05-02 returning anyway the LWGEOM geometry,
		   / even if it has a mismatching type */
		int type = -1;
		switch (lwgeom->type)
		  {
		  case POINTTYPE:
		      type = GAIA_POINT;
		      break;
		  case LINETYPE:
		      type = GAIA_LINESTRING;
		      break;
		  case POLYGONTYPE:
		      type = GAIA_POLYGON;
		      break;
		  case MULTIPOINTTYPE:
		      type = GAIA_MULTIPOINT;
		      break;
		  case MULTILINETYPE:
		      type = GAIA_MULTILINESTRING;
		      break;
		  case MULTIPOLYGONTYPE:
		      type = GAIA_MULTIPOLYGON;
		      break;
		  };
		if (type >= 0)
		    gaia = fromLWGeom (lwgeom, dimension_model, type);
	    }
	  break;
      }
    return gaia;
}

static gaiaGeomCollPtr
fromLWGeomDiscarded (const LWGEOM * lwgeom, const int dimension_model,
		     const int declared_type)
{
/* 
/ converting a LWGEOM Geometry into a GAIA Geometry 
/ second collection - discarded items
*/
    gaiaGeomCollPtr gaia = NULL;
    LWGEOM *lwg2 = NULL;
    LWCOLLECTION *lwc = NULL;
    int ngeoms;
    int ig;

    if (lwgeom == NULL)
	return NULL;
    if (lwgeom_is_empty (lwgeom))
	return NULL;

    if (lwgeom->type == COLLECTIONTYPE)
      {
	  if (dimension_model == GAIA_XY_Z)
	      gaia = gaiaAllocGeomCollXYZ ();
	  else if (dimension_model == GAIA_XY_M)
	      gaia = gaiaAllocGeomCollXYM ();
	  else if (dimension_model == GAIA_XY_Z_M)
	      gaia = gaiaAllocGeomCollXYZM ();
	  else
	      gaia = gaiaAllocGeomColl ();
	  lwc = (LWCOLLECTION *) lwgeom;
	  ngeoms = lwc->ngeoms;
	  for (ig = 0; ig < ngeoms; ig++)
	    {
		lwg2 = lwc->geoms[ig];
		if (!check_valid_type (lwg2, declared_type))
		    fromLWGeomIncremental (gaia, lwg2);
	    }
      }
/*
Andrea Peri: 2013-05-02
when a single geometry is returned by LWGEOM it's always "valid"
and there are no discarded items at all

    else if (!check_valid_type (lwgeom, declared_type))
	gaia = fromLWGeom (lwgeom, dimension_model, declared_type);
*/
    return gaia;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaMakeValid (gaiaGeomCollPtr geom)
{
/* wrapping LWGEOM MakeValid [collecting valid items] */
    LWGEOM *g1;
    LWGEOM *g2;
    gaiaGeomCollPtr result = NULL;

    if (!geom)
	return NULL;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    g1 = toLWGeom (geom);
    g2 = lwgeom_make_valid (g1);
    if (!g2)
      {
	  lwgeom_free (g1);
	  goto done;
      }
    result = fromLWGeomValidated (g2, geom->DimensionModel, geom->DeclaredType);
    spatialite_init_geos ();
    lwgeom_free (g1);
    lwgeom_free (g2);
    if (result == NULL)
	goto done;
    result->Srid = geom->Srid;

  done:
/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaMakeValidDiscarded (gaiaGeomCollPtr geom)
{
/* wrapping LWGEOM MakeValid [collecting discarder items] */
    LWGEOM *g1;
    LWGEOM *g2;
    gaiaGeomCollPtr result = NULL;

    if (!geom)
	return NULL;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    g1 = toLWGeom (geom);
    g2 = lwgeom_make_valid (g1);
    if (!g2)
      {
	  lwgeom_free (g1);
	  goto done;
      }
    result = fromLWGeomDiscarded (g2, geom->DimensionModel, geom->DeclaredType);
    spatialite_init_geos ();
    lwgeom_free (g1);
    lwgeom_free (g2);
    if (result == NULL)
	goto done;
    result->Srid = geom->Srid;

  done:
/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaSegmentize (gaiaGeomCollPtr geom, double dist)
{
/* wrapping LWGEOM Segmentize */
    LWGEOM *g1;
    LWGEOM *g2;
    gaiaGeomCollPtr result = NULL;

    if (!geom)
	return NULL;
    if (dist <= 0.0)
	return NULL;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    g1 = toLWGeom (geom);
    g2 = lwgeom_segmentize2d (g1, dist);
    if (!g2)
      {
	  lwgeom_free (g1);
	  goto done;
      }
    result = fromLWGeom (g2, geom->DimensionModel, geom->DeclaredType);
    spatialite_init_geos ();
    lwgeom_free (g1);
    lwgeom_free (g2);
    if (result == NULL)
	goto done;
    result->Srid = geom->Srid;

  done:
/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return result;
}

static int
check_split_args (gaiaGeomCollPtr input, gaiaGeomCollPtr blade)
{
/* testing Split arguments */
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    int i_lns = 0;
    int i_pgs = 0;
    int b_pts = 0;
    int b_lns = 0;

    if (!input)
	return 0;
    if (!blade)
	return 0;

/* testing the Input type */
    if (input->FirstPoint != NULL)
      {
	  /* Point(s) on Input is forbidden !!!! */
	  return 0;
      }
    ln = input->FirstLinestring;
    while (ln)
      {
	  /* counting how many Linestrings are there */
	  i_lns++;
	  ln = ln->Next;
      }
    pg = input->FirstPolygon;
    while (pg)
      {
	  /* counting how many Polygons are there */
	  i_pgs++;
	  pg = pg->Next;
      }
    if (i_lns + i_pgs == 0)
      {
	  /* empty Input */
	  return 0;
      }

/* testing the Blade type */
    pt = blade->FirstPoint;
    while (pt)
      {
	  /* counting how many Points are there */
	  b_pts++;
	  pt = pt->Next;
      }
    if (b_pts > 1)
      {
	  /* MultiPoint on Blade is forbidden !!!! */
	  return 0;
      }
    ln = blade->FirstLinestring;
    while (ln)
      {
	  /* counting how many Linestrings are there */
	  b_lns++;
	  ln = ln->Next;
      }
    if (b_lns > 1)
      {
	  /* MultiLinestring on Blade is forbidden !!!! */
	  return 0;
      }
    if (blade->FirstPolygon != NULL)
      {
	  /* Polygon(s) on Blade is forbidden !!!! */
	  return 0;
      }
    if (b_pts + b_lns == 0)
      {
	  /* empty Blade */
	  return 0;
      }
    if (b_pts + b_lns > 1)
      {
	  /* invalid Blade [point + linestring] */
	  return 0;
      }

/* compatibility check */
    if (b_lns == 1)
      {
	  /* Linestring blade is always valid */
	  return 1;
      }
    if (i_lns >= 1 && b_pts == 1)
      {
	  /* Linestring or MultiLinestring input and Point blade is allowed */
	  return 1;
      }

    return 0;
}

static void
set_split_gtype (gaiaGeomCollPtr geom)
{
/* assignign the actual geometry type */
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    int pts = 0;
    int lns = 0;
    int pgs = 0;

    pt = geom->FirstPoint;
    while (pt)
      {
	  /* counting how many Points are there */
	  pts++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  /* counting how many Linestrings are there */
	  lns++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  /* counting how many Polygons are there */
	  pgs++;
	  pg = pg->Next;
      }

    if (pts == 1 && lns == 0 && pgs == 0)
      {
	  geom->DeclaredType = GAIA_POINT;
	  return;
      }
    if (pts > 1 && lns == 0 && pgs == 0)
      {
	  geom->DeclaredType = GAIA_MULTIPOINT;
	  return;
      }
    if (pts == 0 && lns == 1 && pgs == 0)
      {
	  geom->DeclaredType = GAIA_LINESTRING;
	  return;
      }
    if (pts == 0 && lns > 1 && pgs == 0)
      {
	  geom->DeclaredType = GAIA_MULTILINESTRING;
	  return;
      }
    if (pts == 0 && lns == 0 && pgs == 1)
      {
	  geom->DeclaredType = GAIA_POLYGON;
	  return;
      }
    if (pts == 0 && lns == 0 && pgs > 1)
      {
	  geom->DeclaredType = GAIA_MULTIPOLYGON;
	  return;
      }
    geom->DeclaredType = GAIA_GEOMETRYCOLLECTION;
}

static LWGEOM *
toLWGeomLinestring (gaiaLinestringPtr ln, int srid)
{
/* converting a GAIA Linestring into a LWGEOM Geometry */
    int iv;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double m = 0.0;
    int has_z = 0;
    int has_m = 0;
    POINTARRAY *pa;
    POINT4D point;

    if (ln->DimensionModel == GAIA_XY_Z || ln->DimensionModel == GAIA_XY_Z_M)
	has_z = 1;
    if (ln->DimensionModel == GAIA_XY_M || ln->DimensionModel == GAIA_XY_Z_M)
	has_m = 1;
    pa = ptarray_construct (has_z, has_m, ln->Points);
    for (iv = 0; iv < ln->Points; iv++)
      {
	  /* copying vertices */
	  if (ln->DimensionModel == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (ln->Coords, iv, &x, &y, &z);
	    }
	  else if (ln->DimensionModel == GAIA_XY_M)
	    {
		gaiaGetPointXYM (ln->Coords, iv, &x, &y, &m);
	    }
	  else if (ln->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (ln->Coords, iv, &x, &y, &z, &m);
	    }
	  else
	    {
		gaiaGetPoint (ln->Coords, iv, &x, &y);
	    }
	  point.x = x;
	  point.y = y;
	  if (has_z)
	      point.z = z;
	  if (has_m)
	      point.m = m;
	  ptarray_set_point4d (pa, iv, &point);
      }
    return (LWGEOM *) lwline_construct (srid, NULL, pa);
}

static LWGEOM *
toLWGeomPolygon (gaiaPolygonPtr pg, int srid)
{
/* converting a GAIA Linestring into a LWGEOM Geometry */
    int iv;
    int ib;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double m = 0.0;
    int ngeoms;
    int has_z = 0;
    int has_m = 0;
    int close_ring;
    gaiaRingPtr rng;
    POINTARRAY **ppaa;
    POINT4D point;

    if (pg->DimensionModel == GAIA_XY_Z || pg->DimensionModel == GAIA_XY_Z_M)
	has_z = 1;
    if (pg->DimensionModel == GAIA_XY_M || pg->DimensionModel == GAIA_XY_Z_M)
	has_m = 1;
    ngeoms = pg->NumInteriors;
    ppaa = lwalloc (sizeof (POINTARRAY *) * (ngeoms + 1));
    rng = pg->Exterior;
    close_ring = check_unclosed_ring (rng);
    if (close_ring)
	ppaa[0] = ptarray_construct (has_z, has_m, rng->Points + 1);
    else
	ppaa[0] = ptarray_construct (has_z, has_m, rng->Points);
    for (iv = 0; iv < rng->Points; iv++)
      {
	  /* copying vertices - Exterior Ring */
	  if (pg->DimensionModel == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
	    }
	  else if (pg->DimensionModel == GAIA_XY_M)
	    {
		gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
	    }
	  else if (pg->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
	    }
	  else
	    {
		gaiaGetPoint (rng->Coords, iv, &x, &y);
	    }
	  point.x = x;
	  point.y = y;
	  if (has_z)
	      point.z = z;
	  if (has_m)
	      point.m = m;
	  ptarray_set_point4d (ppaa[0], iv, &point);
      }
    if (close_ring)
      {
	  /* making an unclosed ring to be closed */
	  if (pg->DimensionModel == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (rng->Coords, 0, &x, &y, &z);
	    }
	  else if (pg->DimensionModel == GAIA_XY_M)
	    {
		gaiaGetPointXYM (rng->Coords, 0, &x, &y, &m);
	    }
	  else if (pg->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (rng->Coords, 0, &x, &y, &z, &m);
	    }
	  else
	    {
		gaiaGetPoint (rng->Coords, 0, &x, &y);
	    }
	  point.x = x;
	  point.y = y;
	  if (has_z)
	      point.z = z;
	  if (has_m)
	      point.m = m;
	  ptarray_set_point4d (ppaa[0], rng->Points, &point);
      }
    for (ib = 0; ib < pg->NumInteriors; ib++)
      {
	  /* copying vertices - Interior Rings */
	  rng = pg->Interiors + ib;
	  close_ring = check_unclosed_ring (rng);
	  if (close_ring)
	      ppaa[1 + ib] = ptarray_construct (has_z, has_m, rng->Points + 1);
	  else
	      ppaa[1 + ib] = ptarray_construct (has_z, has_m, rng->Points);
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		if (pg->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		  }
		else if (pg->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		  }
		else if (pg->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		  }
		point.x = x;
		point.y = y;
		if (has_z)
		    point.z = z;
		if (has_m)
		    point.m = m;
		ptarray_set_point4d (ppaa[1 + ib], iv, &point);
	    }
	  if (close_ring)
	    {
		/* making an unclosed ring to be closed */
		if (pg->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (rng->Coords, 0, &x, &y, &z);
		  }
		else if (pg->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (rng->Coords, 0, &x, &y, &m);
		  }
		else if (pg->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (rng->Coords, 0, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (rng->Coords, 0, &x, &y);
		  }
		point.x = x;
		point.y = y;
		if (has_z)
		    point.z = z;
		if (has_m)
		    point.m = m;
		ptarray_set_point4d (ppaa[0], rng->Points, &point);
	    }
      }
    return (LWGEOM *) lwpoly_construct (srid, NULL, ngeoms + 1, ppaa);
}

static gaiaGeomCollPtr
fromLWGeomLeft (gaiaGeomCollPtr gaia, const LWGEOM * lwgeom)
{
/* 
/ converting a LWGEOM Geometry into a GAIA Geometry 
/ collecting "left side" items
*/
    LWGEOM *lwg2 = NULL;
    LWCOLLECTION *lwc = NULL;
    int ngeoms;
    int ig;

    if (lwgeom == NULL)
	return NULL;
    if (lwgeom_is_empty (lwgeom))
	return NULL;

    if (lwgeom->type == COLLECTIONTYPE)
      {
	  lwc = (LWCOLLECTION *) lwgeom;
	  ngeoms = lwc->ngeoms;
	  for (ig = 0; ig < ngeoms; ig += 2)
	    {
		lwg2 = lwc->geoms[ig];
		fromLWGeomIncremental (gaia, lwg2);
	    }
      }
    else
	gaia = fromLWGeom (lwgeom, gaia->DimensionModel, gaia->DeclaredType);

    return gaia;
}

static gaiaGeomCollPtr
fromLWGeomRight (gaiaGeomCollPtr gaia, const LWGEOM * lwgeom)
{
/* 
/ converting a LWGEOM Geometry into a GAIA Geometry 
/ collecting "right side" items
*/
    LWGEOM *lwg2 = NULL;
    LWCOLLECTION *lwc = NULL;
    int ngeoms;
    int ig;

    if (lwgeom == NULL)
	return NULL;
    if (lwgeom_is_empty (lwgeom))
	return NULL;

    if (lwgeom->type == COLLECTIONTYPE)
      {
	  lwc = (LWCOLLECTION *) lwgeom;
	  ngeoms = lwc->ngeoms;
	  for (ig = 1; ig < ngeoms; ig += 2)
	    {
		lwg2 = lwc->geoms[ig];
		fromLWGeomIncremental (gaia, lwg2);
	    }
      }

    return gaia;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaSplit (gaiaGeomCollPtr input, gaiaGeomCollPtr blade)
{
/* wrapping LWGEOM Split */
    LWGEOM *g1;
    LWGEOM *g2;
    LWGEOM *g3;
    gaiaGeomCollPtr result = NULL;

    if (!check_split_args (input, blade))
	return NULL;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    g1 = toLWGeom (input);
    g2 = toLWGeom (blade);
    g3 = lwgeom_split (g1, g2);
    if (!g3)
      {
	  lwgeom_free (g1);
	  lwgeom_free (g2);
	  goto done;
      }
    result = fromLWGeom (g3, input->DimensionModel, input->DeclaredType);
    spatialite_init_geos ();
    lwgeom_free (g1);
    lwgeom_free (g2);
    lwgeom_free (g3);
    if (result == NULL)
	goto done;
    result->Srid = input->Srid;
    set_split_gtype (result);

  done:
/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaSplitLeft (gaiaGeomCollPtr input, gaiaGeomCollPtr blade)
{
/* wrapping LWGEOM Split [left half] */
    LWGEOM *g1;
    LWGEOM *g2;
    LWGEOM *g3;
    gaiaGeomCollPtr result = NULL;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;

    if (!check_split_args (input, blade))
	return NULL;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    if (input->DimensionModel == GAIA_XY_Z)
	result = gaiaAllocGeomCollXYZ ();
    else if (input->DimensionModel == GAIA_XY_M)
	result = gaiaAllocGeomCollXYM ();
    else if (input->DimensionModel == GAIA_XY_Z_M)
	result = gaiaAllocGeomCollXYZM ();
    else
	result = gaiaAllocGeomColl ();

    g2 = toLWGeom (blade);

    ln = input->FirstLinestring;
    while (ln)
      {
	  /* splitting some Linestring */
	  g1 = toLWGeomLinestring (ln, input->Srid);
	  g3 = lwgeom_split (g1, g2);
	  if (g3)
	    {
		result = fromLWGeomLeft (result, g3);
		lwgeom_free (g3);
	    }
	  spatialite_init_geos ();
	  lwgeom_free (g1);
	  ln = ln->Next;
      }
    pg = input->FirstPolygon;
    while (pg)
      {
	  /* splitting some Polygon */
	  g1 = toLWGeomPolygon (pg, input->Srid);
	  g3 = lwgeom_split (g1, g2);
	  if (g3)
	    {
		result = fromLWGeomLeft (result, g3);
		lwgeom_free (g3);
	    }
	  spatialite_init_geos ();
	  lwgeom_free (g1);
	  pg = pg->Next;
      }

    lwgeom_free (g2);
    if (result == NULL)
	goto done;
    if (result->FirstPoint == NULL && result->FirstLinestring == NULL
	&& result->FirstPolygon == NULL)
      {
	  gaiaFreeGeomColl (result);
	  result = NULL;
	  goto done;
      }
    result->Srid = input->Srid;
    set_split_gtype (result);

  done:
/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaSplitRight (gaiaGeomCollPtr input, gaiaGeomCollPtr blade)
{
/* wrapping LWGEOM Split [right half] */
    LWGEOM *g1;
    LWGEOM *g2;
    LWGEOM *g3;
    gaiaGeomCollPtr result = NULL;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;

    if (!check_split_args (input, blade))
	return NULL;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    if (input->DimensionModel == GAIA_XY_Z)
	result = gaiaAllocGeomCollXYZ ();
    else if (input->DimensionModel == GAIA_XY_M)
	result = gaiaAllocGeomCollXYM ();
    else if (input->DimensionModel == GAIA_XY_Z_M)
	result = gaiaAllocGeomCollXYZM ();
    else
	result = gaiaAllocGeomColl ();

    g2 = toLWGeom (blade);

    ln = input->FirstLinestring;
    while (ln)
      {
	  /* splitting some Linestring */
	  g1 = toLWGeomLinestring (ln, input->Srid);
	  g3 = lwgeom_split (g1, g2);
	  if (g3)
	    {
		result = fromLWGeomRight (result, g3);
		lwgeom_free (g3);
	    }
	  spatialite_init_geos ();
	  lwgeom_free (g1);
	  ln = ln->Next;
      }
    pg = input->FirstPolygon;
    while (pg)
      {
	  /* splitting some Polygon */
	  g1 = toLWGeomPolygon (pg, input->Srid);
	  g3 = lwgeom_split (g1, g2);
	  if (g3)
	    {
		result = fromLWGeomRight (result, g3);
		lwgeom_free (g3);
	    }
	  spatialite_init_geos ();
	  lwgeom_free (g1);
	  pg = pg->Next;
      }

    lwgeom_free (g2);
    if (result == NULL)
	goto done;
    if (result->FirstPoint == NULL && result->FirstLinestring == NULL
	&& result->FirstPolygon == NULL)
      {
	  gaiaFreeGeomColl (result);
	  result = NULL;
	  goto done;
      }
    result->Srid = input->Srid;
    set_split_gtype (result);

  done:
/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return result;
}

GAIAGEO_DECLARE int
gaiaAzimuth (double xa, double ya, double xb, double yb, double *azimuth)
{
/* wrapping LWGEOM Azimuth */
    POINT2D pt1;
    POINT2D pt2;
    double az;
    int ret = 1;
    pt1.x = xa;
    pt1.y = ya;
    pt2.x = xb;
    pt2.y = yb;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    if (!azimuth_pt_pt (&pt1, &pt2, &az))
	ret = 0;
    *azimuth = az;

/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return ret;
}

GAIAGEO_DECLARE int
gaiaEllipsoidAzimuth (double xa, double ya, double xb, double yb, double a,
		      double b, double *azimuth)
{
/* wrapping LWGEOM AzimuthSpheroid */
    LWPOINT *pt1 = lwpoint_make2d (0, xa, ya);
    LWPOINT *pt2 = lwpoint_make2d (0, xb, yb);
    SPHEROID ellips;
    int ret = 1;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    spheroid_init (&ellips, a, b);
    *azimuth = lwgeom_azumith_spheroid (pt1, pt2, &ellips);
    lwpoint_free (pt1);
    lwpoint_free (pt2);

/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return ret;
}

GAIAGEO_DECLARE int
gaiaProjectedPoint (double x1, double y1, double a, double b, double distance,
		    double azimuth, double *x2, double *y2)
{
/* wrapping LWGEOM Project */
    LWPOINT *pt1 = lwpoint_make2d (0, x1, y1);
    LWPOINT *pt2;
    SPHEROID ellips;
    int ret = 0;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    spheroid_init (&ellips, a, b);
    pt2 = lwgeom_project_spheroid (pt1, &ellips, distance, azimuth);
    lwpoint_free (pt1);
    if (pt2 != NULL)
      {
	  *x2 = lwpoint_get_x (pt2);
	  *y2 = lwpoint_get_y (pt2);
	  lwpoint_free (pt2);
	  ret = 1;
      }

/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeodesicArea (gaiaGeomCollPtr geom, double a, double b, int use_ellipsoid,
		  double *area)
{
/* wrapping LWGEOM AreaSphere and AreaSpheroid */
    LWGEOM *g = toLWGeom (geom);
    SPHEROID ellips;
    GBOX gbox;
    double tolerance = 1e-12;
    int ret = 1;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    spheroid_init (&ellips, a, b);
    if (g == NULL)
      {
	  ret = 0;
	  goto done;
      }
    lwgeom_calculate_gbox_geodetic (g, &gbox);
    if (use_ellipsoid)
      {
	  /* testing for "forbidden" calculations on the ellipsoid */
	  if ((gbox.zmax + tolerance) >= 1.0 || (gbox.zmin - tolerance) <= -1.0)
	      use_ellipsoid = 0;	/* can't circle the poles */
	  if (gbox.zmax > 0.0 && gbox.zmin < 0.0)
	      use_ellipsoid = 0;	/* can't cross the equator */
      }
    if (use_ellipsoid)
	*area = lwgeom_area_spheroid (g, &ellips);
    else
	*area = lwgeom_area_sphere (g, &ellips);
    lwgeom_free (g);

  done:
/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return ret;
}

GAIAGEO_DECLARE char *
gaiaGeoHash (gaiaGeomCollPtr geom, int precision)
{
/* wrapping LWGEOM GeoHash */
    LWGEOM *g;
    char *result;
    char *geo_hash = NULL;
    int len;

    if (!geom)
	return NULL;
    gaiaMbrGeometry (geom);
    if (geom->MinX < -180.0 || geom->MaxX > 180.0 || geom->MinY < -90.0
	|| geom->MaxY > 90.0)
	return NULL;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    g = toLWGeom (geom);
    result = lwgeom_geohash (g, precision);
    lwgeom_free (g);
    if (result == NULL)
	goto done;
    len = strlen (result);
    if (len == 0)
      {
	  lwfree (result);
	  goto done;
      }
    geo_hash = malloc (len + 1);
    strcpy (geo_hash, result);
    lwfree (result);

  done:
/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return geo_hash;
}

GAIAGEO_DECLARE char *
gaiaAsX3D (gaiaGeomCollPtr geom, const char *srs, int precision, int options,
	   const char *defid)
{
/* wrapping LWGEOM AsX3D */
    LWGEOM *g;
    char *result;
    char *x3d = NULL;
    int len;

    if (!geom)
	return NULL;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    gaiaMbrGeometry (geom);
    g = toLWGeom (geom);
    result = lwgeom_to_x3d3 (g, (char *) srs, precision, options, defid);
    lwgeom_free (g);
    if (result == NULL)
	goto done;
    len = strlen (result);
    if (len == 0)
      {
	  lwfree (result);
	  goto done;
      }
    x3d = malloc (len + 1);
    strcpy (x3d, result);
    lwfree (result);

  done:
/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return x3d;
}

GAIAGEO_DECLARE int
gaia3DDistance (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2, double *dist)
{
/* wrapping LWGEOM mindistance3d */
    LWGEOM *g1;
    LWGEOM *g2;
    double d;
    int ret = 1;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    g1 = toLWGeom (geom1);
    g2 = toLWGeom (geom2);

    d = lwgeom_mindistance3d (g1, g2);
    lwgeom_free (g1);
    lwgeom_free (g2);
    *dist = d;

/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return ret;
}

GAIAGEO_DECLARE int
gaiaMaxDistance (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2, double *dist)
{
/* wrapping LWGEOM maxdistance2d */
    LWGEOM *g1;
    LWGEOM *g2;
    double d;
    int ret = 1;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    g1 = toLWGeom (geom1);
    g2 = toLWGeom (geom2);

    d = lwgeom_maxdistance2d (g1, g2);
    lwgeom_free (g1);
    lwgeom_free (g2);
    *dist = d;

/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return ret;
}

GAIAGEO_DECLARE int
gaia3DMaxDistance (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2, double *dist)
{
/* wrapping LWGEOM maxdistance2d */
    LWGEOM *g1;
    LWGEOM *g2;
    double d;
    int ret = 1;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    g1 = toLWGeom (geom1);
    g2 = toLWGeom (geom2);

    d = lwgeom_maxdistance3d (g1, g2);
    lwgeom_free (g1);
    lwgeom_free (g2);
    *dist = d;

/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return ret;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaNodeLines (gaiaGeomCollPtr geom)
{
/* wrapping LWGEOM lwgeom_node */
    LWGEOM *g1;
    LWGEOM *g2;
    gaiaGeomCollPtr result = NULL;

    if (!geom)
	return NULL;

/* locking the semaphore */
    splite_lwgeom_semaphore_lock ();

    g1 = toLWGeom (geom);
    g2 = lwgeom_node (g1);
    if (!g2)
      {
	  lwgeom_free (g1);
	  goto done;
      }
    result = fromLWGeom (g2, geom->DimensionModel, geom->DeclaredType);
    spatialite_init_geos ();
    lwgeom_free (g1);
    lwgeom_free (g2);
    if (result == NULL)
	goto done;
    result->Srid = geom->Srid;

  done:
/* unlocking the semaphore */
    splite_lwgeom_semaphore_unlock ();
    return result;
}

#endif /* end enabling LWGEOM support */
