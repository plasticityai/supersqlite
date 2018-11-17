/*

 gg_geodesic.c -- Gaia functions for geodesic calculations
  
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>

#include <spatialite/gaiageo.h>

#define DEG2RAD	0.0174532925199432958
#define PI	3.14159265358979323846

struct ellipses
{
    char *name;
    double a;			/* equatorial radius - meters */
    double rf;			/* reverse flattening */
    double b;			/* polar radius - meters */
};

#if defined(_WIN32) && !defined(__MINGW32__)
#define isnan	_isnan
#endif /* not WIN32-MSVC */

GAIAGEO_DECLARE int
gaiaEllipseParams (const char *name, double *a, double *b, double *rf)
{
/* trying to find ellipse params */
    struct ellipses ellps_list[] = {
	{"MERIT", 6378137.0, 298.257, -1.0},
	{"SGS85", 6378136.0, 298.257, -1.0},
	{"GRS80", 6378137.0, 298.257222101, -1.0},
	{"IAU76", 6378140.0, 298.257, -1.0},
	{"airy", 6377563.396, -1.0, 6356256.910},
	{"APL4.9", 6378137.0, 298.25, -1.0},
	{"NWL9D", 6378145.0, 298.25, -1.0},
	{"mod_airy", 6377340.189, -1.0, 6356034.446},
	{"andrae", 6377104.43, 300.0, -1.0},
	{"aust_SA", 378160.0, 298.25, -1.0},
	{"GRS67", 6378160.0, 298.2471674270, -1.0},
	{"bessel", 6377397.155, 299.1528128, -1.0},
	{"bess_nam", 6377483.865, 299.1528128, -1.0},
	{"clrk66", 6378206.4, -1.0, 6356583.8},
	{"clrk80", 6378249.145, 293.4663, -1.0},
	{"CPM", 6375738.7, 334.29, -1.0},
	{"delmbr", 6376428.0, 311.5, -1.0},
	{"engelis", 6378136.05, 298.2566, -1.0},
	{"evrst30", 6377276.345, 300.8017, -1.0},
	{"evrst48", 6377304.063, 300.8017, -1.0},
	{"evrst56", 6377301.243, 300.8017, -1.0},
	{"evrst69", 6377295.664, 300.8017, -1.0},
	{"evrstSS", 6377298.556, 300.8017, -1.0},
	{"fschr60", 6378166.0, 298.3, -1.0},
	{"fschr60m", 6378155.0, 298.3, -1.0},
	{"fschr68", 6378150.0, 298.3, -1.0},
	{"helmert", 6378200.0, 298.3, -1.0},
	{"hough", 6378270.0, 297.0, -1.0},
	{"intl", 6378388.0, 297.0, -1.0},
	{"krass", 6378245.0, 298.3, -1.0},
	{"kaula", 6378163.0, 298.24, -1.0},
	{"lerch", 6378139.0, 298.257, -1.0},
	{"mprts", 6397300.0, 191.0, -1.0},
	{"new_intl", 6378157.5, -1.0, 6356772.2},
	{"plessis", 6376523.0, -1.0, 6355863.0},
	{"SEasia", 6378155.0, -1.0, 6356773.3205},
	{"walbeck", 6376896.0, -1.0, 6355834.8467},
	{"WGS60", 6378165.0, 298.3, -1.0},
	{"WGS66", 6378145.0, 298.25, -1.0},
	{"WGS72", 6378135.0, 298.26, -1.0},
	{"WGS84", 6378137.0, 298.257223563, -1.0},
	{"sphere", 6370997.0, -1.0, 6370997.0},
	{NULL, -1.0, -1.0, -1.0}
    };
    struct ellipses *pe = ellps_list;
    while (1)
      {
	  if (pe->name == NULL)
	      break;
	  if (strcmp (pe->name, name) == 0)
	    {
		*a = pe->a;
		if (pe->rf < 0.0)
		  {
		      *b = pe->b;
		      *rf = 1.0 / ((pe->a - pe->b) / pe->a);
		  }
		else
		  {
		      *b = (pe->a * (1.0 - (1.0 / pe->rf)));
		      *rf = pe->rf;
		  }
		return 1;
	    }
	  pe++;
      }
    return 0;
}

