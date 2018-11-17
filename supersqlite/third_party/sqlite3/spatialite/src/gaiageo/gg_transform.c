/*

 gg_transform.c -- Gaia PROJ.4 wrapping
  
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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#ifndef OMIT_PROJ		/* including PROJ.4 */
#include <proj_api.h>
#endif

#include <spatialite/sqlite.h>
#include <spatialite_private.h>

#include <spatialite/gaiageo.h>

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaMakeCircle (double cx, double cy, double radius, double step)
{
/* return a Linestring approximating a Circle */
    return gaiaMakeEllipse (cx, cy, radius, radius, step);
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaMakeArc (double cx,
	     double cy, double radius, double start, double stop, double step)
{
/* return a Linestring approximating a Circular Arc */
    return gaiaMakeEllipticArc (cx, cy, radius, radius, start, stop, step);
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaMakeEllipse (double cx, double cy, double x_axis, double y_axis,
		 double step)
{
/* return a Linestring approximating an Ellipse */
    gaiaDynamicLinePtr dyn;
    double x;
    double y;
    double angle = 0.0;
    int points = 0;
    gaiaPointPtr pt;
    gaiaGeomCollPtr geom;
    gaiaLinestringPtr ln;
    int iv = 0;

    if (step < 0.0)
	step *= -1.0;
    if (step == 0.0)
	step = 10.0;
    if (step < 0.1)
	step = 0.1;
    if (step > 45.0)
	step = 45.0;
    if (x_axis < 0.0)
	x_axis *= -1.0;
    if (y_axis < 0.0)
	y_axis *= -1.0;
    dyn = gaiaAllocDynamicLine ();
    while (angle < 360.0)
      {
	  double rads = angle * .0174532925199432958;
	  x = cx + (x_axis * cos (rads));
	  y = cy + (y_axis * sin (rads));
	  gaiaAppendPointToDynamicLine (dyn, x, y);
	  angle += step;
      }
/* closing the ellipse */
    gaiaAppendPointToDynamicLine (dyn, dyn->First->X, dyn->First->Y);

    pt = dyn->First;
    while (pt)
      {
	  /* counting how many points */
	  points++;
	  pt = pt->Next;
      }
    if (points == 0)
      {
	  gaiaFreeDynamicLine (dyn);
	  return NULL;
      }

    geom = gaiaAllocGeomColl ();
    ln = gaiaAddLinestringToGeomColl (geom, points);
    pt = dyn->First;
    while (pt)
      {
	  /* setting Vertices */
	  gaiaSetPoint (ln->Coords, iv, pt->X, pt->Y);
	  iv++;
	  pt = pt->Next;
      }
    gaiaFreeDynamicLine (dyn);
    return geom;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaMakeEllipticArc (double cx,
		     double cy, double x_axis, double y_axis, double start,
		     double stop, double step)
{
/* return a Linestring approximating an Elliptic Arc */
    gaiaDynamicLinePtr dyn;
    double x;
    double y;
    double angle;
    double rads;
    int points = 0;
    gaiaPointPtr pt;
    gaiaGeomCollPtr geom;
    gaiaLinestringPtr ln;
    int iv = 0;

    if (step < 0.0)
	step *= -1.0;
    if (step == 0.0)
	step = 10.0;
    if (step < 0.1)
	step = 0.1;
    if (step > 45.0)
	step = 45.0;
    if (x_axis < 0.0)
	x_axis *= -1.0;
    if (y_axis < 0.0)
	y_axis *= -1.0;
/* normalizing Start/Stop angles */
    while (start >= 360.0)
	start -= 360.0;
    while (start <= -720.0)
	start += 360;
    while (stop >= 360.0)
	stop -= 360.0;
    while (stop <= -720.0)
	stop += 360;
    if (start < 0.0)
	start = 360.0 + start;
    if (stop < 0.0)
	stop = 360.0 + stop;
    if (start > stop)
	stop += 360.0;

    dyn = gaiaAllocDynamicLine ();
    angle = start;
    while (angle < stop)
      {
	  rads = angle * .0174532925199432958;
	  x = cx + (x_axis * cos (rads));
	  y = cy + (y_axis * sin (rads));
	  gaiaAppendPointToDynamicLine (dyn, x, y);
	  angle += step;
	  points++;
      }
    if (points == 0)
      {
	  gaiaFreeDynamicLine (dyn);
	  return NULL;
      }
/* closing the arc */
    rads = stop * .0174532925199432958;
    x = cx + (x_axis * cos (rads));
    y = cy + (y_axis * sin (rads));
    if (x != dyn->Last->X || y != dyn->Last->Y)
	gaiaAppendPointToDynamicLine (dyn, x, y);

    pt = dyn->First;
    points = 0;
    while (pt)
      {
	  /* counting how many points */
	  points++;
	  pt = pt->Next;
      }
    if (points == 0)
      {
	  gaiaFreeDynamicLine (dyn);
	  return NULL;
      }

    geom = gaiaAllocGeomColl ();
    ln = gaiaAddLinestringToGeomColl (geom, points);
    pt = dyn->First;
    while (pt)
      {
	  /* setting Vertices */
	  gaiaSetPoint (ln->Coords, iv, pt->X, pt->Y);
	  iv++;
	  pt = pt->Next;
      }
    gaiaFreeDynamicLine (dyn);
    return geom;
}

GAIAGEO_DECLARE void
gaiaShiftCoords (gaiaGeomCollPtr geom, double shift_x, double shift_y)
{
/* returns a geometry that is the old geometry with required shifting applied to coordinates */
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* shifting POINTs */
	  point->X += shift_x;
	  point->Y += shift_y;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* shifting LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		x += shift_x;
		y += shift_y;
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (line->Coords, iv, x, y, z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (line->Coords, iv, x, y, m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (line->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (line->Coords, iv, x, y);
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* shifting POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* shifting the EXTERIOR RING */
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		  }
		x += shift_x;
		y += shift_y;
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ring->Coords, iv, x, y, m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* shifting the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			}
		      x += shift_x;
		      y += shift_y;
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, iv, x, y, m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, iv, x, y);
			}
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

GAIAGEO_DECLARE void
gaiaShiftCoords3D (gaiaGeomCollPtr geom, double shift_x, double shift_y,
		   double shift_z)
{
/* returns a geometry that is the old geometry with required shifting applied to coordinates */
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* shifting POINTs */
	  point->X += shift_x;
	  point->Y += shift_y;
	  if (point->DimensionModel == GAIA_XY_Z
	      || point->DimensionModel == GAIA_XY_Z_M)
	      point->Z += shift_z;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* shifting LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		x += shift_x;
		y += shift_y;
		z += shift_z;
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (line->Coords, iv, x, y, z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (line->Coords, iv, x, y, m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (line->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (line->Coords, iv, x, y);
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* shifting POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* shifting the EXTERIOR RING */
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		  }
		x += shift_x;
		y += shift_y;
		z += shift_z;
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ring->Coords, iv, x, y, m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* shifting the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			}
		      x += shift_x;
		      y += shift_y;
		      z += shift_z;
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, iv, x, y, m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, iv, x, y);
			}
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

GAIAGEO_DECLARE void
gaiaShiftLongitude (gaiaGeomCollPtr geom)
{
/* returns a geometry that is the old geometry with negative longitudes shift by 360 */
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* shifting POINTs */
	  if (point->X < 0)
	    {
		point->X += 360.0;
	    }
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* shifting LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		if (x < 0)
		  {
		      x += 360.0;
		  }
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (line->Coords, iv, x, y, z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (line->Coords, iv, x, y, m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (line->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (line->Coords, iv, x, y);
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* shifting POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* shifting the EXTERIOR RING */
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		  }
		if (x < 0)
		  {
		      x += 360.0;
		  }
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ring->Coords, iv, x, y, m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* shifting the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			}
		      if (x < 0)
			{
			    x += 360.0;
			}
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, iv, x, y, m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, iv, x, y);
			}
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

static void
normalizePoint (double *x, double *y)
{
    if ((-180.0 <= *x) && (*x <= 180.0) && (-90.0 <= *y) && (*y <= 90.0))
      {
	  /* then this point is already OK */
	  return;
      }
    if ((*x > 180.0) || (*x < -180.0))
      {
	  int numCycles = (int) (*x / 360.0);
	  *x -= numCycles * 360.0;
      }
    if (*x > 180.0)
      {
	  *x -= 360.0;
      }
    if (*x < -180.0)
      {
	  *x += 360.0;
      }
    if ((*y > 90.0) || (*y < -90.0))
      {
	  int numCycles = (int) (*y / 360.0);
	  *y -= numCycles * 360.0;
      }
    if (*y > 180.0)
      {
	  *y = -1.0 * (*y - 180.0);
      }
    if (*y < -180.0)
      {
	  *y = -1.0 * (*y + 180.0);
      }
    if (*y > 90.0)
      {
	  *y = 180 - *y;
      }
    if (*y < -90.0)
      {
	  *y = -180.0 - *y;
      }
}

GAIAGEO_DECLARE void
gaiaNormalizeLonLat (gaiaGeomCollPtr geom)
{
/* returns a geometry that is the old geometry with all latitudes shifted
 into the range -90 to 90, and all longitudes shifted into the range -180 to
 180.
*/
    int ib;
    int iv;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double m = 0.0;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  normalizePoint (&(point->X), &(point->Y));
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* shifting LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		normalizePoint (&x, &y);
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (line->Coords, iv, x, y, z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (line->Coords, iv, x, y, m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (line->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (line->Coords, iv, x, y);
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* shifting POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* shifting the EXTERIOR RING */
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		  }
		normalizePoint (&x, &y);
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ring->Coords, iv, x, y, m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* shifting the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			}
		      normalizePoint (&x, &y);
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, iv, x, y, m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, iv, x, y);
			}
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

GAIAGEO_DECLARE void
gaiaScaleCoords (gaiaGeomCollPtr geom, double scale_x, double scale_y)
{
/* returns a geometry that is the old geometry with required scaling applied to coordinates */
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* scaling POINTs */
	  point->X *= scale_x;
	  point->Y *= scale_y;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* scaling LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		x *= scale_x;
		y *= scale_y;
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (line->Coords, iv, x, y, z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (line->Coords, iv, x, y, m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (line->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (line->Coords, iv, x, y);
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* scaling POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* scaling the EXTERIOR RING */
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		  }
		x *= scale_x;
		y *= scale_y;
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ring->Coords, iv, x, y, m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* scaling the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			}
		      x *= scale_x;
		      y *= scale_y;
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, iv, x, y, m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, iv, x, y);
			}
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

GAIAGEO_DECLARE void
gaiaRotateCoords (gaiaGeomCollPtr geom, double angle)
{
/* returns a geometry that is the old geometry with required rotation applied to coordinates */
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    double nx;
    double ny;
    double rad = angle * 0.0174532925199432958;
    double cosine = cos (rad);
    double sine = sin (rad);
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* shifting POINTs */
	  x = point->X;
	  y = point->Y;
	  point->X = (x * cosine) + (y * sine);
	  point->Y = (y * cosine) - (x * sine);
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* rotating LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		nx = (x * cosine) + (y * sine);
		ny = (y * cosine) - (x * sine);
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (line->Coords, iv, nx, ny, z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (line->Coords, iv, nx, ny, m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (line->Coords, iv, nx, ny, z, m);
		  }
		else
		  {
		      gaiaSetPoint (line->Coords, iv, nx, ny);
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* rotating POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* rotating the EXTERIOR RING */
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		  }
		nx = (x * cosine) + (y * sine);
		ny = (y * cosine) - (x * sine);
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ring->Coords, iv, nx, ny, z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ring->Coords, iv, nx, ny, m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ring->Coords, iv, nx, ny, z, m);
		  }
		else
		  {
		      gaiaSetPoint (ring->Coords, iv, nx, ny);
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* rotating the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			}
		      nx = (x * cosine) + (y * sine);
		      ny = (y * cosine) - (x * sine);
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, iv, nx, ny, z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, iv, nx, ny, m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, iv, nx, ny, z, m);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, iv, nx, ny);
			}
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

