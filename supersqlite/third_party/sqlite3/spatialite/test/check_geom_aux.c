/*

 check_geoms.c -- SpatiaLite Test Case

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

#include "sqlite3.h"
#include "spatialite.h"

#include <spatialite/gaiageo.h>

int
main (int argc, char *argv[])
{
    int ret;
    sqlite3 *handle;
    gaiaDynamicLinePtr dyn;
    gaiaDynamicLinePtr dyn2;
    gaiaDynamicLinePtr dyn3;
    gaiaPointPtr pt;
    gaiaPointPtr pt1;
    gaiaPointPtr pt2;
    gaiaGeomCollPtr geom_pt;
    gaiaGeomCollPtr geom1;
    gaiaGeomCollPtr geom2;
    gaiaGeomCollPtr geom3;
    gaiaLinestringPtr ln;
    gaiaLinestringPtr line1;
    gaiaLinestringPtr line2;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaRingPtr ring1;
    gaiaRingPtr ring2;
    gaiaPolygonPtr polyg1;
    gaiaPolygonPtr polyg2;
    double xy[6];
    unsigned char *blob;
    int size;
    double min;
    double max;
    int cnt;
    gaiaOutBuffer wkt;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1000;
      }

    spatialite_init_ex (handle, cache, 0);

    gaiaOutBufferInitialize (&wkt);

/* testing Dynamic Line */
    dyn = gaiaAllocDynamicLine ();
    pt1 = gaiaAppendPointToDynamicLine (dyn, 1.0, 2.0);
    pt2 = gaiaAppendPointToDynamicLine (dyn, 10.0, 11.0);
    gaiaDynamicLineInsertAfter (dyn, pt1, 2.0, 3.0);
    gaiaDynamicLineInsertBefore (dyn, pt2, 9.0, 10.0);
    gaiaAppendPointToDynamicLine (dyn, 11.0, 12.0);
    gaiaDynamicLineDeletePoint (dyn, pt2);
    dyn2 = gaiaCloneDynamicLine (dyn);
    dyn3 = gaiaReverseDynamicLine (dyn2);

    pt = dyn3->First;
    cnt = 0;
    while (pt)
      {
	  /* testing result */
	  switch (cnt)
	    {
	    case 0:
		if (pt->X != 11.0 || pt->Y != 12.0)
		  {
		      fprintf (stderr,
			       "ReverseDynamicLine: unexpected result (0)\n");
		      return -1;
		  }
		break;
	    case 1:
		if (pt->X != 9.0 || pt->Y != 10.0)
		  {
		      fprintf (stderr,
			       "ReverseDynamicLine: unexpected result (1)\n");
		      return -3;
		  }
		break;
	    case 2:
		if (pt->X != 2.0 || pt->Y != 3.0)
		  {
		      fprintf (stderr,
			       "ReverseDynamicLine: unexpected result (2)\n");
		      return -4;
		  }
		break;
	    case 3:
		if (pt->X != 1.0 || pt->Y != 2.0)
		  {
		      fprintf (stderr,
			       "ReverseDynamicLine: unexpected result (3)\n");
		      return -5;
		  }
		break;
	    default:
		fprintf (stderr, "ReverseDynamicLine: Point count > 4\n");
		return -6;
	    };
	  cnt++;
	  pt = pt->Next;
      }

    gaiaFreeDynamicLine (dyn3);
    gaiaFreeDynamicLine (dyn2);
    xy[0] = 10.0;
    xy[1] = 20.0;
    xy[2] = 11.0;
    xy[3] = 21.0;
    xy[4] = 12.0;
    xy[5] = 22.0;
    dyn3 = gaiaCreateDynamicLine (xy, 3);
    dyn2 = gaiaDynamicLineJoinAfter (dyn, pt1, dyn3);

    pt = dyn2->First;
    cnt = 0;
    while (pt)
      {
	  /* testing result */
	  switch (cnt)
	    {
	    case 0:
		if (pt->X != 1.0 || pt->Y != 2.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinAfter: unexpected result (0)\n");
		      return -7;
		  }
		break;
	    case 1:
		if (pt->X != 10.0 || pt->Y != 20.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinAfter: unexpected result (1)\n");
		      return -8;
		  }
		break;
	    case 2:
		if (pt->X != 11.0 || pt->Y != 21.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinAfter: unexpected result (2)\n");
		      return -9;
		  }
		break;
	    case 3:
		if (pt->X != 12.0 || pt->Y != 22.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinAfter: unexpected result (3)\n");
		      return -10;
		  }
		break;
	    case 4:
		if (pt->X != 2.0 || pt->Y != 3.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinAfter: unexpected result (4)\n");
		      return -11;
		  }
		break;
	    case 5:
		if (pt->X != 9.0 || pt->Y != 10.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinAfter: unexpected result (5)\n");
		      return -12;
		  }
		break;
	    case 6:
		if (pt->X != 11.0 || pt->Y != 12.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinAfter: unexpected result (6)\n");
		      return -13;
		  }
		break;
	    default:
		fprintf (stderr, "DynamicLineJoinAfter: Point count > 7\n");
		return -14;
	    };
	  cnt++;
	  pt = pt->Next;
      }

    gaiaFreeDynamicLine (dyn2);
    dyn2 = gaiaDynamicLineJoinBefore (dyn, pt1, dyn3);

    pt = dyn2->First;
    cnt = 0;
    while (pt)
      {
	  /* testing result */
	  switch (cnt)
	    {
	    case 0:
		if (pt->X != 10.0 || pt->Y != 20.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinBefore: unexpected result (0)\n");
		      return -15;
		  }
		break;
	    case 1:
		if (pt->X != 11.0 || pt->Y != 21.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinBefore: unexpected result (1)\n");
		      return -16;
		  }
		break;
	    case 2:
		if (pt->X != 12.0 || pt->Y != 22.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinBefore: unexpected result (2)\n");
		      return -17;
		  }
		break;
	    case 3:
		if (pt->X != 1.0 || pt->Y != 2.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinBefore: unexpected result (3)\n");
		      return -18;
		  }
		break;
	    case 4:
		if (pt->X != 2.0 || pt->Y != 3.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinBefore: unexpected result (4)\n");
		      return -19;
		  }
		break;
	    case 5:
		if (pt->X != 9.0 || pt->Y != 10.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinBefore: unexpected result (5)\n");
		      return -20;
		  }
		break;
	    case 6:
		if (pt->X != 11.0 || pt->Y != 12.0)
		  {
		      fprintf (stderr,
			       "DynamicLineJoinBefore: unexpected result (6)\n");
		      return -21;
		  }
		break;
	    default:
		fprintf (stderr, "DynamicLineJoinBefore: Point count > 7\n");
		return -22;
	    };
	  cnt++;
	  pt = pt->Next;
      }

    gaiaFreeDynamicLine (dyn3);
    gaiaFreeDynamicLine (dyn2);
    dyn3 = gaiaCloneDynamicLine (dyn);
    pt1 = gaiaDynamicLineFindByCoords (dyn, 9.0, 10.0);
    dyn2 = gaiaDynamicLineSplitBefore (dyn, pt1);

    pt = dyn2->First;
    cnt = 0;
    while (pt)
      {
	  /* testing result */
	  switch (cnt)
	    {
	    case 0:
		if (pt->X != 1.0 || pt->Y != 2.0)
		  {
		      fprintf (stderr,
			       "DynamicLineSplitBefore: unexpected result (0)\n");
		      return -23;
		  }
		break;
	    case 1:
		if (pt->X != 2.0 || pt->Y != 3.0)
		  {
		      fprintf (stderr,
			       "DynamicLineSplitBefore: unexpected result (1)\n");
		      return -24;
		  }
		break;
	    default:
		fprintf (stderr, "DynamicLineSplitBefore: Point count > 2\n");
		return -25;
	    };
	  cnt++;
	  pt = pt->Next;
      }

    gaiaFreeDynamicLine (dyn2);
    gaiaFreeDynamicLine (dyn);
    pt1 = gaiaDynamicLineFindByPos (dyn3, 1);

    dyn2 = gaiaDynamicLineSplitAfter (dyn3, pt1);

    pt = dyn2->First;
    cnt = 0;
    while (pt)
      {
	  /* testing result */
	  switch (cnt)
	    {
	    case 0:
		if (pt->X != 9.0 || pt->Y != 10.0)
		  {
		      fprintf (stderr,
			       "DynamicLineSplitAfter: unexpected result (0)\n");
		      return -26;
		  }
		break;
	    case 1:
		if (pt->X != 11.0 || pt->Y != 12.0)
		  {
		      fprintf (stderr,
			       "DynamicLineSplitAfter: unexpected result (1)\n");
		      return -27;
		  }
		break;
	    default:
		fprintf (stderr, "DynamicLineSplitAfter: Point count > 2\n");
		return -28;
	    };
	  cnt++;
	  pt = pt->Next;
      }

    gaiaFreeDynamicLine (dyn2);
    gaiaFreeDynamicLine (dyn3);

