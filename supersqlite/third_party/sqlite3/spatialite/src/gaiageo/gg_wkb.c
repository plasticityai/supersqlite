/*

 gg_wkb.c -- Gaia common support for WKB encoded geometries
  
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

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>

#include <spatialite/gaiageo.h>
#include <spatialite/geopackage.h>

static void
ParseWkbPoint (gaiaGeomCollPtr geo)
{
/* decodes a POINT from WKB */
    double x;
    double y;
    if (geo->size < geo->offset + 16)
	return;
    x = gaiaImport64 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
		      geo->endian_arch);
    geo->offset += 16;
    gaiaAddPointToGeomColl (geo, x, y);
}

static void
ParseWkbPointZ (gaiaGeomCollPtr geo)
{
/* decodes a POINTZ from WKB */
    double x;
    double y;
    double z;
    if (geo->size < geo->offset + 24)
	return;
    x = gaiaImport64 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
		      geo->endian_arch);
    z = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
		      geo->endian_arch);
    geo->offset += 24;
    gaiaAddPointToGeomCollXYZ (geo, x, y, z);
}

static void
ParseWkbPointM (gaiaGeomCollPtr geo)
{
/* decodes a POINTM from WKB */
    double x;
    double y;
    double m;
    if (geo->size < geo->offset + 24)
	return;
    x = gaiaImport64 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
		      geo->endian_arch);
    m = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
		      geo->endian_arch);
    geo->offset += 24;
    gaiaAddPointToGeomCollXYM (geo, x, y, m);
}

static void
ParseWkbPointZM (gaiaGeomCollPtr geo)
{
/* decodes a POINTZM from WKB */
    double x;
    double y;
    double z;
    double m;
    if (geo->size < geo->offset + 32)
	return;
    x = gaiaImport64 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
		      geo->endian_arch);
    z = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
		      geo->endian_arch);
    m = gaiaImport64 (geo->blob + (geo->offset + 24), geo->endian,
		      geo->endian_arch);
    geo->offset += 32;
    gaiaAddPointToGeomCollXYZM (geo, x, y, z, m);
}

static void
ParseWkbLine (gaiaGeomCollPtr geo)
{
/* decodes a LINESTRING from WKB */
    int points;
    int iv;
    double x;
    double y;
    gaiaLinestringPtr line;
    if (geo->size < geo->offset + 4)
	return;
    points =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    if (geo->size < geo->offset + (16 * points))
	return;
    line = gaiaAddLinestringToGeomColl (geo, points);
    for (iv = 0; iv < points; iv++)
      {
	  x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
			    geo->endian_arch);
	  gaiaSetPoint (line->Coords, iv, x, y);
	  geo->offset += 16;
      }
}

static void
ParseWkbLineZ (gaiaGeomCollPtr geo)
{
/* decodes a LINESTRINGZ from WKB */
    int points;
    int iv;
    double x;
    double y;
    double z;
    gaiaLinestringPtr line;
    if (geo->size < geo->offset + 4)
	return;
    points =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    if (geo->size < geo->offset + (24 * points))
	return;
    line = gaiaAddLinestringToGeomColl (geo, points);
    for (iv = 0; iv < points; iv++)
      {
	  x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
			    geo->endian_arch);
	  z = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
			    geo->endian_arch);
	  gaiaSetPointXYZ (line->Coords, iv, x, y, z);
	  geo->offset += 24;
      }
}

static void
ParseWkbLineM (gaiaGeomCollPtr geo)
{
/* decodes a LINESTRINGM from WKB */
    int points;
    int iv;
    double x;
    double y;
    double m;
    gaiaLinestringPtr line;
    if (geo->size < geo->offset + 4)
	return;
    points =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    if (geo->size < geo->offset + (24 * points))
	return;
    line = gaiaAddLinestringToGeomColl (geo, points);
    for (iv = 0; iv < points; iv++)
      {
	  x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
			    geo->endian_arch);
	  m = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
			    geo->endian_arch);
	  gaiaSetPointXYM (line->Coords, iv, x, y, m);
	  geo->offset += 24;
      }
}

static void
ParseWkbLineZM (gaiaGeomCollPtr geo)
{
/* decodes a LINESTRINGZM from WKB */
    int points;
    int iv;
    double x;
    double y;
    double z;
    double m;
    gaiaLinestringPtr line;
    if (geo->size < geo->offset + 4)
	return;
    points =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    if (geo->size < geo->offset + (32 * points))
	return;
    line = gaiaAddLinestringToGeomColl (geo, points);
    for (iv = 0; iv < points; iv++)
      {
	  x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
			    geo->endian_arch);
	  z = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
			    geo->endian_arch);
	  m = gaiaImport64 (geo->blob + (geo->offset + 24), geo->endian,
			    geo->endian_arch);
	  gaiaSetPointXYZM (line->Coords, iv, x, y, z, m);
	  geo->offset += 32;
      }
}

static void
ParseWkbPolygon (gaiaGeomCollPtr geo)
{
/* decodes a POLYGON from WKB */
    int rings;
    int nverts;
    int iv;
    int ib;
    double x;
    double y;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring;
    if (geo->size < geo->offset + 4)
	return;
    rings =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (geo->size < geo->offset + 4)
	      return;
	  nverts =
	      gaiaImport32 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  geo->offset += 4;
	  if (geo->size < geo->offset + (16 * nverts))
	      return;
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geo, nverts, rings - 1);
		ring = polyg->Exterior;
	    }
	  else
	      ring = gaiaAddInteriorRing (polyg, ib - 1, nverts);
	  for (iv = 0; iv < nverts; iv++)
	    {
		x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
				  geo->endian_arch);
		y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		geo->offset += 16;
		gaiaSetPoint (ring->Coords, iv, x, y);
	    }
      }
}

static void
ParseWkbPolygonZ (gaiaGeomCollPtr geo)
{
/* decodes a POLYGONZ from WKB */
    int rings;
    int nverts;
    int iv;
    int ib;
    double x;
    double y;
    double z;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring;
    if (geo->size < geo->offset + 4)
	return;
    rings =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (geo->size < geo->offset + 4)
	      return;
	  nverts =
	      gaiaImport32 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  geo->offset += 4;
	  if (geo->size < geo->offset + (24 * nverts))
	      return;
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geo, nverts, rings - 1);
		ring = polyg->Exterior;
	    }
	  else
	      ring = gaiaAddInteriorRing (polyg, ib - 1, nverts);
	  for (iv = 0; iv < nverts; iv++)
	    {
		x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
				  geo->endian_arch);
		y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		z = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
				  geo->endian_arch);
		geo->offset += 24;
		gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
	    }
      }
}

static void
ParseWkbPolygonM (gaiaGeomCollPtr geo)
{
/* decodes a POLYGONM from WKB */
    int rings;
    int nverts;
    int iv;
    int ib;
    double x;
    double y;
    double m;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring;
    if (geo->size < geo->offset + 4)
	return;
    rings =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (geo->size < geo->offset + 4)
	      return;
	  nverts =
	      gaiaImport32 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  geo->offset += 4;
	  if (geo->size < geo->offset + (24 * nverts))
	      return;
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geo, nverts, rings - 1);
		ring = polyg->Exterior;
	    }
	  else
	      ring = gaiaAddInteriorRing (polyg, ib - 1, nverts);
	  for (iv = 0; iv < nverts; iv++)
	    {
		x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
				  geo->endian_arch);
		y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		m = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
				  geo->endian_arch);
		geo->offset += 24;
		gaiaSetPointXYM (ring->Coords, iv, x, y, m);
	    }
      }
}

static void
ParseWkbPolygonZM (gaiaGeomCollPtr geo)
{
/* decodes a POLYGONZM from WKB */
    int rings;
    int nverts;
    int iv;
    int ib;
    double x;
    double y;
    double z;
    double m;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring;
    if (geo->size < geo->offset + 4)
	return;
    rings =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (geo->size < geo->offset + 4)
	      return;
	  nverts =
	      gaiaImport32 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  geo->offset += 4;
	  if (geo->size < geo->offset + (32 * nverts))
	      return;
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geo, nverts, rings - 1);
		ring = polyg->Exterior;
	    }
	  else
	      ring = gaiaAddInteriorRing (polyg, ib - 1, nverts);
	  for (iv = 0; iv < nverts; iv++)
	    {
		x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
				  geo->endian_arch);
		y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		z = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
				  geo->endian_arch);
		m = gaiaImport64 (geo->blob + (geo->offset + 24), geo->endian,
				  geo->endian_arch);
		geo->offset += 32;
		gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
	    }
      }
}

static void
ParseCompressedWkbLine (gaiaGeomCollPtr geo)
{
/* decodes a COMPRESSED LINESTRING from WKB */
    int points;
    int iv;
    double x;
    double y;
    double last_x = 0.0;
    double last_y = 0.0;
    float fx;
    float fy;
    gaiaLinestringPtr line;
    if (geo->size < geo->offset + 4)
	return;
    points =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    if (geo->size < geo->offset + (8 * points) + 16)
	return;
    line = gaiaAddLinestringToGeomColl (geo, points);
    for (iv = 0; iv < points; iv++)
      {
	  if (iv == 0 || iv == (points - 1))
	    {
		/* first and last vertices are uncompressed */
		x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
				  geo->endian_arch);
		y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		geo->offset += 16;
	    }
	  else
	    {
		/* any other intermediate vertex is compressed */
		fx = gaiaImportF32 (geo->blob + geo->offset, geo->endian,
				    geo->endian_arch);
		fy = gaiaImportF32 (geo->blob + (geo->offset + 4), geo->endian,
				    geo->endian_arch);
		x = last_x + fx;
		y = last_y + fy;
		geo->offset += 8;
	    }
	  gaiaSetPoint (line->Coords, iv, x, y);
	  last_x = x;
	  last_y = y;
      }
}

static void
ParseCompressedWkbLineZ (gaiaGeomCollPtr geo)
{
/* decodes a COMPRESSED LINESTRINGZ from WKB */
    int points;
    int iv;
    double x;
    double y;
    double z;
    double last_x = 0.0;
    double last_y = 0.0;
    double last_z = 0.0;
    float fx;
    float fy;
    float fz;
    gaiaLinestringPtr line;
    if (geo->size < geo->offset + 4)
	return;
    points =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    if (geo->size < geo->offset + (12 * points) + 24)
	return;
    line = gaiaAddLinestringToGeomColl (geo, points);
    for (iv = 0; iv < points; iv++)
      {
	  if (iv == 0 || iv == (points - 1))
	    {
		/* first and last vertices are uncompressed */
		x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
				  geo->endian_arch);
		y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		z = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
				  geo->endian_arch);
		geo->offset += 24;
	    }
	  else
	    {
		/* any other intermediate vertex is compressed */
		fx = gaiaImportF32 (geo->blob + geo->offset, geo->endian,
				    geo->endian_arch);
		fy = gaiaImportF32 (geo->blob + (geo->offset + 4), geo->endian,
				    geo->endian_arch);
		fz = gaiaImportF32 (geo->blob + (geo->offset + 8), geo->endian,
				    geo->endian_arch);
		x = last_x + fx;
		y = last_y + fy;
		z = last_z + fz;
		geo->offset += 12;
	    }
	  gaiaSetPointXYZ (line->Coords, iv, x, y, z);
	  last_x = x;
	  last_y = y;
	  last_z = z;
      }
}

static void
ParseCompressedWkbLineM (gaiaGeomCollPtr geo)
{
/* decodes a COMPRESSED LINESTRINGM from WKB */
    int points;
    int iv;
    double x;
    double y;
    double m;
    double last_x = 0.0;
    double last_y = 0.0;
    float fx;
    float fy;
    gaiaLinestringPtr line;
    if (geo->size < geo->offset + 4)
	return;
    points =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    if (geo->size < geo->offset + (16 * points) + 16)
	return;
    line = gaiaAddLinestringToGeomColl (geo, points);
    for (iv = 0; iv < points; iv++)
      {
	  if (iv == 0 || iv == (points - 1))
	    {
		/* first and last vertices are uncompressed */
		x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
				  geo->endian_arch);
		y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		m = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
				  geo->endian_arch);
		geo->offset += 24;
	    }
	  else
	    {
		/* any other intermediate vertex is compressed */
		fx = gaiaImportF32 (geo->blob + geo->offset, geo->endian,
				    geo->endian_arch);
		fy = gaiaImportF32 (geo->blob + (geo->offset + 4), geo->endian,
				    geo->endian_arch);
		m = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		x = last_x + fx;
		y = last_y + fy;
		geo->offset += 16;
	    }
	  gaiaSetPointXYM (line->Coords, iv, x, y, m);
	  last_x = x;
	  last_y = y;
      }
}

static void
ParseCompressedWkbLineZM (gaiaGeomCollPtr geo)
{
/* decodes a COMPRESSED LINESTRINGZM from WKB */
    int points;
    int iv;
    double x;
    double y;
    double z;
    double m;
    double last_x = 0.0;
    double last_y = 0.0;
    double last_z = 0.0;
    float fx;
    float fy;
    float fz;
    gaiaLinestringPtr line;
    if (geo->size < geo->offset + 4)
	return;
    points =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    if (geo->size < geo->offset + (20 * points) + 24)
	return;
    line = gaiaAddLinestringToGeomColl (geo, points);
    for (iv = 0; iv < points; iv++)
      {
	  if (iv == 0 || iv == (points - 1))
	    {
		/* first and last vertices are uncompressed */
		x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
				  geo->endian_arch);
		y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		z = gaiaImport64 (geo->blob + (geo->offset + 16), geo->endian,
				  geo->endian_arch);
		m = gaiaImport64 (geo->blob + (geo->offset + 24), geo->endian,
				  geo->endian_arch);
		geo->offset += 32;
	    }
	  else
	    {
		/* any other intermediate vertex is compressed */
		fx = gaiaImportF32 (geo->blob + geo->offset, geo->endian,
				    geo->endian_arch);
		fy = gaiaImportF32 (geo->blob + (geo->offset + 4), geo->endian,
				    geo->endian_arch);
		fz = gaiaImportF32 (geo->blob + (geo->offset + 8), geo->endian,
				    geo->endian_arch);
		m = gaiaImport64 (geo->blob + (geo->offset + 12), geo->endian,
				  geo->endian_arch);
		x = last_x + fx;
		y = last_y + fy;
		z = last_z + fz;
		geo->offset += 20;
	    }
	  gaiaSetPointXYZM (line->Coords, iv, x, y, z, m);
	  last_x = x;
	  last_y = y;
	  last_z = z;
      }
}

static void
ParseCompressedWkbPolygon (gaiaGeomCollPtr geo)
{
/* decodes a COMPRESSED POLYGON from WKB */
    int rings;
    int nverts;
    int iv;
    int ib;
    double x;
    double y;
    double last_x = 0.0;
    double last_y = 0.0;
    float fx;
    float fy;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring;
    if (geo->size < geo->offset + 4)
	return;
    rings =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (geo->size < geo->offset + 4)
	      return;
	  nverts =
	      gaiaImport32 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  geo->offset += 4;
	  if (geo->size < geo->offset + (8 * nverts) + 16)
	      return;
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geo, nverts, rings - 1);
		ring = polyg->Exterior;
	    }
	  else
	      ring = gaiaAddInteriorRing (polyg, ib - 1, nverts);
	  for (iv = 0; iv < nverts; iv++)
	    {
		if (iv == 0 || iv == (nverts - 1))
		  {
		      /* first and last vertices are uncompressed */
		      x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
					geo->endian_arch);
		      y = gaiaImport64 (geo->blob + (geo->offset + 8),
					geo->endian, geo->endian_arch);
		      geo->offset += 16;
		  }
		else
		  {
		      /* any other intermediate vertex is compressed */
		      fx = gaiaImportF32 (geo->blob + geo->offset, geo->endian,
					  geo->endian_arch);
		      fy = gaiaImportF32 (geo->blob + (geo->offset + 4),
					  geo->endian, geo->endian_arch);
		      x = last_x + fx;
		      y = last_y + fy;
		      geo->offset += 8;
		  }
		gaiaSetPoint (ring->Coords, iv, x, y);
		last_x = x;
		last_y = y;
	    }
      }
}

static void
ParseCompressedWkbPolygonZ (gaiaGeomCollPtr geo)
{
/* decodes a COMPRESSED POLYGONZ from WKB */
    int rings;
    int nverts;
    int iv;
    int ib;
    double x;
    double y;
    double z;
    double last_x = 0.0;
    double last_y = 0.0;
    double last_z = 0.0;
    float fx;
    float fy;
    float fz;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring;
    if (geo->size < geo->offset + 4)
	return;
    rings =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (geo->size < geo->offset + 4)
	      return;
	  nverts =
	      gaiaImport32 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  geo->offset += 4;
	  if (geo->size < geo->offset + (12 * nverts) + 24)
	      return;
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geo, nverts, rings - 1);
		ring = polyg->Exterior;
	    }
	  else
	      ring = gaiaAddInteriorRing (polyg, ib - 1, nverts);
	  for (iv = 0; iv < nverts; iv++)
	    {
		if (iv == 0 || iv == (nverts - 1))
		  {
		      /* first and last vertices are uncompressed */
		      x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
					geo->endian_arch);
		      y = gaiaImport64 (geo->blob + (geo->offset + 8),
					geo->endian, geo->endian_arch);
		      z = gaiaImport64 (geo->blob + (geo->offset + 16),
					geo->endian, geo->endian_arch);
		      geo->offset += 24;
		  }
		else
		  {
		      /* any other intermediate vertex is compressed */
		      fx = gaiaImportF32 (geo->blob + geo->offset, geo->endian,
					  geo->endian_arch);
		      fy = gaiaImportF32 (geo->blob + (geo->offset + 4),
					  geo->endian, geo->endian_arch);
		      fz = gaiaImportF32 (geo->blob + (geo->offset + 8),
					  geo->endian, geo->endian_arch);
		      x = last_x + fx;
		      y = last_y + fy;
		      z = last_z + fz;
		      geo->offset += 12;
		  }
		gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
		last_x = x;
		last_y = y;
		last_z = z;
	    }
      }
}

static void
ParseCompressedWkbPolygonM (gaiaGeomCollPtr geo)
{
/* decodes a COMPRESSED POLYGONM from WKB */
    int rings;
    int nverts;
    int iv;
    int ib;
    double x;
    double y;
    double m;
    double last_x = 0.0;
    double last_y = 0.0;
    float fx;
    float fy;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring;
    if (geo->size < geo->offset + 4)
	return;
    rings =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (geo->size < geo->offset + 4)
	      return;
	  nverts =
	      gaiaImport32 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  geo->offset += 4;
	  if (geo->size < geo->offset + (16 * nverts) + 16)
	      return;
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geo, nverts, rings - 1);
		ring = polyg->Exterior;
	    }
	  else
	      ring = gaiaAddInteriorRing (polyg, ib - 1, nverts);
	  for (iv = 0; iv < nverts; iv++)
	    {
		if (iv == 0 || iv == (nverts - 1))
		  {
		      /* first and last vertices are uncompressed */
		      x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
					geo->endian_arch);
		      y = gaiaImport64 (geo->blob + (geo->offset + 8),
					geo->endian, geo->endian_arch);
		      m = gaiaImport64 (geo->blob + (geo->offset + 16),
					geo->endian, geo->endian_arch);
		      geo->offset += 24;
		  }
		else
		  {
		      /* any other intermediate vertex is compressed */
		      fx = gaiaImportF32 (geo->blob + geo->offset, geo->endian,
					  geo->endian_arch);
		      fy = gaiaImportF32 (geo->blob + (geo->offset + 4),
					  geo->endian, geo->endian_arch);
		      m = gaiaImport64 (geo->blob + (geo->offset + 8),
					geo->endian, geo->endian_arch);
		      x = last_x + fx;
		      y = last_y + fy;
		      geo->offset += 16;
		  }
		gaiaSetPointXYM (ring->Coords, iv, x, y, m);
		last_x = x;
		last_y = y;
	    }
      }
}

