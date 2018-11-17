/*

 gg_extras.c -- Gaia extra functions support
    
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

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>
#include <spatialite/gaiageo.h>

#if defined(_WIN32) && !defined(__MINGW32__)
static double
rint (double x)
{
    return floor (x + 0.5);
}
#endif

static void
auxGridSnapPoint (int dimension_model, gaiaPointPtr pt, gaiaGeomCollPtr result,
		  double origin_x, double origin_y, double origin_z,
		  double origin_m, double size_x, double size_y, double size_z,
		  double size_m)
{
/* snapping a Point to a regular Grid */
    double x = pt->X;
    double y = pt->Y;
    double z = 0.0;
    double m = 0.0;
    int has_z = 0;
    int has_m = 0;
    gaiaPointPtr ptx;

    if (pt == NULL || result == NULL)
	return;
    if (dimension_model == GAIA_XY_Z || dimension_model == GAIA_XY_Z_M)
	has_z = 1;
    if (dimension_model == GAIA_XY_M || dimension_model == GAIA_XY_Z_M)
	has_m = 1;
    if (has_z)
	z = pt->Z;
    if (has_m)
	m = pt->M;

/* snapping coords to the given grid */
    if (size_x > 0.0)
	x = rint ((x - origin_x) / size_x) * size_x + origin_x;
    if (size_y > 0.0)
	y = rint ((y - origin_y) / size_y) * size_y + origin_y;
    if (has_z && size_z > 0.0)
	z = rint ((z - origin_z) / size_z) * size_z + origin_z;
    if (has_m && size_m > 0.0)
	m = rint ((m - origin_m) / size_m) * size_m + origin_m;

    ptx = result->FirstPoint;
    while (ptx)
      {
	  /* checking if already defined */
	  if (has_z && has_m)
	    {
		if (ptx->X == x && ptx->Y == y && ptx->Z == z && ptx->M == m)
		    return;
	    }
	  else if (has_z)
	    {
		if (ptx->X == x && ptx->Y == y && ptx->Z == z)
		    return;
	    }
	  else if (has_m)
	    {
		if (ptx->X == x && ptx->Y == y && ptx->M == m)
		    return;
	    }
	  else
	    {
		if (ptx->X == x && ptx->Y == y)
		    return;
	    }
	  ptx = ptx->Next;
      }

/* inserting the snapped Point into the result Geometry */
    if (has_z && has_m)
	gaiaAddPointToGeomCollXYZM (result, x, y, z, m);
    else if (has_z)
	gaiaAddPointToGeomCollXYZ (result, x, y, z);
    else if (has_m)
	gaiaAddPointToGeomCollXYM (result, x, y, m);
    else
	gaiaAddPointToGeomColl (result, x, y);
}