/* testing Geometry */
    geom_pt = gaiaAllocGeomColl ();
    gaiaAddPointToGeomColl (geom_pt, 5.0, 5.0);
    geom1 = gaiaAllocGeomColl ();
    gaiaAddPointToGeomColl (geom1, 15.0, 15.0);
    ln = gaiaAddLinestringToGeomColl (geom1, 2);
    gaiaLineSetPoint (ln, 0, 1.0, 1.0, 0.0, 0.0);
    gaiaLineSetPoint (ln, 1, 2.0, 2.0, 0.0, 0.0);
    pg = gaiaAddPolygonToGeomColl (geom1, 5, 0);
    rng = pg->Exterior;
    gaiaRingSetPoint (rng, 0, 1.0, 1.0, 0.0, 0.0);
    gaiaRingSetPoint (rng, 1, 5.0, 1.0, 0.0, 0.0);
    gaiaRingSetPoint (rng, 2, 5.0, 5.0, 0.0, 0.0);
    gaiaRingSetPoint (rng, 3, 1.0, 5.0, 0.0, 0.0);
    gaiaRingSetPoint (rng, 4, 1.0, 1.0, 0.0, 0.0);
    rng = gaiaAllocRing (5);
    gaiaRingSetPoint (rng, 0, 2.0, 2.0, 0.0, 0.0);
    gaiaRingSetPoint (rng, 1, 3.0, 2.0, 0.0, 0.0);
    gaiaRingSetPoint (rng, 2, 3.0, 3.0, 0.0, 0.0);
    gaiaRingSetPoint (rng, 3, 2.0, 3.0, 0.0, 0.0);
    gaiaRingSetPoint (rng, 4, 2.0, 2.0, 0.0, 0.0);
    gaiaInsertInteriorRing (pg, rng);
    gaiaFreeRing (rng);
    rng = gaiaAllocRing (5);
    gaiaRingSetPoint (rng, 0, 3.5, 3.5, 0.0, 0.0);
    gaiaRingSetPoint (rng, 1, 4.0, 3.5, 0.0, 0.0);
    gaiaRingSetPoint (rng, 2, 4.0, 4.0, 0.0, 0.0);
    gaiaRingSetPoint (rng, 3, 3.5, 4.0, 0.0, 0.0);
    gaiaRingSetPoint (rng, 4, 3.5, 3.5, 0.0, 0.0);
    gaiaInsertInteriorRing (pg, rng);
    gaiaFreeRing (rng);
    geom2 = gaiaCloneGeomColl (geom1);

    gaiaOutWkt (&wkt, geom2);
    if (wkt.Error || wkt.Buffer == NULL)
      {
	  fprintf (stderr, "Geom2D: unable to get WKT\n");
	  return -29;
      }
    else
      {
	  /* testing WKT result */
	  if (strcmp
	      (wkt.Buffer,
	       "GEOMETRYCOLLECTION(POINT(15 15), LINESTRING(1 1, 2 2), POLYGON((1 1, 5 1, 5 5, 1 5, 1 1), (2 2, 3 2, 3 3, 2 3, 2 2), (3.5 3.5, 4 3.5, 4 4, 3.5 4, 3.5 3.5)))")
	      != 0)
	    {
		/* unexpected result */
		fprintf (stderr, "Geom2D: unexpected result %s|\n", wkt.Buffer);
		return -30;
	    }
	  gaiaOutBufferReset (&wkt);
      }

    gaiaMakeLine (NULL, geom2, &blob, &size);
    if (blob)
	free (blob);
    gaiaMakeLine (geom_pt, NULL, &blob, &size);
    if (blob)
	free (blob);
    gaiaMakeLine (geom_pt, geom2, &blob, &size);
    if (blob)
	free (blob);
    gaiaMakeLine (geom1, geom2, &blob, &size);
    if (blob)
	free (blob);
    gaiaFreeGeomColl (geom2);
    geom2 = gaiaCloneGeomCollPoints (geom1);

    gaiaOutWkt (&wkt, geom2);
    if (wkt.Error || wkt.Buffer == NULL)
      {
	  fprintf (stderr, "Geom2D Points: unable to get WKT\n");
	  return -30;
      }
    else
      {
	  /* testing WKT result */
	  if (strcmp (wkt.Buffer, "MULTIPOINT(15 15)") != 0)
	    {
		/* unexpected result */
		fprintf (stderr, "Geom2D Points: unexpected result %s|\n",
			 wkt.Buffer);
		return -31;
	    }
	  gaiaOutBufferReset (&wkt);
      }

    gaiaFreeGeomColl (geom2);
    geom2 = gaiaCloneGeomCollLinestrings (geom1);

    gaiaOutWkt (&wkt, geom2);
    if (wkt.Error || wkt.Buffer == NULL)
      {
	  fprintf (stderr, "Geom2D Lines: unable to get WKT\n");
	  return -32;
      }
    else
      {
	  /* testing WKT result */
	  if (strcmp (wkt.Buffer, "MULTILINESTRING((1 1, 2 2))") != 0)
	    {
		/* unexpected result */
		fprintf (stderr, "Geom2D Lines: unexpected result %s|\n",
			 wkt.Buffer);
		return -33;
	    }
	  gaiaOutBufferReset (&wkt);
      }

    gaiaFreeGeomColl (geom2);
    geom2 = gaiaCloneGeomCollPolygons (geom1);

    gaiaOutWkt (&wkt, geom2);
    if (wkt.Error || wkt.Buffer == NULL)
      {
	  fprintf (stderr, "Geom2D Polygons: unable to get WKT\n");
	  return -34;
      }
    else
      {
	  /* testing WKT result */
	  if (strcmp
	      (wkt.Buffer,
	       "MULTIPOLYGON(((1 1, 5 1, 5 5, 1 5, 1 1), (2 2, 3 2, 3 3, 2 3, 2 2), (3.5 3.5, 4 3.5, 4 4, 3.5 4, 3.5 3.5)))")
	      != 0)
	    {
		/* unexpected result */
		fprintf (stderr, "Geom2D Polygons: unexpected result %s|\n",
			 wkt.Buffer);
		return -35;
	    }
	  gaiaOutBufferReset (&wkt);
      }

    gaiaFreeGeomColl (geom2);
    min = gaiaMeasureLength (geom1->FirstLinestring->DimensionModel,
			     geom1->FirstLinestring->Coords,
			     geom1->FirstLinestring->Points);
    rng = geom1->FirstPolygon->Exterior;
    min = gaiaMeasureArea (rng);
    gaiaRingCentroid (rng, &min, &max);
    gaiaIsPointOnPolygonSurface (geom1->FirstPolygon, 17.0, 22.0);
    min = gaiaMinDistance (1.0, 2.0, geom1->FirstLinestring->DimensionModel,
			   geom1->FirstLinestring->Coords,
			   geom1->FirstLinestring->Points);