static void
ParseCompressedWkbPolygonZM (gaiaGeomCollPtr geo)
{
/* decodes a COMPRESSED POLYGONZM from WKB */
    int rings;
    int nverts;
    int iv;
    int ib;
    double x;
    double y;
    double z;
    double m;
    double last_x = 0.0;
    double last_y = 0.0;
    double last_z = 0.0;
    float fx;
    float fy;
    float fz;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring;
    if (geo->size < geo->offset + 4)
	return;
    rings =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (geo->size < geo->offset + 4)
	      return;
	  nverts =
	      gaiaImport32 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  geo->offset += 4;
	  if (geo->size < geo->offset + (20 * nverts) + 24)
	      return;
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geo, nverts, rings - 1);
		ring = polyg->Exterior;
	    }
	  else
	      ring = gaiaAddInteriorRing (polyg, ib - 1, nverts);
	  for (iv = 0; iv < nverts; iv++)
	    {
		if (iv == 0 || iv == (nverts - 1))
		  {
		      /* first and last vertices are uncompressed */
		      x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
					geo->endian_arch);
		      y = gaiaImport64 (geo->blob + (geo->offset + 8),
					geo->endian, geo->endian_arch);
		      z = gaiaImport64 (geo->blob + (geo->offset + 16),
					geo->endian, geo->endian_arch);
		      m = gaiaImport64 (geo->blob + (geo->offset + 24),
					geo->endian, geo->endian_arch);
		      geo->offset += 32;
		  }
		else
		  {
		      /* any other intermediate vertex is compressed */
		      fx = gaiaImportF32 (geo->blob + geo->offset, geo->endian,
					  geo->endian_arch);
		      fy = gaiaImportF32 (geo->blob + (geo->offset + 4),
					  geo->endian, geo->endian_arch);
		      fz = gaiaImportF32 (geo->blob + (geo->offset + 8),
					  geo->endian, geo->endian_arch);
		      m = gaiaImport64 (geo->blob + (geo->offset + 12),
					geo->endian, geo->endian_arch);
		      x = last_x + fx;
		      y = last_y + fy;
		      z = last_z + fz;
		      geo->offset += 20;
		  }
		gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
		last_x = x;
		last_y = y;
		last_z = z;
	    }
      }
}

static void
ParseWkbGeometry (gaiaGeomCollPtr geo, int isWKB)
{
/* decodes a MULTIxx or GEOMETRYCOLLECTION from SpatiaLite BLOB */
    int entities;
    int type;
    int ie;
    if (geo->size < geo->offset + 4)
	return;
    entities =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ie = 0; ie < entities; ie++)
      {
	  if (geo->size < geo->offset + 5)
	      return;
	  if (isWKB)
	    {
		/* vanilla WKB could be encoded as mixed big-/little-endian sub-items */
		if (*(geo->blob + geo->offset) == 0x01)
		    geo->endian = GAIA_LITTLE_ENDIAN;
		else
		    geo->endian = GAIA_BIG_ENDIAN;
	    }
	  type =
	      gaiaImport32 (geo->blob + geo->offset + 1, geo->endian,
			    geo->endian_arch);
	  geo->offset += 5;
	  switch (type)
	    {
	    case GAIA_POINT:
		ParseWkbPoint (geo);
		break;
	    case GAIA_POINTZ:
	    case GAIA_GEOSWKB_POINTZ:
		ParseWkbPointZ (geo);
		break;
	    case GAIA_POINTM:
		ParseWkbPointM (geo);
		break;
	    case GAIA_POINTZM:
		ParseWkbPointZM (geo);
		break;
	    case GAIA_LINESTRING:
		ParseWkbLine (geo);
		break;
	    case GAIA_LINESTRINGZ:
	    case GAIA_GEOSWKB_LINESTRINGZ:
		ParseWkbLineZ (geo);
		break;
	    case GAIA_LINESTRINGM:
		ParseWkbLineM (geo);
		break;
	    case GAIA_LINESTRINGZM:
		ParseWkbLineZM (geo);
		break;
	    case GAIA_POLYGON:
		ParseWkbPolygon (geo);
		break;
	    case GAIA_POLYGONZ:
	    case GAIA_GEOSWKB_POLYGONZ:
		ParseWkbPolygonZ (geo);
		break;
	    case GAIA_POLYGONM:
		ParseWkbPolygonM (geo);
		break;
	    case GAIA_POLYGONZM:
		ParseWkbPolygonZM (geo);
		break;
	    case GAIA_COMPRESSED_LINESTRING:
		ParseCompressedWkbLine (geo);
		break;
	    case GAIA_COMPRESSED_LINESTRINGZ:
		ParseCompressedWkbLineZ (geo);
		break;
	    case GAIA_COMPRESSED_LINESTRINGM:
		ParseCompressedWkbLineM (geo);
		break;
	    case GAIA_COMPRESSED_LINESTRINGZM:
		ParseCompressedWkbLineZM (geo);
		break;
	    case GAIA_COMPRESSED_POLYGON:
		ParseCompressedWkbPolygon (geo);
		break;
	    case GAIA_COMPRESSED_POLYGONZ:
		ParseCompressedWkbPolygonZ (geo);
		break;
	    case GAIA_COMPRESSED_POLYGONM:
		ParseCompressedWkbPolygonM (geo);
		break;
	    case GAIA_COMPRESSED_POLYGONZM:
		ParseCompressedWkbPolygonZM (geo);
		break;
	    default:
		break;
	    };
      }
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromSpatiaLiteBlobWkbEx (const unsigned char *blob, unsigned int size,
			     int gpkg_mode, int gpkg_amphibious)
{
/* decoding from SpatiaLite BLOB to GEOMETRY */
    int type;
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    gaiaGeomCollPtr geo = NULL;

    if (gpkg_amphibious || gpkg_mode)
      {
#ifdef ENABLE_GEOPACKAGE	/* GEOPACKAGE enabled: supporting GPKG geometries */
	  if (gaiaIsValidGPB (blob, size))
	    {
		geo = gaiaFromGeoPackageGeometryBlob (blob, size);
		if (geo != NULL)
		    return geo;
	    }
	  if (gpkg_mode)
	      return NULL;	/* must accept only GPKG geometries */
#else
	  ;
#endif /* end GEOPACKAGE: supporting GPKG geometries */
      }

    if (size < 45)
	return NULL;		/* cannot be an internal BLOB WKB geometry */
    if (*(blob + 0) != GAIA_MARK_START)
	return NULL;		/* failed to recognize START signature */
    if (*(blob + (size - 1)) != GAIA_MARK_END)
	return NULL;		/* failed to recognize END signature */
    if (*(blob + 38) != GAIA_MARK_MBR)
	return NULL;		/* failed to recognize MBR signature */
    if (*(blob + 1) == GAIA_LITTLE_ENDIAN)
	little_endian = 1;
    else if (*(blob + 1) == GAIA_BIG_ENDIAN)
	little_endian = 0;
    else
	return NULL;		/* unknown encoding; nor little-endian neither big-endian */
    type = gaiaImport32 (blob + 39, little_endian, endian_arch);
    geo = gaiaAllocGeomColl ();
    geo->Srid = gaiaImport32 (blob + 2, little_endian, endian_arch);
    geo->endian_arch = (char) endian_arch;
    geo->endian = (char) little_endian;
    geo->blob = blob;
    geo->size = size;
    geo->offset = 43;
    switch (type)
      {
	  /* setting up DimensionModel */
      case GAIA_POINTZ:
      case GAIA_LINESTRINGZ:
      case GAIA_POLYGONZ:
      case GAIA_MULTIPOINTZ:
      case GAIA_MULTILINESTRINGZ:
      case GAIA_MULTIPOLYGONZ:
      case GAIA_GEOMETRYCOLLECTIONZ:
      case GAIA_COMPRESSED_LINESTRINGZ:
      case GAIA_COMPRESSED_POLYGONZ:
	  geo->DimensionModel = GAIA_XY_Z;
	  break;
      case GAIA_POINTM:
      case GAIA_LINESTRINGM:
      case GAIA_POLYGONM:
      case GAIA_MULTIPOINTM:
      case GAIA_MULTILINESTRINGM:
      case GAIA_MULTIPOLYGONM:
      case GAIA_GEOMETRYCOLLECTIONM:
      case GAIA_COMPRESSED_LINESTRINGM:
      case GAIA_COMPRESSED_POLYGONM:
	  geo->DimensionModel = GAIA_XY_M;
	  break;
      case GAIA_POINTZM:
      case GAIA_LINESTRINGZM:
      case GAIA_POLYGONZM:
      case GAIA_MULTIPOINTZM:
      case GAIA_MULTILINESTRINGZM:
      case GAIA_MULTIPOLYGONZM:
      case GAIA_GEOMETRYCOLLECTIONZM:
      case GAIA_COMPRESSED_LINESTRINGZM:
      case GAIA_COMPRESSED_POLYGONZM:
	  geo->DimensionModel = GAIA_XY_Z_M;
	  break;
      default:
	  geo->DimensionModel = GAIA_XY;
	  break;
      };
    switch (type)
      {
	  /* parsing elementary geometries */
      case GAIA_POINT:
	  ParseWkbPoint (geo);
	  break;
      case GAIA_POINTZ:
	  ParseWkbPointZ (geo);
	  break;
      case GAIA_POINTM:
	  ParseWkbPointM (geo);
	  break;
      case GAIA_POINTZM:
	  ParseWkbPointZM (geo);
	  break;
      case GAIA_LINESTRING:
	  ParseWkbLine (geo);
	  break;
      case GAIA_LINESTRINGZ:
	  ParseWkbLineZ (geo);
	  break;
      case GAIA_LINESTRINGM:
	  ParseWkbLineM (geo);
	  break;
      case GAIA_LINESTRINGZM:
	  ParseWkbLineZM (geo);
	  break;
      case GAIA_POLYGON:
	  ParseWkbPolygon (geo);
	  break;
      case GAIA_POLYGONZ:
	  ParseWkbPolygonZ (geo);
	  break;
      case GAIA_POLYGONM:
	  ParseWkbPolygonM (geo);
	  break;
      case GAIA_POLYGONZM:
	  ParseWkbPolygonZM (geo);
	  break;
      case GAIA_COMPRESSED_LINESTRING:
	  ParseCompressedWkbLine (geo);
	  break;
      case GAIA_COMPRESSED_LINESTRINGZ:
	  ParseCompressedWkbLineZ (geo);
	  break;
      case GAIA_COMPRESSED_LINESTRINGM:
	  ParseCompressedWkbLineM (geo);
	  break;
      case GAIA_COMPRESSED_LINESTRINGZM:
	  ParseCompressedWkbLineZM (geo);
	  break;
      case GAIA_COMPRESSED_POLYGON:
	  ParseCompressedWkbPolygon (geo);
	  break;
      case GAIA_COMPRESSED_POLYGONZ:
	  ParseCompressedWkbPolygonZ (geo);
	  break;
      case GAIA_COMPRESSED_POLYGONM:
	  ParseCompressedWkbPolygonM (geo);
	  break;
      case GAIA_COMPRESSED_POLYGONZM:
	  ParseCompressedWkbPolygonZM (geo);
	  break;
      case GAIA_MULTIPOINT:
      case GAIA_MULTIPOINTZ:
      case GAIA_MULTIPOINTM:
      case GAIA_MULTIPOINTZM:
      case GAIA_MULTILINESTRING:
      case GAIA_MULTILINESTRINGZ:
      case GAIA_MULTILINESTRINGM:
      case GAIA_MULTILINESTRINGZM:
      case GAIA_MULTIPOLYGON:
      case GAIA_MULTIPOLYGONZ:
      case GAIA_MULTIPOLYGONM:
      case GAIA_MULTIPOLYGONZM:
      case GAIA_GEOMETRYCOLLECTION:
      case GAIA_GEOMETRYCOLLECTIONZ:
      case GAIA_GEOMETRYCOLLECTIONM:
      case GAIA_GEOMETRYCOLLECTIONZM:
	  ParseWkbGeometry (geo, 0);
	  break;
      default:
	  break;
      };
    geo->MinX = gaiaImport64 (blob + 6, little_endian, endian_arch);
    geo->MinY = gaiaImport64 (blob + 14, little_endian, endian_arch);
    geo->MaxX = gaiaImport64 (blob + 22, little_endian, endian_arch);
    geo->MaxY = gaiaImport64 (blob + 30, little_endian, endian_arch);
    switch (type)
      {
	  /* setting up DeclaredType */
      case GAIA_POINT:
      case GAIA_POINTZ:
      case GAIA_POINTM:
      case GAIA_POINTZM:
	  geo->DeclaredType = GAIA_POINT;
	  break;
      case GAIA_LINESTRING:
      case GAIA_LINESTRINGZ:
      case GAIA_LINESTRINGM:
      case GAIA_LINESTRINGZM:
      case GAIA_COMPRESSED_LINESTRING:
      case GAIA_COMPRESSED_LINESTRINGZ:
      case GAIA_COMPRESSED_LINESTRINGM:
      case GAIA_COMPRESSED_LINESTRINGZM:
	  geo->DeclaredType = GAIA_LINESTRING;
	  break;
      case GAIA_POLYGON:
      case GAIA_POLYGONZ:
      case GAIA_POLYGONM:
      case GAIA_POLYGONZM:
      case GAIA_COMPRESSED_POLYGON:
      case GAIA_COMPRESSED_POLYGONZ:
      case GAIA_COMPRESSED_POLYGONM:
      case GAIA_COMPRESSED_POLYGONZM:
	  geo->DeclaredType = GAIA_POLYGON;
	  break;
      case GAIA_MULTIPOINT:
      case GAIA_MULTIPOINTZ:
      case GAIA_MULTIPOINTM:
      case GAIA_MULTIPOINTZM:
	  geo->DeclaredType = GAIA_MULTIPOINT;
	  break;
      case GAIA_MULTILINESTRING:
      case GAIA_MULTILINESTRINGZ:
      case GAIA_MULTILINESTRINGM:
      case GAIA_MULTILINESTRINGZM:
	  geo->DeclaredType = GAIA_MULTILINESTRING;
	  break;
      case GAIA_MULTIPOLYGON:
      case GAIA_MULTIPOLYGONZ:
      case GAIA_MULTIPOLYGONM:
      case GAIA_MULTIPOLYGONZM:
	  geo->DeclaredType = GAIA_MULTIPOLYGON;
	  break;
      case GAIA_GEOMETRYCOLLECTION:
      case GAIA_GEOMETRYCOLLECTIONZ:
      case GAIA_GEOMETRYCOLLECTIONM:
      case GAIA_GEOMETRYCOLLECTIONZM:
	  geo->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  break;
      default:
	  geo->DeclaredType = GAIA_UNKNOWN;
	  break;
      };
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromSpatiaLiteBlobWkb (const unsigned char *blob, unsigned int size)
{
/* 
* decoding from SpatiaLite BLOB to GEOMETRY 
* convenience method - always disabling GPKG compatibility Modes
*/
    return gaiaFromSpatiaLiteBlobWkbEx (blob, size, 0, 0);
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromSpatiaLiteBlobMbr (const unsigned char *blob, unsigned int size)
{
/* decoding from SpatiaLite BLOB to GEOMETRY [MBR only] */
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    double minx;
    double miny;
    double maxx;
    double maxy;
    gaiaGeomCollPtr geo = NULL;
    gaiaPolygonPtr polyg;
    gaiaRingPtr ring;
    if (size < 45)
	return NULL;		/* cannot be an internal BLOB WKB geometry */
    if (*(blob + 0) != GAIA_MARK_START)
	return NULL;		/* failed to recognize START signature */
    if (*(blob + (size - 1)) != GAIA_MARK_END)
	return NULL;		/* failed to recognize END signature */
    if (*(blob + 38) != GAIA_MARK_MBR)
	return NULL;		/* failed to recognize MBR signature */
    if (*(blob + 1) == GAIA_LITTLE_ENDIAN)
	little_endian = 1;
    else if (*(blob + 1) == GAIA_BIG_ENDIAN)
	little_endian = 0;
    else
	return NULL;		/* unknown encoding; nor litte-endian neither big-endian */
    geo = gaiaAllocGeomColl ();
    polyg = gaiaAddPolygonToGeomColl (geo, 5, 0);
    ring = polyg->Exterior;
    minx = gaiaImport64 (blob + 6, little_endian, endian_arch);
    miny = gaiaImport64 (blob + 14, little_endian, endian_arch);
    maxx = gaiaImport64 (blob + 22, little_endian, endian_arch);
    maxy = gaiaImport64 (blob + 30, little_endian, endian_arch);
    gaiaSetPoint (ring->Coords, 0, minx, miny);	/* vertex # 1 */
    gaiaSetPoint (ring->Coords, 1, maxx, miny);	/* vertex # 2 */
    gaiaSetPoint (ring->Coords, 2, maxx, maxy);	/* vertex # 3 */
    gaiaSetPoint (ring->Coords, 3, minx, maxy);	/* vertex # 4 */
    gaiaSetPoint (ring->Coords, 4, minx, miny);	/* vertex # 5 [same as vertex # 1 to close the polygon] */
    return geo;
}

GAIAGEO_DECLARE void
gaiaToSpatiaLiteBlobWkbEx (gaiaGeomCollPtr geom, unsigned char **result,
			   int *size, int gpkg_mode)
{
/* builds the SpatiaLite BLOB representation for this GEOMETRY */
    int ib;
    int iv;
    double x;
    double y;
    double z = 0.0;
    double m = 0.0;
    int entities = 0;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    int type;
    unsigned char *ptr;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    int endian_arch = gaiaEndianArch ();
    gaiaMbrGeometry (geom);

    if (gpkg_mode)
      {
#ifdef ENABLE_GEOPACKAGE	/* only if GeoPackage support is enabled */
	  /* GeoPackage Mode enabled */
	  gaiaToGPB (geom, result, size);
#endif /* end GEOPACKAGE conditional */
	  return;
      }

/* how many entities, and of what kind, do we have ? */
    pt = geom->FirstPoint;
    while (pt)
      {
	  point = pt;
	  entities++;
	  n_points++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  line = ln;
	  entities++;
	  n_linestrings++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  polyg = pg;
	  entities++;
	  n_polygons++;
	  pg = pg->Next;
      }
    *size = 0;
    *result = NULL;
    if (n_points == 0 && n_polygons == 0 && n_linestrings == 0)
	return;
/* ok, we can determine the geometry class */
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOINTZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOINTM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOINTZM;
		else
		    type = GAIA_MULTIPOINT;
	    }
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_POINTZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_POINTM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_POINTZM;
		else
		    type = GAIA_POINT;
	    }
      }
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOINTZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOINTM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOINTZM;
		else
		    type = GAIA_MULTIPOINT;
	    }
      }
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTILINESTRINGZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTILINESTRINGM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTILINESTRINGZM;
		else
		    type = GAIA_MULTILINESTRING;
	    }
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_LINESTRINGZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_LINESTRINGM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_LINESTRINGZM;
		else
		    type = GAIA_LINESTRING;
	    }
      }
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTILINESTRINGZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTILINESTRINGM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTILINESTRINGZM;
		else
		    type = GAIA_MULTILINESTRING;
	    }
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOLYGONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOLYGONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOLYGONZM;
		else
		    type = GAIA_MULTIPOLYGON;
	    }
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_POLYGONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_POLYGONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_POLYGONZM;
		else
		    type = GAIA_POLYGON;
	    }
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOLYGONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOLYGONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOLYGONZM;
		else
		    type = GAIA_MULTIPOLYGON;
	    }
      }
    else
      {
	  if (geom->DimensionModel == GAIA_XY_Z)
	      type = GAIA_GEOMETRYCOLLECTIONZ;
	  else if (geom->DimensionModel == GAIA_XY_M)
	      type = GAIA_GEOMETRYCOLLECTIONM;
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	      type = GAIA_GEOMETRYCOLLECTIONZM;
	  else
	      type = GAIA_GEOMETRYCOLLECTION;
      }
