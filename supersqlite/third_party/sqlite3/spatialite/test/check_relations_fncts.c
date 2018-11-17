/*

 check_relations_fncts.c -- SpatiaLite Test Case
 
 This does "boundary conditions" and error checks for gg_relations
 functions that are hard to test with SQL.

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
#include "spatialite/gaiageo.h"

static const double double_eps = 0.00000001;

int
test_extra_mode ()
{
#ifndef OMIT_GEOS		/* only if GEOS is supported */
    int ret;
    sqlite3 *handle;
    int result;
    int returnValue = 0;
    gaiaGeomCollPtr geom_pt1;
    gaiaGeomCollPtr geom_pt2;
    gaiaGeomCollPtr geom_ln1;
    gaiaGeomCollPtr geom_ln2;
    gaiaGeomCollPtr geom_pg;
    gaiaGeomCollPtr g;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    void *cache = spatialite_alloc_connection ();

    /* Tests start here */
    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -101;
      }

    spatialite_init_ex (handle, cache, 0);

    /* preparing a Point */
    geom_pt1 = gaiaAllocGeomColl ();
    gaiaAddPointToGeomColl (geom_pt1, 30.0, 30.0);
    gaiaMbrGeometry (geom_pt1);

    /* preparing another Point */
    geom_pt2 = gaiaAllocGeomColl ();
    gaiaAddPointToGeomColl (geom_pt2, 10.5, 1.5);
    gaiaMbrGeometry (geom_pt2);

    /* preparing a Linestring */
    geom_ln1 = gaiaAllocGeomColl ();
    ln = gaiaAddLinestringToGeomColl (geom_ln1, 3);
    gaiaSetPoint (ln->Coords, 0, 5.0, 0.0);
    gaiaSetPoint (ln->Coords, 1, 5.0, 20.0);
    gaiaSetPoint (ln->Coords, 2, 0.0, 20.0);
    gaiaMbrGeometry (geom_ln1);

    /* preparing another Linestring */
    geom_ln2 = gaiaAllocGeomColl ();
    ln = gaiaAddLinestringToGeomColl (geom_ln2, 4);
    gaiaSetPoint (ln->Coords, 0, 0.0, 20.0);
    gaiaSetPoint (ln->Coords, 1, 5.0, 20.0);
    gaiaSetPoint (ln->Coords, 2, 5.0, 0.0);
    gaiaSetPoint (ln->Coords, 3, 0.0, 0.0);
    gaiaMbrGeometry (geom_ln2);

    /* preparing a Polygon */
    geom_pg = gaiaAllocGeomColl ();
    pg = gaiaAddPolygonToGeomColl (geom_pg, 5, 1);
    rng = pg->Exterior;
    gaiaSetPoint (rng->Coords, 0, 1.5, 1.5);
    gaiaSetPoint (rng->Coords, 1, 10.5, 1.5);
    gaiaSetPoint (rng->Coords, 2, 10.5, 10.5);
    gaiaSetPoint (rng->Coords, 3, 1.5, 10.5);
    gaiaSetPoint (rng->Coords, 4, 1.5, 1.5);
    rng = gaiaAddInteriorRing (pg, 0, 5);
    gaiaSetPoint (rng->Coords, 0, 5.5, 5.5);
    gaiaSetPoint (rng->Coords, 1, 6.5, 1.5);
    gaiaSetPoint (rng->Coords, 2, 6.5, 6.5);
    gaiaSetPoint (rng->Coords, 3, 5.5, 6.5);
    gaiaSetPoint (rng->Coords, 4, 5.5, 5.5);
    gaiaMbrGeometry (geom_pg);

    result = gaiaGeomCollDisjoint_r (cache, geom_pt1, geom_ln1);
    if (result != 1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -102;
	  goto exit;
      }
    result = gaiaGeomCollDisjoint_r (cache, geom_pt2, geom_pg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -103;
	  goto exit;
      }
    result = gaiaGeomCollDisjoint_r (cache, geom_pg, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -104;
	  goto exit;
      }

    result = gaiaGeomCollOverlaps_r (cache, geom_pt2, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -105;
	  goto exit;
      }
    result = gaiaGeomCollOverlaps_r (cache, geom_pt2, geom_pg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -106;
	  goto exit;
      }
    result = gaiaGeomCollOverlaps_r (cache, geom_pg, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -107;
	  goto exit;
      }

    result = gaiaGeomCollCrosses_r (cache, geom_pt2, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -108;
	  goto exit;
      }
    result = gaiaGeomCollCrosses_r (cache, geom_pt2, geom_pg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -109;
	  goto exit;
      }
    result = gaiaGeomCollCrosses_r (cache, geom_pg, geom_ln1);
    if (result != 1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -110;
	  goto exit;
      }

    result = gaiaGeomCollTouches_r (cache, geom_pt2, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -111;
	  goto exit;
      }
    result = gaiaGeomCollTouches_r (cache, geom_pt2, geom_pg);
    if (result != 1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -112;
	  goto exit;
      }
    result = gaiaGeomCollTouches_r (cache, geom_pg, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -113;
	  goto exit;
      }

    result = gaiaGeomCollWithin_r (cache, geom_pt2, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -114;
	  goto exit;
      }
    result = gaiaGeomCollWithin_r (cache, geom_pt2, geom_pg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -115;
	  goto exit;
      }
    result = gaiaGeomCollWithin_r (cache, geom_pg, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -116;
	  goto exit;
      }

    result = gaiaGeomCollContains_r (cache, geom_pt2, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -117;
	  goto exit;
      }
    result = gaiaGeomCollContains_r (cache, geom_pg, geom_pt2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -118;
	  goto exit;
      }
    result = gaiaGeomCollContains_r (cache, geom_pg, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -119;
	  goto exit;
      }

    result = gaiaGeomCollCovers_r (cache, geom_pt1, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -120;
	  goto exit;
      }
    result = gaiaGeomCollCovers_r (cache, geom_pg, geom_pt2);
    if (result != 1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -121;
	  goto exit;
      }
    result = gaiaGeomCollCovers_r (cache, geom_pg, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -122;
	  goto exit;
      }

    result = gaiaGeomCollCoveredBy_r (cache, geom_pt2, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -120;
	  goto exit;
      }
    result = gaiaGeomCollCoveredBy_r (cache, geom_pt2, geom_pg);
    if (result != 1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -121;
	  goto exit;
      }
    result = gaiaGeomCollCoveredBy_r (cache, geom_pg, geom_ln1);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -122;
	  goto exit;
      }

    g = gaiaSharedPaths_r (cache, geom_ln1, geom_ln2);
    if (g == NULL)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -123;
	  goto exit;
      }
    gaiaFreeGeomColl (g);
    g = gaiaSharedPaths_r (cache, geom_ln1, geom_ln2);
    if (g == NULL)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -124;
	  goto exit;
      }
    gaiaFreeGeomColl (g);

    /* Cleanup and exit */
  exit:
    gaiaFreeGeomColl (geom_pt1);
    gaiaFreeGeomColl (geom_pt2);
    gaiaFreeGeomColl (geom_ln1);
    gaiaFreeGeomColl (geom_ln2);
    gaiaFreeGeomColl (geom_pg);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -133;
      }

    spatialite_cleanup_ex (cache);
    return returnValue;