/* cast XYZ */
    geom2 = gaiaCastGeomCollToXY (geom1);
    gaiaFreeGeomColl (geom2);
    geom2 = gaiaCastGeomCollToXYZ (geom1);
    geom3 = gaiaCloneGeomColl (geom2);
    gaiaFreeGeomColl (geom3);
    geom3 = gaiaCloneGeomCollPoints (geom2);
    gaiaFreeGeomColl (geom3);
    geom3 = gaiaCloneGeomCollLinestrings (geom2);
    gaiaFreeGeomColl (geom3);
    geom3 = gaiaCloneGeomCollPolygons (geom2);
    gaiaFreeGeomColl (geom3);
    rng = geom2->FirstPolygon->Exterior;

    min = gaiaMeasureLength (geom2->FirstLinestring->DimensionModel,
			     geom2->FirstLinestring->Coords,
			     geom2->FirstLinestring->Points);
    if (min < 1.41421 || min > 1.41422)
      {
	  /* unexpected result */
	  fprintf (stderr, "Geom3D Length: unexpected result %1.6f|\n", min);
	  return -36;
      }

    min = gaiaMeasureArea (rng);
    if (min != 16.0)
      {
	  /* unexpected result */
	  fprintf (stderr, "Geom3D Area: unexpected result %1.6f|\n", min);
	  return -37;
      }

    gaiaRingCentroid (rng, &min, &max);
    if (min != 3.0 || max != 3.0)
      {
	  /* unexpected result */
	  fprintf (stderr, "Geom3D Centroid: unexpected result %1.6f %1.6f|\n",
		   min, max);
	  return -38;
      }

    cnt = gaiaIsPointOnPolygonSurface (geom2->FirstPolygon, 17.0, 22.0);
    if (cnt)
      {
	  /* unexpected result */
	  fprintf (stderr,
		   "Geom3D IsPointOnPolygonSurface: unexpected result %d|\n",
		   cnt);
	  return -39;
      }

    min = gaiaMinDistance (1.0, 2.0, geom2->FirstLinestring->DimensionModel,
			   geom2->FirstLinestring->Coords,
			   geom2->FirstLinestring->Points);
    if (min < 0.70710 || min > 0.70711)
      {
	  /* unexpected result */
	  fprintf (stderr, "Geom3D MinDistance: unexpected result %1.6f|\n",
		   min);
	  return -40;
      }

    gaiaFreeGeomColl (geom2);