/* and now we compute the size of BLOB */
    *size = 44;			/* header size */
    switch (type)
      {
      case GAIA_POINT:
	  *size += (sizeof (double) * 2);	/* [x,y] coords */
	  break;
      case GAIA_POINTZ:
	  *size += (sizeof (double) * 3);	/* [x,y,z] coords */
	  break;
      case GAIA_POINTM:
	  *size += (sizeof (double) * 3);	/* [x,y,m] coords */
	  break;
      case GAIA_POINTZM:
	  *size += (sizeof (double) * 4);	/* [x,y,z,m] coords */
	  break;
      case GAIA_LINESTRING:
	  *size += (4 + ((sizeof (double) * 2) * line->Points));	/* # points + [x,y] for each vertex */
	  break;
      case GAIA_LINESTRINGZ:
	  *size += (4 + ((sizeof (double) * 3) * line->Points));	/* # points + [x,y,z] for each vertex */
	  break;
      case GAIA_LINESTRINGM:
	  *size += (4 + ((sizeof (double) * 3) * line->Points));	/* # points + [x,y,m] for each vertex */
	  break;
      case GAIA_LINESTRINGZM:
	  *size += (4 + ((sizeof (double) * 4) * line->Points));	/* # points + [x,y,z,m] for each vertex */
	  break;
      case GAIA_POLYGON:
	  rng = polyg->Exterior;
	  *size += (8 + ((sizeof (double) * 2) * rng->Points));	/* # rings + # points + [x.y] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + ((sizeof (double) * 2) * rng->Points));	/* # points + [x,y] array - interior ring */
	    }
	  break;
      case GAIA_POLYGONZ:
	  rng = polyg->Exterior;
	  *size += (8 + ((sizeof (double) * 3) * rng->Points));	/* # rings + # points + [x,y,z] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + ((sizeof (double) * 3) * rng->Points));	/* # points + [x,y,z] array - interior ring */
	    }
	  break;
      case GAIA_POLYGONM:
	  rng = polyg->Exterior;
	  *size += (8 + ((sizeof (double) * 3) * rng->Points));	/* # rings + # points + [x,y,m] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + ((sizeof (double) * 3) * rng->Points));	/* # points + [x,y,m] array - interior ring */
	    }
	  break;
      case GAIA_POLYGONZM:
	  rng = polyg->Exterior;
	  *size += (8 + ((sizeof (double) * 4) * rng->Points));	/* # rings + # points + [x,y,z,m] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + ((sizeof (double) * 4) * rng->Points));	/* # points + [x,y,z,m] array - interior ring */
	    }
	  break;
      default:
	  /* this one is not a simple geometry; should be a MULTIxxxx or a GEOMETRYCOLLECTION */
	  *size += 4;		/* # entities */
	  point = geom->FirstPoint;
	  while (point)
	    {
		*size += 5;	/* entity header */
		if (geom->DimensionModel == GAIA_XY_Z
		    || geom->DimensionModel == GAIA_XY_M)
		    *size += (sizeof (double) * 3);	/* three doubles for each POINT */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    *size += (sizeof (double) * 4);	/* four doubles for each POINT */
		else
		    *size += (sizeof (double) * 2);	/* two doubles for each POINT */
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		*size += 5;	/* entity header */
		if (geom->DimensionModel == GAIA_XY_Z
		    || geom->DimensionModel == GAIA_XY_M)
		    *size += (4 + ((sizeof (double) * 3) * line->Points));	/* # points + [x,y,z] for each vertex */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    *size += (4 + ((sizeof (double) * 4) * line->Points));	/* # points + [x,y,z,m] for each vertex */
		else
		    *size += (4 + ((sizeof (double) * 2) * line->Points));	/* # points + [x,y] for each vertex */
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		*size += 5;	/* entity header */
		rng = polyg->Exterior;
		if (geom->DimensionModel == GAIA_XY_Z
		    || geom->DimensionModel == GAIA_XY_M)
		    *size += (8 + ((sizeof (double) * 3) * rng->Points));	/* # rings + # points + [x,y,z] array - exterior ring */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    *size += (8 + ((sizeof (double) * 4) * rng->Points));	/* # rings + # points + [x,y,z,m] array - exterior ring */
		else
		    *size += (8 + ((sizeof (double) * 2) * rng->Points));	/* # rings + # points + [x,y] array - exterior ring */
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      rng = polyg->Interiors + ib;
		      if (geom->DimensionModel == GAIA_XY_Z
			  || geom->DimensionModel == GAIA_XY_M)
			  *size += (4 + ((sizeof (double) * 3) * rng->Points));	/* # points + [x,y,z] array - interior ring */
		      else if (geom->DimensionModel == GAIA_XY_Z_M)
			  *size += (4 + ((sizeof (double) * 4) * rng->Points));	/* # points + [x,y,z,m] array - interior ring */
		      else
			  *size += (4 + ((sizeof (double) * 2) * rng->Points));	/* # points + [x,y] array - interior ring */
		  }
		polyg = polyg->Next;
	    }
      };
    *result = malloc (*size);
    ptr = *result;
/* and finally we build the BLOB */
    switch (type)
      {
      case GAIA_POINT:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POINT, 1, endian_arch);	/* class POINT */
	  gaiaExport64 (ptr + 43, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 51, point->Y, 1, endian_arch);	/* Y */
	  *(ptr + 59) = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POINTZ:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POINTZ, 1, endian_arch);	/* class POINT XYZ */
	  gaiaExport64 (ptr + 43, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 51, point->Y, 1, endian_arch);	/* Y */
	  gaiaExport64 (ptr + 59, point->Z, 1, endian_arch);	/* Z */
	  *(ptr + 67) = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POINTM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POINTM, 1, endian_arch);	/* class POINT XYM */
	  gaiaExport64 (ptr + 43, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 51, point->Y, 1, endian_arch);	/* Y */
	  gaiaExport64 (ptr + 59, point->M, 1, endian_arch);	/* M */
	  *(ptr + 67) = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POINTZM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POINTZM, 1, endian_arch);	/* class POINT XYZM */
	  gaiaExport64 (ptr + 43, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 51, point->Y, 1, endian_arch);	/* Y */
	  gaiaExport64 (ptr + 59, point->Z, 1, endian_arch);	/* M */
	  gaiaExport64 (ptr + 67, point->M, 1, endian_arch);	/* Z */
	  *(ptr + 75) = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_LINESTRING:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_LINESTRING, 1, endian_arch);	/* class LINESTRING */
	  gaiaExport32 (ptr + 43, line->Points, 1, endian_arch);	/* # points */
	  ptr += 47;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, 1, endian_arch);
		gaiaExport64 (ptr + 8, y, 1, endian_arch);
		ptr += 16;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_LINESTRINGZ:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_LINESTRINGZ, 1, endian_arch);	/* class LINESTRING XYZ */
	  gaiaExport32 (ptr + 43, line->Points, 1, endian_arch);	/* # points */
	  ptr += 47;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		gaiaExport64 (ptr, x, 1, endian_arch);
		gaiaExport64 (ptr + 8, y, 1, endian_arch);
		gaiaExport64 (ptr + 16, z, 1, endian_arch);
		ptr += 24;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_LINESTRINGM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_LINESTRINGM, 1, endian_arch);	/* class LINESTRING XYM */
	  gaiaExport32 (ptr + 43, line->Points, 1, endian_arch);	/* # points */
	  ptr += 47;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		gaiaExport64 (ptr, x, 1, endian_arch);
		gaiaExport64 (ptr + 8, y, 1, endian_arch);
		gaiaExport64 (ptr + 16, m, 1, endian_arch);
		ptr += 24;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_LINESTRINGZM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_LINESTRINGZM, 1, endian_arch);	/* class LINESTRING XYZM */
	  gaiaExport32 (ptr + 43, line->Points, 1, endian_arch);	/* # points */
	  ptr += 47;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		gaiaExport64 (ptr, x, 1, endian_arch);
		gaiaExport64 (ptr + 8, y, 1, endian_arch);
		gaiaExport64 (ptr + 16, z, 1, endian_arch);
		gaiaExport64 (ptr + 24, m, 1, endian_arch);
		ptr += 32;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POLYGON:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POLYGON, 1, endian_arch);	/* class POLYGON */
	  gaiaExport32 (ptr + 43, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 47, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 51;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPoint (rng->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		ptr += 16;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
		      ptr += 16;
		  }
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POLYGONZ:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POLYGONZ, 1, endian_arch);	/* class POLYGON XYZ */
	  gaiaExport32 (ptr + 43, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 47, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 51;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		gaiaExport64 (ptr + 16, z, 1, endian_arch);	/* Z - exterior ring */
		ptr += 24;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
		      gaiaExport64 (ptr + 16, z, 1, endian_arch);	/* Z - interior ring */
		      ptr += 24;
		  }
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POLYGONM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POLYGONM, 1, endian_arch);	/* class POLYGON XYM */
	  gaiaExport32 (ptr + 43, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 47, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 51;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		gaiaExport64 (ptr + 16, m, 1, endian_arch);	/* M - exterior ring */
		ptr += 24;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
		      gaiaExport64 (ptr + 16, m, 1, endian_arch);	/* M - interior ring */
		      ptr += 24;
		  }
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POLYGONZM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POLYGONZM, 1, endian_arch);	/* class POLYGON */
	  gaiaExport32 (ptr + 43, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 47, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 51;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		gaiaExport64 (ptr + 16, z, 1, endian_arch);	/* Z - exterior ring */
		gaiaExport64 (ptr + 24, m, 1, endian_arch);	/* M - exterior ring */
		ptr += 32;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
		      gaiaExport64 (ptr + 16, z, 1, endian_arch);	/* Z - exterior ring */
		      gaiaExport64 (ptr + 24, m, 1, endian_arch);	/* M - exterior ring */
		      ptr += 32;
		  }
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      default:
	  /* this one is a MULTIxxxx or a GEOMETRYCOLLECTION - building the main header */
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, type, 1, endian_arch);	/* geometric class */
	  gaiaExport32 (ptr + 43, entities, 1, endian_arch);	/* # entities */
	  ptr += 47;
	  point = geom->FirstPoint;
	  while (point)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		if (geom->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaExport32 (ptr + 1, GAIA_POINTZ, 1, endian_arch);	/* class POINT XYZ */
		      gaiaExport64 (ptr + 5, point->X, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 13, point->Y, 1, endian_arch);	/* Y */
		      gaiaExport64 (ptr + 21, point->Z, 1, endian_arch);	/* Z */
		      ptr += 29;
		  }
		else if (geom->DimensionModel == GAIA_XY_M)
		  {
		      gaiaExport32 (ptr + 1, GAIA_POINTM, 1, endian_arch);	/* class POINT XYM */
		      gaiaExport64 (ptr + 5, point->X, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 13, point->Y, 1, endian_arch);	/* Y */
		      gaiaExport64 (ptr + 21, point->M, 1, endian_arch);	/* M */
		      ptr += 29;
		  }
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaExport32 (ptr + 1, GAIA_POINTZM, 1, endian_arch);	/* class POINT XYZM */
		      gaiaExport64 (ptr + 5, point->X, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 13, point->Y, 1, endian_arch);	/* Y */
		      gaiaExport64 (ptr + 21, point->Z, 1, endian_arch);	/* Z */
		      gaiaExport64 (ptr + 29, point->M, 1, endian_arch);	/* M */
		      ptr += 37;
		  }
		else
		  {
		      gaiaExport32 (ptr + 1, GAIA_POINT, 1, endian_arch);	/* class POINT */
		      gaiaExport64 (ptr + 5, point->X, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 13, point->Y, 1, endian_arch);	/* Y */
		      ptr += 21;
		  }
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		if (geom->DimensionModel == GAIA_XY_Z)
		    gaiaExport32 (ptr + 1, GAIA_LINESTRINGZ, 1, endian_arch);	/* class LINESTRING XYZ */
		else if (geom->DimensionModel == GAIA_XY_M)
		    gaiaExport32 (ptr + 1, GAIA_LINESTRINGM, 1, endian_arch);	/* class LINESTRING XYM */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    gaiaExport32 (ptr + 1, GAIA_LINESTRINGZM, 1, endian_arch);	/* class LINESTRING XYZM */
		else
		    gaiaExport32 (ptr + 1, GAIA_LINESTRING, 1, endian_arch);	/* class LINESTRING */
		gaiaExport32 (ptr + 5, line->Points, 1, endian_arch);	/* # points */
		ptr += 9;
		for (iv = 0; iv < line->Points; iv++)
		  {
		      if (geom->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
			}
		      else if (geom->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
			}
		      else if (geom->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (line->Coords, iv, &x, &y);
			}
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y */
		      ptr += 16;
		      if (geom->DimensionModel == GAIA_XY_Z)
			{
			    gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
			    ptr += 8;
			}
		      if (geom->DimensionModel == GAIA_XY_M)
			{
			    gaiaExport64 (ptr, m, 1, endian_arch);	/* M */
			    ptr += 8;
			}
		      if (geom->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
			    gaiaExport64 (ptr + 8, m, 1, endian_arch);	/* M */
			    ptr += 16;
			}
		  }
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		if (geom->DimensionModel == GAIA_XY_Z)
		    gaiaExport32 (ptr + 1, GAIA_POLYGONZ, 1, endian_arch);	/* class POLYGON XYZ */
		else if (geom->DimensionModel == GAIA_XY_M)
		    gaiaExport32 (ptr + 1, GAIA_POLYGONM, 1, endian_arch);	/* class POLYGON XYM */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    gaiaExport32 (ptr + 1, GAIA_POLYGONZM, 1, endian_arch);	/* class POLYGON XYZM */
		else
		    gaiaExport32 (ptr + 1, GAIA_POLYGON, 1, endian_arch);	/* class POLYGON */
		gaiaExport32 (ptr + 5, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
		rng = polyg->Exterior;
		gaiaExport32 (ptr + 9, rng->Points, 1, endian_arch);	/* # points - exterior ring */
		ptr += 13;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      if (geom->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
			}
		      else if (geom->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
			}
		      else if (geom->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (rng->Coords, iv, &x, &y);
			}
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		      ptr += 16;
		      if (geom->DimensionModel == GAIA_XY_Z)
			{
			    gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
			    ptr += 8;
			}
		      if (geom->DimensionModel == GAIA_XY_M)
			{
			    gaiaExport64 (ptr, m, 1, endian_arch);	/* M */
			    ptr += 8;
			}
		      if (geom->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
			    gaiaExport64 (ptr + 8, m, 1, endian_arch);	/* M */
			    ptr += 16;
			}
		  }
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      rng = polyg->Interiors + ib;
		      gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		      ptr += 4;
		      for (iv = 0; iv < rng->Points; iv++)
			{
			    if (geom->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
			      }
			    else if (geom->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
			      }
			    else if (geom->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (rng->Coords, iv, &x, &y,
						    &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (rng->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
			    gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
			    ptr += 16;
			    if (geom->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
				  ptr += 8;
			      }
			    if (geom->DimensionModel == GAIA_XY_M)
			      {
				  gaiaExport64 (ptr, m, 1, endian_arch);	/* M */
				  ptr += 8;
			      }
			    if (geom->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
				  gaiaExport64 (ptr + 8, m, 1, endian_arch);	/* M */
				  ptr += 16;
			      }
			}
		  }
		polyg = polyg->Next;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
      };
}

GAIAGEO_DECLARE void
gaiaToSpatiaLiteBlobWkb (gaiaGeomCollPtr geom, unsigned char **result,
			 int *size)
{
/* 
* builds the SpatiaLite BLOB representation for this GEOMETRY 
* convenience method - always disabling GPKG compatibility Modes
*/
    gaiaToSpatiaLiteBlobWkbEx (geom, result, size, 0);
}

GAIAGEO_DECLARE void
gaiaToCompressedBlobWkb (gaiaGeomCollPtr geom, unsigned char **result,
			 int *size)
{
/* 
/ builds the SpatiaLite BLOB representation for this GEOMETRY 
/ geometry-compression will be applied to LINESTRINGs and RINGs
*/
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    double last_x = 0.0;
    double last_y = 0.0;
    double last_z = 0.0;
    float fx;
    float fy;
    float fz;
    int entities = 0;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    int type;
    unsigned char *ptr;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    int endian_arch = gaiaEndianArch ();
    gaiaMbrGeometry (geom);
/* how many entities, and of what kind, do we have ? */
    pt = geom->FirstPoint;
    while (pt)
      {
	  point = pt;
	  entities++;
	  n_points++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  line = ln;
	  entities++;
	  n_linestrings++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  polyg = pg;
	  entities++;
	  n_polygons++;
	  pg = pg->Next;
      }
    *size = 0;
    *result = NULL;
    if (n_points == 0 && n_polygons == 0 && n_linestrings == 0)
	return;
/* ok, we can determine the geometry class */
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOINTZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOINTM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOINTZM;
		else
		    type = GAIA_MULTIPOINT;
	    }
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_POINTZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_POINTM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_POINTZM;
		else
		    type = GAIA_POINT;
	    }
      }
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOINTZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOINTM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOINTZM;
		else
		    type = GAIA_MULTIPOINT;
	    }
      }
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTILINESTRINGZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTILINESTRINGM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTILINESTRINGZM;
		else
		    type = GAIA_MULTILINESTRING;
	    }
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_LINESTRINGZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_LINESTRINGM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_LINESTRINGZM;
		else
		    type = GAIA_LINESTRING;
	    }
      }
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTILINESTRINGZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTILINESTRINGM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTILINESTRINGZM;
		else
		    type = GAIA_MULTILINESTRING;
	    }
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOLYGONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOLYGONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOLYGONZM;
		else
		    type = GAIA_MULTIPOLYGON;
	    }
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_POLYGONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_POLYGONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_POLYGONZM;
		else
		    type = GAIA_POLYGON;
	    }
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOLYGONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOLYGONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOLYGONZM;
		else
		    type = GAIA_MULTIPOLYGON;
	    }
      }
    else
      {
	  if (geom->DimensionModel == GAIA_XY_Z)
	      type = GAIA_GEOMETRYCOLLECTIONZ;
	  else if (geom->DimensionModel == GAIA_XY_M)
	      type = GAIA_GEOMETRYCOLLECTIONM;
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	      type = GAIA_GEOMETRYCOLLECTIONZM;
	  else
	      type = GAIA_GEOMETRYCOLLECTION;
      }
/* and now we compute the size of BLOB */
    *size = 44;			/* header size */
    switch (type)
      {
      case GAIA_POINT:
	  *size += (sizeof (double) * 2);	/* [x,y] coords */
	  break;
      case GAIA_POINTZ:
	  *size += (sizeof (double) * 3);	/* [x,y,z] coords */
	  break;
      case GAIA_POINTM:
	  *size += (sizeof (double) * 3);	/* [x,y,m] coords */
	  break;
      case GAIA_POINTZM:
	  *size += (sizeof (double) * 4);	/* [x,y,z,m] coords */
	  break;
      case GAIA_LINESTRING:
	  *size += (4 + (8 * line->Points) + 16);	/* # points + [x,y] for each vertex */
	  break;
      case GAIA_LINESTRINGZ:
	  *size += (4 + (12 * line->Points) + 24);	/* # points + [x,y,z] for each vertex */
	  break;
      case GAIA_LINESTRINGM:
	  *size += (4 + (16 * line->Points) + 16);	/* # points + [x,y,m] for each vertex */
	  break;
      case GAIA_LINESTRINGZM:
	  *size += (4 + (20 * line->Points) + 24);	/* # points + [x,y,z,m] for each vertex */
	  break;
      case GAIA_POLYGON:
	  rng = polyg->Exterior;
	  *size += (8 + (8 * rng->Points) + 16);	/* # rings + # points + [x.y] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + (8 * rng->Points) + 16);	/* # points + [x,y] array - interior ring */
	    }
	  break;
      case GAIA_POLYGONZ:
	  rng = polyg->Exterior;
	  *size += (8 + (12 * rng->Points) + 24);	/* # rings + # points + [x,y,z] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + (12 * rng->Points) + 24);	/* # points + [x,y,z] array - interior ring */
	    }
	  break;
      case GAIA_POLYGONM:
	  rng = polyg->Exterior;
	  *size += (8 + (16 * rng->Points) + 16);	/* # rings + # points + [x,y,m] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + (16 * rng->Points) + 16);	/* # points + [x,y,m] array - interior ring */
	    }
	  break;
      case GAIA_POLYGONZM:
	  rng = polyg->Exterior;
	  *size += (8 + (20 * rng->Points) + 24);	/* # rings + # points + [x,y,z,m] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + (20 * rng->Points) + 24);	/* # points + [x,y,z,m] array - interior ring */
	    }
	  break;
      default:
	  /* this one is not a simple geometry; should be a MULTIxxxx or a GEOMETRYCOLLECTION */
	  *size += 4;		/* # entities */
	  point = geom->FirstPoint;
	  while (point)
	    {
		*size += 5;	/* entity header */
		if (geom->DimensionModel == GAIA_XY_Z
		    || geom->DimensionModel == GAIA_XY_M)
		    *size += (sizeof (double) * 3);	/* three doubles for each POINT */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    *size += (sizeof (double) * 4);	/* four doubles for each POINT */
		else
		    *size += (sizeof (double) * 2);	/* two doubles for each POINT */
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		*size += 5;	/* entity header */
		if (geom->DimensionModel == GAIA_XY_Z)
		    *size += (4 + (12 * line->Points) + 24);	/* # points + [x,y,z] for each vertex */
		else if (geom->DimensionModel == GAIA_XY_M)
		    *size += (4 + (16 * line->Points) + 16);	/* # points + [x,y,m] for each vertex */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    *size += (4 + (20 * line->Points) + 24);	/* # points + [x,y,z,m] for each vertex */
		else
		    *size += (4 + (8 * line->Points) + 16);	/* # points + [x,y] for each vertex */
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		*size += 5;	/* entity header */
		rng = polyg->Exterior;
		if (geom->DimensionModel == GAIA_XY_Z)
		    *size += (8 + (12 * rng->Points) + 24);	/* # rings + # points + [x,y,z] array - exterior ring */
		else if (geom->DimensionModel == GAIA_XY_M)
		    *size += (8 + (16 * rng->Points) + 16);	/* # rings + # points + [x,y,m] array - exterior ring */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    *size += (8 + (20 * rng->Points) + 24);	/* # rings + # points + [x,y,z,m] array - exterior ring */
		else
		    *size += (8 + (8 * rng->Points) + 16);	/* # rings + # points + [x,y] array - exterior ring */
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      rng = polyg->Interiors + ib;
		      if (geom->DimensionModel == GAIA_XY_Z)
			  *size += (4 + (12 * rng->Points) + 24);	/* # points + [x,y,z] array - interior ring */
		      else if (geom->DimensionModel == GAIA_XY_M)
			  *size += (4 + (16 * rng->Points) + 16);	/* # points + [x,y,m] array - interior ring */
		      else if (geom->DimensionModel == GAIA_XY_Z_M)
			  *size += (4 + (20 * rng->Points) + 24);	/* # points + [x,y,z,m] array - interior ring */
		      else
			  *size += (4 + (8 * rng->Points) + 16);	/* # points + [x,y] array - interior ring */
		  }
		polyg = polyg->Next;
	    }
      };
    *result = malloc (*size);
    ptr = *result;