static void
auxGridSnapLinestring (gaiaLinestringPtr ln, gaiaGeomCollPtr result,
		       double origin_x, double origin_y, double origin_z,
		       double origin_m, double size_x, double size_y,
		       double size_z, double size_m)
{
/* snapping a Linestring to a regular Grid */
    double x;
    double y;
    double z;
    double m;
    int has_z = 0;
    int has_m = 0;
    int iv;
    gaiaDynamicLinePtr dyn;
    gaiaPointPtr pt;
    gaiaLinestringPtr lnx;
    int count = 0;

    if (ln == NULL || result == NULL)
	return;
    if (ln->DimensionModel == GAIA_XY_Z || ln->DimensionModel == GAIA_XY_Z_M)
	has_z = 1;
    if (ln->DimensionModel == GAIA_XY_M || ln->DimensionModel == GAIA_XY_Z_M)
	has_m = 1;
    dyn = gaiaAllocDynamicLine ();

    for (iv = 0; iv < ln->Points; iv++)
      {
	  /* snapping each Vertex to the given grid */
	  int to_be_inserted = 0;
	  z = 0.0;
	  m = 0.0;
	  if (has_z && has_m)
	    {
		gaiaGetPointXYZM (ln->Coords, iv, &x, &y, &z, &m);
	    }
	  else if (has_z)
	    {
		gaiaGetPointXYZ (ln->Coords, iv, &x, &y, &z);
	    }
	  else if (has_m)
	    {
		gaiaGetPointXYM (ln->Coords, iv, &x, &y, &m);
	    }
	  else
	    {
		gaiaGetPoint (ln->Coords, iv, &x, &y);
	    }
	  /* snapping coords to the given grid */
	  if (size_x > 0.0)
	      x = rint ((x - origin_x) / size_x) * size_x + origin_x;
	  if (size_y > 0.0)
	      y = rint ((y - origin_y) / size_y) * size_y + origin_y;
	  if (has_z && size_z > 0.0)
	      z = rint ((z - origin_z) / size_z) * size_z + origin_z;
	  if (has_m && size_m > 0.0)
	      m = rint ((m - origin_m) / size_m) * size_m + origin_m;

	  if (dyn->Last == NULL)
	      to_be_inserted = 1;
	  else
	    {
		/* skipping repeated points */
		pt = dyn->Last;
		if (has_z && has_m)
		  {
		      if (pt->X == x && pt->Y == y && pt->Z == z && pt->M == m)
			  ;
		      else
			  to_be_inserted = 1;
		  }
		else if (has_z)
		  {
		      if (pt->X == x && pt->Y == y && pt->Z == z)
			  ;
		      else
			  to_be_inserted = 1;
		  }
		else if (has_m)
		  {
		      if (pt->X == x && pt->Y == y && pt->M == m)
			  ;
		      else
			  to_be_inserted = 1;
		  }
		else
		  {
		      if (pt->X == x && pt->Y == y)
			  ;
		      else
			  to_be_inserted = 1;
		  }
	    }
	  if (to_be_inserted)
	    {
		if (has_z && has_m)
		    gaiaAppendPointZMToDynamicLine (dyn, x, y, z, m);
		else if (has_z)
		    gaiaAppendPointZToDynamicLine (dyn, x, y, z);
		else if (has_m)
		    gaiaAppendPointMToDynamicLine (dyn, x, y, m);
		else
		    gaiaAppendPointToDynamicLine (dyn, x, y);
	    }
      }

/* checking for validity */
    pt = dyn->First;
    while (pt)
      {
	  /* counting how many points are there */
	  count++;
	  pt = pt->Next;
      }
    if (count < 2)
      {
	  /* skipping any collapsed line */
	  gaiaFreeDynamicLine (dyn);
	  return;
      }

/* inserting into the result Geometry */
    lnx = gaiaAddLinestringToGeomColl (result, count);
    iv = 0;
    pt = dyn->First;
    while (pt)
      {
	  /* copying points */
	  if (lnx->DimensionModel == GAIA_XY_Z)
	    {
		gaiaSetPointXYZ (lnx->Coords, iv, pt->X, pt->Y, pt->Z);
	    }
	  else if (lnx->DimensionModel == GAIA_XY_M)
	    {
		gaiaSetPointXYM (lnx->Coords, iv, pt->X, pt->Y, pt->M);
	    }
	  else if (lnx->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaSetPointXYZM (lnx->Coords, iv, pt->X, pt->Y, pt->Z, pt->M);
	    }
	  else
	    {
		gaiaSetPoint (lnx->Coords, iv, pt->X, pt->Y);
	    }
	  iv++;
	  pt = pt->Next;
      }
    gaiaFreeDynamicLine (dyn);
}

static gaiaDynamicLinePtr
auxGridSnapRing (gaiaRingPtr rng, double origin_x, double origin_y,
		 double origin_z, double origin_m, double size_x, double size_y,
		 double size_z, double size_m)
{
/* snapping a Ring to a regular Grid */
    double x;
    double y;
    double z;
    double m;
    int has_z = 0;
    int has_m = 0;
    int iv;
    gaiaDynamicLinePtr dyn;
    gaiaPointPtr pt0;
    gaiaPointPtr pt;
    int count = 0;

    if (rng == NULL)
	return NULL;
    if (rng->DimensionModel == GAIA_XY_Z || rng->DimensionModel == GAIA_XY_Z_M)
	has_z = 1;
    if (rng->DimensionModel == GAIA_XY_M || rng->DimensionModel == GAIA_XY_Z_M)
	has_m = 1;
    dyn = gaiaAllocDynamicLine ();

    for (iv = 0; iv < rng->Points; iv++)
      {
	  /* snapping each Vertex to the given grid */
	  int to_be_inserted = 0;
	  z = 0.0;
	  m = 0.0;
	  if (has_z && has_m)
	    {
		gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
	    }
	  else if (has_z)
	    {
		gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
	    }
	  else if (has_m)
	    {
		gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
	    }
	  else
	    {
		gaiaGetPoint (rng->Coords, iv, &x, &y);
	    }
	  /* snapping coords to the given grid */
	  if (size_x > 0.0)
	      x = rint ((x - origin_x) / size_x) * size_x + origin_x;
	  if (size_y > 0.0)
	      y = rint ((y - origin_y) / size_y) * size_y + origin_y;
	  if (has_z && size_z > 0.0)
	      z = rint ((z - origin_z) / size_z) * size_z + origin_z;
	  if (has_m && size_m > 0.0)
	      m = rint ((m - origin_m) / size_m) * size_m + origin_m;

	  if (dyn->Last == NULL)
	      to_be_inserted = 1;
	  else
	    {
		/* skipping repeated points */
		pt = dyn->Last;
		if (has_z && has_m)
		  {
		      if (pt->X == x && pt->Y == y && pt->Z == z && pt->M == m)
			  ;
		      else
			  to_be_inserted = 1;
		  }
		else if (has_z)
		  {
		      if (pt->X == x && pt->Y == y && pt->Z == z)
			  ;
		      else
			  to_be_inserted = 1;
		  }
		else if (has_m)
		  {
		      if (pt->X == x && pt->Y == y && pt->M == m)
			  ;
		      else
			  to_be_inserted = 1;
		  }
		else
		  {
		      if (pt->X == x && pt->Y == y)
			  ;
		      else
			  to_be_inserted = 1;
		  }
	    }
	  if (to_be_inserted)
	    {
		if (has_z && has_m)
		    gaiaAppendPointZMToDynamicLine (dyn, x, y, z, m);
		else if (has_z)
		    gaiaAppendPointZToDynamicLine (dyn, x, y, z);
		else if (has_m)
		    gaiaAppendPointMToDynamicLine (dyn, x, y, m);
		else
		    gaiaAppendPointToDynamicLine (dyn, x, y);
	    }
      }
/* ensuring for Ring closure */
    pt0 = dyn->First;
    pt = dyn->Last;
    if (has_z && has_m)
      {
	  if (pt0->X == pt->X && pt0->Y == pt->Y && pt0->Z == pt->Z
	      && pt0->M == pt->M)
	      ;
	  else
	      gaiaAppendPointZMToDynamicLine (dyn, pt->X, pt->Y, pt->Z, pt->M);
      }
    else if (has_z)
      {
	  if (pt0->X == pt->X && pt0->Y == pt->Y && pt0->Z == pt->Z)
	      ;
	  else
	      gaiaAppendPointZToDynamicLine (dyn, pt->X, pt->Y, pt->Z);
      }
    else if (has_m)
      {
	  if (pt0->X == pt->X && pt0->Y == pt->Y && pt0->M == pt->M)
	      ;
	  else
	      gaiaAppendPointMToDynamicLine (dyn, pt->X, pt->Y, pt->M);
      }
    else
      {
	  if (pt0->X == pt->X && pt0->Y == pt->Y)
	      ;
	  else
	      gaiaAppendPointToDynamicLine (dyn, pt->X, pt->Y);
      }

/* checking for validity */
    pt = dyn->First;
    while (pt)
      {
	  /* counting how many points are there */
	  count++;
	  pt = pt->Next;
      }
    if (count < 4)
      {
	  /* skipping any collapsed ring */
	  gaiaFreeDynamicLine (dyn);
	  return NULL;
      }
    return dyn;
}