/* cast to XYM */
    geom2 = gaiaCastGeomCollToXYM (geom1);
    geom3 = gaiaCloneGeomColl (geom2);
    gaiaFreeGeomColl (geom3);
    geom3 = gaiaCloneGeomCollPoints (geom2);
    gaiaFreeGeomColl (geom3);
    geom3 = gaiaCloneGeomCollLinestrings (geom2);
    gaiaFreeGeomColl (geom3);
    geom3 = gaiaCloneGeomCollPolygons (geom2);
    gaiaFreeGeomColl (geom3);
    min = gaiaMeasureLength (geom2->FirstLinestring->DimensionModel,
			     geom2->FirstLinestring->Coords,
			     geom2->FirstLinestring->Points);
    if (min < 1.41421 || min > 1.41422)
      {
	  /* unexpected result */
	  fprintf (stderr, "GeomM Length: unexpected result %1.6f|\n", min);
	  return -41;
      }

    rng = geom2->FirstPolygon->Exterior;
    min = gaiaMeasureArea (rng);
    if (min != 16.0)
      {
	  /* unexpected result */
	  fprintf (stderr, "GeomM Area: unexpected result %1.6f|\n", min);
	  return -42;
      }

    gaiaRingCentroid (rng, &min, &max);
    if (min != 3.0 || max != 3.0)
      {
	  /* unexpected result */
	  fprintf (stderr, "GeomM Centroid: unexpected result %1.6f %1.6f|\n",
		   min, max);
	  return -43;
      }

    gaiaIsPointOnPolygonSurface (geom2->FirstPolygon, 17.0, 22.0);
    if (cnt)
      {
	  /* unexpected result */
	  fprintf (stderr,
		   "GeomM IsPointOnPolygonSurface: unexpected result %d|\n",
		   cnt);
	  return -44;
      }

    min = gaiaMinDistance (1.0, 2.0, geom2->FirstLinestring->DimensionModel,
			   geom2->FirstLinestring->Coords,
			   geom2->FirstLinestring->Points);
    if (min < 0.70710 || min > 0.70711)
      {
	  /* unexpected result */
	  fprintf (stderr, "GeomM MinDistance: unexpected result %1.6f|\n",
		   min);
	  return -45;
      }

    gaiaFreeGeomColl (geom2);