#endif /* end GEOS conditional */
    return 0;
}

int
test_current_mode ()
{
#ifndef OMIT_GEOS		/* only if GEOS is supported */
    int ret;
    sqlite3 *handle;
    int result;
    double resultDouble;
    int returnValue = 0;
    gaiaGeomCollPtr geom;
    gaiaGeomCollPtr g;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    void *cache = spatialite_alloc_connection ();

    /* Common setup */
    gaiaGeomCollPtr validGeometry = gaiaAllocGeomColl ();
    double dummyResultArg = 0.0;
    double dummyResultArg2 = 0.0;

    /* Tests start here */
    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }

    spatialite_init_ex (handle, cache, 0);

    /* null inputs for a range of geometry functions */
    result = gaiaGeomCollEquals_r (cache, 0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -1;
	  goto exit;
      }

    result = gaiaGeomCollEquals_r (cache, validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -2;
	  goto exit;
      }

    result = gaiaGeomCollEquals_r (cache, 0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -3;
	  goto exit;
      }

    result = gaiaGeomCollIntersects_r (cache, 0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -4;
	  goto exit;
      }

    result = gaiaGeomCollIntersects_r (cache, validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -5;
	  goto exit;
      }

    result = gaiaGeomCollIntersects_r (cache, 0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -6;
	  goto exit;
      }

    result = gaiaGeomCollOverlaps_r (cache, 0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -7;
	  goto exit;
      }

    result = gaiaGeomCollOverlaps_r (cache, validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -8;
	  goto exit;
      }

    result = gaiaGeomCollOverlaps_r (cache, 0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -9;
	  goto exit;
      }

    result = gaiaGeomCollCrosses_r (cache, 0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -10;
	  goto exit;
      }

    result = gaiaGeomCollCrosses_r (cache, validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -11;
	  goto exit;
      }

    result = gaiaGeomCollCrosses_r (cache, 0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -12;
	  goto exit;
      }

    result = gaiaGeomCollTouches_r (cache, 0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -13;
	  goto exit;
      }

    result = gaiaGeomCollTouches_r (cache, validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -14;
	  goto exit;
      }

    result = gaiaGeomCollTouches_r (cache, 0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -15;
	  goto exit;
      }

    result = gaiaGeomCollDisjoint_r (cache, 0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -16;
	  goto exit;
      }

    result = gaiaGeomCollDisjoint_r (cache, validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -17;
	  goto exit;
      }

    result = gaiaGeomCollDisjoint_r (cache, 0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -18;
	  goto exit;
      }

    result = gaiaGeomCollWithin_r (cache, 0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -19;
	  goto exit;
      }

    result = gaiaGeomCollWithin_r (cache, validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -20;
	  goto exit;
      }

    result = gaiaGeomCollWithin_r (cache, 0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -21;
	  goto exit;
      }

    result = gaiaGeomCollContains_r (cache, 0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -22;
	  goto exit;
      }

    result = gaiaGeomCollContains_r (cache, validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -23;
	  goto exit;
      }

    result = gaiaGeomCollContains_r (cache, 0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -24;
	  goto exit;
      }

    result = gaiaGeomCollRelate_r (cache, 0, validGeometry, "T********");
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -25;
	  goto exit;
      }

    result = gaiaGeomCollRelate_r (cache, validGeometry, 0, "T********");
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -26;
	  goto exit;
      }

    result = gaiaGeomCollRelate_r (cache, 0, 0, "T********");
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -27;
	  goto exit;
      }

    result = gaiaHausdorffDistance_r (cache, 0, validGeometry, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -34;
	  goto exit;
      }

    result = gaiaHausdorffDistance_r (cache, validGeometry, 0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -35;
	  goto exit;
      }

    result = gaiaHausdorffDistance_r (cache, 0, 0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -36;
	  goto exit;
      }

    result = gaiaGeomCollCovers_r (cache, 0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -37;
	  goto exit;
      }

    result = gaiaGeomCollCovers_r (cache, validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -38;
	  goto exit;
      }

    result = gaiaGeomCollCovers_r (cache, 0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -39;
	  goto exit;
      }

    result = gaiaGeomCollCoveredBy_r (cache, 0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -40;
	  goto exit;
      }

    result = gaiaGeomCollCoveredBy_r (cache, validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -41;
	  goto exit;
      }

    result = gaiaGeomCollCoveredBy_r (cache, 0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -42;
	  goto exit;
      }

    result = gaiaGeomCollDistance_r (cache, 0, validGeometry, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -43;
	  goto exit;
      }

    result = gaiaGeomCollDistance_r (cache, validGeometry, 0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -44;
	  goto exit;
      }

    result = gaiaGeomCollDistance_r (cache, 0, 0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -45;
	  goto exit;
      }
    /* Check some of the single geometry analysis routines */

    result = gaiaGeomCollLength_r (cache, 0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -46;
	  goto exit;
      }

    result = gaiaGeomCollArea_r (cache, 0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -47;
	  goto exit;
      }

    result =
	gaiaGeomCollCentroid_r (cache, 0, &dummyResultArg, &dummyResultArg2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -48;
	  goto exit;
      }

    result =
	gaiaGetPointOnSurface_r (cache, 0, &dummyResultArg, &dummyResultArg2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -49;
	  goto exit;
      }

    result = gaiaIsSimple_r (cache, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -32;
	  goto exit;
      }

    result = gaiaIsValid_r (cache, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -33;
	  goto exit;
      }

    resultDouble = gaiaLineLocatePoint_r (cache, 0, validGeometry);
    if (abs (resultDouble - -1.0) > double_eps)
      {
	  fprintf (stderr, "bad result at %s:%i: %f\n", __FILE__, __LINE__,
		   resultDouble);
	  returnValue = -62;
	  goto exit;
      }

    resultDouble = gaiaLineLocatePoint_r (cache, validGeometry, 0);
    if (abs (resultDouble - -1.0) > double_eps)
      {
	  fprintf (stderr, "bad result at %s:%i: %f\n", __FILE__, __LINE__,
		   resultDouble);
	  returnValue = -63;
	  goto exit;
      }

    resultDouble = gaiaLineLocatePoint_r (cache, 0, 0);
    if (abs (resultDouble - -1.0) > double_eps)
      {
	  fprintf (stderr, "bad result at %s:%i: %f\n", __FILE__, __LINE__,
		   resultDouble);
	  returnValue = -64;
	  goto exit;
      }

    /* geometry generating functionality */
    geom = NULL;
    geom = gaiaLinesCutAtNodes (0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -50;
	  goto exit;
      }

    geom = gaiaLinesCutAtNodes (validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -51;
	  goto exit;
      }

    geom = gaiaLinesCutAtNodes (0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -52;
	  goto exit;
      }

    geom = gaiaUnaryUnion_r (cache, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -53;
	  goto exit;
      }

    geom = gaiaLineMerge (0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -54;
	  goto exit;
      }

    geom = gaiaSnap_r (cache, 0, validGeometry, 4);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -55;
	  goto exit;
      }

    geom = gaiaSnap_r (cache, validGeometry, 0, 4);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -56;
	  goto exit;
      }

    geom = gaiaSnap_r (cache, 0, 0, 4);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -57;
	  goto exit;
      }

    geom = gaiaShortestLine_r (cache, 0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -58;
	  goto exit;
      }

    geom = gaiaShortestLine_r (cache, validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -59;
	  goto exit;
      }

    geom = gaiaShortestLine_r (cache, 0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -60;
	  goto exit;
      }

    geom = gaiaLineSubstring_r (cache, 0, 0, 1);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -61;
	  goto exit;
      }

    geom = gaiaGeometryIntersection_r (cache, 0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -65;
	  goto exit;
      }

    geom = gaiaGeometryIntersection_r (cache, validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -66;
	  goto exit;
      }

    geom = gaiaGeometryIntersection_r (cache, 0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -67;
	  goto exit;
      }

    geom = gaiaGeometryUnion_r (cache, 0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -68;
	  goto exit;
      }

    geom = gaiaGeometryUnion_r (cache, validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -69;
	  goto exit;
      }

    geom = gaiaGeometryUnion_r (cache, 0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -70;
	  goto exit;
      }

    geom = gaiaGeometryDifference_r (cache, 0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -71;
	  goto exit;
      }

    geom = gaiaGeometryDifference_r (cache, validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -72;
	  goto exit;
      }

    geom = gaiaGeometryDifference_r (cache, 0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -73;
	  goto exit;
      }

    geom = gaiaGeometrySymDifference_r (cache, 0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -74;
	  goto exit;
      }

    geom = gaiaGeometrySymDifference_r (cache, validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -75;
	  goto exit;
      }

    geom = gaiaGeometrySymDifference_r (cache, 0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -76;
	  goto exit;
      }

    geom = gaiaBoundary_r (cache, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -77;
	  goto exit;
      }

    geom = gaiaGeomCollSimplify_r (cache, 0, 1.0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -78;
	  goto exit;
      }

    geom = gaiaGeomCollSimplifyPreserveTopology_r (cache, 0, 1.0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -79;
	  goto exit;
      }

    geom = gaiaConvexHull_r (cache, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -80;
	  goto exit;
      }

    geom = gaiaGeomCollBuffer_r (cache, 0, 0.1, 10);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -81;
	  goto exit;
      }

    geom = gaiaOffsetCurve_r (cache, 0, 1.5, 10, 1);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -82;
	  goto exit;
      }

    geom = gaiaSingleSidedBuffer_r (cache, 0, 1.5, 10, 1);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -83;
	  goto exit;
      }

    geom = gaiaSharedPaths_r (cache, 0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -84;
	  goto exit;
      }

    geom = gaiaSharedPaths_r (cache, validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -85;
	  goto exit;
      }

    geom = gaiaSharedPaths_r (cache, 0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -86;
	  goto exit;
      }

    geom = gaiaLineInterpolatePoint_r (cache, 0, 0.6);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -87;
	  goto exit;
      }

    /* Test some strange conditions */
    result = gaiaGeomCollLength_r (cache, validGeometry, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -88;
	  goto exit;
      }

    /* preparing a collection of Polygons */
    geom = gaiaAllocGeomColl ();
    pg = gaiaAddPolygonToGeomColl (geom, 5, 1);
    rng = pg->Exterior;
    gaiaSetPoint (rng->Coords, 0, 1.5, 1.5);
    gaiaSetPoint (rng->Coords, 1, 10.5, 1.5);
    gaiaSetPoint (rng->Coords, 2, 10.5, 10.5);
    gaiaSetPoint (rng->Coords, 3, 1.5, 10.5);
    gaiaSetPoint (rng->Coords, 4, 1.5, 1.5);
    rng = gaiaAddInteriorRing (pg, 0, 5);
    gaiaSetPoint (rng->Coords, 0, 5.5, 5.5);
    gaiaSetPoint (rng->Coords, 1, 6.5, 1.5);
    gaiaSetPoint (rng->Coords, 2, 6.5, 6.5);
    gaiaSetPoint (rng->Coords, 3, 5.5, 6.5);
    gaiaSetPoint (rng->Coords, 4, 5.5, 5.5);
    pg = gaiaAddPolygonToGeomColl (geom, 5, 1);
    rng = pg->Exterior;
    gaiaSetPoint (rng->Coords, 0, 8.5, 8.5);
    gaiaSetPoint (rng->Coords, 1, 18.5, 8.5);
    gaiaSetPoint (rng->Coords, 2, 18.5, 18.5);
    gaiaSetPoint (rng->Coords, 3, 8.5, 18.5);
    gaiaSetPoint (rng->Coords, 4, 8.5, 8.5);
    rng = gaiaAddInteriorRing (pg, 0, 5);
    gaiaSetPoint (rng->Coords, 0, 11.5, 11.5);
    gaiaSetPoint (rng->Coords, 1, 12.5, 11.5);
    gaiaSetPoint (rng->Coords, 2, 12.5, 12.5);
    gaiaSetPoint (rng->Coords, 3, 11.5, 12.5);
    gaiaSetPoint (rng->Coords, 4, 11.5, 11.5);
    pg = gaiaAddPolygonToGeomColl (geom, 5, 0);
    rng = pg->Exterior;
    gaiaSetPoint (rng->Coords, 0, 4.5, 4.5);
    gaiaSetPoint (rng->Coords, 1, 30.5, 4.5);
    gaiaSetPoint (rng->Coords, 2, 30.5, 30.5);
    gaiaSetPoint (rng->Coords, 3, 4.5, 30.5);
    gaiaSetPoint (rng->Coords, 4, 4.5, 4.5);

    /* Tests Polygons UnaryUnion [as in aggregate ST_Union] */
    g = gaiaUnaryUnion_r (cache, geom);
    if (g == NULL)
      {
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -89;
	  goto exit;
      }
    result = gaiaGeomCollLength_r (cache, g, &dummyResultArg);
    if (result == 1
	&& (dummyResultArg >= 122.84232 && dummyResultArg <= 122.84233))
	;
    else
      {
	  fprintf (stderr, "bad result at %s:%i: %i %f\n", __FILE__, __LINE__,
		   result, dummyResultArg);
	  returnValue = -90;
	  goto exit;
      }
    gaiaFreeGeomColl (g);

    /* Tests Polygons UnionCascaded [as in aggregate ST_Union] */
    g = gaiaUnionCascaded_r (cache, geom);
    if (g == NULL)
      {
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -91;
	  goto exit;
      }
    result = gaiaGeomCollLength_r (cache, g, &dummyResultArg);
    if (result == 1
	&& (dummyResultArg >= 122.84232 && dummyResultArg <= 122.84233))
	;
    else
      {
	  fprintf (stderr, "bad result at %s:%i: %i %f\n", __FILE__, __LINE__,
		   result, dummyResultArg);
	  returnValue = -92;
	  goto exit;
      }
    gaiaFreeGeomColl (g);
    gaiaFreeGeomColl (geom);

    /* Cleanup and exit */
  exit:
    gaiaFreeGeomColl (validGeometry);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -133;
      }

    spatialite_cleanup_ex (cache);
    return returnValue;