GAIAGEO_DECLARE void
gaiaReflectCoords (gaiaGeomCollPtr geom, int x_axis, int y_axis)
{
/* returns a geometry that is the old geometry with required reflection applied to coordinates */
    int ib;
    int iv;
    double x;
    double y;
    double z = 0.0;
    double m = 0.0;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* reflecting POINTs */
	  if (x_axis)
	      point->X *= -1.0;
	  if (y_axis)
	      point->Y *= -1.0;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* reflecting LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		if (x_axis)
		    x *= -1.0;
		if (y_axis)
		    y *= -1.0;
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (line->Coords, iv, x, y, z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (line->Coords, iv, x, y, m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (line->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (line->Coords, iv, x, y);
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* reflecting POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* reflecting the EXTERIOR RING */
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		  }
		if (x_axis)
		    x *= -1.0;
		if (y_axis)
		    y *= -1.0;
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ring->Coords, iv, x, y, m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* reflecting the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			}
		      if (x_axis)
			  x *= -1.0;
		      if (y_axis)
			  y *= -1.0;
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, iv, x, y, m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, iv, x, y);
			}
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

GAIAGEO_DECLARE void
gaiaSwapCoords (gaiaGeomCollPtr geom)
{
/* returns a geometry that is the old geometry with swapped x- and y-coordinates */
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    double sv;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* swapping POINTs */
	  sv = point->X;
	  point->X = point->Y;
	  point->Y = sv;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* swapping LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		sv = x;
		x = y;
		y = sv;
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (line->Coords, iv, x, y, z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (line->Coords, iv, x, y, m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (line->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (line->Coords, iv, x, y);
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* swapping POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* shifting the EXTERIOR RING */
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		  }
		sv = x;
		x = y;
		y = sv;
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ring->Coords, iv, x, y, m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* swapping the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			}
		      sv = x;
		      x = y;
		      y = sv;
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, iv, x, y, m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, iv, x, y);
			}
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