/* cast to XYZM */
    geom2 = gaiaCastGeomCollToXYZM (geom1);
    geom3 = gaiaCloneGeomColl (geom2);
    gaiaFreeGeomColl (geom3);
    gaiaMRangeGeometry (geom2, &min, &max);
    gaiaZRangeGeometry (geom2, &min, &max);
    geom3 = gaiaCloneGeomCollPoints (geom2);
    gaiaFreeGeomColl (geom3);
    geom3 = gaiaCloneGeomCollLinestrings (geom2);
    gaiaFreeGeomColl (geom3);
    geom3 = gaiaCloneGeomCollPolygons (geom2);
    gaiaFreeGeomColl (geom3);
    min = gaiaMeasureLength (geom2->FirstLinestring->DimensionModel,
			     geom2->FirstLinestring->Coords,
			     geom2->FirstLinestring->Points);
    if (min < 1.41421 || min > 1.41422)
      {
	  /* unexpected result */
	  fprintf (stderr, "GeomZM Length: unexpected result %1.6f|\n", min);
	  return -46;
      }

    rng = geom2->FirstPolygon->Exterior;
    min = gaiaMeasureArea (rng);
    if (min != 16.0)
      {
	  /* unexpected result */
	  fprintf (stderr, "GeomZM Area: unexpected result %1.6f|\n", min);
	  return -47;
      }

    gaiaRingCentroid (rng, &min, &max);
    if (min != 3.0 || max != 3.0)
      {
	  /* unexpected result */
	  fprintf (stderr, "GeomZM Centroid: unexpected result %1.6f %1.6f|\n",
		   min, max);
	  return -48;
      }

    gaiaIsPointOnPolygonSurface (geom2->FirstPolygon, 17.0, 22.0);
    if (cnt)
      {
	  /* unexpected result */
	  fprintf (stderr,
		   "GeomZM IsPointOnPolygonSurface: unexpected result %d|\n",
		   cnt);
	  return -49;
      }

    min = gaiaMinDistance (1.0, 2.0, geom2->FirstLinestring->DimensionModel,
			   geom2->FirstLinestring->Coords,
			   geom2->FirstLinestring->Points);
    if (min < 0.70710 || min > 0.70711)
      {
	  /* unexpected result */
	  fprintf (stderr, "GeomZM MinDistance: unexpected result %1.6f|\n",
		   min);
	  return -50;
      }

    gaiaFreeGeomColl (geom2);


    gaiaFreeGeomColl (geom_pt);
    gaiaFreeGeomColl (geom1);

    gaiaIntersect (&min, &max, 0.0, 10.0, 20.0, 10.0, 5.0, 0.0, 5.0, 20.0);
    if (min != 5.0 || max != 10.0)
      {
	  /* unexpected result */
	  fprintf (stderr, "gaiaIntersect: unexpected result %1.6f %1.6f|\n",
		   min, max);
	  return -51;
      }