/* and finally we build the BLOB */
    switch (type)
      {
      case GAIA_POINT:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POINT, 1, endian_arch);	/* class POINT */
	  gaiaExport64 (ptr + 43, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 51, point->Y, 1, endian_arch);	/* Y */
	  *(ptr + 59) = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POINTZ:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POINTZ, 1, endian_arch);	/* class POINT XYZ */
	  gaiaExport64 (ptr + 43, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 51, point->Y, 1, endian_arch);	/* Y */
	  gaiaExport64 (ptr + 59, point->Z, 1, endian_arch);	/* Z */
	  *(ptr + 67) = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POINTM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POINTM, 1, endian_arch);	/* class POINT XYM */
	  gaiaExport64 (ptr + 43, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 51, point->Y, 1, endian_arch);	/* Y */
	  gaiaExport64 (ptr + 59, point->M, 1, endian_arch);	/* M */
	  *(ptr + 67) = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POINTZM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POINTZM, 1, endian_arch);	/* class POINT XYZM */
	  gaiaExport64 (ptr + 43, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 51, point->Y, 1, endian_arch);	/* Y */
	  gaiaExport64 (ptr + 59, point->Z, 1, endian_arch);	/* M */
	  gaiaExport64 (ptr + 67, point->M, 1, endian_arch);	/* Z */
	  *(ptr + 75) = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_LINESTRING:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_COMPRESSED_LINESTRING, 1, endian_arch);	/* class LINESTRING */
	  gaiaExport32 (ptr + 43, line->Points, 1, endian_arch);	/* # points */
	  ptr += 47;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		if (iv == 0 || iv == (line->Points - 1))
		  {
		      /* first and last vertices are uncompressed */
		      gaiaExport64 (ptr, x, 1, endian_arch);
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);
		      ptr += 16;
		  }
		else
		  {
		      /* compressing any other intermediate vertex */
		      fx = (float) (x - last_x);
		      fy = (float) (y - last_y);
		      gaiaExportF32 (ptr, fx, 1, endian_arch);
		      gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
		      ptr += 8;
		  }
		last_x = x;
		last_y = y;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_LINESTRINGZ:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_COMPRESSED_LINESTRINGZ, 1, endian_arch);	/* class LINESTRING XYZ */
	  gaiaExport32 (ptr + 43, line->Points, 1, endian_arch);	/* # points */
	  ptr += 47;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		if (iv == 0 || iv == (line->Points - 1))
		  {
		      /* first and last vertices are uncompressed */
		      gaiaExport64 (ptr, x, 1, endian_arch);
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);
		      gaiaExport64 (ptr + 16, z, 1, endian_arch);
		      ptr += 24;
		  }
		else
		  {
		      /* compressing any other intermediate vertex */
		      fx = (float) (x - last_x);
		      fy = (float) (y - last_y);
		      fz = (float) (z - last_z);
		      gaiaExportF32 (ptr, fx, 1, endian_arch);
		      gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
		      gaiaExportF32 (ptr + 8, fz, 1, endian_arch);
		      ptr += 12;
		  }
		last_x = x;
		last_y = y;
		last_z = z;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_LINESTRINGM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_COMPRESSED_LINESTRINGM, 1, endian_arch);	/* class LINESTRING XYM */
	  gaiaExport32 (ptr + 43, line->Points, 1, endian_arch);	/* # points */
	  ptr += 47;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		if (iv == 0 || iv == (line->Points - 1))
		  {
		      /* first and last vertices are uncompressed */
		      gaiaExport64 (ptr, x, 1, endian_arch);
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);
		      gaiaExport64 (ptr + 16, m, 1, endian_arch);
		      ptr += 24;
		  }
		else
		  {
		      /* compressing any other intermediate vertex */
		      fx = (float) (x - last_x);
		      fy = (float) (y - last_y);
		      gaiaExportF32 (ptr, fx, 1, endian_arch);
		      gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
		      gaiaExport64 (ptr + 8, m, 1, endian_arch);
		      ptr += 16;
		  }
		last_x = x;
		last_y = y;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_LINESTRINGZM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_COMPRESSED_LINESTRINGZM, 1, endian_arch);	/* class LINESTRING XYZM */
	  gaiaExport32 (ptr + 43, line->Points, 1, endian_arch);	/* # points */
	  ptr += 47;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		if (iv == 0 || iv == (line->Points - 1))
		  {
		      /* first and last vertices are uncompressed */
		      gaiaExport64 (ptr, x, 1, endian_arch);
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);
		      gaiaExport64 (ptr + 16, z, 1, endian_arch);
		      gaiaExport64 (ptr + 24, m, 1, endian_arch);
		      ptr += 32;
		  }
		else
		  {
		      /* compressing any other intermediate vertex */
		      fx = (float) (x - last_x);
		      fy = (float) (y - last_y);
		      fz = (float) (z - last_z);
		      gaiaExportF32 (ptr, fx, 1, endian_arch);
		      gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
		      gaiaExportF32 (ptr + 8, fz, 1, endian_arch);
		      gaiaExport64 (ptr + 12, m, 1, endian_arch);
		      ptr += 20;
		  }
		last_x = x;
		last_y = y;
		last_z = z;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POLYGON:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_COMPRESSED_POLYGON, 1, endian_arch);	/* class POLYGON */
	  gaiaExport32 (ptr + 43, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 47, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 51;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPoint (rng->Coords, iv, &x, &y);
		if (iv == 0 || iv == (rng->Points - 1))
		  {
		      /* first and last vertices are uncompressed */
		      gaiaExport64 (ptr, x, 1, endian_arch);
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);
		      ptr += 16;
		  }
		else
		  {
		      /* compressing any other intermediate vertex */
		      fx = (float) (x - last_x);
		      fy = (float) (y - last_y);
		      gaiaExportF32 (ptr, fx, 1, endian_arch);
		      gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
		      ptr += 8;
		  }
		last_x = x;
		last_y = y;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      if (iv == 0 || iv == (rng->Points - 1))
			{
			    /* first and last vertices are uncompressed */
			    gaiaExport64 (ptr, x, 1, endian_arch);
			    gaiaExport64 (ptr + 8, y, 1, endian_arch);
			    ptr += 16;
			}
		      else
			{
			    /* compressing any other intermediate vertex */
			    fx = (float) (x - last_x);
			    fy = (float) (y - last_y);
			    gaiaExportF32 (ptr, fx, 1, endian_arch);
			    gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
			    ptr += 8;
			}
		      last_x = x;
		      last_y = y;
		  }
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POLYGONZ:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_COMPRESSED_POLYGONZ, 1, endian_arch);	/* class POLYGON XYZ */
	  gaiaExport32 (ptr + 43, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 47, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 51;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		if (iv == 0 || iv == (rng->Points - 1))
		  {
		      /* first and last vertices are uncompressed */
		      gaiaExport64 (ptr, x, 1, endian_arch);
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);
		      gaiaExport64 (ptr + 16, z, 1, endian_arch);
		      ptr += 24;
		  }
		else
		  {
		      /* compressing any other intermediate vertex */
		      fx = (float) (x - last_x);
		      fy = (float) (y - last_y);
		      fz = (float) (z - last_z);
		      gaiaExportF32 (ptr, fx, 1, endian_arch);
		      gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
		      gaiaExportF32 (ptr + 8, fz, 1, endian_arch);
		      ptr += 12;
		  }
		last_x = x;
		last_y = y;
		last_z = z;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		      if (iv == 0 || iv == (rng->Points - 1))
			{
			    /* first and last vertices are uncompressed */
			    gaiaExport64 (ptr, x, 1, endian_arch);
			    gaiaExport64 (ptr + 8, y, 1, endian_arch);
			    gaiaExport64 (ptr + 16, z, 1, endian_arch);
			    ptr += 24;
			}
		      else
			{
			    /* compressing any other intermediate vertex */
			    fx = (float) (x - last_x);
			    fy = (float) (y - last_y);
			    fz = (float) (z - last_z);
			    gaiaExportF32 (ptr, fx, 1, endian_arch);
			    gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
			    gaiaExportF32 (ptr + 8, fz, 1, endian_arch);
			    ptr += 12;
			}
		      last_x = x;
		      last_y = y;
		      last_z = z;
		  }
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POLYGONM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_COMPRESSED_POLYGONM, 1, endian_arch);	/* class POLYGON XYM */
	  gaiaExport32 (ptr + 43, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 47, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 51;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		if (iv == 0 || iv == (rng->Points - 1))
		  {
		      /* first and last vertices are uncompressed */
		      gaiaExport64 (ptr, x, 1, endian_arch);
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);
		      gaiaExport64 (ptr + 16, m, 1, endian_arch);
		      ptr += 24;
		  }
		else
		  {
		      /* compressing any other intermediate vertex */
		      fx = (float) (x - last_x);
		      fy = (float) (y - last_y);
		      gaiaExportF32 (ptr, fx, 1, endian_arch);
		      gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
		      gaiaExport64 (ptr + 8, m, 1, endian_arch);
		      ptr += 16;
		  }
		last_x = x;
		last_y = y;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		      if (iv == 0 || iv == (rng->Points - 1))
			{
			    /* first and last vertices are uncompressed */
			    gaiaExport64 (ptr, x, 1, endian_arch);
			    gaiaExport64 (ptr + 8, y, 1, endian_arch);
			    gaiaExport64 (ptr + 16, m, 1, endian_arch);
			    ptr += 24;
			}
		      else
			{
			    /* compressing any other intermediate vertex */
			    fx = (float) (x - last_x);
			    fy = (float) (y - last_y);
			    gaiaExportF32 (ptr, fx, 1, endian_arch);
			    gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
			    gaiaExport64 (ptr + 8, m, 1, endian_arch);
			    ptr += 16;
			}
		      last_x = x;
		      last_y = y;
		  }
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      case GAIA_POLYGONZM:
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_COMPRESSED_POLYGONZM, 1, endian_arch);	/* class POLYGON */
	  gaiaExport32 (ptr + 43, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 47, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 51;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		if (iv == 0 || iv == (rng->Points - 1))
		  {
		      /* first and last vertices are uncompressed */
		      gaiaExport64 (ptr, x, 1, endian_arch);
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);
		      gaiaExport64 (ptr + 16, z, 1, endian_arch);
		      gaiaExport64 (ptr + 24, m, 1, endian_arch);
		      ptr += 32;
		  }
		else
		  {
		      /* compressing any other intermediate vertex */
		      fx = (float) (x - last_x);
		      fy = (float) (y - last_y);
		      fz = (float) (z - last_z);
		      gaiaExportF32 (ptr, fx, 1, endian_arch);
		      gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
		      gaiaExportF32 (ptr + 8, fz, 1, endian_arch);
		      gaiaExport64 (ptr + 12, m, 1, endian_arch);
		      ptr += 20;
		  }
		last_x = x;
		last_y = y;
		last_z = z;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		      if (iv == 0 || iv == (rng->Points - 1))
			{
			    /* first and last vertices are uncompressed */
			    gaiaExport64 (ptr, x, 1, endian_arch);
			    gaiaExport64 (ptr + 8, y, 1, endian_arch);
			    gaiaExport64 (ptr + 16, z, 1, endian_arch);
			    gaiaExport64 (ptr + 24, m, 1, endian_arch);
			    ptr += 32;
			}
		      else
			{
			    /* compressing any other intermediate vertex */
			    fx = (float) (x - last_x);
			    fy = (float) (y - last_y);
			    fz = (float) (z - last_z);
			    gaiaExportF32 (ptr, fx, 1, endian_arch);
			    gaiaExportF32 (ptr + 4, fy, 1, endian_arch);
			    gaiaExportF32 (ptr + 8, fz, 1, endian_arch);
			    gaiaExport64 (ptr + 12, m, 1, endian_arch);
			    ptr += 20;
			}
		      last_x = x;
		      last_y = y;
		      last_z = z;
		  }
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
	  break;
      default:
	  /* this one is a MULTIxxxx or a GEOMETRYCOLLECTION - building the main header */
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimum X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimum Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximum X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximum Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, type, 1, endian_arch);	/* geometric class */
	  gaiaExport32 (ptr + 43, entities, 1, endian_arch);	/* # entities */
	  ptr += 47;
	  point = geom->FirstPoint;
	  while (point)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		if (geom->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaExport32 (ptr + 1, GAIA_POINTZ, 1, endian_arch);	/* class POINT XYZ */
		      gaiaExport64 (ptr + 5, point->X, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 13, point->Y, 1, endian_arch);	/* Y */
		      gaiaExport64 (ptr + 21, point->Z, 1, endian_arch);	/* Z */
		      ptr += 29;
		  }
		else if (geom->DimensionModel == GAIA_XY_M)
		  {
		      gaiaExport32 (ptr + 1, GAIA_POINTM, 1, endian_arch);	/* class POINT XYM */
		      gaiaExport64 (ptr + 5, point->X, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 13, point->Y, 1, endian_arch);	/* Y */
		      gaiaExport64 (ptr + 21, point->M, 1, endian_arch);	/* M */
		      ptr += 29;
		  }
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaExport32 (ptr + 1, GAIA_POINTZM, 1, endian_arch);	/* class POINT XYZM */
		      gaiaExport64 (ptr + 5, point->X, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 13, point->Y, 1, endian_arch);	/* Y */
		      gaiaExport64 (ptr + 21, point->Z, 1, endian_arch);	/* Z */
		      gaiaExport64 (ptr + 29, point->M, 1, endian_arch);	/* M */
		      ptr += 37;
		  }
		else
		  {
		      gaiaExport32 (ptr + 1, GAIA_POINT, 1, endian_arch);	/* class POINT */
		      gaiaExport64 (ptr + 5, point->X, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 13, point->Y, 1, endian_arch);	/* Y */
		      ptr += 21;
		  }
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		if (geom->DimensionModel == GAIA_XY_Z)
		    gaiaExport32 (ptr + 1, GAIA_COMPRESSED_LINESTRINGZ, 1, endian_arch);	/* class LINESTRING XYZ */
		else if (geom->DimensionModel == GAIA_XY_M)
		    gaiaExport32 (ptr + 1, GAIA_COMPRESSED_LINESTRINGM, 1, endian_arch);	/* class LINESTRING XYM */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    gaiaExport32 (ptr + 1, GAIA_COMPRESSED_LINESTRINGZM, 1, endian_arch);	/* class LINESTRING XYZM */
		else
		    gaiaExport32 (ptr + 1, GAIA_COMPRESSED_LINESTRING, 1, endian_arch);	/* class LINESTRING */
		gaiaExport32 (ptr + 5, line->Points, 1, endian_arch);	/* # points */
		ptr += 9;
		for (iv = 0; iv < line->Points; iv++)
		  {
		      z = 0.0;
		      m = 0.0;
		      if (geom->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
			}
		      else if (geom->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
			}
		      else if (geom->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (line->Coords, iv, &x, &y);
			}
		      if (iv == 0 || iv == (line->Points - 1))
			{
			    /* first and last vertices are uncompressed */
			    gaiaExport64 (ptr, x, 1, endian_arch);	/* X */
			    gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y */
			    ptr += 16;
			}
		      else
			{
			    /* compressing any other intermediate vertex */
			    fx = (float) (x - last_x);
			    fy = (float) (y - last_y);
			    gaiaExportF32 (ptr, fx, 1, endian_arch);	/* X */
			    gaiaExportF32 (ptr + 4, fy, 1, endian_arch);	/* Y */
			    ptr += 8;
			}
		      if (geom->DimensionModel == GAIA_XY_Z)
			{
			    if (iv == 0 || iv == (line->Points - 1))
			      {
				  /* first and last vertices are uncompressed */
				  gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
				  ptr += 8;
			      }
			    else
			      {
				  /* compressing any other intermediate vertex */
				  fz = (float) (z - last_z);
				  gaiaExportF32 (ptr, fz, 1, endian_arch);	/* Z */
				  ptr += 4;
			      }
			}
		      if (geom->DimensionModel == GAIA_XY_M)
			{
			    gaiaExport64 (ptr, m, 1, endian_arch);	/* M */
			    ptr += 8;
			}
		      if (geom->DimensionModel == GAIA_XY_Z_M)
			{
			    if (iv == 0 || iv == (line->Points - 1))
			      {
				  /* first and last vertices are uncompressed */
				  gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
				  ptr += 8;
			      }
			    else
			      {
				  /* compressing any other intermediate vertex */
				  fz = (float) (z - last_z);
				  gaiaExportF32 (ptr, fz, 1, endian_arch);	/* Z */
				  ptr += 4;
			      }
			    gaiaExport64 (ptr, m, 1, endian_arch);	/* M */
			    ptr += 8;
			}
		      last_x = x;
		      last_y = y;
		      last_z = z;
		  }
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		if (geom->DimensionModel == GAIA_XY_Z)
		    gaiaExport32 (ptr + 1, GAIA_COMPRESSED_POLYGONZ, 1, endian_arch);	/* class POLYGON XYZ */
		else if (geom->DimensionModel == GAIA_XY_M)
		    gaiaExport32 (ptr + 1, GAIA_COMPRESSED_POLYGONM, 1, endian_arch);	/* class POLYGON XYM */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    gaiaExport32 (ptr + 1, GAIA_COMPRESSED_POLYGONZM, 1, endian_arch);	/* class POLYGON XYZM */
		else
		    gaiaExport32 (ptr + 1, GAIA_COMPRESSED_POLYGON, 1, endian_arch);	/* class POLYGON */
		gaiaExport32 (ptr + 5, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
		rng = polyg->Exterior;
		gaiaExport32 (ptr + 9, rng->Points, 1, endian_arch);	/* # points - exterior ring */
		ptr += 13;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      z = 0.0;
		      m = 0.0;
		      if (geom->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
			}
		      else if (geom->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
			}
		      else if (geom->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (rng->Coords, iv, &x, &y);
			}
		      if (iv == 0 || iv == (rng->Points - 1))
			{
			    /* first and last vertices are uncompressed */
			    gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
			    gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
			    ptr += 16;
			}
		      else
			{
			    /* compressing any other intermediate vertex */
			    fx = (float) (x - last_x);
			    fy = (float) (y - last_y);
			    gaiaExportF32 (ptr, fx, 1, endian_arch);	/* X */
			    gaiaExportF32 (ptr + 4, fy, 1, endian_arch);	/* Y */
			    ptr += 8;
			}
		      if (geom->DimensionModel == GAIA_XY_Z)
			{
			    if (iv == 0 || iv == (rng->Points - 1))
			      {
				  /* first and last vertices are uncompressed */
				  gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
				  ptr += 8;
			      }
			    else
			      {
				  /* compressing any other intermediate vertex */
				  fz = (float) (z - last_z);
				  gaiaExportF32 (ptr, fz, 1, endian_arch);	/* Z */
				  ptr += 4;
			      }
			}
		      if (geom->DimensionModel == GAIA_XY_M)
			{
			    gaiaExport64 (ptr, m, 1, endian_arch);	/* M */
			    ptr += 8;
			}
		      if (geom->DimensionModel == GAIA_XY_Z_M)
			{
			    if (iv == 0 || iv == (rng->Points - 1))
			      {
				  /* first and last vertices are uncompressed */
				  gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
				  ptr += 8;
			      }
			    else
			      {
				  /* compressing any other intermediate vertex */
				  fz = (float) (z - last_z);
				  gaiaExportF32 (ptr, fz, 1, endian_arch);	/* Z */
				  ptr += 4;
			      }
			    gaiaExport64 (ptr, m, 1, endian_arch);	/* M */
			    ptr += 8;
			}
		      last_x = x;
		      last_y = y;
		      last_z = z;
		  }
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      rng = polyg->Interiors + ib;
		      gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		      ptr += 4;
		      for (iv = 0; iv < rng->Points; iv++)
			{
			    z = 0.0;
			    m = 0.0;
			    if (geom->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
			      }
			    else if (geom->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
			      }
			    else if (geom->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z,
						    &m);
			      }
			    else
			      {
				  gaiaGetPoint (rng->Coords, iv, &x, &y);
			      }
			    if (iv == 0 || iv == (rng->Points - 1))
			      {
				  /* first and last vertices are uncompressed */
				  gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
				  gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
				  ptr += 16;
			      }
			    else
			      {
				  /* compressing any other intermediate vertex */
				  fx = (float) (x - last_x);
				  fy = (float) (y - last_y);
				  gaiaExportF32 (ptr, fx, 1, endian_arch);	/* X */
				  gaiaExportF32 (ptr + 4, fy, 1, endian_arch);	/* Y */
				  ptr += 8;
			      }
			    if (geom->DimensionModel == GAIA_XY_Z)
			      {
				  if (iv == 0 || iv == (rng->Points - 1))
				    {
					/* first and last vertices are uncompressed */
					gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
					ptr += 8;
				    }
				  else
				    {
					/* compressing any other intermediate vertex */
					fz = (float) (z - last_z);
					gaiaExportF32 (ptr, fz, 1, endian_arch);	/* Z */
					ptr += 4;
				    }
			      }
			    if (geom->DimensionModel == GAIA_XY_M)
			      {
				  gaiaExport64 (ptr, m, 1, endian_arch);	/* M */
				  ptr += 8;
			      }
			    if (geom->DimensionModel == GAIA_XY_Z_M)
			      {
				  if (iv == 0 || iv == (rng->Points - 1))
				    {
					/* first and last vertices are uncompressed */
					gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
					ptr += 8;
				    }
				  else
				    {
					/* compressing any other intermediate vertex */
					fz = (float) (z - last_z);
					gaiaExportF32 (ptr, fz, 1, endian_arch);	/* Z */
					ptr += 4;
				    }
				  gaiaExport64 (ptr, m, 1, endian_arch);	/* M */
				  ptr += 8;
			      }
			    last_x = x;
			    last_y = y;
			    last_z = z;
			}
		  }
		polyg = polyg->Next;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
      };
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromWkb (const unsigned char *blob, unsigned int size)
{
/* decoding from WKB to GEOMETRY  */
    int type;
    int little_endian;
    gaiaGeomCollPtr geo = NULL;
    int endian_arch = gaiaEndianArch ();
    if (size < 5)
	return NULL;
    if (*(blob + 0) == 0x01)
	little_endian = GAIA_LITTLE_ENDIAN;
    else
	little_endian = GAIA_BIG_ENDIAN;
    type = gaiaImport32 (blob + 1, little_endian, endian_arch);
    if (type == GAIA_POINTZ || type == GAIA_LINESTRINGZ || type == GAIA_POLYGONZ
	|| type == GAIA_MULTIPOINTZ || type == GAIA_MULTILINESTRINGZ
	|| type == GAIA_MULTIPOLYGONZ || type == GAIA_GEOMETRYCOLLECTIONZ
	|| type == GAIA_GEOSWKB_POINTZ || type == GAIA_GEOSWKB_LINESTRINGZ
	|| type == GAIA_GEOSWKB_POLYGONZ || type == GAIA_GEOSWKB_MULTIPOINTZ
	|| type == GAIA_GEOSWKB_MULTILINESTRINGZ
	|| type == GAIA_GEOSWKB_MULTIPOLYGONZ
	|| type == GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ)
	geo = gaiaAllocGeomCollXYZ ();
    else if (type == GAIA_POINTM || type == GAIA_LINESTRINGM
	     || type == GAIA_POLYGONM || type == GAIA_MULTIPOINTM
	     || type == GAIA_MULTILINESTRINGM || type == GAIA_MULTIPOLYGONM
	     || type == GAIA_GEOMETRYCOLLECTIONM)
	geo = gaiaAllocGeomCollXYM ();
    else if (type == GAIA_POINTZM || type == GAIA_LINESTRINGZM
	     || type == GAIA_POLYGONZM || type == GAIA_MULTIPOINTZM
	     || type == GAIA_MULTILINESTRINGZM || type == GAIA_MULTIPOLYGONZM
	     || type == GAIA_GEOMETRYCOLLECTIONZM)
	geo = gaiaAllocGeomCollXYZM ();
    else
	geo = gaiaAllocGeomColl ();
    geo->Srid = 0;
    geo->endian_arch = (char) endian_arch;
    geo->endian = (char) little_endian;
    geo->blob = blob;
    geo->size = size;
    geo->offset = 5;
    switch (type)
      {
      case GAIA_POINT:
	  ParseWkbPoint (geo);
	  break;
      case GAIA_POINTZ:
      case GAIA_GEOSWKB_POINTZ:
	  ParseWkbPointZ (geo);
	  break;
      case GAIA_POINTM:
	  ParseWkbPointM (geo);
	  break;
      case GAIA_POINTZM:
	  ParseWkbPointZM (geo);
	  break;
      case GAIA_LINESTRING:
	  ParseWkbLine (geo);
	  break;
      case GAIA_LINESTRINGZ:
      case GAIA_GEOSWKB_LINESTRINGZ:
	  ParseWkbLineZ (geo);
	  break;
      case GAIA_LINESTRINGM:
	  ParseWkbLineM (geo);
	  break;
      case GAIA_LINESTRINGZM:
	  ParseWkbLineZM (geo);
	  break;
      case GAIA_POLYGON:
	  ParseWkbPolygon (geo);
	  break;
      case GAIA_POLYGONZ:
      case GAIA_GEOSWKB_POLYGONZ:
	  ParseWkbPolygonZ (geo);
	  break;
      case GAIA_POLYGONM:
	  ParseWkbPolygonM (geo);
	  break;
      case GAIA_POLYGONZM:
	  ParseWkbPolygonZM (geo);
	  break;
      case GAIA_MULTIPOINT:
      case GAIA_MULTILINESTRING:
      case GAIA_MULTIPOLYGON:
      case GAIA_GEOMETRYCOLLECTION:
      case GAIA_MULTIPOINTZ:
      case GAIA_MULTILINESTRINGZ:
      case GAIA_MULTIPOLYGONZ:
      case GAIA_GEOMETRYCOLLECTIONZ:
      case GAIA_GEOSWKB_MULTIPOINTZ:
      case GAIA_GEOSWKB_MULTILINESTRINGZ:
      case GAIA_GEOSWKB_MULTIPOLYGONZ:
      case GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ:
      case GAIA_MULTIPOINTM:
      case GAIA_MULTILINESTRINGM:
      case GAIA_MULTIPOLYGONM:
      case GAIA_GEOMETRYCOLLECTIONM:
      case GAIA_MULTIPOINTZM:
      case GAIA_MULTILINESTRINGZM:
      case GAIA_MULTIPOLYGONZM:
      case GAIA_GEOMETRYCOLLECTIONZM:
	  ParseWkbGeometry (geo, 1);
	  break;
      default:
	  break;
      };
    gaiaMbrGeometry (geo);
    switch (type)
      {
      case GAIA_POINT:
      case GAIA_POINTZ:
      case GAIA_GEOSWKB_POINTZ:
      case GAIA_POINTM:
      case GAIA_POINTZM:
	  geo->DeclaredType = GAIA_POINT;
	  break;
      case GAIA_LINESTRING:
      case GAIA_LINESTRINGZ:
      case GAIA_GEOSWKB_LINESTRINGZ:
      case GAIA_LINESTRINGM:
      case GAIA_LINESTRINGZM:
	  geo->DeclaredType = GAIA_LINESTRING;
	  break;
      case GAIA_POLYGON:
      case GAIA_POLYGONZ:
      case GAIA_GEOSWKB_POLYGONZ:
      case GAIA_POLYGONM:
      case GAIA_POLYGONZM:
	  geo->DeclaredType = GAIA_POLYGON;
	  break;
      case GAIA_MULTIPOINT:
      case GAIA_MULTIPOINTZ:
      case GAIA_GEOSWKB_MULTIPOINTZ:
      case GAIA_MULTIPOINTM:
      case GAIA_MULTIPOINTZM:
	  geo->DeclaredType = GAIA_MULTIPOINT;
	  break;
      case GAIA_MULTILINESTRING:
      case GAIA_MULTILINESTRINGZ:
      case GAIA_GEOSWKB_MULTILINESTRINGZ:
      case GAIA_MULTILINESTRINGM:
      case GAIA_MULTILINESTRINGZM:
	  geo->DeclaredType = GAIA_MULTILINESTRING;
	  break;
      case GAIA_MULTIPOLYGON:
      case GAIA_MULTIPOLYGONZ:
      case GAIA_GEOSWKB_MULTIPOLYGONZ:
      case GAIA_MULTIPOLYGONM:
      case GAIA_MULTIPOLYGONZM:
	  geo->DeclaredType = GAIA_MULTIPOLYGON;
	  break;
      case GAIA_GEOMETRYCOLLECTION:
      case GAIA_GEOMETRYCOLLECTIONZ:
      case GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ:
      case GAIA_GEOMETRYCOLLECTIONM:
      case GAIA_GEOMETRYCOLLECTIONZM:
	  geo->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  break;
      }
    return geo;
}