GAIAGEO_DECLARE double
gaiaGreatCircleDistance (double a, double b, double lat1, double lon1,
			 double lat2, double lon2)
{
/*
/ Calculate great-circle distance (in m) between two points specified by 
/ latitude/longitude (in decimal degrees) using Aviation Formulary
/
/ http://williams.best.vwh.net/avform.htm#Dist
/
*/
    double latrad1 = lat1 * DEG2RAD;
    double lonrad1 = lon1 * DEG2RAD;
    double latrad2 = lat2 * DEG2RAD;
    double lonrad2 = lon2 * DEG2RAD;
    double avg_radius;
    double k1 = (sin ((latrad1 - latrad2) / 2.0));
    double k2 = (sin ((lonrad1 - lonrad2) / 2.0));
    double dist;
    dist =
	2.0 * asin (sqrt (k1 * k1 + cos (latrad1) * cos (latrad2) * k2 * k2));
    if (dist < 0.0)
	dist = dist + PI;
    if (a == b)
	avg_radius = a;
    else
	avg_radius = (2.0 * a + b) / 3.0;
    dist = dist * avg_radius;
    return dist;
}

GAIAGEO_DECLARE double
gaiaGeodesicDistance (double a, double b, double rf, double lat1, double lon1,
		      double lat2, double lon2)
{
/*
/ Calculate geodesic distance (in m) 
/ between two points specified by latitude/longitude 
/ (in decimal degrees) using Vincenty inverse formula for ellipsoids
/
/ based on original JavaScript by (c) Chris Veness 2002-2008 
/ http://www.movable-type.co.uk/scripts/latlong-vincenty.html
/
*/
    double f = 1.0 / rf;
    double L = (lon2 - lon1) * DEG2RAD;
    double U1 = atan ((1.0 - f) * tan (lat1 * DEG2RAD));
    double U2 = atan ((1.0 - f) * tan (lat2 * DEG2RAD));
    double sinU1 = sin (U1);
    double cosU1 = cos (U1);
    double sinU2 = sin (U2);
    double cosU2 = cos (U2);
    double lambda = L;
    double lambdaP;
    double sinLambda;
    double cosLambda;
    double sinSigma;
    double cosSigma;
    double sigma;
    double sinAlpha;
    double cosSqAlpha;
    double cos2SigmaM;
    double C;
    double uSq;
    double A;
    double B;
    double deltaSigma;
    double s;
    int iterLimit = 100;
    do
      {
	  sinLambda = sin (lambda);
	  cosLambda = cos (lambda);
	  sinSigma =
	      sqrt ((cosU2 * sinLambda) * (cosU2 * sinLambda) +
		    (cosU1 * sinU2 -
		     sinU1 * cosU2 * cosLambda) * (cosU1 * sinU2 -
						   sinU1 * cosU2 * cosLambda));
	  if (sinSigma == 0.0)
	      return 0.0;	/* co-incident points */
	  cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
	  sigma = atan2 (sinSigma, cosSigma);
	  sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
	  cosSqAlpha = 1.0 - sinAlpha * sinAlpha;
	  cos2SigmaM = cosSigma - 2.0 * sinU1 * sinU2 / cosSqAlpha;
	  if (isnan (cos2SigmaM))
	      cos2SigmaM = 0;	/* equatorial line */
	  C = f / 16.0 * cosSqAlpha * (4.0 + f * (4.0 - 3.0 * cosSqAlpha));
	  lambdaP = lambda;
	  lambda =
	      L + (1.0 - C) * f * sinAlpha * (sigma +
					      C * sinSigma * (cos2SigmaM +
							      C * cosSigma *
							      (-1.0 +
							       2.0 *
							       cos2SigmaM *
							       cos2SigmaM)));
      }
    while (fabs (lambda - lambdaP) > 1e-12 && --iterLimit > 0);
    if (iterLimit == 0)
	return -1.0;		/* formula failed to converge */
    uSq = cosSqAlpha * (a * a - b * b) / (b * b);
    A = 1.0 + uSq / 16384.0 * (4096.0 +
			       uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq)));
    B = uSq / 1024.0 * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));
    deltaSigma =
	B * sinSigma * (cos2SigmaM +
			B / 4.0 * (cosSigma *
				   (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM) -
				   B / 6.0 * cos2SigmaM * (-3.0 +
							   4.0 * sinSigma *
							   sinSigma) * (-3.0 +
									4.0 *
									cos2SigmaM
									*
									cos2SigmaM)));
    s = b * A * (sigma - deltaSigma);
    return s;
}