/* same, reversed line direction */
    gaiaIntersect (&min, &max, 20.0, 10.0, 0.0, 10.0, 5.0, 20.0, 5.0, 0.0);
    if (min != 5.0 || max != 10.0)
      {
	  /* unexpected result */
	  fprintf (stderr, "gaiaIntersect: unexpected result %1.6f %1.6f|\n",
		   min, max);
	  return -52;
      }

/* non-intersecting MBRs*/
    min = -20;
    max = -20;
    gaiaIntersect (&min, &max, 20.0, 12.0, 0.0, 10.0, 25.0, 40.0, 45.0, 40.0);
    if (min != -20 || max != -20)
      {
	  /* unexpected result */
	  fprintf (stderr, "gaiaIntersect: unexpected result %1.6f %1.6f|\n",
		   min, max);
	  return -53;
      }

/* Just intersecting */
    min = 0;
    max = 0;
    gaiaIntersect (&min, &max, 0.0, 10.0, 20.0, 10.0, 5.0, 0.0, 5.0, 10.0);
    if (min != 5.0 || max != 10.0)
      {
	  /* unexpected result */
	  fprintf (stderr, "gaiaIntersect: unexpected result %1.6f %1.6f|\n",
		   min, max);
	  return -54;
      }

/* cloning a Linestring XY */
    line1 = gaiaAllocLinestring (4);
    gaiaLineSetPoint (line1, 0, 1.1, 1.2, 100.1, 10.1);
    gaiaLineSetPoint (line1, 1, 1.3, 1.4, 100.2, 10.2);
    gaiaLineSetPoint (line1, 2, 1.5, 1.6, 100.3, 10.3);
    gaiaLineSetPoint (line1, 3, 1.7, 1.8, 100.4, 10.4);
    line2 = gaiaCloneLinestringSpecial (line1, GAIA_SAME_ORDER);
    gaiaFreeLinestring (line2);
    line2 = gaiaCloneLinestringSpecial (line1, GAIA_REVERSE_ORDER);
    gaiaFreeLinestring (line2);
    gaiaFreeLinestring (line1);

/* cloning a Linestring XYZ */
    line1 = gaiaAllocLinestringXYZ (4);
    gaiaLineSetPoint (line1, 0, 1.1, 1.2, 100.1, 10.1);
    gaiaLineSetPoint (line1, 1, 1.3, 1.4, 100.2, 10.2);
    gaiaLineSetPoint (line1, 2, 1.5, 1.6, 100.3, 10.3);
    gaiaLineSetPoint (line1, 3, 1.7, 1.8, 100.4, 10.4);
    line2 = gaiaCloneLinestringSpecial (line1, GAIA_SAME_ORDER);
    gaiaFreeLinestring (line2);
    line2 = gaiaCloneLinestringSpecial (line1, GAIA_REVERSE_ORDER);
    gaiaFreeLinestring (line2);
    gaiaFreeLinestring (line1);