GAIAGEO_DECLARE char *
gaiaToHexWkb (gaiaGeomCollPtr geom)
{
/* builds the hexadecimal WKB representation for this GEOMETRY */
    unsigned char *wkb = NULL;
    int size = 0;
    char *hexbuf = NULL;
    int i;
    char hex[16];
    char *p;
    gaiaToWkb (geom, &wkb, &size);
    if (!wkb)
	return NULL;
    hexbuf = malloc ((size * 2) + 1);
    p = hexbuf;
    for (i = 0; i < size; i++)
      {
	  sprintf (hex, "%02X", *(wkb + i));
	  *p++ = hex[0];
	  *p++ = hex[1];
      }
    *p = '\0';
    return hexbuf;
}

GAIAGEO_DECLARE void
gaiaToWkb (gaiaGeomCollPtr geom, unsigned char **result, int *size)
{
/* builds the WKB representation for this GEOMETRY */
    int ib;
    int iv;
    double x;
    double y;
    double z = 0.0;
    double m = 0.0;
    int entities = 0;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    int type;
    unsigned char *ptr;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    int endian_arch = gaiaEndianArch ();
    gaiaMbrGeometry (geom);
/* how many entities, and of what kind, do we have ? */
    pt = geom->FirstPoint;
    while (pt)
      {
	  point = pt;
	  entities++;
	  n_points++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  line = ln;
	  entities++;
	  n_linestrings++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  polyg = pg;
	  entities++;
	  n_polygons++;
	  pg = pg->Next;
      }
    *size = 0;
    *result = NULL;
    if (n_points == 0 && n_polygons == 0 && n_linestrings == 0)
	return;
/* ok, we can determine the geometry class */
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOINTZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOINTM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOINTZM;
		else
		    type = GAIA_MULTIPOINT;
	    }
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_POINTZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_POINTM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_POINTZM;
		else
		    type = GAIA_POINT;
	    }
      }
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOINTZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOINTM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOINTZM;
		else
		    type = GAIA_MULTIPOINT;
	    }
      }
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTILINESTRINGZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTILINESTRINGM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTILINESTRINGZM;
		else
		    type = GAIA_MULTILINESTRING;
	    }
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_LINESTRINGZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_LINESTRINGM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_LINESTRINGZM;
		else
		    type = GAIA_LINESTRING;
	    }
      }
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTILINESTRINGZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTILINESTRINGM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTILINESTRINGZM;
		else
		    type = GAIA_MULTILINESTRING;
	    }
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOLYGONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOLYGONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOLYGONZM;
		else
		    type = GAIA_MULTIPOLYGON;
	    }
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_POLYGONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_POLYGONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_POLYGONZM;
		else
		    type = GAIA_POLYGON;
	    }
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_GEOMETRYCOLLECTIONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_GEOMETRYCOLLECTIONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_GEOMETRYCOLLECTIONZM;
		else
		    type = GAIA_GEOMETRYCOLLECTION;
	    }
	  else
	    {
		if (geom->DimensionModel == GAIA_XY_Z)
		    type = GAIA_MULTIPOLYGONZ;
		else if (geom->DimensionModel == GAIA_XY_M)
		    type = GAIA_MULTIPOLYGONM;
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    type = GAIA_MULTIPOLYGONZM;
		else
		    type = GAIA_MULTIPOLYGON;
	    }
      }
    else
      {
	  if (geom->DimensionModel == GAIA_XY_Z)
	      type = GAIA_GEOMETRYCOLLECTIONZ;
	  else if (geom->DimensionModel == GAIA_XY_M)
	      type = GAIA_GEOMETRYCOLLECTIONM;
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	      type = GAIA_GEOMETRYCOLLECTIONZM;
	  else
	      type = GAIA_GEOMETRYCOLLECTION;
      }
/* and now we compute the size of WKB */
    *size = 5;			/* header size */
    if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION
	|| type == GAIA_MULTIPOINTZ || type == GAIA_MULTILINESTRINGZ
	|| type == GAIA_MULTIPOLYGONZ || type == GAIA_GEOMETRYCOLLECTIONZ
	|| type == GAIA_MULTIPOINTM || type == GAIA_MULTILINESTRINGM
	|| type == GAIA_MULTIPOLYGONM || type == GAIA_GEOMETRYCOLLECTIONM
	|| type == GAIA_MULTIPOINTZM || type == GAIA_MULTILINESTRINGZM
	|| type == GAIA_MULTIPOLYGONZM || type == GAIA_GEOMETRYCOLLECTIONZM)
	*size += 4;
    point = geom->FirstPoint;
    while (point)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	      || type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION
	      || type == GAIA_MULTIPOINTZ || type == GAIA_MULTILINESTRINGZ
	      || type == GAIA_MULTIPOLYGONZ || type == GAIA_GEOMETRYCOLLECTIONZ
	      || type == GAIA_MULTIPOINTM || type == GAIA_MULTILINESTRINGM
	      || type == GAIA_MULTIPOLYGONM || type == GAIA_GEOMETRYCOLLECTIONM
	      || type == GAIA_MULTIPOINTZM || type == GAIA_MULTILINESTRINGZM
	      || type == GAIA_MULTIPOLYGONZM
	      || type == GAIA_GEOMETRYCOLLECTIONZM)
	      *size += 5;
	  if (geom->DimensionModel == GAIA_XY_Z
	      || geom->DimensionModel == GAIA_XY_M)
	      *size += (sizeof (double) * 3);	/* three doubles for each POINT */
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	      *size += (sizeof (double) * 4);	/* four doubles for each POINT */
	  else
	      *size += (sizeof (double) * 2);	/* two doubles for each POINT */
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	      || type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION
	      || type == GAIA_MULTIPOINTZ || type == GAIA_MULTILINESTRINGZ
	      || type == GAIA_MULTIPOLYGONZ || type == GAIA_GEOMETRYCOLLECTIONZ
	      || type == GAIA_MULTIPOINTM || type == GAIA_MULTILINESTRINGM
	      || type == GAIA_MULTIPOLYGONM || type == GAIA_GEOMETRYCOLLECTIONM
	      || type == GAIA_MULTIPOINTZM || type == GAIA_MULTILINESTRINGZM
	      || type == GAIA_MULTIPOLYGONZM
	      || type == GAIA_GEOMETRYCOLLECTIONZM)
	      *size += 5;
	  if (geom->DimensionModel == GAIA_XY_Z
	      || geom->DimensionModel == GAIA_XY_M)
	      *size += (4 + ((sizeof (double) * 3) * line->Points));	/* # points + [x,y,z] for each vertex */
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	      *size += (4 + ((sizeof (double) * 4) * line->Points));	/* # points + [x,y,z,m] for each vertex */
	  else
	      *size += (4 + ((sizeof (double) * 2) * line->Points));	/* # points + [x,y] for each vertex */
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	      || type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION
	      || type == GAIA_MULTIPOINTZ || type == GAIA_MULTILINESTRINGZ
	      || type == GAIA_MULTIPOLYGONZ || type == GAIA_GEOMETRYCOLLECTIONZ
	      || type == GAIA_MULTIPOINTM || type == GAIA_MULTILINESTRINGM
	      || type == GAIA_MULTIPOLYGONM || type == GAIA_GEOMETRYCOLLECTIONM
	      || type == GAIA_MULTIPOINTZM || type == GAIA_MULTILINESTRINGZM
	      || type == GAIA_MULTIPOLYGONZM
	      || type == GAIA_GEOMETRYCOLLECTIONZM)
	      *size += 5;
	  rng = polyg->Exterior;
	  if (geom->DimensionModel == GAIA_XY_Z
	      || geom->DimensionModel == GAIA_XY_M)
	      *size += (8 + ((sizeof (double) * 3) * rng->Points));	/* # rings + # points + [x,y,z] array - exterior ring */
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	      *size += (8 + ((sizeof (double) * 4) * rng->Points));	/* # rings + # points + [x,y,z,m] array - exterior ring */
	  else
	      *size += (8 + ((sizeof (double) * 2) * rng->Points));	/* # rings + # points + [x,y] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		if (geom->DimensionModel == GAIA_XY_Z
		    || geom->DimensionModel == GAIA_XY_M)
		    *size += (4 + ((sizeof (double) * 3) * rng->Points));	/* # points + [x,y,z] array - interior ring */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    *size += (4 + ((sizeof (double) * 4) * rng->Points));	/* # points + [x,y,z,m] array - interior ring */
		else
		    *size += (4 + ((sizeof (double) * 2) * rng->Points));	/* # points + [x,y] array - interior ring */
	    }
	  polyg = polyg->Next;
      }
    *result = malloc (*size);
    ptr = *result;