static void
auxGridSnapPolygon (gaiaPolygonPtr pg, gaiaGeomCollPtr result, double origin_x,
		    double origin_y, double origin_z, double origin_m,
		    double size_x, double size_y, double size_z, double size_m)
{
/* snapping a Polygon to a regular Grid */
    int ib;
    int holes = 0;
    int count;
    int next_hole = 0;
    int iv;
    gaiaRingPtr rng;
    gaiaPolygonPtr pgx;
    gaiaPointPtr pt;
    gaiaDynamicLinePtr rng_ext;
    gaiaDynamicLinePtr dyn;
    gaiaDynamicLinePtr *rng_ints = NULL;

    if (pg == NULL || result == NULL)
	return;
/* snapping the Exterior Ring */
    rng = pg->Exterior;
    rng_ext =
	auxGridSnapRing (rng, origin_x, origin_y, origin_z, origin_m, size_x,
			 size_y, size_z, size_m);
    if (rng_ext == NULL)	/* skipping any collaped Polygon */
	return;

    if (pg->NumInteriors)
      {
	  /* snapping any Interior Ring */
	  rng_ints = malloc (sizeof (gaiaRingPtr *) * pg->NumInteriors);
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		rng = pg->Interiors + ib;
		*(rng_ints + ib) =
		    auxGridSnapRing (rng, origin_x, origin_y, origin_z,
				     origin_m, size_x, size_y, size_z, size_m);
		if (*(rng_ints + ib) != NULL)
		    holes++;
	    }
      }

/* inserting into the result Geometry */
    pt = rng_ext->First;
    count = 0;
    while (pt)
      {
	  /* counting how many points are in the Exterior Ring */
	  count++;
	  pt = pt->Next;
      }
    pgx = gaiaAddPolygonToGeomColl (result, count, holes);
    rng = pgx->Exterior;
    iv = 0;
    pt = rng_ext->First;
    while (pt)
      {
	  /* copying Exterior Ring points */
	  if (rng->DimensionModel == GAIA_XY_Z)
	    {
		gaiaSetPointXYZ (rng->Coords, iv, pt->X, pt->Y, pt->Z);
	    }
	  else if (rng->DimensionModel == GAIA_XY_M)
	    {
		gaiaSetPointXYM (rng->Coords, iv, pt->X, pt->Y, pt->M);
	    }
	  else if (rng->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaSetPointXYZM (rng->Coords, iv, pt->X, pt->Y, pt->Z, pt->M);
	    }
	  else
	    {
		gaiaSetPoint (rng->Coords, iv, pt->X, pt->Y);
	    }
	  iv++;
	  pt = pt->Next;
      }
    if (holes > 0)
      {
	  /* setting up any not-collapsed Hole */
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		if (*(rng_ints + ib) == NULL)
		    continue;
		dyn = *(rng_ints + ib);
		pt = dyn->First;
		count = 0;
		while (pt)
		  {
		      /* counting how many points are in the Exterior Ring */
		      count++;
		      pt = pt->Next;
		  }
		rng = gaiaAddInteriorRing (pgx, next_hole++, count);
		iv = 0;
		pt = dyn->First;
		while (pt)
		  {
		      /* copying Interior Ring points */
		      if (rng->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (rng->Coords, iv, pt->X, pt->Y,
					     pt->Z);
			}
		      else if (rng->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (rng->Coords, iv, pt->X, pt->Y,
					     pt->M);
			}
		      else if (rng->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (rng->Coords, iv, pt->X, pt->Y,
					      pt->Z, pt->M);
			}
		      else
			{
			    gaiaSetPoint (rng->Coords, iv, pt->X, pt->Y);
			}
		      iv++;
		      pt = pt->Next;
		  }
	    }
      }