/* cloning a Linestring XYM */
    line1 = gaiaAllocLinestringXYM (4);
    gaiaLineSetPoint (line1, 0, 1.1, 1.2, 100.1, 10.1);
    gaiaLineSetPoint (line1, 1, 1.3, 1.4, 100.2, 10.2);
    gaiaLineSetPoint (line1, 2, 1.5, 1.6, 100.3, 10.3);
    gaiaLineSetPoint (line1, 3, 1.7, 1.8, 100.4, 10.4);
    line2 = gaiaCloneLinestringSpecial (line1, GAIA_SAME_ORDER);
    gaiaFreeLinestring (line2);
    line2 = gaiaCloneLinestringSpecial (line1, GAIA_REVERSE_ORDER);
    gaiaFreeLinestring (line2);
    gaiaFreeLinestring (line1);

/* cloning a Linestring XYZM */
    line1 = gaiaAllocLinestringXYZM (4);
    gaiaLineSetPoint (line1, 0, 1.1, 1.2, 100.1, 10.1);
    gaiaLineSetPoint (line1, 1, 1.3, 1.4, 100.2, 10.2);
    gaiaLineSetPoint (line1, 2, 1.5, 1.6, 100.3, 10.3);
    gaiaLineSetPoint (line1, 3, 1.7, 1.8, 100.4, 10.4);
    line2 = gaiaCloneLinestringSpecial (line1, GAIA_SAME_ORDER);
    gaiaFreeLinestring (line2);
    line2 = gaiaCloneLinestringSpecial (line1, GAIA_REVERSE_ORDER);
    gaiaFreeLinestring (line2);
    gaiaFreeLinestring (line1);