/* and finally we build the WKB */
    *ptr = 0x01;		/* little endian byte order */
    gaiaExport32 (ptr + 1, type, 1, endian_arch);	/* the main CLASS TYPE */
    ptr += 5;
    if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION
	|| type == GAIA_MULTIPOINTZ || type == GAIA_MULTILINESTRINGZ
	|| type == GAIA_MULTIPOLYGONZ || type == GAIA_GEOMETRYCOLLECTIONZ
	|| type == GAIA_MULTIPOINTM || type == GAIA_MULTILINESTRINGM
	|| type == GAIA_MULTIPOLYGONM || type == GAIA_GEOMETRYCOLLECTIONM
	|| type == GAIA_MULTIPOINTZM || type == GAIA_MULTILINESTRINGZM
	|| type == GAIA_MULTIPOLYGONZM || type == GAIA_GEOMETRYCOLLECTIONZM)
      {
	  gaiaExport32 (ptr, entities, 1, endian_arch);	/* it's a collection; # entities */
	  ptr += 4;
      }
    point = geom->FirstPoint;
    while (point)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_GEOMETRYCOLLECTION
	      || type == GAIA_MULTIPOINTZ || type == GAIA_GEOMETRYCOLLECTIONZ
	      || type == GAIA_MULTIPOINTM || type == GAIA_GEOMETRYCOLLECTIONM
	      || type == GAIA_MULTIPOINTZM || type == GAIA_GEOMETRYCOLLECTIONZM)
	    {
		*ptr = 0x01;
		/* it's a collection: the CLASS TYPE for this element */
		if (type == GAIA_MULTIPOINTZ
		    || type == GAIA_GEOMETRYCOLLECTIONZ)
		    gaiaExport32 (ptr + 1, GAIA_POINTZ, 1, endian_arch);
		else if (type == GAIA_MULTIPOINTM
			 || type == GAIA_GEOMETRYCOLLECTIONM)
		    gaiaExport32 (ptr + 1, GAIA_POINTM, 1, endian_arch);
		else if (type == GAIA_MULTIPOINTZM
			 || type == GAIA_GEOMETRYCOLLECTIONZM)
		    gaiaExport32 (ptr + 1, GAIA_POINTZM, 1, endian_arch);
		else
		    gaiaExport32 (ptr + 1, GAIA_POINT, 1, endian_arch);
		ptr += 5;
	    }
	  gaiaExport64 (ptr, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 8, point->Y, 1, endian_arch);	/* Y */
	  ptr += 16;
	  if (type == GAIA_POINTZ || type == GAIA_MULTIPOINTZ
	      || type == GAIA_GEOMETRYCOLLECTIONZ)
	    {
		gaiaExport64 (ptr, point->Z, 1, endian_arch);	/* Z */
		ptr += 8;
	    }
	  if (type == GAIA_POINTM || type == GAIA_MULTIPOINTM
	      || type == GAIA_GEOMETRYCOLLECTIONM)
	    {
		gaiaExport64 (ptr, point->M, 1, endian_arch);	/* M */
		ptr += 8;
	    }
	  if (type == GAIA_POINTZM || type == GAIA_MULTIPOINTZM
	      || type == GAIA_GEOMETRYCOLLECTIONZM)
	    {
		gaiaExport64 (ptr, point->Z, 1, endian_arch);	/* Z */
		gaiaExport64 (ptr + 8, point->M, 1, endian_arch);	/* M */
		ptr += 16;
	    }
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  if (type == GAIA_MULTILINESTRING || type == GAIA_GEOMETRYCOLLECTION
	      || type == GAIA_MULTILINESTRINGZ
	      || type == GAIA_GEOMETRYCOLLECTIONZ
	      || type == GAIA_MULTILINESTRINGM
	      || type == GAIA_GEOMETRYCOLLECTIONM
	      || type == GAIA_MULTILINESTRINGZM
	      || type == GAIA_GEOMETRYCOLLECTIONZM)
	    {
		*ptr = 0x01;
		/* it's a collection: the CLASS TYPE for this element */
		if (type == GAIA_MULTILINESTRINGZ
		    || type == GAIA_GEOMETRYCOLLECTIONZ)
		    gaiaExport32 (ptr + 1, GAIA_LINESTRINGZ, 1, endian_arch);
		else if (type == GAIA_MULTILINESTRINGM
			 || type == GAIA_GEOMETRYCOLLECTIONM)
		    gaiaExport32 (ptr + 1, GAIA_LINESTRINGM, 1, endian_arch);
		else if (type == GAIA_MULTILINESTRINGZM
			 || type == GAIA_GEOMETRYCOLLECTIONZM)
		    gaiaExport32 (ptr + 1, GAIA_LINESTRINGZM, 1, endian_arch);
		else
		    gaiaExport32 (ptr + 1, GAIA_LINESTRING, 1, endian_arch);
		ptr += 5;
	    }
	  gaiaExport32 (ptr, line->Points, 1, endian_arch);	/* # points */
	  ptr += 4;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		if (type == GAIA_LINESTRINGZ || type == GAIA_MULTILINESTRINGZ
		    || type == GAIA_GEOMETRYCOLLECTIONZ)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (type == GAIA_LINESTRINGM
			 || type == GAIA_MULTILINESTRINGM
			 || type == GAIA_GEOMETRYCOLLECTIONM)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (type == GAIA_LINESTRINGZM
			 || type == GAIA_MULTILINESTRINGZM
			 || type == GAIA_GEOMETRYCOLLECTIONZM)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y */
		ptr += 16;
		if (type == GAIA_LINESTRINGZ || type == GAIA_MULTILINESTRINGZ
		    || type == GAIA_GEOMETRYCOLLECTIONZ)
		  {
		      gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
		      ptr += 8;
		  }
		if (type == GAIA_LINESTRINGM || type == GAIA_MULTILINESTRINGM
		    || type == GAIA_GEOMETRYCOLLECTIONM)
		  {
		      gaiaExport64 (ptr, m, 1, endian_arch);	/* M */
		      ptr += 8;
		  }
		if (type == GAIA_LINESTRINGZM || type == GAIA_MULTILINESTRINGZM
		    || type == GAIA_GEOMETRYCOLLECTIONZM)
		  {
		      gaiaExport64 (ptr, z, 1, endian_arch);	/* Z */
		      gaiaExport64 (ptr + 8, m, 1, endian_arch);	/* M */
		      ptr += 16;
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  if (type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION
	      || type == GAIA_MULTIPOLYGONZ || type == GAIA_GEOMETRYCOLLECTIONZ
	      || type == GAIA_MULTIPOLYGONM || type == GAIA_GEOMETRYCOLLECTIONM
	      || type == GAIA_MULTIPOLYGONZM
	      || type == GAIA_GEOMETRYCOLLECTIONZM)
	    {
		*ptr = 0x01;
		/* it's a collection: the CLASS TYPE for this element */
		if (type == GAIA_MULTIPOLYGONZ
		    || type == GAIA_GEOMETRYCOLLECTIONZ)
		    gaiaExport32 (ptr + 1, GAIA_POLYGONZ, 1, endian_arch);
		else if (type == GAIA_MULTIPOLYGONM
			 || type == GAIA_GEOMETRYCOLLECTIONM)
		    gaiaExport32 (ptr + 1, GAIA_POLYGONM, 1, endian_arch);
		else if (type == GAIA_MULTIPOLYGONZM
			 || type == GAIA_GEOMETRYCOLLECTIONZM)
		    gaiaExport32 (ptr + 1, GAIA_POLYGONZM, 1, endian_arch);
		else
		    gaiaExport32 (ptr + 1, GAIA_POLYGON, 1, endian_arch);
		ptr += 5;
	    }
	  gaiaExport32 (ptr, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 4, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 8;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		if (type == GAIA_POLYGONZ || type == GAIA_MULTIPOLYGONZ
		    || type == GAIA_GEOMETRYCOLLECTIONZ)
		  {
		      gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		  }
		else if (type == GAIA_POLYGONM || type == GAIA_MULTIPOLYGONM
			 || type == GAIA_GEOMETRYCOLLECTIONM)
		  {
		      gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		  }
		else if (type == GAIA_POLYGONZM || type == GAIA_MULTIPOLYGONZM
			 || type == GAIA_GEOMETRYCOLLECTIONZM)
		  {
		      gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		  }
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		ptr += 16;
		if (type == GAIA_POLYGONZ || type == GAIA_MULTIPOLYGONZ
		    || type == GAIA_GEOMETRYCOLLECTIONZ)
		  {
		      gaiaExport64 (ptr, z, 1, endian_arch);	/* Z - exterior ring */
		      ptr += 8;
		  }
		if (type == GAIA_POLYGONM || type == GAIA_MULTIPOLYGONM
		    || type == GAIA_GEOMETRYCOLLECTIONM)
		  {
		      gaiaExport64 (ptr, m, 1, endian_arch);	/* M - exterior ring */
		      ptr += 8;
		  }
		if (type == GAIA_POLYGONZM || type == GAIA_MULTIPOLYGONZM
		    || type == GAIA_GEOMETRYCOLLECTIONZM)
		  {
		      gaiaExport64 (ptr, z, 1, endian_arch);	/* Z - exterior ring */
		      gaiaExport64 (ptr + 8, m, 1, endian_arch);	/* M - exterior ring */
		      ptr += 16;
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      if (type == GAIA_POLYGONZ || type == GAIA_MULTIPOLYGONZ
			  || type == GAIA_GEOMETRYCOLLECTIONZ)
			{
			    gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
			}
		      else if (type == GAIA_POLYGONM
			       || type == GAIA_MULTIPOLYGONM
			       || type == GAIA_GEOMETRYCOLLECTIONM)
			{
			    gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
			}
		      else if (type == GAIA_POLYGONZM
			       || type == GAIA_MULTIPOLYGONZM
			       || type == GAIA_GEOMETRYCOLLECTIONZM)
			{
			    gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (rng->Coords, iv, &x, &y);
			}
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
		      ptr += 16;
		      if (type == GAIA_POLYGONZ || type == GAIA_MULTIPOLYGONZ
			  || type == GAIA_GEOMETRYCOLLECTIONZ)
			{
			    gaiaExport64 (ptr, z, 1, endian_arch);	/* Z - exterior ring */
			    ptr += 8;
			}
		      if (type == GAIA_POLYGONM || type == GAIA_MULTIPOLYGONM
			  || type == GAIA_GEOMETRYCOLLECTIONM)
			{
			    gaiaExport64 (ptr, m, 1, endian_arch);	/* M - exterior ring */
			    ptr += 8;
			}
		      if (type == GAIA_POLYGONZM || type == GAIA_MULTIPOLYGONZM
			  || type == GAIA_GEOMETRYCOLLECTIONZM)
			{
			    gaiaExport64 (ptr, z, 1, endian_arch);	/* Z - exterior ring */
			    gaiaExport64 (ptr + 8, m, 1, endian_arch);	/* M - exterior ring */
			    ptr += 16;
			}
		  }
	    }
	  polyg = polyg->Next;
      }
}

GAIAGEO_DECLARE int
gaiaEwkbGetPoint (gaiaGeomCollPtr geom, unsigned char *blob,
		  int offset, int blob_size, int endian, int endian_arch,
		  int dims)
{
/* decodes a POINT from PostGIS EWKB binary GEOMETRY */
    double x;
    double y;
    double z;
    double m;
    switch (dims)
      {
      case GAIA_XY_Z_M:
	  if (blob_size < offset + 32)
	      return -1;
	  break;
      case GAIA_XY_Z:
      case GAIA_XY_M:
	  if (blob_size < offset + 24)
	      return -1;
	  break;
      default:
	  if (blob_size < offset + 16)
	      return -1;
	  break;
      }
    x = gaiaImport64 (blob + offset, endian, endian_arch);
    offset += 8;
    y = gaiaImport64 (blob + offset, endian, endian_arch);
    offset += 8;
    if (dims == GAIA_XY_Z_M)
      {
	  z = gaiaImport64 (blob + offset, endian, endian_arch);
	  offset += 8;
	  m = gaiaImport64 (blob + offset, endian, endian_arch);
	  offset += 8;
	  gaiaAddPointToGeomCollXYZM (geom, x, y, z, m);
      }
    else if (dims == GAIA_XY_Z)
      {
	  z = gaiaImport64 (blob + offset, endian, endian_arch);
	  offset += 8;
	  gaiaAddPointToGeomCollXYZ (geom, x, y, z);
      }
    else if (dims == GAIA_XY_M)
      {
	  m = gaiaImport64 (blob + offset, endian, endian_arch);
	  offset += 8;
	  gaiaAddPointToGeomCollXYM (geom, x, y, m);
      }
    else
	gaiaAddPointToGeomColl (geom, x, y);
    return offset;
}

GAIAGEO_DECLARE int
gaiaEwkbGetLinestring (gaiaGeomCollPtr geom, unsigned char *blob,
		       int offset, int blob_size, int endian,
		       int endian_arch, int dims)
{
/* decodes a LINESTRING from PostGIS binary GEOMETRY */
    int npoints;
    int iv;
    double x;
    double y;
    double z;
    double m;
    gaiaLinestringPtr ln;
    if (blob_size < offset + 4)
	return -1;
    npoints = gaiaImport32 (blob + offset, endian, endian_arch);
    offset += 4;
    switch (dims)
      {
      case GAIA_XY_Z_M:
	  if (blob_size < offset + (32 * npoints))
	      return -1;
	  break;
      case GAIA_XY_Z:
      case GAIA_XY_M:
	  if (blob_size < offset + (24 * npoints))
	      return -1;
	  break;
      default:
	  if (blob_size < offset + (16 * npoints))
	      return -1;
	  break;
      }
    ln = gaiaAddLinestringToGeomColl (geom, npoints);
    for (iv = 0; iv < npoints; iv++)
      {
	  x = gaiaImport64 (blob + offset, endian, endian_arch);
	  offset += 8;
	  y = gaiaImport64 (blob + offset, endian, endian_arch);
	  offset += 8;
	  if (dims == GAIA_XY_Z_M)
	    {
		z = gaiaImport64 (blob + offset, endian, endian_arch);
		offset += 8;
		m = gaiaImport64 (blob + offset, endian, endian_arch);
		offset += 8;
		gaiaSetPointXYZM (ln->Coords, iv, x, y, z, m);
	    }
	  else if (dims == GAIA_XY_Z)
	    {
		z = gaiaImport64 (blob + offset, endian, endian_arch);
		offset += 8;
		gaiaSetPointXYZ (ln->Coords, iv, x, y, z);
	    }
	  else if (dims == GAIA_XY_M)
	    {
		m = gaiaImport64 (blob + offset, endian, endian_arch);
		offset += 8;
		gaiaSetPointXYM (ln->Coords, iv, x, y, m);
	    }
	  else
	      gaiaSetPoint (ln->Coords, iv, x, y);
      }
    return offset;
}

GAIAGEO_DECLARE int
gaiaEwkbGetPolygon (gaiaGeomCollPtr geom, unsigned char *blob,
		    int offset, int blob_size, int endian,
		    int endian_arch, int dims)
{
/* decodes a POLYGON from PostGIS binary GEOMETRY */
    int rings;
    int npoints;
    int iv;
    int ib;
    double x;
    double y;
    double z;
    double m;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr rng;
    if (blob_size < offset + 4)
	return -1;
    rings = gaiaImport32 (blob + offset, endian, endian_arch);
    offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (blob_size < offset + 4)
	      return -1;
	  npoints = gaiaImport32 (blob + offset, endian, endian_arch);
	  offset += 4;
	  switch (dims)
	    {
	    case GAIA_XY_Z_M:
		if (blob_size < offset + (32 * npoints))
		    return -1;
		break;
	    case GAIA_XY_Z:
	    case GAIA_XY_M:
		if (blob_size < offset + (24 * npoints))
		    return -1;
		break;
	    default:
		if (blob_size < offset + (16 * npoints))
		    return -1;
		break;
	    }
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geom, npoints, rings - 1);
		rng = polyg->Exterior;
	    }
	  else
	      rng = gaiaAddInteriorRing (polyg, ib - 1, npoints);
	  for (iv = 0; iv < npoints; iv++)
	    {
		x = gaiaImport64 (blob + offset, endian, endian_arch);
		offset += 8;
		y = gaiaImport64 (blob + offset, endian, endian_arch);
		offset += 8;
		if (dims == GAIA_XY_Z_M)
		  {
		      z = gaiaImport64 (blob + offset, endian, endian_arch);
		      offset += 8;
		      m = gaiaImport64 (blob + offset, endian, endian_arch);
		      offset += 8;
		      gaiaSetPointXYZM (rng->Coords, iv, x, y, z, m);
		  }
		else if (dims == GAIA_XY_Z)
		  {
		      z = gaiaImport64 (blob + offset, endian, endian_arch);
		      offset += 8;
		      gaiaSetPointXYZ (rng->Coords, iv, x, y, z);
		  }
		else if (dims == GAIA_XY_M)
		  {
		      m = gaiaImport64 (blob + offset, endian, endian_arch);
		      offset += 8;
		      gaiaSetPointXYM (rng->Coords, iv, x, y, m);
		  }
		else
		    gaiaSetPoint (rng->Coords, iv, x, y);
	    }
      }
    return offset;
}

GAIAGEO_DECLARE int
gaiaEwkbGetMultiGeometry (gaiaGeomCollPtr geom, unsigned char *blob,
			  int offset, int blob_size, int endian,
			  int endian_arch, int dims)
{
/* decodes a MultiGeometry from PostGIS EWKB binary GEOMETRY */
    int entities;
    int type;
    unsigned char xtype[4];
    int ie;
    int off;
    if (blob_size < offset + 4)
	return -1;
    entities = gaiaImport32 (blob + offset, endian, endian_arch);
    offset += 4;
    for (ie = 0; ie < entities; ie++)
      {
	  if (blob_size < offset + 5)
	      return -1;
	  memcpy (xtype, blob + offset + 1, 4);
	  if (endian)
	      xtype[3] = 0x00;
	  else
	      xtype[0] = 0x00;
	  type = gaiaImport32 (xtype, endian, endian_arch);
	  offset += 5;
	  switch (type)
	    {
	    case GAIA_POINT:
		off =
		    gaiaEwkbGetPoint (geom, blob, offset, blob_size, endian,
				      endian_arch, dims);
		if (off < 0)
		    return -1;
		offset = off;
		break;
	    case GAIA_LINESTRING:
		off =
		    gaiaEwkbGetLinestring (geom, blob, offset, blob_size,
					   endian, endian_arch, dims);
		if (off < 0)
		    return -1;
		offset = off;
		break;
	    case GAIA_POLYGON:
		off =
		    gaiaEwkbGetPolygon (geom, blob, offset, blob_size, endian,
					endian_arch, dims);
		if (off < 0)
		    return -1;
		offset = off;
		break;
	    default:		/* unexpected: invalid EWKB */
		return -1;
	    };
      }
    return offset;
}

static int
parseHexEwkbByte (const unsigned char high, const unsigned char low,
		  unsigned char *byte)
{
/* parsing an Hexadecimal byte */
    unsigned char hex;
    switch (high)
      {
      case '0':
	  hex = 16 * 0;
	  break;
      case '1':
	  hex = 16 * 1;
	  break;
      case '2':
	  hex = 16 * 2;
	  break;
      case '3':
	  hex = 16 * 3;
	  break;
      case '4':
	  hex = 16 * 4;
	  break;
      case '5':
	  hex = 16 * 5;
	  break;
      case '6':
	  hex = 16 * 6;
	  break;
      case '7':
	  hex = 16 * 7;
	  break;
      case '8':
	  hex = 16 * 8;
	  break;
      case '9':
	  hex = 16 * 9;
	  break;
      case 'A':
      case 'a':
	  hex = 16 * 10;
	  break;
      case 'B':
      case 'b':
	  hex = 16 * 11;
	  break;
      case 'C':
      case 'c':
	  hex = 16 * 12;
	  break;
      case 'D':
      case 'd':
	  hex = 16 * 13;
	  break;
      case 'E':
      case 'e':
	  hex = 16 * 14;
	  break;
      case 'F':
      case 'f':
	  hex = 16 * 15;
	  break;
      default:
	  return 0;
      };
    switch (low)
      {
      case '0':
	  hex += 0;
	  break;
      case '1':
	  hex += 1;
	  break;
      case '2':
	  hex += 2;
	  break;
      case '3':
	  hex += 3;
	  break;
      case '4':
	  hex += 4;
	  break;
      case '5':
	  hex += 5;
	  break;
      case '6':
	  hex += 6;
	  break;
      case '7':
	  hex += 7;
	  break;
      case '8':
	  hex += 8;
	  break;
      case '9':
	  hex += 9;
	  break;
      case 'A':
      case 'a':
	  hex += 10;
	  break;
      case 'B':
      case 'b':
	  hex += 11;
	  break;
      case 'C':
      case 'c':
	  hex += 12;
	  break;
      case 'D':
      case 'd':
	  hex += 13;
	  break;
      case 'E':
      case 'e':
	  hex += 14;
	  break;
      case 'F':
      case 'f':
	  hex += 15;
	  break;
      default:
	  return 0;
      };
    *byte = hex;
    return 1;
}

GAIAGEO_DECLARE unsigned char *
gaiaParseHexEWKB (const unsigned char *blob_hex, int *blob_size)
{
/* parsing an Hexadecimal EWKB Geometry */
    unsigned char *blob;
    unsigned char *p_out;
    const unsigned char *p_in;
    char high;
    char low;
    unsigned char hex;
    int size;
    int len = strlen ((const char *) blob_hex);
    size = len / 2;
    if (size * 2 != len)
	return NULL;
    blob = malloc (size);
    if (!blob)
	return NULL;
    *blob_size = size;
    p_in = blob_hex;
    p_out = blob;
    while (*p_in != '\0')
      {
	  high = *p_in++;
	  low = *p_in++;
	  if (!parseHexEwkbByte (high, low, &hex))
	    {
		free (blob);
		return NULL;
	    }
	  *p_out++ = hex;
      }
    *blob_size = size;
    return blob;
}

gaiaGeomCollPtr
gaiaFromEWKB (const unsigned char *in_buffer)
{
/* creates a Gaia own Geometry from GEOS/PostGIS EWKB */
    unsigned char *blob;
    int blob_size;
    unsigned char xtype[4];
    unsigned char xdims;
    int type;
    int has_z = 0;
    int has_m = 0;
    int dims = GAIA_XY;
    int srid;
    int ret;
    int endian;
    int endian_arch = gaiaEndianArch ();
    gaiaGeomCollPtr geom = NULL;
    blob = gaiaParseHexEWKB (in_buffer, &blob_size);
    if (!blob)
	return NULL;
    if (blob_size < 9)
      {
	  free (blob);
	  return NULL;
      }
    if (*(blob + 0) == 0x01)
	endian = 1;
    else
	endian = 0;
    memcpy (xtype, blob + 1, 4);
    if (endian)
      {
	  xdims = xtype[3];
	  xtype[3] = 0x00;
      }
    else
      {
	  xdims = xtype[0];
	  xtype[0] = 0x00;
      }
    type = gaiaImport32 (xtype, endian, endian_arch);
    if (xdims & 0x40)
	has_m = 1;
    if (xdims & 0x80)
	has_z = 1;
    if (has_m && has_z)
      {
	  dims = GAIA_XY_Z_M;
	  geom = gaiaAllocGeomCollXYZM ();
      }
    else if (has_m)
      {
	  dims = GAIA_XY_M;
	  geom = gaiaAllocGeomCollXYM ();
      }
    else if (has_z)
      {
	  dims = GAIA_XY_Z;
	  geom = gaiaAllocGeomCollXYZ ();
      }
    else
      {
	  dims = GAIA_XY;
	  geom = gaiaAllocGeomColl ();
      }
    srid = gaiaImport32 (blob + 5, endian, endian_arch);
    geom->Srid = srid;
    if (geom->Srid <= 0)
	geom->Srid = 0;
    switch (type)
      {
      case GAIA_POINT:
	  ret =
	      gaiaEwkbGetPoint (geom, blob, 9, blob_size, endian, endian_arch,
				dims);
	  break;
      case GAIA_LINESTRING:
	  ret =
	      gaiaEwkbGetLinestring (geom, blob, 9, blob_size, endian,
				     endian_arch, dims);
	  break;
      case GAIA_POLYGON:
	  ret =
	      gaiaEwkbGetPolygon (geom, blob, 9, blob_size, endian, endian_arch,
				  dims);
	  break;
      default:
	  ret =
	      gaiaEwkbGetMultiGeometry (geom, blob, 9, blob_size, endian,
					endian_arch, dims);
	  break;
      };
    free (blob);
    if (ret < 0)
      {
	  /* invalid EWKB !!! */
	  gaiaFreeGeomColl (geom);
	  return NULL;
      }
    return geom;
}