/* memory clean-up */
    gaiaFreeDynamicLine (rng_ext);
    if (rng_ints)
      {
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		dyn = *(rng_ints + ib);
		if (dyn)
		    gaiaFreeDynamicLine (dyn);
	    }
	  free (rng_ints);
      }
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaSnapToGrid (gaiaGeomCollPtr geom, double origin_x, double origin_y,
		double origin_z, double origin_m, double size_x, double size_y,
		double size_z, double size_m)
{
/* creating a Geometry snapped to a regular Grid */
    gaiaGeomCollPtr result;
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;

    if (!geom)
	return NULL;

/* creating the output Geometry */
    if (geom->DimensionModel == GAIA_XY_Z)
	result = gaiaAllocGeomCollXYZ ();
    else if (geom->DimensionModel == GAIA_XY_M)
	result = gaiaAllocGeomCollXYM ();
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	result = gaiaAllocGeomCollXYZM ();
    else
	result = gaiaAllocGeomColl ();

/* snapping elementary Geometries to the given Grid */
    pt = geom->FirstPoint;
    while (pt)
      {
	  /* snapping POINTs */
	  auxGridSnapPoint (geom->DimensionModel, pt, result, origin_x,
			    origin_y, origin_z, origin_m, size_x, size_y,
			    size_z, size_m);
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  /* snapping LINESTRINGs */
	  auxGridSnapLinestring (ln, result, origin_x, origin_y, origin_z,
				 origin_m, size_x, size_y, size_z, size_m);
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  /* snapping POLYGONs */
	  auxGridSnapPolygon (pg, result, origin_x, origin_y, origin_z,
			      origin_m, size_x, size_y, size_z, size_m);
	  pg = pg->Next;
      }

/* validating the output Geometry */
    pt = result->FirstPoint;
    while (pt)
      {
	  /* counting how many POINTs are there */
	  pts++;
	  pt = pt->Next;
      }
    ln = result->FirstLinestring;
    while (ln)
      {
	  /* counting how many LINESTRINGs are there */
	  lns++;
	  ln = ln->Next;
      }
    pg = result->FirstPolygon;
    while (pg)
      {
	  /* counting how many POLYGONs are there */
	  pgs++;
	  pg = pg->Next;
      }
    if (pts == 0 && lns == 0 && pgs == 0)
      {
	  /* empty result */
	  gaiaFreeGeomColl (result);
	  return NULL;
      }

/* final adjustment */
    result->Srid = geom->Srid;
    if (pts == 1 && lns == 0 && pgs == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      result->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  else if (geom->DeclaredType == GAIA_MULTIPOINT)
	      result->DeclaredType = GAIA_MULTIPOINT;
	  else
	      result->DeclaredType = GAIA_POINT;
      }
    else if (pts == 0 && lns == 1 && pgs == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      result->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  else if (geom->DeclaredType == GAIA_MULTILINESTRING)
	      result->DeclaredType = GAIA_MULTILINESTRING;
	  else
	      result->DeclaredType = GAIA_LINESTRING;
      }
    else if (pts == 0 && lns == 0 && pgs == 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      result->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  else if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	      result->DeclaredType = GAIA_MULTIPOLYGON;
	  else
	      result->DeclaredType = GAIA_POLYGON;
      }
    else if (pts > 1 && lns == 0 && pgs == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      result->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  else
	      result->DeclaredType = GAIA_MULTIPOINT;
      }
    else if (pts == 0 && lns > 1 && pgs == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      result->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  else
	      result->DeclaredType = GAIA_MULTILINESTRING;
      }
    else if (pts == 0 && lns == 0 && pgs > 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      result->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  else
	      result->DeclaredType = GAIA_MULTIPOLYGON;
      }
    else
	result->DeclaredType = GAIA_GEOMETRYCOLLECTION;
    return result;
}

#ifndef OMIT_GEOS		/* only if GEOS is enabled */

static void
get_grid_bbox (gaiaGeomCollPtr geom, double *min_x, double *min_y,
	       double *max_x, double *max_y)
{
/* extracting the BBOX representing the input geometry */
    gaiaMbrGeometry (geom);
    *min_x = geom->MinX;
    *min_y = geom->MinY;
    *max_x = geom->MaxX;
    *max_y = geom->MaxY;
}