#endif /* end GEOS conditional */
    return 0;
}

int
test_legacy_mode ()
{
#ifndef OMIT_GEOS		/* only if GEOS is supported */
    int ret;
    sqlite3 *handle;
    int result;
    double resultDouble;
    int returnValue = 0;
    gaiaGeomCollPtr geom;
    gaiaGeomCollPtr g;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;

    /* Common setup */
    gaiaGeomCollPtr validGeometry = gaiaAllocGeomColl ();
    double dummyResultArg = 0.0;
    double dummyResultArg2 = 0.0;

    spatialite_init (0);

    /* Tests start here */
    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }

    /* null inputs for a range of geometry functions */
    result = gaiaGeomCollEquals (0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -1;
	  goto exit;
      }

    result = gaiaGeomCollEquals (validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -2;
	  goto exit;
      }

    result = gaiaGeomCollEquals (0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -3;
	  goto exit;
      }

    result = gaiaGeomCollIntersects (0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -4;
	  goto exit;
      }

    result = gaiaGeomCollIntersects (validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -5;
	  goto exit;
      }

    result = gaiaGeomCollIntersects (0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -6;
	  goto exit;
      }

    result = gaiaGeomCollOverlaps (0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -7;
	  goto exit;
      }

    result = gaiaGeomCollOverlaps (validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -8;
	  goto exit;
      }

    result = gaiaGeomCollOverlaps (0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -9;
	  goto exit;
      }

    result = gaiaGeomCollCrosses (0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -10;
	  goto exit;
      }

    result = gaiaGeomCollCrosses (validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -11;
	  goto exit;
      }

    result = gaiaGeomCollCrosses (0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -12;
	  goto exit;
      }

    result = gaiaGeomCollTouches (0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -13;
	  goto exit;
      }

    result = gaiaGeomCollTouches (validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -14;
	  goto exit;
      }

    result = gaiaGeomCollTouches (0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -15;
	  goto exit;
      }

    result = gaiaGeomCollDisjoint (0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -16;
	  goto exit;
      }

    result = gaiaGeomCollDisjoint (validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -17;
	  goto exit;
      }

    result = gaiaGeomCollDisjoint (0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -18;
	  goto exit;
      }

    result = gaiaGeomCollWithin (0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -19;
	  goto exit;
      }

    result = gaiaGeomCollWithin (validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -20;
	  goto exit;
      }

    result = gaiaGeomCollWithin (0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -21;
	  goto exit;
      }

    result = gaiaGeomCollContains (0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -22;
	  goto exit;
      }

    result = gaiaGeomCollContains (validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -23;
	  goto exit;
      }

    result = gaiaGeomCollContains (0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -24;
	  goto exit;
      }

    result = gaiaGeomCollRelate (0, validGeometry, "T********");
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -25;
	  goto exit;
      }

    result = gaiaGeomCollRelate (validGeometry, 0, "T********");
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -26;
	  goto exit;
      }

    result = gaiaGeomCollRelate (0, 0, "T********");
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -27;
	  goto exit;
      }

    result = gaiaHausdorffDistance (0, validGeometry, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -34;
	  goto exit;
      }

    result = gaiaHausdorffDistance (validGeometry, 0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -35;
	  goto exit;
      }

    result = gaiaHausdorffDistance (0, 0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -36;
	  goto exit;
      }

    result = gaiaGeomCollCovers (0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -37;
	  goto exit;
      }

    result = gaiaGeomCollCovers (validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -38;
	  goto exit;
      }

    result = gaiaGeomCollCovers (0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -39;
	  goto exit;
      }

    result = gaiaGeomCollCoveredBy (0, validGeometry);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -40;
	  goto exit;
      }

    result = gaiaGeomCollCoveredBy (validGeometry, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -41;
	  goto exit;
      }

    result = gaiaGeomCollCoveredBy (0, 0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -42;
	  goto exit;
      }

    result = gaiaGeomCollDistance (0, validGeometry, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -43;
	  goto exit;
      }

    result = gaiaGeomCollDistance (validGeometry, 0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -44;
	  goto exit;
      }

    result = gaiaGeomCollDistance (0, 0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -45;
	  goto exit;
      }
    /* Check some of the single geometry analysis routines */

    result = gaiaGeomCollLength (0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -46;
	  goto exit;
      }

    result = gaiaGeomCollArea (0, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -47;
	  goto exit;
      }

    result = gaiaGeomCollCentroid (0, &dummyResultArg, &dummyResultArg2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -48;
	  goto exit;
      }

    result = gaiaGetPointOnSurface (0, &dummyResultArg, &dummyResultArg2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -49;
	  goto exit;
      }

    result = gaiaIsSimple (0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -32;
	  goto exit;
      }

    result = gaiaIsValid (0);
    if (result != -1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -33;
	  goto exit;
      }

    resultDouble = gaiaLineLocatePoint (0, validGeometry);
    if (abs (resultDouble - -1.0) > double_eps)
      {
	  fprintf (stderr, "bad result at %s:%i: %f\n", __FILE__, __LINE__,
		   resultDouble);
	  returnValue = -62;
	  goto exit;
      }

    resultDouble = gaiaLineLocatePoint (validGeometry, 0);
    if (abs (resultDouble - -1.0) > double_eps)
      {
	  fprintf (stderr, "bad result at %s:%i: %f\n", __FILE__, __LINE__,
		   resultDouble);
	  returnValue = -63;
	  goto exit;
      }

    resultDouble = gaiaLineLocatePoint (0, 0);
    if (abs (resultDouble - -1.0) > double_eps)
      {
	  fprintf (stderr, "bad result at %s:%i: %f\n", __FILE__, __LINE__,
		   resultDouble);
	  returnValue = -64;
	  goto exit;
      }

    /* geometry generating functionality */
    geom = NULL;
    geom = gaiaLinesCutAtNodes (0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -50;
	  goto exit;
      }

    geom = gaiaLinesCutAtNodes (validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -51;
	  goto exit;
      }

    geom = gaiaLinesCutAtNodes (0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -52;
	  goto exit;
      }

    geom = gaiaUnaryUnion (0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -53;
	  goto exit;
      }

    geom = gaiaLineMerge (0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -54;
	  goto exit;
      }

    geom = gaiaSnap (0, validGeometry, 4);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -55;
	  goto exit;
      }

    geom = gaiaSnap (validGeometry, 0, 4);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -56;
	  goto exit;
      }

    geom = gaiaSnap (0, 0, 4);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -57;
	  goto exit;
      }

    geom = gaiaShortestLine (0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -58;
	  goto exit;
      }

    geom = gaiaShortestLine (validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -59;
	  goto exit;
      }

    geom = gaiaShortestLine (0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -60;
	  goto exit;
      }

    geom = gaiaLineSubstring (0, 0, 1);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -61;
	  goto exit;
      }

    geom = gaiaGeometryIntersection (0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -65;
	  goto exit;
      }

    geom = gaiaGeometryIntersection (validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -66;
	  goto exit;
      }

    geom = gaiaGeometryIntersection (0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -67;
	  goto exit;
      }

    geom = gaiaGeometryUnion (0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -68;
	  goto exit;
      }

    geom = gaiaGeometryUnion (validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -69;
	  goto exit;
      }

    geom = gaiaGeometryUnion (0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -70;
	  goto exit;
      }

    geom = gaiaGeometryDifference (0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -71;
	  goto exit;
      }

    geom = gaiaGeometryDifference (validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -72;
	  goto exit;
      }

    geom = gaiaGeometryDifference (0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -73;
	  goto exit;
      }

    geom = gaiaGeometrySymDifference (0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -74;
	  goto exit;
      }

    geom = gaiaGeometrySymDifference (validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -75;
	  goto exit;
      }

    geom = gaiaGeometrySymDifference (0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -76;
	  goto exit;
      }

    geom = gaiaBoundary (0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -77;
	  goto exit;
      }

    geom = gaiaGeomCollSimplify (0, 1.0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -78;
	  goto exit;
      }

    geom = gaiaGeomCollSimplifyPreserveTopology (0, 1.0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -79;
	  goto exit;
      }

    geom = gaiaConvexHull (0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -80;
	  goto exit;
      }

    geom = gaiaGeomCollBuffer (0, 0.1, 10);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -81;
	  goto exit;
      }

    geom = gaiaOffsetCurve (0, 1.5, 10, 1);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -82;
	  goto exit;
      }

    geom = gaiaSingleSidedBuffer (0, 1.5, 10, 1);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -83;
	  goto exit;
      }

    geom = gaiaSharedPaths (0, validGeometry);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -84;
	  goto exit;
      }

    geom = gaiaSharedPaths (validGeometry, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -85;
	  goto exit;
      }

    geom = gaiaSharedPaths (0, 0);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -86;
	  goto exit;
      }

    geom = gaiaLineInterpolatePoint (0, 0.6);
    if (geom != NULL)
      {
	  gaiaFree (geom);
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -87;
	  goto exit;
      }

    /* Test some strange conditions */
    result = gaiaGeomCollLength (validGeometry, &dummyResultArg);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -88;
	  goto exit;
      }

    /* preparing a collection of Polygons */
    geom = gaiaAllocGeomColl ();
    pg = gaiaAddPolygonToGeomColl (geom, 5, 1);
    rng = pg->Exterior;
    gaiaSetPoint (rng->Coords, 0, 1.5, 1.5);
    gaiaSetPoint (rng->Coords, 1, 10.5, 1.5);
    gaiaSetPoint (rng->Coords, 2, 10.5, 10.5);
    gaiaSetPoint (rng->Coords, 3, 1.5, 10.5);
    gaiaSetPoint (rng->Coords, 4, 1.5, 1.5);
    rng = gaiaAddInteriorRing (pg, 0, 5);
    gaiaSetPoint (rng->Coords, 0, 5.5, 5.5);
    gaiaSetPoint (rng->Coords, 1, 6.5, 1.5);
    gaiaSetPoint (rng->Coords, 2, 6.5, 6.5);
    gaiaSetPoint (rng->Coords, 3, 5.5, 6.5);
    gaiaSetPoint (rng->Coords, 4, 5.5, 5.5);
    pg = gaiaAddPolygonToGeomColl (geom, 5, 1);
    rng = pg->Exterior;
    gaiaSetPoint (rng->Coords, 0, 8.5, 8.5);
    gaiaSetPoint (rng->Coords, 1, 18.5, 8.5);
    gaiaSetPoint (rng->Coords, 2, 18.5, 18.5);
    gaiaSetPoint (rng->Coords, 3, 8.5, 18.5);
    gaiaSetPoint (rng->Coords, 4, 8.5, 8.5);
    rng = gaiaAddInteriorRing (pg, 0, 5);
    gaiaSetPoint (rng->Coords, 0, 11.5, 11.5);
    gaiaSetPoint (rng->Coords, 1, 12.5, 11.5);
    gaiaSetPoint (rng->Coords, 2, 12.5, 12.5);
    gaiaSetPoint (rng->Coords, 3, 11.5, 12.5);
    gaiaSetPoint (rng->Coords, 4, 11.5, 11.5);
    pg = gaiaAddPolygonToGeomColl (geom, 5, 0);
    rng = pg->Exterior;
    gaiaSetPoint (rng->Coords, 0, 4.5, 4.5);
    gaiaSetPoint (rng->Coords, 1, 30.5, 4.5);
    gaiaSetPoint (rng->Coords, 2, 30.5, 30.5);
    gaiaSetPoint (rng->Coords, 3, 4.5, 30.5);
    gaiaSetPoint (rng->Coords, 4, 4.5, 4.5);

    /* Tests Polygons UnaryUnion [as in aggregate ST_Union] */
    g = gaiaUnaryUnion (geom);
    if (g == NULL)
      {
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -89;
	  goto exit;
      }
    result = gaiaGeomCollLength (g, &dummyResultArg);
    if (result == 1
	&& (dummyResultArg >= 122.84232 && dummyResultArg <= 122.84233))
	;
    else
      {
	  fprintf (stderr, "bad result at %s:%i: %i %f\n", __FILE__, __LINE__,
		   result, dummyResultArg);
	  returnValue = -90;
	  goto exit;
      }
    gaiaFreeGeomColl (g);

    /* Tests Polygons UnionCascaded [as in aggregate ST_Union] */
    g = gaiaUnionCascaded (geom);
    if (g == NULL)
      {
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -91;
	  goto exit;
      }
    result = gaiaGeomCollLength (g, &dummyResultArg);
    if (result == 1
	&& (dummyResultArg >= 122.84232 && dummyResultArg <= 122.84233))
	;
    else
      {
	  fprintf (stderr, "bad result at %s:%i: %i %f\n", __FILE__, __LINE__,
		   result, dummyResultArg);
	  returnValue = -92;
	  goto exit;
      }
    gaiaFreeGeomColl (g);
    gaiaFreeGeomColl (geom);

    /* Cleanup and exit */
  exit:
    gaiaFreeGeomColl (validGeometry);

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -133;
      }

    spatialite_cleanup ();
    return returnValue;

#endif /* end GEOS conditional */
    return 0;
}

int
main (int argc, char *argv[])
{
    int ret;
    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */
    ret = test_current_mode ();
    if (ret != 0)
	return ret;
    ret = test_extra_mode ();
    if (ret != 0)
	return ret;
    ret = test_legacy_mode ();
    if (ret != 0)
	return ret;
    return 0;
}