GAIAGEO_DECLARE void
gaiaFree (void *ptr)
{
/* freeing a generic memory allocation */
    if (!ptr)
	return;
    free (ptr);
}

GAIAGEO_DECLARE double
gaiaGreatCircleTotalLength (double a, double b, int dims, double *coords,
			    int vert)
{
/* computing the GreatCircle total length for some Linestring/Ring */
    int iv;
    double x1 = 0.0;
    double y1 = 0.0;
    double x2;
    double y2;
    double z;
    double m;
    double len = 0.0;
    for (iv = 0; iv < vert; iv++)
      {
	  if (dims == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (coords, iv, &x2, &y2, &z);
	    }
	  else if (dims == GAIA_XY_M)
	    {
		gaiaGetPointXYM (coords, iv, &x2, &y2, &m);
	    }
	  else if (dims == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (coords, iv, &x2, &y2, &z, &m);
	    }
	  else
	    {
		gaiaGetPoint (coords, iv, &x2, &y2);
	    }
	  if (iv > 0)
	      len += gaiaGreatCircleDistance (a, b, y1, x1, y2, x2);
	  x1 = x2;
	  y1 = y2;
      }
    return len;
}

GAIAGEO_DECLARE double
gaiaGeodesicTotalLength (double a, double b, double rf, int dims,
			 double *coords, int vert)
{
/* computing the Geodesic total length for some Linestring/Ring */
    int iv;
    double x1 = 0.0;
    double y1 = 0.0;
    double x2;
    double y2;
    double z;
    double m;
    double l;
    double len = 0.0;
    for (iv = 0; iv < vert; iv++)
      {
	  if (dims == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (coords, iv, &x2, &y2, &z);
	    }
	  else if (dims == GAIA_XY_M)
	    {
		gaiaGetPointXYM (coords, iv, &x2, &y2, &m);
	    }
	  else if (dims == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (coords, iv, &x2, &y2, &z, &m);
	    }
	  else
	    {
		gaiaGetPoint (coords, iv, &x2, &y2);
	    }
	  if (iv > 0)
	    {
		l = gaiaGeodesicDistance (a, b, rf, y1, x1, y2, x2);
		if (l < 0.0)
		    return -1.0;
		len += l;
	    }
	  x1 = x2;
	  y1 = y2;
      }
    return len;
}

GAIAGEO_DECLARE int
gaiaConvertLength (double value, int unit_from, int unit_to, double *cvt)
{
/* converting length from one unit to another */
    double m;
    double factors[] = {
	1000.0, 1.0, 0.1, 0.01, 0.001, 1852.0, 0.0254, 0.3048, 0.9144,
	1609.344, 1.8288, 20.1168, 0.201168, 1.0, 0.304800609601219,
	0.914401828803658, 20.11684023368047, 1609.347218694437, 0.91439523,
	0.30479841, 20.11669506
    };
    factors[GAIA_US_IN] /= 39.37;
    if (unit_from < GAIA_MIN_UNIT || unit_from > GAIA_MAX_UNIT)
	return 0;
    if (unit_to < GAIA_MIN_UNIT || unit_to > GAIA_MAX_UNIT)
	return 0;
    if (unit_from == unit_to)
      {
	  /* same unit */
	  *cvt = value;
      }
    else if (unit_from == GAIA_M)
      {
	  /* from Meters to .. */
	  *cvt = value / factors[unit_to];
      }
    else if (unit_to == GAIA_M)
      {
	  /* from .. to Meters */
	  *cvt = value * factors[unit_from];
      }
    else
      {
	  m = value * factors[unit_from];
	  *cvt = m / factors[unit_to];
      }
    return 1;
}

#undef DEG2RAD
#undef PI