static void
get_grid_base (double min_x, double min_y, double origin_x, double origin_y,
	       double size, double *base_x, double *base_y)
{
/* determining the grid base-point [MinX/MinY] */
    double x = rint ((min_x - origin_x) / size) * size + origin_x;
    double y = rint ((min_y - origin_y) / size) * size + origin_y;
    if (x > min_x)
	*base_x = x - size;
    else
	*base_x = x;
    if (y > min_y)
	*base_y = y - size;
    else
	*base_y = y;
}

static gaiaGeomCollPtr
gaiaSquareGridCommon (const void *p_cache, gaiaGeomCollPtr geom,
		      double origin_x, double origin_y, double size,
		      int edges_only)
{
/* creating a regular grid [Square cells] */
    double min_x;
    double min_y;
    double max_x;
    double max_y;
    double base_x;
    double base_y;
    double x1;
    double y1;
    double x2;
    double y2;
    double x3;
    double y3;
    double x4;
    double y4;
    int count = 0;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaLinestringPtr ln;
    gaiaGeomCollPtr result = NULL;
    gaiaGeomCollPtr item = NULL;
    int ret;

    if (size <= 0.0)
	return NULL;

    result = gaiaAllocGeomColl ();
    result->Srid = geom->Srid;
    get_grid_bbox (geom, &min_x, &min_y, &max_x, &max_y);
    get_grid_base (min_x, min_y, origin_x, origin_y, size, &base_x, &base_y);
    while (base_y < max_y)
      {
	  /* looping on grid rows */
	  x1 = base_x;
	  y1 = base_y;
	  x2 = x1 + size;
	  y2 = y1;
	  x3 = x2;
	  y3 = y1 + size;
	  x4 = x1;
	  y4 = y3;
	  while (x1 < max_x)
	    {
		/* looping on grid columns */
		item = gaiaAllocGeomColl ();

		pg = gaiaAddPolygonToGeomColl (item, 5, 0);
		rng = pg->Exterior;
		gaiaSetPoint (rng->Coords, 0, x1, y1);
		gaiaSetPoint (rng->Coords, 1, x2, y2);
		gaiaSetPoint (rng->Coords, 2, x3, y3);
		gaiaSetPoint (rng->Coords, 3, x4, y4);
		gaiaSetPoint (rng->Coords, 4, x1, y1);

		gaiaMbrGeometry (item);
		if (p_cache != NULL)
		    ret = gaiaGeomCollIntersects_r (p_cache, geom, item);
		else
		    ret = gaiaGeomCollIntersects (geom, item);
		if (ret == 1)
		  {
		      /* ok, inserting a valid cell */
		      count++;
		      if (edges_only)
			{
			    /* multilinestring */
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x1, y1);
			    gaiaSetPoint (ln->Coords, 1, x2, y2);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x2, y2);
			    gaiaSetPoint (ln->Coords, 1, x3, y3);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x3, y3);
			    gaiaSetPoint (ln->Coords, 1, x4, y4);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x4, y4);
			    gaiaSetPoint (ln->Coords, 1, x1, y1);
			}
		      else
			{
			    /* polygon */
			    pg = gaiaAddPolygonToGeomColl (result, 5, 0);
			    rng = pg->Exterior;
			    gaiaSetPoint (rng->Coords, 0, x1, y1);
			    gaiaSetPoint (rng->Coords, 1, x2, y2);
			    gaiaSetPoint (rng->Coords, 2, x3, y3);
			    gaiaSetPoint (rng->Coords, 3, x4, y4);
			    gaiaSetPoint (rng->Coords, 4, x1, y1);
			}
		  }
		gaiaFreeGeomColl (item);
		x1 += size;
		x2 += size;
		x3 += size;
		x4 += size;
	    }
	  base_y += size;
      }

