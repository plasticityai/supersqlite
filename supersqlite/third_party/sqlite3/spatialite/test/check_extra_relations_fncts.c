/*

 check_extra_relations_fncts.c -- SpatiaLite Test Case
 
 This tests a couple of functions in gg_relations.c that aren't used in
 spatialite, but are provided for backwards compatibility.

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

#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gaiageo.h"

static const double double_eps = 0.00000001;

int
main (int argc, char *argv[])
{
    int result;
    int returnValue = 0;
    gaiaRingPtr interior;

    /* Common setup */
    gaiaLinestringPtr linestr1 = gaiaAllocLinestring (0);
    gaiaLinestringPtr linestr2 = gaiaAllocLinestring (0);
    gaiaPolygonPtr poly1 = gaiaAllocPolygon (0, 0);
    gaiaPolygonPtr poly2 = gaiaAllocPolygon (0, 0);

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    /* Tests start here */

    /* zero length linestring */
    result = gaiaLinestringEquals (linestr1, linestr2);
    if (result != 1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -1;
	  goto exit;
      }

    /* non-matching linestring lengths */
    gaiaFreeLinestring (linestr1);
    gaiaFreeLinestring (linestr2);
    linestr1 = gaiaAllocLinestring (2);
    linestr2 = gaiaAllocLinestring (3);
    gaiaSetPoint (linestr1->Coords, 0, 1, 3);	/* line1, first point */
    gaiaSetPoint (linestr1->Coords, 1, 2, 4);	/* line1, second point */
    gaiaSetPoint (linestr2->Coords, 0, 4, -2);	/* line2, first point */
    gaiaSetPoint (linestr2->Coords, 1, 1, 5);	/* line2, second point */
    gaiaSetPoint (linestr2->Coords, 2, 3, 4);	/* line2, third point */
    result = gaiaLinestringEquals (linestr1, linestr2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -2;
	  goto exit;
      }

    /* identical lines */
    gaiaFreeLinestring (linestr2);
    linestr2 = gaiaCloneLinestring (linestr1);
    result = gaiaLinestringEquals (linestr1, linestr2);
    if (result != 1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -3;
	  goto exit;
      }

    /* not quite identical lines */
    gaiaSetPoint (linestr2->Coords, 1, 2, -4);
    result = gaiaLinestringEquals (linestr1, linestr2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -4;
	  goto exit;
      }

    /* zero length polygon */
    result = gaiaPolygonEquals (poly1, poly2);
    if (result != 1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -5;
	  goto exit;
      }

    /* matching polygons */
    gaiaFreePolygon (poly1);
    gaiaFreePolygon (poly2);
    poly1 = gaiaAllocPolygon (5, 0);
    gaiaSetPoint (poly1->Exterior->Coords, 0, 0, 0);
    gaiaSetPoint (poly1->Exterior->Coords, 1, 10, 0);
    gaiaSetPoint (poly1->Exterior->Coords, 2, 10, 10);
    gaiaSetPoint (poly1->Exterior->Coords, 3, 0, 10);
    gaiaSetPoint (poly1->Exterior->Coords, 4, 0, 0);
    poly2 = gaiaClonePolygon (poly1);
    result = gaiaPolygonEquals (poly1, poly2);
    if (result != 1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -6;
	  goto exit;
      }

    /* not quite matching polygons */
    gaiaSetPoint (poly2->Exterior->Coords, 2, 10, -10);
    result = gaiaPolygonEquals (poly1, poly2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -7;
	  goto exit;
      }

    /* polygons with different numbers of interiors */
    gaiaFreePolygon (poly2);
    poly2 = gaiaAllocPolygon (5, 1);
    gaiaSetPoint (poly2->Exterior->Coords, 0, 0, 0);
    gaiaSetPoint (poly2->Exterior->Coords, 1, 10, 0);
    gaiaSetPoint (poly2->Exterior->Coords, 2, 10, 10);
    gaiaSetPoint (poly2->Exterior->Coords, 3, 0, 10);
    gaiaSetPoint (poly2->Exterior->Coords, 4, 0, 0);
    interior = gaiaAddInteriorRing (poly2, 0, 4);
    gaiaSetPoint (interior->Coords, 0, 1, 1);
    gaiaSetPoint (interior->Coords, 1, 3, 2);
    gaiaSetPoint (interior->Coords, 2, 3, 1);
    gaiaSetPoint (interior->Coords, 3, 1, 1);
    result = gaiaPolygonEquals (poly1, poly2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -8;
	  goto exit;
      }

    /* same exteriors and interiors */
    gaiaFreePolygon (poly1);
    poly1 = gaiaClonePolygon (poly2);
    result = gaiaPolygonEquals (poly1, poly2);
    if (result != 1)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -9;
	  goto exit;
      }

    /* slightly different interiors */
    gaiaSetPoint (interior->Coords, 2, 3, 3);
    result = gaiaPolygonEquals (poly1, poly2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -10;
	  goto exit;
      }

    /* different number of exterior points */
    gaiaFreePolygon (poly2);
    poly2 = gaiaAllocPolygon (4, 1);
    gaiaSetPoint (poly2->Exterior->Coords, 0, 0, 0);
    gaiaSetPoint (poly2->Exterior->Coords, 1, 10, 0);
    gaiaSetPoint (poly2->Exterior->Coords, 2, 10, 10);
    gaiaSetPoint (poly2->Exterior->Coords, 3, 0, 0);
    interior = gaiaAddInteriorRing (poly2, 0, 4);
    gaiaSetPoint (interior->Coords, 0, 1, 1);
    gaiaSetPoint (interior->Coords, 1, 3, 2);
    gaiaSetPoint (interior->Coords, 2, 3, 1);
    gaiaSetPoint (interior->Coords, 3, 1, 1);
    result = gaiaPolygonEquals (poly1, poly2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -11;
	  goto exit;
      }

    /* same exterior points, but different number of points on first interior */
    gaiaFreePolygon (poly2);
    poly2 = gaiaAllocPolygon (5, 1);
    gaiaSetPoint (poly2->Exterior->Coords, 0, 0, 0);
    gaiaSetPoint (poly2->Exterior->Coords, 1, 10, 0);
    gaiaSetPoint (poly2->Exterior->Coords, 2, 10, 10);
    gaiaSetPoint (poly2->Exterior->Coords, 3, 0, 10);
    gaiaSetPoint (poly2->Exterior->Coords, 4, 0, 0);
    interior = gaiaAddInteriorRing (poly2, 0, 5);
    gaiaSetPoint (interior->Coords, 0, 1, 1);
    gaiaSetPoint (interior->Coords, 1, 3, 2);
    gaiaSetPoint (interior->Coords, 2, 3, 3);
    gaiaSetPoint (interior->Coords, 3, 1, 3);
    gaiaSetPoint (interior->Coords, 4, 1, 1);
    result = gaiaPolygonEquals (poly1, poly2);
    if (result != 0)
      {
	  fprintf (stderr, "bad result at %s:%i: %i\n", __FILE__, __LINE__,
		   result);
	  returnValue = -12;
	  goto exit;
      }

    /* Cleanup and exit */
  exit:
    gaiaFreeLinestring (linestr1);
    gaiaFreeLinestring (linestr2);
    gaiaFreePolygon (poly1);
    gaiaFreePolygon (poly2);

    spatialite_shutdown ();
    return returnValue;
}