#ifndef OMIT_PROJ		/* including PROJ.4 */

static int
gaiaIsLongLat (char *str)
{
/* checks if we have to do with ANGLES if +proj=longlat is defined */
    if (strstr (str, "+proj=longlat") != NULL)
	return 1;
    return 0;
}

GAIAGEO_DECLARE double
gaiaRadsToDegs (double rads)
{
/* converts an ANGLE from radians to degrees */
    return rads * RAD_TO_DEG;
}

GAIAGEO_DECLARE double
gaiaDegsToRads (double degs)
{
/* converts an ANGLE from degrees to radians */
    return degs * DEG_TO_RAD;
}

static gaiaGeomCollPtr
gaiaTransformCommon (projCtx handle, gaiaGeomCollPtr org, char *proj_from,
		     char *proj_to)
{
/* creates a new GEOMETRY reprojecting coordinates from the original one */
    int ib;
    int cnt;
    int i;
    double *xx;
    double *yy;
    double *zz;
    double *mm = NULL;
    double x;
    double y;
    double z = 0.0;
    double m = 0.0;
    int error = 0;
    int from_angle;
    int to_angle;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaLinestringPtr dst_ln;
    gaiaPolygonPtr pg;
    gaiaPolygonPtr dst_pg;
    gaiaRingPtr rng;
    gaiaRingPtr dst_rng;
    projPJ from_cs;
    projPJ to_cs;
    gaiaGeomCollPtr dst;
    if (handle != NULL)
      {
	  from_cs = pj_init_plus_ctx (handle, proj_from);
	  to_cs = pj_init_plus_ctx (handle, proj_to);
      }
    else
      {
	  from_cs = pj_init_plus (proj_from);
	  to_cs = pj_init_plus (proj_to);
      }
    if (!from_cs)
      {
	  if (to_cs)
	      pj_free (to_cs);
	  return NULL;
      }
    if (!to_cs)
      {
	  pj_free (from_cs);
	  return NULL;
      }
    if (org->DimensionModel == GAIA_XY_Z)
	dst = gaiaAllocGeomCollXYZ ();
    else if (org->DimensionModel == GAIA_XY_M)
	dst = gaiaAllocGeomCollXYM ();
    else if (org->DimensionModel == GAIA_XY_Z_M)
	dst = gaiaAllocGeomCollXYZM ();
    else
	dst = gaiaAllocGeomColl ();
/* setting up projection parameters */
    from_angle = gaiaIsLongLat (proj_from);
    to_angle = gaiaIsLongLat (proj_to);
    cnt = 0;
    pt = org->FirstPoint;
    while (pt)
      {
	  /* counting POINTs */
	  cnt++;
	  pt = pt->Next;
      }
    if (cnt)
      {
	  /* reprojecting POINTs */
	  xx = malloc (sizeof (double) * cnt);
	  yy = malloc (sizeof (double) * cnt);
	  zz = malloc (sizeof (double) * cnt);
	  if (org->DimensionModel == GAIA_XY_M
	      || org->DimensionModel == GAIA_XY_Z_M)
	      mm = malloc (sizeof (double) * cnt);
	  i = 0;
	  pt = org->FirstPoint;
	  while (pt)
	    {
		/* inserting points to be converted in temporary arrays */
		if (from_angle)
		  {
		      xx[i] = gaiaDegsToRads (pt->X);
		      yy[i] = gaiaDegsToRads (pt->Y);
		  }
		else
		  {
		      xx[i] = pt->X;
		      yy[i] = pt->Y;
		  }
		if (org->DimensionModel == GAIA_XY_Z
		    || org->DimensionModel == GAIA_XY_Z_M)
		    zz[i] = pt->Z;
		else
		    zz[i] = 0.0;
		if (org->DimensionModel == GAIA_XY_M
		    || org->DimensionModel == GAIA_XY_Z_M)
		    mm[i] = pt->M;
		i++;
		pt = pt->Next;
	    }
	  /* applying reprojection        */
	  if (pj_transform (from_cs, to_cs, cnt, 0, xx, yy, zz) == 0)
	    {
		/* inserting the reprojected POINTs in the new GEOMETRY */
		for (i = 0; i < cnt; i++)
		  {
		      if (to_angle)
			{
			    x = gaiaRadsToDegs (xx[i]);
			    y = gaiaRadsToDegs (yy[i]);
			}
		      else
			{
			    x = xx[i];
			    y = yy[i];
			}
		      if (org->DimensionModel == GAIA_XY_Z
			  || org->DimensionModel == GAIA_XY_Z_M)
			  z = zz[i];
		      else
			  z = 0.0;
		      if (org->DimensionModel == GAIA_XY_M
			  || org->DimensionModel == GAIA_XY_Z_M)
			  m = mm[i];
		      else
			  m = 0.0;
		      if (dst->DimensionModel == GAIA_XY_Z)
			  gaiaAddPointToGeomCollXYZ (dst, x, y, z);
		      else if (dst->DimensionModel == GAIA_XY_M)
			  gaiaAddPointToGeomCollXYM (dst, x, y, m);
		      else if (dst->DimensionModel == GAIA_XY_Z_M)
			  gaiaAddPointToGeomCollXYZM (dst, x, y, z, m);
		      else
			  gaiaAddPointToGeomColl (dst, x, y);
		  }
	    }
	  else
	      error = 1;
	  free (xx);
	  free (yy);
	  free (zz);
	  if (org->DimensionModel == GAIA_XY_M
	      || org->DimensionModel == GAIA_XY_Z_M)
	      free (mm);
      }
    if (error)
	goto stop;
    ln = org->FirstLinestring;
    while (ln)
      {
	  /* reprojecting LINESTRINGs */
	  cnt = ln->Points;
	  xx = malloc (sizeof (double) * cnt);
	  yy = malloc (sizeof (double) * cnt);
	  zz = malloc (sizeof (double) * cnt);
	  if (ln->DimensionModel == GAIA_XY_M
	      || ln->DimensionModel == GAIA_XY_Z_M)
	      mm = malloc (sizeof (double) * cnt);
	  for (i = 0; i < cnt; i++)
	    {
		/* inserting points to be converted in temporary arrays */
		if (ln->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ln->Coords, i, &x, &y, &z);
		  }
		else if (ln->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ln->Coords, i, &x, &y, &m);
		  }
		else if (ln->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ln->Coords, i, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ln->Coords, i, &x, &y);
		  }
		if (from_angle)
		  {
		      xx[i] = gaiaDegsToRads (x);
		      yy[i] = gaiaDegsToRads (y);
		  }
		else
		  {
		      xx[i] = x;
		      yy[i] = y;
		  }
		if (ln->DimensionModel == GAIA_XY_Z
		    || ln->DimensionModel == GAIA_XY_Z_M)
		    zz[i] = z;
		else
		    zz[i] = 0.0;
		if (ln->DimensionModel == GAIA_XY_M
		    || ln->DimensionModel == GAIA_XY_Z_M)
		    mm[i] = m;
	    }
	  /* applying reprojection        */
	  if (pj_transform (from_cs, to_cs, cnt, 0, xx, yy, zz) == 0)
	    {
		/* inserting the reprojected LINESTRING in the new GEOMETRY */
		dst_ln = gaiaAddLinestringToGeomColl (dst, cnt);
		for (i = 0; i < cnt; i++)
		  {
		      /* setting LINESTRING points */
		      if (to_angle)
			{
			    x = gaiaRadsToDegs (xx[i]);
			    y = gaiaRadsToDegs (yy[i]);
			}
		      else
			{
			    x = xx[i];
			    y = yy[i];
			}
		      if (ln->DimensionModel == GAIA_XY_Z
			  || ln->DimensionModel == GAIA_XY_Z_M)
			  z = zz[i];
		      else
			  z = 0.0;
		      if (ln->DimensionModel == GAIA_XY_M
			  || ln->DimensionModel == GAIA_XY_Z_M)
			  m = mm[i];
		      else
			  m = 0.0;
		      if (dst_ln->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (dst_ln->Coords, i, x, y, z);
			}
		      else if (dst_ln->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (dst_ln->Coords, i, x, y, m);
			}
		      else if (dst_ln->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (dst_ln->Coords, i, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (dst_ln->Coords, i, x, y);
			}
		  }
	    }
	  else
	      error = 1;
	  free (xx);
	  free (yy);
	  free (zz);
	  if (ln->DimensionModel == GAIA_XY_M
	      || ln->DimensionModel == GAIA_XY_Z_M)
	      free (mm);
	  if (error)
	      goto stop;
	  ln = ln->Next;
      }
    pg = org->FirstPolygon;
    while (pg)
      {
	  /* reprojecting POLYGONs */
	  rng = pg->Exterior;
	  cnt = rng->Points;
	  dst_pg = gaiaAddPolygonToGeomColl (dst, cnt, pg->NumInteriors);
	  xx = malloc (sizeof (double) * cnt);
	  yy = malloc (sizeof (double) * cnt);
	  zz = malloc (sizeof (double) * cnt);
	  if (rng->DimensionModel == GAIA_XY_M
	      || rng->DimensionModel == GAIA_XY_Z_M)
	      mm = malloc (sizeof (double) * cnt);
	  for (i = 0; i < cnt; i++)
	    {
		/* inserting points to be converted in temporary arrays [EXTERIOR RING] */
		if (rng->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (rng->Coords, i, &x, &y, &z);
		  }
		else if (rng->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (rng->Coords, i, &x, &y, &m);
		  }
		else if (rng->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (rng->Coords, i, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (rng->Coords, i, &x, &y);
		  }
		if (from_angle)
		  {
		      xx[i] = gaiaDegsToRads (x);
		      yy[i] = gaiaDegsToRads (y);
		  }
		else
		  {
		      xx[i] = x;
		      yy[i] = y;
		  }
		if (rng->DimensionModel == GAIA_XY_Z
		    || rng->DimensionModel == GAIA_XY_Z_M)
		    zz[i] = z;
		else
		    zz[i] = 0.0;
		if (rng->DimensionModel == GAIA_XY_M
		    || rng->DimensionModel == GAIA_XY_Z_M)
		    mm[i] = m;
	    }
	  /* applying reprojection        */
	  if (pj_transform (from_cs, to_cs, cnt, 0, xx, yy, zz) == 0)
	    {
		/* inserting the reprojected POLYGON in the new GEOMETRY */
		dst_rng = dst_pg->Exterior;
		for (i = 0; i < cnt; i++)
		  {
		      /* setting EXTERIOR RING points */
		      if (to_angle)
			{
			    x = gaiaRadsToDegs (xx[i]);
			    y = gaiaRadsToDegs (yy[i]);
			}
		      else
			{
			    x = xx[i];
			    y = yy[i];
			}
		      if (rng->DimensionModel == GAIA_XY_Z
			  || rng->DimensionModel == GAIA_XY_Z_M)
			  z = zz[i];
		      else
			  z = 0.0;
		      if (rng->DimensionModel == GAIA_XY_M
			  || rng->DimensionModel == GAIA_XY_Z_M)
			  m = mm[i];
		      else
			  m = 0.0;
		      if (dst_rng->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (dst_rng->Coords, i, x, y, z);
			}
		      else if (dst_rng->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (dst_rng->Coords, i, x, y, m);
			}
		      else if (dst_rng->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (dst_rng->Coords, i, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (dst_rng->Coords, i, x, y);
			}
		  }
	    }
	  else
	      error = 1;
	  free (xx);
	  free (yy);
	  free (zz);
	  if (rng->DimensionModel == GAIA_XY_M
	      || rng->DimensionModel == GAIA_XY_Z_M)
	      free (mm);
	  if (error)
	      goto stop;
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		/* processing INTERIOR RINGS */
		rng = pg->Interiors + ib;
		cnt = rng->Points;
		xx = malloc (sizeof (double) * cnt);
		yy = malloc (sizeof (double) * cnt);
		zz = malloc (sizeof (double) * cnt);
		if (rng->DimensionModel == GAIA_XY_M
		    || rng->DimensionModel == GAIA_XY_Z_M)
		    mm = malloc (sizeof (double) * cnt);
		for (i = 0; i < cnt; i++)
		  {
		      /* inserting points to be converted in temporary arrays [INTERIOR RING] */
		      if (rng->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (rng->Coords, i, &x, &y, &z);
			}
		      else if (rng->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (rng->Coords, i, &x, &y, &m);
			}
		      else if (rng->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (rng->Coords, i, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (rng->Coords, i, &x, &y);
			}
		      if (from_angle)
			{
			    xx[i] = gaiaDegsToRads (x);
			    yy[i] = gaiaDegsToRads (y);
			}
		      else
			{
			    xx[i] = x;
			    yy[i] = y;
			}
		      if (rng->DimensionModel == GAIA_XY_Z
			  || rng->DimensionModel == GAIA_XY_Z_M)
			  zz[i] = z;
		      else
			  zz[i] = 0.0;
		      if (rng->DimensionModel == GAIA_XY_M
			  || rng->DimensionModel == GAIA_XY_Z_M)
			  mm[i] = m;
		  }
		/* applying reprojection        */
		if (pj_transform (from_cs, to_cs, cnt, 0, xx, yy, zz) == 0)
		  {
		      /* inserting the reprojected POLYGON in the new GEOMETRY */
		      dst_rng = gaiaAddInteriorRing (dst_pg, ib, cnt);
		      for (i = 0; i < cnt; i++)
			{
			    /* setting INTERIOR RING points */
			    if (to_angle)
			      {
				  x = gaiaRadsToDegs (xx[i]);
				  y = gaiaRadsToDegs (yy[i]);
			      }
			    else
			      {
				  x = xx[i];
				  y = yy[i];
			      }
			    if (rng->DimensionModel == GAIA_XY_Z
				|| rng->DimensionModel == GAIA_XY_Z_M)
				z = zz[i];
			    else
				z = 0.0;
			    if (rng->DimensionModel == GAIA_XY_M
				|| rng->DimensionModel == GAIA_XY_Z_M)
				m = mm[i];
			    else
				m = 0.0;
			    if (dst_rng->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaSetPointXYZ (dst_rng->Coords, i, x, y, z);
			      }
			    else if (dst_rng->DimensionModel == GAIA_XY_M)
			      {
				  gaiaSetPointXYM (dst_rng->Coords, i, x, y, m);
			      }
			    else if (dst_rng->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaSetPointXYZM (dst_rng->Coords, i, x, y, z,
						    m);
			      }
			    else
			      {
				  gaiaSetPoint (dst_rng->Coords, i, x, y);
			      }
			}
		  }
		else
		    error = 1;
		free (xx);
		free (yy);
		free (zz);
		if (rng->DimensionModel == GAIA_XY_M
		    || rng->DimensionModel == GAIA_XY_Z_M)
		    free (mm);
		if (error)
		    goto stop;
	    }
	  pg = pg->Next;
      }
/* destroying the PROJ4 params */
  stop:
    pj_free (from_cs);
    pj_free (to_cs);
    if (error)
      {
	  /* some error occurred */
	  gaiaPointPtr pP;
	  gaiaPointPtr pPn;
	  gaiaLinestringPtr pL;
	  gaiaLinestringPtr pLn;
	  gaiaPolygonPtr pA;
	  gaiaPolygonPtr pAn;
	  pP = dst->FirstPoint;
	  while (pP != NULL)
	    {
		pPn = pP->Next;
		gaiaFreePoint (pP);
		pP = pPn;
	    }
	  pL = dst->FirstLinestring;
	  while (pL != NULL)
	    {
		pLn = pL->Next;
		gaiaFreeLinestring (pL);
		pL = pLn;
	    }
	  pA = dst->FirstPolygon;
	  while (pA != NULL)
	    {
		pAn = pA->Next;
		gaiaFreePolygon (pA);
		pA = pAn;
	    }
	  dst->FirstPoint = NULL;
	  dst->LastPoint = NULL;
	  dst->FirstLinestring = NULL;
	  dst->LastLinestring = NULL;
	  dst->FirstPolygon = NULL;
	  dst->LastPolygon = NULL;
      }
    if (dst)
      {
	  gaiaMbrGeometry (dst);
	  dst->DeclaredType = org->DeclaredType;
      }
    return dst;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaTransform (gaiaGeomCollPtr org, char *proj_from, char *proj_to)
{
    return gaiaTransformCommon (NULL, org, proj_from, proj_to);
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaTransform_r (const void *p_cache, gaiaGeomCollPtr org, char *proj_from,
		 char *proj_to)
{
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    projCtx handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->PROJ_handle;
    if (handle == NULL)
	return NULL;
    return gaiaTransformCommon (handle, org, proj_from, proj_to);
}

#endif /* end including PROJ.4 */