/* cloning a Ring XY */
    ring1 = gaiaAllocRing (5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.2, 1.1, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.1, 1.2, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    ring2 = gaiaCloneRingSpecial (ring1, GAIA_SAME_ORDER);
    gaiaFreeRing (ring2);
    ring2 = gaiaCloneRingSpecial (ring1, GAIA_REVERSE_ORDER);
    gaiaFreeRing (ring2);
    gaiaFreeRing (ring1);

/* cloning a Ring XYZ */
    ring1 = gaiaAllocRingXYZ (5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.2, 1.1, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.1, 1.2, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    ring2 = gaiaCloneRingSpecial (ring1, GAIA_SAME_ORDER);
    gaiaFreeRing (ring2);
    ring2 = gaiaCloneRingSpecial (ring1, GAIA_REVERSE_ORDER);
    gaiaFreeRing (ring2);
    gaiaFreeRing (ring1);

/* cloning a Ring XYM */
    ring1 = gaiaAllocRingXYM (5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.2, 1.1, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.1, 1.2, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    ring2 = gaiaCloneRingSpecial (ring1, GAIA_SAME_ORDER);
    gaiaFreeRing (ring2);
    ring2 = gaiaCloneRingSpecial (ring1, GAIA_REVERSE_ORDER);
    gaiaFreeRing (ring2);
    gaiaFreeRing (ring1);

/* cloning a Ring XYZM */
    ring1 = gaiaAllocRingXYZM (4);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.2, 1.1, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.1, 1.2, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    ring2 = gaiaCloneRingSpecial (ring1, GAIA_SAME_ORDER);
    gaiaFreeRing (ring2);
    ring2 = gaiaCloneRingSpecial (ring1, GAIA_REVERSE_ORDER);
    gaiaFreeRing (ring2);
    gaiaFreeRing (ring1);

/* cloning a Polygon XY (1) */
    polyg1 = gaiaAllocPolygon (5, 1);
    ring1 = polyg1->Exterior;
    gaiaRingSetPoint (ring1, 0, 1.0, 1.0, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 2.0, 1.0, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 2.0, 2.0, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.0, 2.0, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.0, 1.0, 100.1, 10.1);
    ring1 = gaiaAddInteriorRing (polyg1, 0, 5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.2, 1.1, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.1, 1.2, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_SAME_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_REVERSE_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_LHR_ORDER);
    gaiaFreePolygon (polyg2);
    gaiaFreePolygon (polyg1);

/* cloning a Polygon XY (2) */
    polyg1 = gaiaAllocPolygon (5, 1);
    ring1 = polyg1->Exterior;
    gaiaRingSetPoint (ring1, 0, 1.0, 1.0, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.0, 2.0, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 2.0, 2.0, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 2.0, 1.0, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.0, 1.0, 100.1, 10.1);
    ring1 = gaiaAddInteriorRing (polyg1, 0, 5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.1, 1.2, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.2, 1.1, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_SAME_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_REVERSE_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_LHR_ORDER);
    gaiaFreePolygon (polyg2);
    gaiaFreePolygon (polyg1);

/* cloning a Polygon XYZ (1) */
    polyg1 = gaiaAllocPolygonXYZ (5, 1);
    ring1 = polyg1->Exterior;
    gaiaRingSetPoint (ring1, 0, 1.0, 1.0, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 2.0, 1.0, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 2.0, 2.0, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.0, 2.0, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.0, 1.0, 100.1, 10.1);
    ring1 = gaiaAddInteriorRing (polyg1, 0, 5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.2, 1.1, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.1, 1.2, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_SAME_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_REVERSE_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_LHR_ORDER);
    gaiaFreePolygon (polyg2);
    gaiaFreePolygon (polyg1);

/* cloning a Polygon XYZ (2) */
    polyg1 = gaiaAllocPolygonXYZ (5, 1);
    ring1 = polyg1->Exterior;
    gaiaRingSetPoint (ring1, 0, 1.0, 1.0, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.0, 2.0, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 2.0, 2.0, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 2.0, 1.0, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.0, 1.0, 100.1, 10.1);
    ring1 = gaiaAddInteriorRing (polyg1, 0, 5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.1, 1.2, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.2, 1.1, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_SAME_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_REVERSE_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_LHR_ORDER);
    gaiaFreePolygon (polyg2);
    gaiaFreePolygon (polyg1);

/* cloning a Polygon XYM (1) */
    polyg1 = gaiaAllocPolygonXYM (5, 1);
    ring1 = polyg1->Exterior;
    gaiaRingSetPoint (ring1, 0, 1.0, 1.0, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 2.0, 1.0, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 2.0, 2.0, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.0, 2.0, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.0, 1.0, 100.1, 10.1);
    ring1 = gaiaAddInteriorRing (polyg1, 0, 5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.2, 1.1, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.1, 1.2, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_SAME_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_REVERSE_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_LHR_ORDER);
    gaiaFreePolygon (polyg2);
    gaiaFreePolygon (polyg1);

/* cloning a Polygon XYM (2) */
    polyg1 = gaiaAllocPolygonXYM (5, 1);
    ring1 = polyg1->Exterior;
    gaiaRingSetPoint (ring1, 0, 1.0, 1.0, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.0, 2.0, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 2.0, 2.0, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 2.0, 1.0, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.0, 1.0, 100.1, 10.1);
    ring1 = gaiaAddInteriorRing (polyg1, 0, 5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.1, 1.2, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.2, 1.1, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_SAME_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_REVERSE_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_LHR_ORDER);
    gaiaFreePolygon (polyg2);
    gaiaFreePolygon (polyg1);

/* cloning a Polygon XYZM (1) */
    polyg1 = gaiaAllocPolygonXYZM (5, 1);
    ring1 = polyg1->Exterior;
    gaiaRingSetPoint (ring1, 0, 1.0, 1.0, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 2.0, 1.0, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 2.0, 2.0, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.0, 2.0, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.0, 1.0, 100.1, 10.1);
    ring1 = gaiaAddInteriorRing (polyg1, 0, 5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.2, 1.1, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.1, 1.2, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_SAME_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_REVERSE_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_LHR_ORDER);
    gaiaFreePolygon (polyg2);
    gaiaFreePolygon (polyg1);

/* cloning a Polygon XYZM (2) */
    polyg1 = gaiaAllocPolygonXYZM (5, 1);
    ring1 = polyg1->Exterior;
    gaiaRingSetPoint (ring1, 0, 1.0, 1.0, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.0, 2.0, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 2.0, 2.0, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 2.0, 1.0, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.0, 1.0, 100.1, 10.1);
    ring1 = gaiaAddInteriorRing (polyg1, 0, 5);
    gaiaRingSetPoint (ring1, 0, 1.1, 1.1, 100.1, 10.1);
    gaiaRingSetPoint (ring1, 1, 1.1, 1.2, 100.2, 10.2);
    gaiaRingSetPoint (ring1, 2, 1.2, 1.2, 100.3, 10.3);
    gaiaRingSetPoint (ring1, 3, 1.2, 1.1, 100.4, 10.4);
    gaiaRingSetPoint (ring1, 4, 1.1, 1.1, 100.1, 10.1);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_SAME_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_REVERSE_ORDER);
    gaiaFreePolygon (polyg2);
    polyg2 = gaiaClonePolygonSpecial (polyg1, GAIA_LHR_ORDER);
    gaiaFreePolygon (polyg2);
    gaiaFreePolygon (polyg1);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -1001;
      }
    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