GAIAGEO_DECLARE void
gaiaToEWKB (gaiaOutBufferPtr out_buf, gaiaGeomCollPtr geom)
{
/* prints the GEOS/PostGIS EWKB text representation of current geometry */
    char buf[2048];
    unsigned char endian_buf[16];
    char byte[3];
    char *ptr;
    int size;
    int type;
    int entities = 0;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    int i;
    int iv;
    int ib;
    double x;
    double y;
    double z;
    double m;
    int endian_arch = gaiaEndianArch ();
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;

/* precomputing the required size */
    size = 5;			/* SRID and terminating '\0' */
    pt = geom->FirstPoint;
    while (pt)
      {
	  point = pt;
	  entities++;
	  n_points++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  line = ln;
	  entities++;
	  n_linestrings++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  polyg = pg;
	  entities++;
	  n_polygons++;
	  pg = pg->Next;
      }
    if (n_points == 0 && n_polygons == 0 && n_linestrings == 0)
	return;
/* ok, we can determine the geometry class */
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT)
	      type = GAIA_MULTIPOINT;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_POINT;
      }
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTIPOINT;
      }
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	      type = GAIA_MULTILINESTRING;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_LINESTRING;
      }
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTILINESTRING;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	      type = GAIA_MULTIPOLYGON;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_POLYGON;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTIPOLYGON;
      }
    else
	type = GAIA_GEOMETRYCOLLECTION;
/* and now we compute the size of EWKB */
    size += 10;			/* header size */
    if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	size += 8;
    point = geom->FirstPoint;
    while (point)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	      || type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	      size += 10;
	  if (geom->DimensionModel == GAIA_XY_Z
	      || geom->DimensionModel == GAIA_XY_M)
	      size += 48;	/* three doubles for each POINT */
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	      size += 64;	/* four doubles for each POINT */
	  else
	      size += 32;	/* two doubles for each POINT */
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	      || type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	      size += 10;
	  if (geom->DimensionModel == GAIA_XY_Z
	      || geom->DimensionModel == GAIA_XY_M)
	      size += 8 + (line->Points * 48);	/* three doubles for each VERTEX */
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	      size += 8 + (line->Points * 64);	/* four doubles for each VERTEX */
	  else
	      size += 8 + (line->Points * 32);	/* two doubles for each VERTEX */
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	      || type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	      size += 10;
	  rng = polyg->Exterior;
	  if (geom->DimensionModel == GAIA_XY_Z
	      || geom->DimensionModel == GAIA_XY_M)
	      size += 16 + (rng->Points * 48);	/* three doubles for each VERTEX */
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	      size += 16 + (rng->Points * 64);	/* four doubles for each VERTEX */
	  else
	      size += 16 + (rng->Points * 32);	/* two doubles for each VERTEX */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		if (geom->DimensionModel == GAIA_XY_Z
		    || geom->DimensionModel == GAIA_XY_M)
		    size += 8 + (rng->Points * 48);	/* three doubles for each VERTEX */
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    size += 8 + (rng->Points * 64);	/* four doubles for each VERTEX */
		else
		    size += 8 + (rng->Points * 32);	/* two doubles for each VERTEX */
	    }
	  polyg = polyg->Next;
      }
/* and finally we build the EWKB expression */
    ptr = buf;
    *ptr++ = '0';		/* little endian byte order */
    *ptr++ = '1';
    gaiaExport32 (endian_buf, type, 1, endian_arch);	/* the main CLASS TYPE */
    for (i = 0; i < 3; i++)
      {
	  /* CAVEAT: the 4th byte in PostGIS encodes M/Z presence !!!! */
	  sprintf (byte, "%02X", endian_buf[i]);
	  *ptr++ = byte[0];
	  *ptr++ = byte[1];
      }