/* final check */
    if (!count)
      {
	  /* empty grid */
	  gaiaFreeGeomColl (result);
	  return NULL;
      }
    if (!edges_only)
      {
	  result->DeclaredType = GAIA_MULTIPOLYGON;
	  return result;
      }

    item = result;
    if (p_cache != NULL)
	result = gaiaUnaryUnion_r (p_cache, item);
    else
	result = gaiaUnaryUnion (item);
    gaiaFreeGeomColl (item);
    result->Srid = geom->Srid;
    result->DeclaredType = GAIA_LINESTRING;
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaSquareGrid (gaiaGeomCollPtr geom, double origin_x, double origin_y,
		double size, int edges_only)
{
    return gaiaSquareGridCommon (NULL, geom, origin_x, origin_y, size,
				 edges_only);
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaSquareGrid_r (const void *p_cache, gaiaGeomCollPtr geom, double origin_x,
		  double origin_y, double size, int edges_only)
{
    return gaiaSquareGridCommon (p_cache, geom, origin_x, origin_y, size,
				 edges_only);
}

static void
get_trigrid_base (double min_x, double min_y, double origin_x, double origin_y,
		  double shift_h_odd, double shift_h_even, double shift_v,
		  int *odd_even, double *base_x, double *base_y)
{
/* determining the grid base-point [MinX/MinY] for a Triangular Grid */
    double bx = origin_x;
    double by = origin_y;
    while (1)
      {
	  /* looping on grid rows */
	  if (min_y < origin_y)
	    {
		/* going southward */
		if (by > min_y)
		    goto next_scanline;
		if (*odd_even)
		    bx = origin_x - shift_h_odd;
		else
		    bx = origin_x;
	    }
	  else
	    {
		/* going northward */
		if (by < min_y)
		    goto next_scanline;
		if (*odd_even)
		    bx = origin_x - shift_h_odd;
		else
		    bx = origin_x;
	    }
	  while (1)
	    {
		/* looping on grid columns */
		if (min_x < origin_x)
		  {
		      /* going eastward */
		      if (bx - shift_h_even - shift_h_odd < min_x)
			{
			    *base_x = bx;
			    *base_y = by;
			    return;
			}
		      bx -= shift_h_even;
		  }
		else
		  {
		      /* going westward */
		      if (bx + shift_h_even > min_x
			  || bx + shift_h_even + shift_h_odd > min_x)
			{
			    *base_x = bx;
			    *base_y = by;
			    return;
			}
		      bx += shift_h_even;
		  }
	    }
	next_scanline:
	  if (min_y < origin_y)
	      by -= shift_v;
	  else
	      by += shift_v;
	  if (*odd_even)
	      *odd_even = 0;
	  else
	      *odd_even = 1;
      }
}

static gaiaGeomCollPtr
gaiaTriangularGridCommon (const void *p_cache, gaiaGeomCollPtr geom,
			  double origin_x, double origin_y, double size,
			  int edges_only)
{
/* creating a regular grid [Triangular cells] */
    double min_x;
    double min_y;
    double max_x;
    double max_y;
    double base_x;
    double base_y;
    double x1;
    double y1;
    double x2;
    double y2;
    double x3;
    double y3;
    double x4;
    double y4;
    double shift_h_odd;
    double shift_h_even;
    double shift_v;
    int count = 0;
    int odd_even = 0;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaLinestringPtr ln;
    gaiaGeomCollPtr result = NULL;
    gaiaGeomCollPtr item = NULL;
    int ret;

    if (size <= 0.0)
	return NULL;
    shift_h_odd = size / 2.0;
    shift_h_even = size;
    shift_v = size * sin (3.14159265358979323846 / 3.0);

    result = gaiaAllocGeomColl ();
    result->Srid = geom->Srid;
    get_grid_bbox (geom, &min_x, &min_y, &max_x, &max_y);
    get_trigrid_base (min_x, min_y, origin_x, origin_y, shift_h_odd,
		      shift_h_even, shift_v, &odd_even, &base_x, &base_y);
    base_x -= base_x;
    base_y -= shift_v;
    while (base_y < max_y)
      {
	  /* looping on grid rows */
	  if (odd_even)
	      x1 = base_x - shift_h_odd;
	  else
	      x1 = base_x;
	  y1 = base_y;
	  x2 = x1 + shift_h_even;
	  y2 = y1;
	  x3 = x1 + shift_h_odd;
	  y3 = y1 + shift_v;
	  x4 = x3 + shift_h_even;
	  y4 = y3;
	  while (x1 < max_x)
	    {
		/* looping on grid columns */
		item = gaiaAllocGeomColl ();

		pg = gaiaAddPolygonToGeomColl (item, 4, 0);
		rng = pg->Exterior;
		gaiaSetPoint (rng->Coords, 0, x1, y1);
		gaiaSetPoint (rng->Coords, 1, x2, y2);
		gaiaSetPoint (rng->Coords, 2, x3, y3);
		gaiaSetPoint (rng->Coords, 3, x1, y1);

		gaiaMbrGeometry (item);
		if (p_cache != NULL)
		    ret = gaiaGeomCollIntersects_r (p_cache, geom, item);
		else
		    ret = gaiaGeomCollIntersects (geom, item);
		if (ret == 1)
		  {
		      /* ok, inserting a valid cell [pointing upside] */
		      count++;
		      if (edges_only)
			{
			    /* multilinestring */
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x1, y1);
			    gaiaSetPoint (ln->Coords, 1, x2, y2);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x2, y2);
			    gaiaSetPoint (ln->Coords, 1, x3, y3);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x3, y3);
			    gaiaSetPoint (ln->Coords, 1, x1, y1);
			}
		      else
			{
			    /* polygon */
			    pg = gaiaAddPolygonToGeomColl (result, 4, 0);
			    rng = pg->Exterior;
			    gaiaSetPoint (rng->Coords, 0, x1, y1);
			    gaiaSetPoint (rng->Coords, 1, x2, y2);
			    gaiaSetPoint (rng->Coords, 2, x3, y3);
			    gaiaSetPoint (rng->Coords, 3, x1, y1);
			}
		  }
		gaiaFreeGeomColl (item);

		item = gaiaAllocGeomColl ();

		pg = gaiaAddPolygonToGeomColl (item, 4, 0);
		rng = pg->Exterior;
		gaiaSetPoint (rng->Coords, 0, x3, y3);
		gaiaSetPoint (rng->Coords, 1, x2, y2);
		gaiaSetPoint (rng->Coords, 2, x4, y4);
		gaiaSetPoint (rng->Coords, 3, x3, y3);

		gaiaMbrGeometry (item);
		if (p_cache != NULL)
		    ret = gaiaGeomCollIntersects_r (p_cache, geom, item);
		else
		    ret = gaiaGeomCollIntersects (geom, item);
		if (ret == 1)
		  {
		      /* ok, inserting a valid cell [pointing downside] */
		      count++;
		      if (edges_only)
			{
			    /* multilinestring */
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x1, y1);
			    gaiaSetPoint (ln->Coords, 1, x2, y2);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x2, y2);
			    gaiaSetPoint (ln->Coords, 1, x3, y3);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x3, y3);
			    gaiaSetPoint (ln->Coords, 1, x1, y1);
			}
		      else
			{
			    /* polygon */
			    pg = gaiaAddPolygonToGeomColl (result, 4, 0);
			    rng = pg->Exterior;
			    gaiaSetPoint (rng->Coords, 0, x3, y3);
			    gaiaSetPoint (rng->Coords, 1, x2, y2);
			    gaiaSetPoint (rng->Coords, 2, x4, y4);
			    gaiaSetPoint (rng->Coords, 3, x3, y3);
			}
		  }
		gaiaFreeGeomColl (item);

		x1 += shift_h_even;
		x2 += shift_h_even;
		x3 += shift_h_even;
		x4 += shift_h_even;
	    }
	  base_y += shift_v;
	  if (odd_even)
	      odd_even = 0;
	  else
	      odd_even = 1;
      }