/* marking dimensions and M/Z presence */
    if (geom->DimensionModel == GAIA_XY_Z)
      {
	  *ptr++ = 'A';
	  *ptr++ = '0';
      }
    else if (geom->DimensionModel == GAIA_XY_M)
      {
	  *ptr++ = '6';
	  *ptr++ = '0';
      }
    else if (geom->DimensionModel == GAIA_XY_Z_M)
      {
	  *ptr++ = 'E';
	  *ptr++ = '0';
      }
    else
      {
	  *ptr++ = '2';
	  *ptr++ = '0';
      }
    gaiaExport32 (endian_buf, geom->Srid, 1, endian_arch);
    for (i = 0; i < 4; i++)
      {
	  sprintf (byte, "%02X", endian_buf[i]);
	  *ptr++ = byte[0];
	  *ptr++ = byte[1];
      }
    *ptr++ = '\0';
    gaiaAppendToOutBuffer (out_buf, buf);
    ptr = buf;
    if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
      {
	  gaiaExport32 (endian_buf, entities, 1, endian_arch);	/* it's a collection; # entities */
	  for (i = 0; i < 4; i++)
	    {
		sprintf (byte, "%02X", endian_buf[i]);
		*ptr++ = byte[0];
		*ptr++ = byte[1];
	    }
      }
    point = geom->FirstPoint;
    while (point)
      {
	  if ((ptr - buf) > 1024)
	    {
		/* flushing the internal buffer */
		*ptr++ = '\0';
		gaiaAppendToOutBuffer (out_buf, buf);
		ptr = buf;
	    }
	  if (type == GAIA_MULTIPOINT || type == GAIA_GEOMETRYCOLLECTION)
	    {
		*ptr++ = '0';
		*ptr++ = '1';
		/* it's a collection: the CLASS TYPE for this element */
		gaiaExport32 (endian_buf, GAIA_POINT, 1, endian_arch);
		for (i = 0; i < 3; i++)
		  {
		      /* CAVEAT: the 4th byte in PostGIS encodes M/Z presence !!!! */
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
		/* marking M/Z presence */
		if (geom->DimensionModel == GAIA_XY_Z)
		    *ptr++ = '8';
		else if (geom->DimensionModel == GAIA_XY_M)
		    *ptr++ = '4';
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    *ptr++ = 'C';
		else
		    *ptr++ = '0';
		*ptr++ = '0';
	    }
	  gaiaExport64 (endian_buf, point->X, 1, endian_arch);	/* X */
	  for (i = 0; i < 8; i++)
	    {
		sprintf (byte, "%02X", endian_buf[i]);
		*ptr++ = byte[0];
		*ptr++ = byte[1];
	    }
	  gaiaExport64 (endian_buf, point->Y, 1, endian_arch);	/* Y */
	  for (i = 0; i < 8; i++)
	    {
		sprintf (byte, "%02X", endian_buf[i]);
		*ptr++ = byte[0];
		*ptr++ = byte[1];
	    }
	  if (geom->DimensionModel == GAIA_XY_Z)
	    {
		gaiaExport64 (endian_buf, point->Z, 1, endian_arch);	/* Z */
		for (i = 0; i < 8; i++)
		  {
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
	    }
	  else if (geom->DimensionModel == GAIA_XY_M)
	    {
		gaiaExport64 (endian_buf, point->M, 1, endian_arch);	/* M */
		for (i = 0; i < 8; i++)
		  {
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
	    }
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaExport64 (endian_buf, point->Z, 1, endian_arch);	/* Z */
		for (i = 0; i < 8; i++)
		  {
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
		gaiaExport64 (endian_buf, point->M, 1, endian_arch);	/* M */
		for (i = 0; i < 8; i++)
		  {
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
	    }
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  if ((ptr - buf) > 1024)
	    {
		/* flushing the internal buffer */
		*ptr++ = '\0';
		gaiaAppendToOutBuffer (out_buf, buf);
		ptr = buf;
	    }
	  if (type == GAIA_MULTILINESTRING || type == GAIA_GEOMETRYCOLLECTION)
	    {
		*ptr++ = '0';
		*ptr++ = '1';
		/* it's a collection: the CLASS TYPE for this element */
		gaiaExport32 (endian_buf, GAIA_LINESTRING, 1, endian_arch);
		for (i = 0; i < 3; i++)
		  {
		      /* CAVEAT: the 4th byte in PostGIS encodes M/Z presence !!!! */
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
		/* marking M/Z presence */
		if (geom->DimensionModel == GAIA_XY_Z)
		    *ptr++ = '8';
		else if (geom->DimensionModel == GAIA_XY_M)
		    *ptr++ = '4';
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    *ptr++ = 'C';
		else
		    *ptr++ = '0';
		*ptr++ = '0';
	    }
	  gaiaExport32 (endian_buf, line->Points, 1, endian_arch);	/* # points */
	  for (i = 0; i < 4; i++)
	    {
		sprintf (byte, "%02X", endian_buf[i]);
		*ptr++ = byte[0];
		*ptr++ = byte[1];
	    }
	  for (iv = 0; iv < line->Points; iv++)
	    {
		if ((ptr - buf) > 1024)
		  {
		      /* flushing the internal buffer */
		      *ptr++ = '\0';
		      gaiaAppendToOutBuffer (out_buf, buf);
		      ptr = buf;
		  }
		gaiaLineGetPoint (line, iv, &x, &y, &z, &m);
		gaiaExport64 (endian_buf, x, 1, endian_arch);	/* X */
		for (i = 0; i < 8; i++)
		  {
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
		gaiaExport64 (endian_buf, y, 1, endian_arch);	/* Y */
		for (i = 0; i < 8; i++)
		  {
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
		if (geom->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaExport64 (endian_buf, z, 1, endian_arch);	/* Z */
		      for (i = 0; i < 8; i++)
			{
			    sprintf (byte, "%02X", endian_buf[i]);
			    *ptr++ = byte[0];
			    *ptr++ = byte[1];
			}
		  }
		else if (geom->DimensionModel == GAIA_XY_M)
		  {
		      gaiaExport64 (endian_buf, m, 1, endian_arch);	/* M */
		      for (i = 0; i < 8; i++)
			{
			    sprintf (byte, "%02X", endian_buf[i]);
			    *ptr++ = byte[0];
			    *ptr++ = byte[1];
			}
		  }
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaExport64 (endian_buf, z, 1, endian_arch);	/* Z */
		      for (i = 0; i < 8; i++)
			{
			    sprintf (byte, "%02X", endian_buf[i]);
			    *ptr++ = byte[0];
			    *ptr++ = byte[1];
			}
		      gaiaExport64 (endian_buf, m, 1, endian_arch);	/* M */
		      for (i = 0; i < 8; i++)
			{
			    sprintf (byte, "%02X", endian_buf[i]);
			    *ptr++ = byte[0];
			    *ptr++ = byte[1];
			}
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  if ((ptr - buf) > 1024)
	    {
		/* flushing the internal buffer */
		*ptr++ = '\0';
		gaiaAppendToOutBuffer (out_buf, buf);
		ptr = buf;
	    }
	  if (type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	    {
		*ptr++ = '0';
		*ptr++ = '1';
		/* it's a collection: the CLASS TYPE for this element */
		gaiaExport32 (endian_buf, GAIA_POLYGON, 1, endian_arch);
		for (i = 0; i < 3; i++)
		  {
		      /* CAVEAT: the 4th byte in PostGIS encodes M/Z presence !!!! */
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
		/* marking M/Z presence */
		if (geom->DimensionModel == GAIA_XY_Z)
		    *ptr++ = '8';
		else if (geom->DimensionModel == GAIA_XY_M)
		    *ptr++ = '4';
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    *ptr++ = 'C';
		else
		    *ptr++ = '0';
		*ptr++ = '0';
	    }
	  gaiaExport32 (endian_buf, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  for (i = 0; i < 4; i++)
	    {
		sprintf (byte, "%02X", endian_buf[i]);
		*ptr++ = byte[0];
		*ptr++ = byte[1];
	    }
	  rng = polyg->Exterior;
	  gaiaExport32 (endian_buf, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  for (i = 0; i < 4; i++)
	    {
		sprintf (byte, "%02X", endian_buf[i]);
		*ptr++ = byte[0];
		*ptr++ = byte[1];
	    }
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		if ((ptr - buf) > 1024)
		  {
		      /* flushing the internal buffer */
		      *ptr++ = '\0';
		      gaiaAppendToOutBuffer (out_buf, buf);
		      ptr = buf;
		  }
		gaiaRingGetPoint (rng, iv, &x, &y, &z, &m);
		gaiaExport64 (endian_buf, x, 1, endian_arch);	/* X - exterior ring */
		for (i = 0; i < 8; i++)
		  {
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
		gaiaExport64 (endian_buf, y, 1, endian_arch);	/* Y - exterior ring */
		for (i = 0; i < 8; i++)
		  {
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
		if (geom->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaExport64 (endian_buf, z, 1, endian_arch);	/* Z */
		      for (i = 0; i < 8; i++)
			{
			    sprintf (byte, "%02X", endian_buf[i]);
			    *ptr++ = byte[0];
			    *ptr++ = byte[1];
			}
		  }
		else if (geom->DimensionModel == GAIA_XY_M)
		  {
		      gaiaExport64 (endian_buf, m, 1, endian_arch);	/* M */
		      for (i = 0; i < 8; i++)
			{
			    sprintf (byte, "%02X", endian_buf[i]);
			    *ptr++ = byte[0];
			    *ptr++ = byte[1];
			}
		  }
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaExport64 (endian_buf, z, 1, endian_arch);	/* Z */
		      for (i = 0; i < 8; i++)
			{
			    sprintf (byte, "%02X", endian_buf[i]);
			    *ptr++ = byte[0];
			    *ptr++ = byte[1];
			}
		      gaiaExport64 (endian_buf, m, 1, endian_arch);	/* M */
		      for (i = 0; i < 8; i++)
			{
			    sprintf (byte, "%02X", endian_buf[i]);
			    *ptr++ = byte[0];
			    *ptr++ = byte[1];
			}
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (endian_buf, rng->Points, 1, endian_arch);	/* # points - interior ring */
		for (i = 0; i < 4; i++)
		  {
		      sprintf (byte, "%02X", endian_buf[i]);
		      *ptr++ = byte[0];
		      *ptr++ = byte[1];
		  }
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      if ((ptr - buf) > 1024)
			{
			    /* flushing the internal buffer */
			    *ptr++ = '\0';
			    gaiaAppendToOutBuffer (out_buf, buf);
			    ptr = buf;
			}
		      gaiaRingGetPoint (rng, iv, &x, &y, &z, &m);
		      gaiaExport64 (endian_buf, x, 1, endian_arch);	/* X - interior ring */
		      for (i = 0; i < 8; i++)
			{
			    sprintf (byte, "%02X", endian_buf[i]);
			    *ptr++ = byte[0];
			    *ptr++ = byte[1];
			}
		      gaiaExport64 (endian_buf, y, 1, endian_arch);	/* Y - interior ring */
		      for (i = 0; i < 8; i++)
			{
			    sprintf (byte, "%02X", endian_buf[i]);
			    *ptr++ = byte[0];
			    *ptr++ = byte[1];
			}
		      if (geom->DimensionModel == GAIA_XY_Z)
			{
			    gaiaExport64 (endian_buf, z, 1, endian_arch);	/* Z */
			    for (i = 0; i < 8; i++)
			      {
				  sprintf (byte, "%02X", endian_buf[i]);
				  *ptr++ = byte[0];
				  *ptr++ = byte[1];
			      }
			}
		      else if (geom->DimensionModel == GAIA_XY_M)
			{
			    gaiaExport64 (endian_buf, m, 1, endian_arch);	/* M */
			    for (i = 0; i < 8; i++)
			      {
				  sprintf (byte, "%02X", endian_buf[i]);
				  *ptr++ = byte[0];
				  *ptr++ = byte[1];
			      }
			}
		      else if (geom->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaExport64 (endian_buf, z, 1, endian_arch);	/* Z */
			    for (i = 0; i < 8; i++)
			      {
				  sprintf (byte, "%02X", endian_buf[i]);
				  *ptr++ = byte[0];
				  *ptr++ = byte[1];
			      }
			    gaiaExport64 (endian_buf, m, 1, endian_arch);	/* M */
			    for (i = 0; i < 8; i++)
			      {
				  sprintf (byte, "%02X", endian_buf[i]);
				  *ptr++ = byte[0];
				  *ptr++ = byte[1];
			      }
			}
		  }
	    }
	  polyg = polyg->Next;
      }

/* terminating the EWKB string */
    *ptr = '\0';
    gaiaAppendToOutBuffer (out_buf, buf);
}

static int
coordDimsFromFgf (int endian_arch, const unsigned char *blob, unsigned int size,
		  int *type)
{
/* decoding the coordinate Dimensions for an FGF Geometry */
    int coord_dims;
    if (size < 4)
	return 0;
    coord_dims = gaiaImport32 (blob, GAIA_LITTLE_ENDIAN, endian_arch);
    *type = coord_dims;
    switch (coord_dims)
      {
      case GAIA_XY:
	  return 2;
      case GAIA_XY_M:
      case GAIA_XY_Z:
	  return 3;
      case GAIA_XY_Z_M:
	  return 4;
      default:
	  return 0;
      }
}

static int
pointFromFgf (gaiaGeomCollPtr geom, int endian_arch, const unsigned char *blob,
	      unsigned int size, unsigned int *consumed)
{
/* decoding a POINT Geometry from FGF  */
    double x;
    double y;
    double z;
    double m;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    int coord_dims;
    int type;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_POINT)
	return 0;
    ptr += 4;
    sz -= 4;
/* checking size */
    if (sz < 4)
	return 0;
    coord_dims = coordDimsFromFgf (endian_arch, ptr, size, &type);
    if (!coord_dims)
	return 0;
    ptr += 4;
    sz -= 4;
    if (sz < (coord_dims * sizeof (double)))
	return 0;
    if (consumed)
	*consumed = coord_dims * sizeof (double);
    if (type == GAIA_XY_Z)
      {
	  /* building the POINTZ */
	  x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
	  y = gaiaImport64 (ptr + 8, GAIA_LITTLE_ENDIAN, endian_arch);
	  z = gaiaImport64 (ptr + 8, GAIA_LITTLE_ENDIAN, endian_arch);
	  gaiaAddPointToGeomCollXYZ (geom, x, y, z);
      }
    else if (type == GAIA_XY_M)
      {
	  /* building the POINTM */
	  x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
	  y = gaiaImport64 (ptr + 8, GAIA_LITTLE_ENDIAN, endian_arch);
	  m = gaiaImport64 (ptr + 8, GAIA_LITTLE_ENDIAN, endian_arch);
	  gaiaAddPointToGeomCollXYM (geom, x, y, m);
      }
    else if (type == GAIA_XY_Z_M)
      {
	  /* building the POINTZM */
	  x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
	  y = gaiaImport64 (ptr + 8, GAIA_LITTLE_ENDIAN, endian_arch);
	  z = gaiaImport64 (ptr + 8, GAIA_LITTLE_ENDIAN, endian_arch);
	  m = gaiaImport64 (ptr + 8, GAIA_LITTLE_ENDIAN, endian_arch);
	  gaiaAddPointToGeomCollXYZM (geom, x, y, z, m);
      }
    else
      {
	  /* building the POINT */
	  x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
	  y = gaiaImport64 (ptr + 8, GAIA_LITTLE_ENDIAN, endian_arch);
	  gaiaAddPointToGeomColl (geom, x, y);
      }
    return 1;
}

static int
linestringFromFgf (gaiaGeomCollPtr geom, int endian_arch,
		   const unsigned char *blob, unsigned int size,
		   unsigned int *consumed)
{
/* decoding a LINESTRING Geometry from FGF  */
    gaiaLinestringPtr ln;
    int pts;
    int iv;
    double x;
    double y;
    unsigned int ln_sz;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    int coord_dims;
    int type;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_LINESTRING)
	return 0;
    ptr += 4;
    sz -= 4;
/* checking size */
    coord_dims = coordDimsFromFgf (endian_arch, ptr, size, &type);
    if (!coord_dims)
	return 0;
    ptr += 4;
    sz -= 4;
/* how many points are there ? */
    if (sz < 4)
	return 0;
    pts = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    if (pts < 2)
	return 0;
    ln_sz = pts * coord_dims * sizeof (double);
    if (sz < ln_sz)
	return 0;
    if (consumed)
	*consumed = (12 + ln_sz);
    if (type == GAIA_XY_Z)
      {
	  /* building the LINESTRINGZ */
	  geom->DimensionModel = GAIA_XY_Z;
	  ln = gaiaAddLinestringToGeomColl (geom, pts);
	  for (iv = 0; iv < pts; iv++)
	    {
		/* inserting vertices into the linestring */
		x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
		y = gaiaImport64 (ptr + sizeof (double), GAIA_LITTLE_ENDIAN,
				  endian_arch);
		ptr += (coord_dims * sizeof (double));
		gaiaSetPoint (ln->Coords, iv, x, y);
	    }
      }
    else if (type == GAIA_XY_M)
      {
	  /* building the LINESTRINGM */
	  geom->DimensionModel = GAIA_XY_M;
	  ln = gaiaAddLinestringToGeomColl (geom, pts);
	  for (iv = 0; iv < pts; iv++)
	    {
		/* inserting vertices into the linestring */
		x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
		y = gaiaImport64 (ptr + sizeof (double), GAIA_LITTLE_ENDIAN,
				  endian_arch);
		ptr += (coord_dims * sizeof (double));
		gaiaSetPoint (ln->Coords, iv, x, y);
	    }
      }
    else if (type == GAIA_XY_Z_M)
      {
	  /* building the LINESTRINGZM */
	  geom->DimensionModel = GAIA_XY_Z_M;
	  ln = gaiaAddLinestringToGeomColl (geom, pts);
	  for (iv = 0; iv < pts; iv++)
	    {
		/* inserting vertices into the linestring */
		x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
		y = gaiaImport64 (ptr + sizeof (double), GAIA_LITTLE_ENDIAN,
				  endian_arch);
		ptr += (coord_dims * sizeof (double));
		gaiaSetPoint (ln->Coords, iv, x, y);
	    }
      }
    else
      {
	  /* building the LINESTRING */
	  geom->DimensionModel = GAIA_XY;
	  ln = gaiaAddLinestringToGeomColl (geom, pts);
	  for (iv = 0; iv < pts; iv++)
	    {
		/* inserting vertices into the linestring */
		x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
		y = gaiaImport64 (ptr + sizeof (double), GAIA_LITTLE_ENDIAN,
				  endian_arch);
		ptr += (coord_dims * sizeof (double));
		gaiaSetPoint (ln->Coords, iv, x, y);
	    }
      }
    return 1;
}

static int
polygonFromFgf (gaiaGeomCollPtr geom, int endian_arch,
		const unsigned char *blob, unsigned int size,
		unsigned int *consumed)
{
/* decoding a POLYGON Geometry from FGF  */
    gaiaPolygonPtr pg = NULL;
    gaiaRingPtr rng;
    int rings;
    int ir;
    int pts;
    int iv;
    double x;
    double y;
    double z;
    double m;
    unsigned int rng_sz;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    int coord_dims;
    int type;
    unsigned int bytes = 0;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_POLYGON)
	return 0;
    ptr += 4;
    sz -= 4;
    bytes += 4;
/* checking size */
    coord_dims = coordDimsFromFgf (endian_arch, ptr, size, &type);
    if (!coord_dims)
	return 0;
    ptr += 4;
    sz -= 4;
    bytes += 4;
/* how many rings are there ? */
    if (sz < 4)
	return 0;
    rings = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    bytes += 4;
    if (rings < 1)
	return 0;
    for (ir = 0; ir < rings; ir++)
      {
	  /* fetching Polygon's rings */
	  if (sz < 4)
	      return 0;
	  pts = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
	  ptr += 4;
	  sz -= 4;
	  bytes += 4;
	  if (pts < 4)
	      return 0;
	  rng_sz = pts * coord_dims * sizeof (double);
	  if (sz < rng_sz)
	      return 0;
	  bytes += rng_sz;
	  if (type == GAIA_XY_Z)
	    {
		/* POLYGONZ */
		geom->DimensionModel = GAIA_XY_Z;
		if (ir == 0)
		  {
		      /* building the EXTERIOR RING */
		      pg = gaiaAddPolygonToGeomColl (geom, pts, rings - 1);
		      rng = pg->Exterior;
		      for (iv = 0; iv < pts; iv++)
			{
			    /* inserting vertices into the EXTERIOR Ring */
			    x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN,
					      endian_arch);
			    y = gaiaImport64 (ptr + sizeof (double),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    z = gaiaImport64 (ptr + (sizeof (double) * 2),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    ptr += (coord_dims * sizeof (double));
			    gaiaSetPointXYZ (rng->Coords, iv, x, y, z);
			}
		  }
		else
		  {
		      /* building an INTERIOR RING */
		      rng = gaiaAddInteriorRing (pg, ir - 1, pts);
		      for (iv = 0; iv < pts; iv++)
			{
			    /* inserting vertices into some INTERIOR Ring */
			    x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN,
					      endian_arch);
			    y = gaiaImport64 (ptr + sizeof (double),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    z = gaiaImport64 (ptr + (sizeof (double) * 2),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    ptr += (coord_dims * sizeof (double));
			    gaiaSetPointXYZ (rng->Coords, iv, x, y, z);
			}
		  }
	    }
	  if (type == GAIA_XY_M)
	    {
		/* POLYGONM */
		geom->DimensionModel = GAIA_XY_M;
		if (ir == 0)
		  {
		      /* building the EXTERIOR RING */
		      pg = gaiaAddPolygonToGeomColl (geom, pts, rings - 1);
		      rng = pg->Exterior;
		      for (iv = 0; iv < pts; iv++)
			{
			    /* inserting vertices into the EXTERIOR Ring */
			    x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN,
					      endian_arch);
			    y = gaiaImport64 (ptr + sizeof (double),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    m = gaiaImport64 (ptr + (sizeof (double) * 2),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    ptr += (coord_dims * sizeof (double));
			    gaiaSetPointXYM (rng->Coords, iv, x, y, m);
			}
		  }
		else
		  {
		      /* building an INTERIOR RING */
		      rng = gaiaAddInteriorRing (pg, ir - 1, pts);
		      for (iv = 0; iv < pts; iv++)
			{
			    /* inserting vertices into some INTERIOR Ring */
			    x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN,
					      endian_arch);
			    y = gaiaImport64 (ptr + sizeof (double),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    m = gaiaImport64 (ptr + (sizeof (double) * 2),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    ptr += (coord_dims * sizeof (double));
			    gaiaSetPointXYM (rng->Coords, iv, x, y, m);
			}
		  }
	    }
	  else if (type == GAIA_XY_Z_M)
	    {
		/* POLYGONZM */
		geom->DimensionModel = GAIA_XY_Z_M;
		if (ir == 0)
		  {
		      /* building the EXTERIOR RING */
		      pg = gaiaAddPolygonToGeomColl (geom, pts, rings - 1);
		      rng = pg->Exterior;
		      for (iv = 0; iv < pts; iv++)
			{
			    /* inserting vertices into the EXTERIOR Ring */
			    x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN,
					      endian_arch);
			    y = gaiaImport64 (ptr + sizeof (double),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    z = gaiaImport64 (ptr + (sizeof (double) * 2),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    m = gaiaImport64 (ptr + (sizeof (double) * 3),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    ptr += (coord_dims * sizeof (double));
			    gaiaSetPointXYZM (rng->Coords, iv, x, y, z, m);
			}
		  }
		else
		  {
		      /* building an INTERIOR RING */
		      rng = gaiaAddInteriorRing (pg, ir - 1, pts);
		      for (iv = 0; iv < pts; iv++)
			{
			    /* inserting vertices into some INTERIOR Ring */
			    x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN,
					      endian_arch);
			    y = gaiaImport64 (ptr + sizeof (double),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    z = gaiaImport64 (ptr + (sizeof (double) * 2),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    m = gaiaImport64 (ptr + (sizeof (double) * 3),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    ptr += (coord_dims * sizeof (double));
			    gaiaSetPointXYZM (rng->Coords, iv, x, y, z, m);
			}
		  }
	    }
	  else
	    {
		/* POLYGON */
		geom->DimensionModel = GAIA_XY;
		if (ir == 0)
		  {
		      /* building the EXTERIOR RING */
		      pg = gaiaAddPolygonToGeomColl (geom, pts, rings - 1);
		      rng = pg->Exterior;
		      for (iv = 0; iv < pts; iv++)
			{
			    /* inserting vertices into the EXTERIOR Ring */
			    x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN,
					      endian_arch);
			    y = gaiaImport64 (ptr + sizeof (double),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    ptr += (coord_dims * sizeof (double));
			    gaiaSetPoint (rng->Coords, iv, x, y);
			}
		  }
		else
		  {
		      /* building an INTERIOR RING */
		      rng = gaiaAddInteriorRing (pg, ir - 1, pts);
		      for (iv = 0; iv < pts; iv++)
			{
			    /* inserting vertices into some INTERIOR Ring */
			    x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN,
					      endian_arch);
			    y = gaiaImport64 (ptr + sizeof (double),
					      GAIA_LITTLE_ENDIAN, endian_arch);
			    ptr += (coord_dims * sizeof (double));
			    gaiaSetPoint (rng->Coords, iv, x, y);
			}
		  }
	    }
	  sz -= rng_sz;
      }
    if (consumed)
	*consumed = bytes;
    return 1;
}

static int
multiPointFromFgf (gaiaGeomCollPtr geom, int endian_arch,
		   const unsigned char *blob, unsigned int size)
{
/* decoding a MULTIPOINT Geometry from FGF  */
    int pts;
    int ipt;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    unsigned int consumed;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_MULTIPOINT)
	return 0;
    ptr += 4;
    sz -= 4;
/* how many points are there ? */
    if (sz < 4)
	return 0;
    pts = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    if (pts < 1)
	return 0;
    for (ipt = 0; ipt < pts; ipt++)
      {
	  /* fetching individual Points from FGF */
	  if (!pointFromFgf (geom, endian_arch, ptr, sz, &consumed))
	      return 0;
	  ptr += consumed;
	  sz -= consumed;
      }
    return 1;
}

static int
multiLinestringFromFgf (gaiaGeomCollPtr geom, int endian_arch,
			const unsigned char *blob, unsigned int size)
{
/* decoding a MULTILINESTRING Geometry from FGF  */
    int lns;
    int iln;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    unsigned int consumed;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32
	(ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_MULTILINESTRING)
	return 0;
    ptr += 4;
    sz -= 4;
/* how many linestrings are there ? */
    if (sz < 4)
	return 0;
    lns = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    if (lns < 1)
	return 0;
    for (iln = 0; iln < lns; iln++)
      {
	  /* fetching individual Linestrings from FGF */
	  if (!linestringFromFgf (geom, endian_arch, ptr, sz, &consumed))
	      return 0;
	  ptr += consumed;
	  sz -= consumed;
      }
    return 1;
}

static int
multiPolygonFromFgf (gaiaGeomCollPtr geom, int endian_arch,
		     const unsigned char *blob, unsigned int size)
{
/* decoding a MULTIPOLYGON Geometry from FGF  */
    int pgs;
    int ipg;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    unsigned int consumed;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32
	(ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_MULTIPOLYGON)
	return 0;
    ptr += 4;
    sz -= 4;
/* how many polygons are there ? */
    if (sz < 4)
	return 0;
    pgs = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    if (pgs < 1)
	return 0;
    for (ipg = 0; ipg < pgs; ipg++)
      {
	  /* fetching individual Polygons from FGF */
	  if (!polygonFromFgf (geom, endian_arch, ptr, sz, &consumed))
	      return 0;
	  ptr += consumed;
	  sz -= consumed;
      }
    return 1;
}

static int
geomCollectionFromFgf (gaiaGeomCollPtr geom, int endian_arch,
		       const unsigned char *blob, unsigned int size)
{
/* decoding a  GEOMETRYCOLLECTION Geometry from FGF  */
    int geoms;
    int ig;
    int geom_type;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    unsigned int consumed;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32
	(ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_GEOMETRYCOLLECTION)
	return 0;
    ptr += 4;
    sz -= 4;
/* how many individual Geometries are there ? */
    if (sz < 4)
	return 0;
    geoms = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    if (geoms < 1)
	return 0;
    for (ig = 0; ig < geoms; ig++)
      {
	  /* fetching individual Geometries from FGF */
	  if (sz < 4)
	      return 0;
	  geom_type = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
	  switch (geom_type)
	    {
	    case GAIA_POINT:
		if (!pointFromFgf (geom, endian_arch, ptr, sz, &consumed))
		    return 0;
		break;
	    case GAIA_LINESTRING:
		if (!linestringFromFgf (geom, endian_arch, ptr, sz, &consumed))
		    return 0;
		break;
	    case GAIA_POLYGON:
		if (!polygonFromFgf (geom, endian_arch, ptr, sz, &consumed))
		    return 0;
		break;
	    default:		/* unsupported geometry type */
		return 0;
		break;
	    };
	  ptr += consumed;
	  sz -= consumed;
      }
    return 1;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromFgf (const unsigned char *blob, unsigned int size)
{
/* decoding from FGF to GEOMETRY  */
    gaiaGeomCollPtr geom = NULL;
    int geom_type;
    int endian_arch = gaiaEndianArch ();
    if (size < 4)
	return NULL;
/* checking FGF type */
    geom_type = gaiaImport32 (blob, GAIA_LITTLE_ENDIAN, endian_arch);
    geom = gaiaAllocGeomColl ();
    geom->DeclaredType = geom_type;
    switch (geom_type)
      {
      case GAIA_POINT:
	  if (pointFromFgf (geom, endian_arch, blob, size, NULL))
	      return geom;
	  break;
      case GAIA_LINESTRING:
	  if (linestringFromFgf (geom, endian_arch, blob, size, NULL))
	      return geom;
	  break;
      case GAIA_POLYGON:
	  if (polygonFromFgf (geom, endian_arch, blob, size, NULL))
	      return geom;
	  break;
      case GAIA_MULTIPOINT:
	  if (multiPointFromFgf (geom, endian_arch, blob, size))
	      return geom;
	  break;
      case GAIA_MULTILINESTRING:
	  if (multiLinestringFromFgf (geom, endian_arch, blob, size))
	      return geom;
	  break;
      case GAIA_MULTIPOLYGON:
	  if (multiPolygonFromFgf (geom, endian_arch, blob, size))
	      return geom;
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  if (geomCollectionFromFgf (geom, endian_arch, blob, size))
	      return geom;
	  break;
      default:			/* unsupported geometry type */
	  break;
      };
    gaiaFreeGeomColl (geom);
    return NULL;
}

GAIAGEO_DECLARE void
gaiaToFgf (gaiaGeomCollPtr geom, unsigned char **result, int *size,
	   int coord_dims)
{
/* builds the FGF representation for this GEOMETRY */
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    int entities = 0;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    int type;
    int n_coords;
    unsigned char *ptr;
    int sz = 0;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    int endian_arch = gaiaEndianArch ();
    gaiaMbrGeometry (geom);
    switch (coord_dims)
      {
      case GAIA_XY:
	  n_coords = 2;
	  break;
      case GAIA_XY_M:
      case GAIA_XY_Z:
	  n_coords = 3;
	  break;
      case GAIA_XY_Z_M:
	  n_coords = 4;
	  break;
      default:
	  n_coords = 0;
	  break;
      }
/* how many entities, and of what kind, do we have ? */
    pt = geom->FirstPoint;
    while (pt)
      {
	  point = pt;
	  entities++;
	  n_points++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  line = ln;
	  entities++;
	  n_linestrings++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  polyg = pg;
	  entities++;
	  n_polygons++;
	  pg = pg->Next;
      }
    *size = 0;
    sz = 0;
    *result = NULL;
    if (n_points == 0 && n_polygons == 0 && n_linestrings == 0)
	return;
/* ok, we can determine the geometry class */
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT)
	      type = GAIA_MULTIPOINT;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_POINT;
      }
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTIPOINT;
      }
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	      type = GAIA_MULTILINESTRING;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_LINESTRING;
      }
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTILINESTRING;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	      type = GAIA_MULTIPOLYGON;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_POLYGON;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTIPOLYGON;
      }
    else
	type = GAIA_GEOMETRYCOLLECTION;
/* and now we compute the size of FGF */
    if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	sz += 8;
    point = geom->FirstPoint;
    while (point)
      {
	  sz += (8 + (n_coords * sizeof (double)));	/* the size of each POINT */
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  sz += (12 + ((n_coords * sizeof (double)) * line->Points));	/* # points + [x,y] for each vertex */
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  rng = polyg->Exterior;
	  sz += (16 + ((n_coords * sizeof (double)) * rng->Points));	/* # rings + # points + [x.y] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		sz += (4 + ((n_coords * sizeof (double)) * rng->Points));	/* # points + [x,y] array - interior ring */
	    }
	  polyg = polyg->Next;
      }
    *size = sz;
    ptr = malloc (sz);
    *result = ptr;
/* and finally we build the FGF */
    if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
      {
	  gaiaExport32 (ptr, type, GAIA_LITTLE_ENDIAN, endian_arch);	/* Geometry Type */
	  ptr += 4;
	  gaiaExport32 (ptr, entities, GAIA_LITTLE_ENDIAN, endian_arch);	/* it's a collection; # entities */
	  ptr += 4;
      }
    point = geom->FirstPoint;
    while (point)
      {
	  gaiaExport32 (ptr, GAIA_POINT, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CLASS TYPE for this element */
	  ptr += 4;
	  gaiaExport32 (ptr, coord_dims, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CoordDimension */
	  ptr += 4;
	  gaiaExport64 (ptr, point->X, GAIA_LITTLE_ENDIAN, endian_arch);	/* X */
	  ptr += 8;
	  gaiaExport64 (ptr, point->Y, GAIA_LITTLE_ENDIAN, endian_arch);	/* Y */
	  ptr += 8;
	  if (n_coords > 2)
	    {
		/* the third coordinate [Z or M] */
		if (coord_dims == GAIA_XY_Z || coord_dims == GAIA_XY_Z_M)
		    gaiaExport64 (ptr, point->Z, GAIA_LITTLE_ENDIAN,
				  endian_arch);
		else
		    gaiaExport64 (ptr, point->M, GAIA_LITTLE_ENDIAN,
				  endian_arch);
		ptr += 8;
	    }
	  if (n_coords > 3)
	    {
		/* the fourth coordinate [M] */
		gaiaExport64 (ptr, point->M, GAIA_LITTLE_ENDIAN, endian_arch);
		ptr += 8;
	    }
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  gaiaExport32 (ptr, GAIA_LINESTRING, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CLASS TYPE for this element */
	  ptr += 4;
	  gaiaExport32 (ptr, coord_dims, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CoordDimension */
	  ptr += 4;
	  gaiaExport32 (ptr, line->Points, GAIA_LITTLE_ENDIAN, endian_arch);	/* # points */
	  ptr += 4;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		z = 0.0;
		m = 0.0;
		if (geom->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (geom->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		gaiaExport64 (ptr, x, GAIA_LITTLE_ENDIAN, endian_arch);	/* X */
		ptr += 8;
		gaiaExport64 (ptr, y, GAIA_LITTLE_ENDIAN, endian_arch);	/* Y */
		ptr += 8;
		if (n_coords > 2)
		  {
		      /* the third coordinate [Z or M] */
		      if (coord_dims == GAIA_XY_Z || coord_dims == GAIA_XY_Z_M)
			  gaiaExport64 (ptr, z, GAIA_LITTLE_ENDIAN,
					endian_arch);
		      else
			  gaiaExport64 (ptr, m, GAIA_LITTLE_ENDIAN,
					endian_arch);
		      ptr += 8;
		  }
		if (n_coords > 3)
		  {
		      /* the fourth coordinate [M]; */
		      gaiaExport64 (ptr, m, GAIA_LITTLE_ENDIAN, endian_arch);
		      ptr += 8;
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  gaiaExport32 (ptr, GAIA_POLYGON, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CLASS TYPE for this element */
	  ptr += 4;
	  gaiaExport32 (ptr, coord_dims, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CoordDimension */
	  ptr += 4;
	  gaiaExport32 (ptr, polyg->NumInteriors + 1, GAIA_LITTLE_ENDIAN, endian_arch);	/* # rings */
	  ptr += 4;
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr, rng->Points, GAIA_LITTLE_ENDIAN, endian_arch);	/* # points - exterior ring */
	  ptr += 4;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		z = 0.0;
		m = 0.0;
		if (geom->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		  }
		else if (geom->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		  }
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		  }
		gaiaExport64 (ptr, x, GAIA_LITTLE_ENDIAN, endian_arch);	/* X - exterior ring */
		ptr += 8;
		gaiaExport64 (ptr, y, GAIA_LITTLE_ENDIAN, endian_arch);	/* Y - exterior ring */
		ptr += 8;
		if (n_coords > 2)
		  {
		      /* the third coordinate [Z or M] */
		      if (coord_dims == GAIA_XY_Z || coord_dims == GAIA_XY_Z_M)
			  gaiaExport64 (ptr, z, GAIA_LITTLE_ENDIAN,
					endian_arch);
		      else
			  gaiaExport64 (ptr, m, GAIA_LITTLE_ENDIAN,
					endian_arch);
		      ptr += 8;
		  }
		if (n_coords > 3)
		  {
		      /* the fourth coordinate [M] */
		      gaiaExport64 (ptr, m, GAIA_LITTLE_ENDIAN, endian_arch);
		      ptr += 8;
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      z = 0.0;
		      m = 0.0;
		      if (geom->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
			}
		      else if (geom->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
			}
		      else if (geom->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (rng->Coords, iv, &x, &y);
			}
		      gaiaExport64 (ptr, x, GAIA_LITTLE_ENDIAN, endian_arch);	/* X - interior ring */
		      ptr += 8;
		      gaiaExport64 (ptr, y, GAIA_LITTLE_ENDIAN, endian_arch);	/* Y - interior ring */
		      ptr += 8;
		      if (n_coords > 2)
			{
			    /* the third coordinate [Z or M]; defaulting to ZERO */
			    if (coord_dims == GAIA_XY_Z
				|| coord_dims == GAIA_XY_Z_M)
				gaiaExport64 (ptr, z, GAIA_LITTLE_ENDIAN,
					      endian_arch);
			    else
				gaiaExport64 (ptr, m, GAIA_LITTLE_ENDIAN,
					      endian_arch);
			    ptr += 8;
			}
		      if (n_coords > 3)
			{
			    /* the fourth coordinate [M] */
			    gaiaExport64 (ptr, m, GAIA_LITTLE_ENDIAN,
					  endian_arch);
			    ptr += 8;
			}
		  }
	    }
	  polyg = polyg->Next;
      }
}