/* final check */
    if (!count)
      {
	  /* empty grid */
	  gaiaFreeGeomColl (result);
	  return NULL;
      }
    if (!edges_only)
      {
	  result->DeclaredType = GAIA_MULTIPOLYGON;
	  return result;
      }

    item = result;
    if (p_cache != NULL)
	result = gaiaUnaryUnion_r (p_cache, item);
    else
	result = gaiaUnaryUnion (item);
    gaiaFreeGeomColl (item);
    result->Srid = geom->Srid;
    result->DeclaredType = GAIA_LINESTRING;
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaTriangularGrid (gaiaGeomCollPtr geom, double origin_x, double origin_y,
		    double size, int edges_only)
{
    return gaiaTriangularGridCommon (NULL, geom, origin_x, origin_y, size,
				     edges_only);
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaTriangularGrid_r (const void *p_cache, gaiaGeomCollPtr geom,
		      double origin_x, double origin_y, double size,
		      int edges_only)
{
    return gaiaTriangularGridCommon (p_cache, geom, origin_x, origin_y, size,
				     edges_only);
}

static void
get_hexgrid_base (double min_x, double min_y, double origin_x, double origin_y,
		  double shift3, double shift4, double shift,
		  int *odd_even, double *base_x, double *base_y)
{
/* determining the grid base-point [MinX/MinY] for am Hexagonal Grid */
    double bx = origin_x;
    double by = origin_y;
    while (1)
      {
	  /* looping on grid rows */
	  if (min_y < origin_y)
	    {
		/* going southward */
		if (by > min_y)
		    goto next_scanline;
		if (*odd_even)
		    bx = origin_x - (shift3 / 2.0);
		else
		    bx = origin_x;
	    }
	  else
	    {
		/* going northward */
		if (by < min_y)
		    goto next_scanline;
		if (*odd_even)
		    bx = origin_x + (shift3 / 2.0);
		else
		    bx = origin_x;
	    }
	  while (1)
	    {
		/* looping on grid columns */
		if (min_x < origin_x)
		  {
		      /* going westward */
		      if (bx - shift4 < min_x)
			{
			    *base_x = bx;
			    *base_y = by;
			    return;
			}
		      bx -= shift3;
		  }
		else
		  {
		      /* going eastward */
		      if (bx + shift4 > min_x)
			{
			    *base_x = bx;
			    *base_y = by;
			    return;
			}
		      bx += shift3;
		  }
	    }
	next_scanline:
	  if (min_y < origin_y)
	      by -= shift;
	  else
	      by += shift;
	  if (*odd_even)
	      *odd_even = 0;
	  else
	      *odd_even = 1;
      }
}

static gaiaGeomCollPtr
gaiaHexagonalGridCommon (const void *p_cache, gaiaGeomCollPtr geom,
			 double origin_x, double origin_y, double size,
			 int edges_only)
{
/* creating a regular grid [Hexagonal cells] */
    double min_x;
    double min_y;
    double max_x;
    double max_y;
    double base_x;
    double base_y;
    double x1;
    double y1;
    double x2;
    double y2;
    double x3;
    double y3;
    double x4;
    double y4;
    double x5;
    double y5;
    double x6;
    double y6;
    int count = 0;
    int odd_even = 0;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaLinestringPtr ln;
    gaiaGeomCollPtr result = NULL;
    gaiaGeomCollPtr item = NULL;
    double shift;
    double shift2;
    double shift3;
    double shift4;
    int ret;

    if (size <= 0.0)
	return NULL;
    shift = size * sin (3.14159265358979323846 / 3.0);
    shift2 = size / 2.0;
    shift3 = size * 3.0;
    shift4 = size * 2.0;

    result = gaiaAllocGeomColl ();
    result->Srid = geom->Srid;
    get_grid_bbox (geom, &min_x, &min_y, &max_x, &max_y);
    get_hexgrid_base (min_x, min_y, origin_x, origin_y, shift3, shift4,
		      shift, &odd_even, &base_x, &base_y);
    base_x -= shift3;
    base_y -= shift;
    while ((base_y - shift) < max_y)
      {
	  /* looping on grid rows */
	  if (odd_even)
	      x1 = base_x - (shift3 / 2.0);
	  else
	      x1 = base_x;
	  y1 = base_y;
	  x2 = x1 + shift2;
	  y2 = y1 - shift;
	  x3 = x2 + size;
	  y3 = y2;
	  x4 = x1 + shift4;
	  y4 = y1;
	  x5 = x3;
	  y5 = y1 + shift;
	  x6 = x2;
	  y6 = y5;
	  while (x1 < max_x)
	    {
		/* looping on grid columns */
		item = gaiaAllocGeomColl ();

		pg = gaiaAddPolygonToGeomColl (item, 7, 0);
		rng = pg->Exterior;
		gaiaSetPoint (rng->Coords, 0, x1, y1);
		gaiaSetPoint (rng->Coords, 1, x2, y2);
		gaiaSetPoint (rng->Coords, 2, x3, y3);
		gaiaSetPoint (rng->Coords, 3, x4, y4);
		gaiaSetPoint (rng->Coords, 4, x5, y5);
		gaiaSetPoint (rng->Coords, 5, x6, y6);
		gaiaSetPoint (rng->Coords, 6, x1, y1);

		gaiaMbrGeometry (item);
		if (p_cache != NULL)
		    ret = gaiaGeomCollIntersects_r (p_cache, geom, item);
		else
		    ret = gaiaGeomCollIntersects (geom, item);
		if (ret == 1)
		  {
		      /* ok, inserting a valid cell */
		      count++;
		      if (edges_only)
			{
			    /* multilinestring */
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x1, y1);
			    gaiaSetPoint (ln->Coords, 1, x2, y2);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x2, y2);
			    gaiaSetPoint (ln->Coords, 1, x3, y3);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x3, y3);
			    gaiaSetPoint (ln->Coords, 1, x4, y4);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x4, y4);
			    gaiaSetPoint (ln->Coords, 1, x5, y5);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x5, y5);
			    gaiaSetPoint (ln->Coords, 1, x6, y6);
			    ln = gaiaAddLinestringToGeomColl (result, 2);
			    gaiaSetPoint (ln->Coords, 0, x6, y6);
			    gaiaSetPoint (ln->Coords, 1, x1, y1);
			}
		      else
			{
			    /* polygon */
			    pg = gaiaAddPolygonToGeomColl (result, 7, 0);
			    rng = pg->Exterior;
			    gaiaSetPoint (rng->Coords, 0, x1, y1);
			    gaiaSetPoint (rng->Coords, 1, x2, y2);
			    gaiaSetPoint (rng->Coords, 2, x3, y3);
			    gaiaSetPoint (rng->Coords, 3, x4, y4);
			    gaiaSetPoint (rng->Coords, 4, x5, y5);
			    gaiaSetPoint (rng->Coords, 5, x6, y6);
			    gaiaSetPoint (rng->Coords, 6, x1, y1);
			}
		  }
		gaiaFreeGeomColl (item);

		x1 += shift3;
		x2 += shift3;
		x3 += shift3;
		x4 += shift3;
		x5 += shift3;
		x6 += shift3;
	    }
	  base_y += shift;
	  if (odd_even)
	      odd_even = 0;
	  else
	      odd_even = 1;
      }

/* final check */
    if (!count)
      {
	  /* empty grid */
	  gaiaFreeGeomColl (result);
	  return NULL;
      }
    if (!edges_only)
      {
	  result->DeclaredType = GAIA_MULTIPOLYGON;
	  return result;
      }

    item = result;
    if (p_cache != NULL)
	result = gaiaUnaryUnion_r (p_cache, item);
    else
	result = gaiaUnaryUnion (item);
    gaiaFreeGeomColl (item);
    result->Srid = geom->Srid;
    result->DeclaredType = GAIA_LINESTRING;
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaHexagonalGrid (gaiaGeomCollPtr geom, double origin_x, double origin_y,
		   double size, int edges_only)
{
    return gaiaHexagonalGridCommon (NULL, geom, origin_x, origin_y, size,
				    edges_only);
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaHexagonalGrid_r (const void *p_cache, gaiaGeomCollPtr geom, double origin_x,
		     double origin_y, double size, int edges_only)
{
    return gaiaHexagonalGridCommon (p_cache, geom, origin_x, origin_y, size,
				    edges_only);
}

#endif /* end including GEOS */
