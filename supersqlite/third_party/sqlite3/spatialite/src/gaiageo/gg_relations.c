/*

 gg_relations.c -- Gaia spatial relations
    
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
#include <string.h>
#include <float.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#ifndef OMIT_GEOS		/* including GEOS */
#include <geos_c.h>
#endif

#include <spatialite_private.h>
#include <spatialite/sqlite.h>

#include <spatialite/gaiageo.h>

/* GLOBAL variables */
char *gaia_geos_error_msg = NULL;
char *gaia_geos_warning_msg = NULL;
char *gaia_geosaux_error_msg = NULL;

SPATIALITE_PRIVATE void
splite_free_geos_cache_item (struct splite_geos_cache_item *p)
{
#ifndef OMIT_GEOS		/* including GEOS */
    if (p->preparedGeosGeom)
	GEOSPreparedGeom_destroy (p->preparedGeosGeom);
    if (p->geosGeom)
	GEOSGeom_destroy (p->geosGeom);
#endif
    p->geosGeom = NULL;
    p->preparedGeosGeom = NULL;
}

SPATIALITE_PRIVATE void
splite_free_geos_cache_item_r (const void *p_cache,
			       struct splite_geos_cache_item *p)
{
#ifndef OMIT_GEOS		/* including GEOS */
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
      {
	  splite_free_geos_cache_item (p);
	  return;
      }
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
      {
	  splite_free_geos_cache_item (p);
	  return;
      }
    handle = cache->GEOS_handle;
    if (handle == NULL)
      {
	  splite_free_geos_cache_item (p);
	  return;
      }
    if (p->preparedGeosGeom)
	GEOSPreparedGeom_destroy_r (handle, p->preparedGeosGeom);
    if (p->geosGeom)
	GEOSGeom_destroy_r (handle, p->geosGeom);
#endif
    p->geosGeom = NULL;
    p->preparedGeosGeom = NULL;
}

GAIAGEO_DECLARE void
gaiaResetGeosMsg ()
{
/* resets the GEOS error and warning messages */
    if (gaia_geos_error_msg != NULL)
	free (gaia_geos_error_msg);
    if (gaia_geos_warning_msg != NULL)
	free (gaia_geos_warning_msg);
    if (gaia_geosaux_error_msg != NULL)
	free (gaia_geosaux_error_msg);
    gaia_geos_error_msg = NULL;
    gaia_geos_warning_msg = NULL;
    gaia_geosaux_error_msg = NULL;
}

GAIAGEO_DECLARE const char *
gaiaGetGeosErrorMsg ()
{
/* return the latest GEOS error message */
    return gaia_geos_error_msg;
}

GAIAGEO_DECLARE const char *
gaiaGetGeosWarningMsg ()
{
/* return the latest GEOS error message */
    return gaia_geos_warning_msg;
}

GAIAGEO_DECLARE const char *
gaiaGetGeosAuxErrorMsg ()
{
/* return the latest GEOS (auxialiary) error message */
    return gaia_geosaux_error_msg;
}

GAIAGEO_DECLARE void
gaiaSetGeosErrorMsg (const char *msg)
{
/* setting the latest GEOS error message */
    int len;
    if (gaia_geos_error_msg != NULL)
	free (gaia_geos_error_msg);
    gaia_geos_error_msg = NULL;
    if (msg == NULL)
	return;
    len = strlen (msg);
    gaia_geos_error_msg = malloc (len + 1);
    strcpy (gaia_geos_error_msg, msg);
}

GAIAGEO_DECLARE void
gaiaSetGeosWarningMsg (const char *msg)
{
/* setting the latest GEOS error message */
    int len;
    if (gaia_geos_warning_msg != NULL)
	free (gaia_geos_warning_msg);
    gaia_geos_warning_msg = NULL;
    if (msg == NULL)
	return;
    len = strlen (msg);
    gaia_geos_warning_msg = malloc (len + 1);
    strcpy (gaia_geos_warning_msg, msg);
}

GAIAGEO_DECLARE void
gaiaSetGeosAuxErrorMsg (const char *msg)
{
/* setting the latest GEOS (auxiliary) error message */
    int len;
    if (gaia_geosaux_error_msg != NULL)
	free (gaia_geosaux_error_msg);
    gaia_geosaux_error_msg = NULL;
    if (msg == NULL)
	return;
    len = strlen (msg);
    gaia_geosaux_error_msg = malloc (len + 1);
    strcpy (gaia_geosaux_error_msg, msg);
}

static int
check_point (double *coords, int points, double x, double y)
{
/* checks if [X,Y] point is defined into this coordinate array [Linestring or Ring] */
    int iv;
    double xx;
    double yy;
    for (iv = 0; iv < points; iv++)
      {
	  gaiaGetPoint (coords, iv, &xx, &yy);
	  if (xx == x && yy == y)
	      return 1;
      }
    return 0;
}

GAIAGEO_DECLARE int
gaiaLinestringEquals (gaiaLinestringPtr line1, gaiaLinestringPtr line2)
{
/* checks if two Linestrings are "spatially equal" */
    int iv;
    double x;
    double y;
    if (line1->Points != line2->Points)
	return 0;
    for (iv = 0; iv < line1->Points; iv++)
      {
	  gaiaGetPoint (line1->Coords, iv, &x, &y);
	  if (!check_point (line2->Coords, line2->Points, x, y))
	      return 0;
      }
    return 1;
}

GAIAGEO_DECLARE int
gaiaPolygonEquals (gaiaPolygonPtr polyg1, gaiaPolygonPtr polyg2)
{
/* checks if two Polygons are "spatially equal" */
    int ib;
    int ib2;
    int iv;
    int ok2;
    double x;
    double y;
    gaiaRingPtr ring1;
    gaiaRingPtr ring2;
    if (polyg1->NumInteriors != polyg2->NumInteriors)
	return 0;
/* checking the EXTERIOR RINGs */
    ring1 = polyg1->Exterior;
    ring2 = polyg2->Exterior;
    if (ring1->Points != ring2->Points)
	return 0;
    for (iv = 0; iv < ring1->Points; iv++)
      {
	  gaiaGetPoint (ring1->Coords, iv, &x, &y);
	  if (!check_point (ring2->Coords, ring2->Points, x, y))
	      return 0;
      }
    for (ib = 0; ib < polyg1->NumInteriors; ib++)
      {
	  /* checking the INTERIOR RINGS */
	  int ok = 0;
	  ring1 = polyg1->Interiors + ib;
	  for (ib2 = 0; ib2 < polyg2->NumInteriors; ib2++)
	    {
		ok2 = 1;
		ring2 = polyg2->Interiors + ib2;
		for (iv = 0; iv < ring1->Points; iv++)
		  {
		      gaiaGetPoint (ring1->Coords, iv, &x, &y);
		      if (!check_point (ring2->Coords, ring2->Points, x, y))
			{
			    ok2 = 0;
			    break;
			}
		  }
		if (ok2)
		  {
		      ok = 1;
		      break;
		  }
	    }
	  if (!ok)
	      return 0;
      }
    return 1;
}

#ifndef OMIT_GEOS		/* including GEOS */

static int
splite_mbr_overlaps (gaiaGeomCollPtr g1, gaiaGeomCollPtr g2)
{
/* checks if two MBRs do overlap */
    if (g1->MaxX < g2->MinX)
	return 0;
    if (g1->MinX > g2->MaxX)
	return 0;
    if (g1->MaxY < g2->MinY)
	return 0;
    if (g1->MinY > g2->MaxY)
	return 0;
    return 1;
}

static int
splite_mbr_contains (gaiaGeomCollPtr g1, gaiaGeomCollPtr g2)
{
/* checks if MBR#1 fully contains MBR#2 */
    if (g2->MinX < g1->MinX)
	return 0;
    if (g2->MaxX > g1->MaxX)
	return 0;
    if (g2->MinY < g1->MinY)
	return 0;
    if (g2->MaxY > g1->MaxY)
	return 0;
    return 1;
}

static int
splite_mbr_within (gaiaGeomCollPtr g1, gaiaGeomCollPtr g2)
{
/* checks if MBR#1 is fully contained within MBR#2 */
    if (g1->MinX < g2->MinX)
	return 0;
    if (g1->MaxX > g2->MaxX)
	return 0;
    if (g1->MinY < g2->MinY)
	return 0;
    if (g1->MaxY > g2->MaxY)
	return 0;
    return 1;
}

static int
splite_mbr_equals (gaiaGeomCollPtr g1, gaiaGeomCollPtr g2)
{
/* checks if MBR#1 equals MBR#2 */
    if (g1->MinX != g2->MinX)
	return 0;
    if (g1->MaxX != g2->MaxX)
	return 0;
    if (g1->MinY != g2->MinY)
	return 0;
    if (g1->MaxY != g2->MaxY)
	return 0;
    return 1;
}

static int
evalGeosCacheItem (unsigned char *blob, int blob_size, uLong crc,
		   struct splite_geos_cache_item *p)
{
/* evaluting if this one could be a valid cache hit */
    if (blob_size != p->gaiaBlobSize)
      {
	  /* surely not a match; different size */
	  return 0;
      }
    if (crc != p->crc32)
      {
	  /* surely not a match: different CRC32 */
	  return 0;
      }

/* the first 46 bytes of the BLOB contain the MBR,
   the SRID and the Type; so are assumed to represent 
   a valid signature */
    if (memcmp (blob, p->gaiaBlob, 46) == 0)
	return 1;
    return 0;
}

static int
evalGeosCache (struct splite_internal_cache *cache, gaiaGeomCollPtr geom1,
	       unsigned char *blob1, int size1, gaiaGeomCollPtr geom2,
	       unsigned char *blob2, int size2, GEOSPreparedGeometry ** gPrep,
	       gaiaGeomCollPtr * geom)
{
/* handling the internal GEOS cache */
    struct splite_geos_cache_item *p1 = &(cache->cacheItem1);
    struct splite_geos_cache_item *p2 = &(cache->cacheItem2);
    uLong crc1 = crc32 (0L, blob1, size1);
    uLong crc2 = crc32 (0L, blob2, size2);
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return 0;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return 0;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return 0;

/* checking the first cache item */
    if (evalGeosCacheItem (blob1, size1, crc1, p1))
      {
	  /* found a matching item */
	  if (p1->preparedGeosGeom == NULL)
	    {
		/* preparing the GeosGeometries */
		p1->geosGeom = gaiaToGeos_r (cache, geom1);
		if (p1->geosGeom)
		  {
		      p1->preparedGeosGeom =
			  (void *) GEOSPrepare_r (handle, p1->geosGeom);
		      if (p1->preparedGeosGeom == NULL)
			{
			    /* unexpected failure */
			    GEOSGeom_destroy_r (handle, p1->geosGeom);
			    p1->geosGeom = NULL;
			}
		  }
	    }
	  if (p1->preparedGeosGeom)
	    {
		/* returning the corresponding GeosPreparedGeometry */
		*gPrep = p1->preparedGeosGeom;
		*geom = geom2;
		return 1;
	    }
	  return 0;
      }

/* checking the second cache item */
    if (evalGeosCacheItem (blob2, size2, crc2, p2))
      {
	  /* found a matching item */
	  if (p2->preparedGeosGeom == NULL)
	    {
		/* preparing the GeosGeometries */
		p2->geosGeom = gaiaToGeos_r (cache, geom2);
		if (p2->geosGeom)
		  {
		      p2->preparedGeosGeom =
			  (void *) GEOSPrepare_r (handle, p2->geosGeom);
		      if (p2->preparedGeosGeom == NULL)
			{
			    /* unexpected failure */
			    GEOSGeom_destroy_r (handle, p2->geosGeom);
			    p2->geosGeom = NULL;
			}
		  }
	    }
	  if (p2->preparedGeosGeom)
	    {
		/* returning the corresponding GeosPreparedGeometry */
		*gPrep = p2->preparedGeosGeom;
		*geom = geom1;
		return 1;
	    }
	  return 0;
      }

/* updating the GEOS cache item#1 */
    memcpy (p1->gaiaBlob, blob1, 46);
    p1->gaiaBlobSize = size1;
    p1->crc32 = crc1;
    if (p1->preparedGeosGeom)
	GEOSPreparedGeom_destroy_r (handle, p1->preparedGeosGeom);
    if (p1->geosGeom)
	GEOSGeom_destroy_r (handle, p1->geosGeom);
    p1->geosGeom = NULL;
    p1->preparedGeosGeom = NULL;

/* updating the GEOS cache item#2 */
    memcpy (p2->gaiaBlob, blob2, 46);
    p2->gaiaBlobSize = size2;
    p2->crc32 = crc2;
    if (p2->preparedGeosGeom)
	GEOSPreparedGeom_destroy_r (handle, p2->preparedGeosGeom);
    if (p2->geosGeom)
	GEOSGeom_destroy_r (handle, p2->geosGeom);
    p2->geosGeom = NULL;
    p2->preparedGeosGeom = NULL;

    return 0;
}

GAIAGEO_DECLARE int
gaiaGeomCollEquals (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries are "spatially equal" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_equals (geom1, geom2))
	return 0;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSEquals (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollEquals_r (const void *p_cache, gaiaGeomCollPtr geom1,
		      gaiaGeomCollPtr geom2)
{
/* checks if two Geometries are "spatially equal" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_equals (geom1, geom2))
	return 0;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSEquals_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollIntersects (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially intersects" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSIntersects (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollIntersects_r (const void *p_cache, gaiaGeomCollPtr geom1,
			  gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially intersects" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSIntersects_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollPreparedIntersects (const void *p_cache, gaiaGeomCollPtr geom1,
				unsigned char *blob1, int size1,
				gaiaGeomCollPtr geom2, unsigned char *blob2,
				int size2)
{
/* checks if two Geometries do "spatially intersects" */
    int ret;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSPreparedGeometry *gPrep;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaGeomCollPtr geom;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

/* handling the internal GEOS cache */
    if (evalGeosCache
	(cache, geom1, blob1, size1, geom2, blob2, size2, &gPrep, &geom))
      {
	  g2 = gaiaToGeos_r (cache, geom);
	  ret = GEOSPreparedIntersects_r (handle, gPrep, g2);
	  GEOSGeom_destroy_r (handle, g2);
	  return ret;
      }
    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSIntersects_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollDisjoint (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries are "spatially disjoint" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 1;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSDisjoint (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollDisjoint_r (const void *p_cache, gaiaGeomCollPtr geom1,
			gaiaGeomCollPtr geom2)
{
/* checks if two Geometries are "spatially disjoint" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 1;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSDisjoint_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollPreparedDisjoint (const void *p_cache, gaiaGeomCollPtr geom1,
			      unsigned char *blob1, int size1,
			      gaiaGeomCollPtr geom2, unsigned char *blob2,
			      int size2)
{
/* checks if two Geometries are "spatially disjoint" */
    int ret;
    GEOSPreparedGeometry *gPrep;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaGeomCollPtr geom;
    GEOSContextHandle_t handle = NULL;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 1;

/* handling the internal GEOS cache */
    if (evalGeosCache
	(cache, geom1, blob1, size1, geom2, blob2, size2, &gPrep, &geom))
      {
	  g2 = gaiaToGeos_r (cache, geom);
	  ret = GEOSPreparedDisjoint_r (handle, gPrep, g2);
	  GEOSGeom_destroy_r (handle, g2);
	  return ret;
      }

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSDisjoint_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollOverlaps (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially overlaps" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSOverlaps (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollOverlaps_r (const void *p_cache, gaiaGeomCollPtr geom1,
			gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially overlaps" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSOverlaps_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollPreparedOverlaps (const void *p_cache, gaiaGeomCollPtr geom1,
			      unsigned char *blob1, int size1,
			      gaiaGeomCollPtr geom2, unsigned char *blob2,
			      int size2)
{
/* checks if two Geometries do "spatially overlaps" */
    int ret;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSPreparedGeometry *gPrep;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaGeomCollPtr geom;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

/* handling the internal GEOS cache */
    if (evalGeosCache
	(cache, geom1, blob1, size1, geom2, blob2, size2, &gPrep, &geom))
      {
	  g2 = gaiaToGeos_r (cache, geom);
	  ret = GEOSPreparedOverlaps_r (handle, gPrep, g2);
	  GEOSGeom_destroy_r (handle, g2);
	  return ret;
      }

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSOverlaps_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollCrosses (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially crosses" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSCrosses (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollCrosses_r (const void *p_cache, gaiaGeomCollPtr geom1,
		       gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially crosses" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSCrosses_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollPreparedCrosses (const void *p_cache, gaiaGeomCollPtr geom1,
			     unsigned char *blob1, int size1,
			     gaiaGeomCollPtr geom2, unsigned char *blob2,
			     int size2)
{
/* checks if two Geometries do "spatially crosses" */
    int ret;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSPreparedGeometry *gPrep;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaGeomCollPtr geom;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

/* handling the internal GEOS cache */
    if (evalGeosCache
	(cache, geom1, blob1, size1, geom2, blob2, size2, &gPrep, &geom))
      {
	  g2 = gaiaToGeos_r (cache, geom);
	  ret = GEOSPreparedCrosses_r (handle, gPrep, g2);
	  GEOSGeom_destroy_r (handle, g2);
	  return ret;
      }

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSCrosses_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollTouches (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially touches" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSTouches (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollTouches_r (const void *p_cache, gaiaGeomCollPtr geom1,
		       gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially touches" */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSTouches_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollPreparedTouches (const void *p_cache, gaiaGeomCollPtr geom1,
			     unsigned char *blob1, int size1,
			     gaiaGeomCollPtr geom2, unsigned char *blob2,
			     int size2)
{
/* checks if two Geometries do "spatially touches" */
    int ret;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSPreparedGeometry *gPrep;
    gaiaGeomCollPtr geom;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return 0;

/* handling the internal GEOS cache */
    if (evalGeosCache
	(cache, geom1, blob1, size1, geom2, blob2, size2, &gPrep, &geom))
      {
	  g2 = gaiaToGeos_r (cache, geom);
	  ret = GEOSPreparedTouches_r (handle, gPrep, g2);
	  GEOSGeom_destroy_r (handle, g2);
	  return ret;
      }

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSTouches_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollWithin (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if GEOM-1 is completely contained within GEOM-2 */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_within (geom1, geom2))
	return 0;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSWithin (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollWithin_r (const void *p_cache, gaiaGeomCollPtr geom1,
		      gaiaGeomCollPtr geom2)
{
/* checks if GEOM-1 is completely contained within GEOM-2 */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_within (geom1, geom2))
	return 0;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSWithin_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollPreparedWithin (const void *p_cache, gaiaGeomCollPtr geom1,
			    unsigned char *blob1, int size1,
			    gaiaGeomCollPtr geom2, unsigned char *blob2,
			    int size2)
{
/* checks if GEOM-1 is completely contained within GEOM-2 */
    int ret;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSPreparedGeometry *gPrep;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaGeomCollPtr geom;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_within (geom1, geom2))
	return 0;

/* handling the internal GEOS cache */
    if (evalGeosCache
	(cache, geom1, blob1, size1, geom2, blob2, size2, &gPrep, &geom))
      {
	  g2 = gaiaToGeos_r (cache, geom);
	  if (geom == geom2)
	      ret = GEOSPreparedWithin_r (handle, gPrep, g2);
	  else
	      ret = GEOSPreparedContains_r (handle, gPrep, g2);
	  GEOSGeom_destroy_r (handle, g2);
	  return ret;
      }

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSWithin_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollContains (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if GEOM-1 completely contains GEOM-2 */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_contains (geom1, geom2))
	return 0;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSContains (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollContains_r (const void *p_cache, gaiaGeomCollPtr geom1,
			gaiaGeomCollPtr geom2)
{
/* checks if GEOM-1 completely contains GEOM-2 */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_contains (geom1, geom2))
	return 0;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSContains_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollPreparedContains (const void *p_cache, gaiaGeomCollPtr geom1,
			      unsigned char *blob1, int size1,
			      gaiaGeomCollPtr geom2, unsigned char *blob2,
			      int size2)
{
/* checks if GEOM-1 completely contains GEOM-2 */
    int ret;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSPreparedGeometry *gPrep;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaGeomCollPtr geom;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_contains (geom1, geom2))
	return 0;

/* handling the internal GEOS cache */
    if (evalGeosCache
	(cache, geom1, blob1, size1, geom2, blob2, size2, &gPrep, &geom))
      {
	  g2 = gaiaToGeos_r (cache, geom);
	  if (geom == geom2)
	      ret = GEOSPreparedContains_r (handle, gPrep, g2);
	  else
	      ret = GEOSPreparedWithin_r (handle, gPrep, g2);
	  GEOSGeom_destroy_r (handle, g2);
	  return ret;
      }

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSContains_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollRelate (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2,
		    const char *pattern)
{
/* checks if if GEOM-1 and GEOM-2 have a spatial relationship as specified by the pattern Matrix */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return -1;
    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSRelatePattern (g1, g2, pattern);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollRelate_r (const void *p_cache, gaiaGeomCollPtr geom1,
		      gaiaGeomCollPtr geom2, const char *pattern)
{
/* checks if if GEOM-1 and GEOM-2 have a spatial relationship as specified by the pattern Matrix */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return -1;
    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSRelatePattern_r (handle, g1, g2, pattern);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollLength (gaiaGeomCollPtr geom, double *xlength)
{
/* computes the total length for this Geometry */
    double length;
    int ret;
    GEOSGeometry *g;
    gaiaResetGeosMsg ();
    if (!geom)
	return 0;
    if (gaiaIsToxic (geom))
	return 0;
    g = gaiaToGeos (geom);
    ret = GEOSLength (g, &length);
    GEOSGeom_destroy (g);
    if (ret)
	*xlength = length;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollLength_r (const void *p_cache, gaiaGeomCollPtr geom,
		      double *xlength)
{
/* computes the total length for this Geometry */
    double length;
    int ret;
    GEOSGeometry *g;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return 0;
    if (gaiaIsToxic_r (cache, geom))
	return 0;
    g = gaiaToGeos_r (cache, geom);
    ret = GEOSLength_r (handle, g, &length);
    GEOSGeom_destroy_r (handle, g);
    if (ret)
	*xlength = length;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollLengthOrPerimeter (gaiaGeomCollPtr geom, int perimeter,
			       double *xlength)
{
/* computes the total length or perimeter for this Geometry */
    double length;
    int ret;
    GEOSGeometry *g;
    int mode = GAIA2GEOS_ONLY_LINESTRINGS;
    if (perimeter)
	mode = GAIA2GEOS_ONLY_POLYGONS;
    gaiaResetGeosMsg ();
    if (!geom)
	return 0;
    if (gaiaIsToxic (geom))
	return 0;
    g = gaiaToGeosSelective (geom, mode);
    if (g == NULL)
      {
	  *xlength = 0.0;
	  return 1;
      }
    ret = GEOSLength (g, &length);
    GEOSGeom_destroy (g);
    if (ret)
	*xlength = length;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollLengthOrPerimeter_r (const void *p_cache, gaiaGeomCollPtr geom,
				 int perimeter, double *xlength)
{
/* computes the total length or perimeter for this Geometry */
    double length;
    int ret;
    int mode = GAIA2GEOS_ONLY_LINESTRINGS;
    GEOSGeometry *g;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    if (perimeter)
	mode = GAIA2GEOS_ONLY_POLYGONS;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return 0;
    if (gaiaIsToxic_r (cache, geom))
	return 0;
    g = gaiaToGeosSelective_r (cache, geom, mode);
    if (g == NULL)
      {
	  *xlength = 0.0;
	  return 1;
      }
    ret = GEOSLength_r (handle, g, &length);
    GEOSGeom_destroy_r (handle, g);
    if (ret)
	*xlength = length;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollArea (gaiaGeomCollPtr geom, double *xarea)
{
/* computes the total area for this Geometry */
    double area;
    int ret;
    GEOSGeometry *g;
    gaiaResetGeosMsg ();
    if (!geom)
	return 0;
    if (gaiaIsToxic (geom))
	return 0;
    g = gaiaToGeos (geom);
    ret = GEOSArea (g, &area);
    GEOSGeom_destroy (g);
    if (ret)
	*xarea = area;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollArea_r (const void *p_cache, gaiaGeomCollPtr geom, double *xarea)
{
/* computes the total area for this Geometry */
    double area;
    int ret;
    GEOSGeometry *g;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return 0;
    if (gaiaIsToxic_r (cache, geom))
	return 0;
    g = gaiaToGeos_r (cache, geom);
    ret = GEOSArea_r (handle, g, &area);
    GEOSGeom_destroy_r (handle, g);
    if (ret)
	*xarea = area;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollDistance (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2,
		      double *xdist)
{
/* computes the minimum distance intercurring between GEOM-1 and GEOM-2 */
    double dist;
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return 0;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return 0;
    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSDistance (g1, g2, &dist);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (ret)
	*xdist = dist;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollDistance_r (const void *p_cache, gaiaGeomCollPtr geom1,
			gaiaGeomCollPtr geom2, double *xdist)
{
/* computes the minimum distance intercurring between GEOM-1 and GEOM-2 */
    double dist;
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return 0;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return 0;
    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSDistance_r (handle, g1, g2, &dist);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    if (ret)
	*xdist = dist;
    return ret;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryIntersection (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial intersection" of GEOM-1 and GEOM-2 */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return NULL;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return NULL;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return NULL;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    g3 = GEOSIntersection (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (!g3)
	return NULL;
    if (geom1->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g3);
    else if (geom1->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g3);
    else if (geom1->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g3);
    else
	geo = gaiaFromGeos_XY (g3);
    GEOSGeom_destroy (g3);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom1->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryIntersection_r (const void *p_cache, gaiaGeomCollPtr geom1,
			    gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial intersection" of GEOM-1 and GEOM-2 */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return NULL;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return NULL;

/* quick check based on MBRs comparison */
    if (!splite_mbr_overlaps (geom1, geom2))
	return NULL;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    g3 = GEOSIntersection_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    if (!g3)
	return NULL;
    if (geom1->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g3);
    else if (geom1->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g3);
    else if (geom1->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g3);
    else
	geo = gaiaFromGeos_XY_r (cache, g3);
    GEOSGeom_destroy_r (handle, g3);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom1->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryUnion (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial union" of GEOM-1 and GEOM-2 */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return NULL;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return NULL;
    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    g3 = GEOSUnion (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (geom1->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g3);
    else if (geom1->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g3);
    else if (geom1->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g3);
    else
	geo = gaiaFromGeos_XY (g3);
    GEOSGeom_destroy (g3);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom1->Srid;
    if (geo->DeclaredType == GAIA_POINT &&
	geom1->DeclaredType == GAIA_MULTIPOINT)
	geo->DeclaredType = GAIA_MULTIPOINT;
    if (geo->DeclaredType == GAIA_LINESTRING &&
	geom1->DeclaredType == GAIA_MULTILINESTRING)
	geo->DeclaredType = GAIA_MULTILINESTRING;
    if (geo->DeclaredType == GAIA_POLYGON &&
	geom1->DeclaredType == GAIA_MULTIPOLYGON)
	geo->DeclaredType = GAIA_MULTIPOLYGON;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryUnion_r (const void *p_cache, gaiaGeomCollPtr geom1,
		     gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial union" of GEOM-1 and GEOM-2 */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return NULL;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return NULL;
    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    g3 = GEOSUnion_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    if (geom1->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g3);
    else if (geom1->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g3);
    else if (geom1->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g3);
    else
	geo = gaiaFromGeos_XY_r (cache, g3);
    GEOSGeom_destroy_r (handle, g3);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom1->Srid;
    if (geo->DeclaredType == GAIA_POINT &&
	geom1->DeclaredType == GAIA_MULTIPOINT)
	geo->DeclaredType = GAIA_MULTIPOINT;
    if (geo->DeclaredType == GAIA_LINESTRING &&
	geom1->DeclaredType == GAIA_MULTILINESTRING)
	geo->DeclaredType = GAIA_MULTILINESTRING;
    if (geo->DeclaredType == GAIA_POLYGON &&
	geom1->DeclaredType == GAIA_MULTIPOLYGON)
	geo->DeclaredType = GAIA_MULTIPOLYGON;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaUnionCascaded (gaiaGeomCollPtr geom)
{
/* UnionCascaded (single Collection of polygons) */
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaGeomCollPtr result;
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaResetGeosMsg ();
    if (!geom)
	return NULL;
    if (gaiaIsToxic (geom))
	return NULL;

/* testing if geom only contains Polygons */
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
    if (pts || lns)
	return NULL;
    if (!pgs)
	return NULL;

    g1 = gaiaToGeos (geom);
    g2 = GEOSUnionCascaded (g1);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	result = gaiaFromGeos_XYZ (g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	result = gaiaFromGeos_XYM (g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	result = gaiaFromGeos_XYZM (g2);
    else
	result = gaiaFromGeos_XY (g2);
    GEOSGeom_destroy (g2);
    if (result == NULL)
	return NULL;
    result->Srid = geom->Srid;
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaUnionCascaded_r (const void *p_cache, gaiaGeomCollPtr geom)
{
/* UnionCascaded (single Collection of polygons) */
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaGeomCollPtr result;
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return NULL;
    if (gaiaIsToxic_r (cache, geom))
	return NULL;

/* testing if geom only contains Polygons */
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
    if (pts || lns)
	return NULL;
    if (!pgs)
	return NULL;

    g1 = gaiaToGeos_r (cache, geom);
    g2 = GEOSUnionCascaded_r (handle, g1);
    GEOSGeom_destroy_r (handle, g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	result = gaiaFromGeos_XYZ_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	result = gaiaFromGeos_XYM_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	result = gaiaFromGeos_XYZM_r (cache, g2);
    else
	result = gaiaFromGeos_XY_r (cache, g2);
    GEOSGeom_destroy_r (handle, g2);
    if (result == NULL)
	return NULL;
    result->Srid = geom->Srid;
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryDifference (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial difference" of GEOM-1 and GEOM-2 */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return NULL;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return NULL;
    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    g3 = GEOSDifference (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (!g3)
	return NULL;
    if (geom1->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g3);
    else if (geom1->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g3);
    else if (geom1->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g3);
    else
	geo = gaiaFromGeos_XY (g3);
    GEOSGeom_destroy (g3);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom1->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryDifference_r (const void *p_cache, gaiaGeomCollPtr geom1,
			  gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial difference" of GEOM-1 and GEOM-2 */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return NULL;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return NULL;
    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    g3 = GEOSDifference_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    if (!g3)
	return NULL;
    if (geom1->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g3);
    else if (geom1->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g3);
    else if (geom1->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g3);
    else
	geo = gaiaFromGeos_XY_r (cache, g3);
    GEOSGeom_destroy_r (handle, g3);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom1->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometrySymDifference (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial symmetric difference" of GEOM-1 and GEOM-2 */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return NULL;
    if (gaiaIsToxic (geom1) || gaiaIsToxic (geom2))
	return NULL;
    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    g3 = GEOSSymDifference (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (!g3)
	return NULL;
    if (geom1->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g3);
    else if (geom1->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g3);
    else if (geom1->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g3);
    else
	geo = gaiaFromGeos_XY (g3);
    GEOSGeom_destroy (g3);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom1->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometrySymDifference_r (const void *p_cache, gaiaGeomCollPtr geom1,
			     gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial symmetric difference" of GEOM-1 and GEOM-2 */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return NULL;
    if (gaiaIsToxic_r (cache, geom1) || gaiaIsToxic_r (cache, geom2))
	return NULL;
    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    g3 = GEOSSymDifference_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    if (!g3)
	return NULL;
    if (geom1->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g3);
    else if (geom1->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g3);
    else if (geom1->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g3);
    else
	geo = gaiaFromGeos_XY_r (cache, g3);
    GEOSGeom_destroy_r (handle, g3);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom1->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaBoundary (gaiaGeomCollPtr geom)
{
/* builds a new geometry representing the combinatorial boundary of GEOM */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom)
	return NULL;
    if (gaiaIsToxic (geom))
	return NULL;
    g1 = gaiaToGeos (geom);
    g2 = GEOSBoundary (g1);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g2);
    else
	geo = gaiaFromGeos_XY (g2);
    GEOSGeom_destroy (g2);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaBoundary_r (const void *p_cache, gaiaGeomCollPtr geom)
{
/* builds a new geometry representing the combinatorial boundary of GEOM */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return NULL;
    if (gaiaIsToxic_r (cache, geom))
	return NULL;
    g1 = gaiaToGeos_r (cache, geom);
    g2 = GEOSBoundary_r (handle, g1);
    GEOSGeom_destroy_r (handle, g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g2);
    else
	geo = gaiaFromGeos_XY_r (cache, g2);
    GEOSGeom_destroy_r (handle, g2);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom->Srid;
    return geo;
}

GAIAGEO_DECLARE int
gaiaGeomCollCentroid (gaiaGeomCollPtr geom, double *x, double *y)
{
/* returns a Point representing the centroid for this Geometry */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom)
	return 0;
    if (gaiaIsToxic (geom))
      {
	  return 0;
      }
    g1 = gaiaToGeos (geom);
    g2 = GEOSGetCentroid (g1);
    GEOSGeom_destroy (g1);
    if (!g2)
	return 0;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g2);
    else
	geo = gaiaFromGeos_XY (g2);
    GEOSGeom_destroy (g2);
    if (geo == NULL)
	return 0;
    if (geo->FirstPoint)
      {
	  *x = geo->FirstPoint->X;
	  *y = geo->FirstPoint->Y;
	  gaiaFreeGeomColl (geo);
	  return 1;
      }
    gaiaFreeGeomColl (geo);
    return 0;
}

GAIAGEO_DECLARE int
gaiaGeomCollCentroid_r (const void *p_cache, gaiaGeomCollPtr geom, double *x,
			double *y)
{
/* returns a Point representing the centroid for this Geometry */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return 0;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return 0;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return 0;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return 0;
    if (gaiaIsToxic_r (cache, geom))
      {
	  return 0;
      }
    g1 = gaiaToGeos_r (cache, geom);
    g2 = GEOSGetCentroid_r (handle, g1);
    GEOSGeom_destroy_r (handle, g1);
    if (!g2)
	return 0;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g2);
    else
	geo = gaiaFromGeos_XY_r (cache, g2);
    GEOSGeom_destroy_r (handle, g2);
    if (geo == NULL)
	return 0;
    if (geo->FirstPoint)
      {
	  *x = geo->FirstPoint->X;
	  *y = geo->FirstPoint->Y;
	  gaiaFreeGeomColl (geo);
	  return 1;
      }
    gaiaFreeGeomColl (geo);
    return 0;
}

GAIAGEO_DECLARE int
gaiaGetPointOnSurface (gaiaGeomCollPtr geom, double *x, double *y)
{
/* returns a Point guaranteed to lie on the Surface */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom)
	return 0;
    if (gaiaIsToxic (geom))
      {
	  return 0;
      }
    g1 = gaiaToGeos (geom);
    g2 = GEOSPointOnSurface (g1);
    GEOSGeom_destroy (g1);
    if (!g2)
	return 0;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g2);
    else
	geo = gaiaFromGeos_XY (g2);
    GEOSGeom_destroy (g2);
    if (geo == NULL)
	return 0;
    if (geo->FirstPoint)
      {
	  *x = geo->FirstPoint->X;
	  *y = geo->FirstPoint->Y;
	  gaiaFreeGeomColl (geo);
	  return 1;
      }
    gaiaFreeGeomColl (geo);
    return 0;
}

GAIAGEO_DECLARE int
gaiaGetPointOnSurface_r (const void *p_cache, gaiaGeomCollPtr geom, double *x,
			 double *y)
{
/* returns a Point guaranteed to lie on the Surface */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return 0;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return 0;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return 0;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return 0;
    if (gaiaIsToxic_r (cache, geom))
      {
	  return 0;
      }
    g1 = gaiaToGeos_r (cache, geom);
    g2 = GEOSPointOnSurface_r (handle, g1);
    GEOSGeom_destroy_r (handle, g1);
    if (!g2)
	return 0;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g2);
    else
	geo = gaiaFromGeos_XY_r (cache, g2);
    GEOSGeom_destroy_r (handle, g2);
    if (geo == NULL)
	return 0;
    if (geo->FirstPoint)
      {
	  *x = geo->FirstPoint->X;
	  *y = geo->FirstPoint->Y;
	  gaiaFreeGeomColl (geo);
	  return 1;
      }
    gaiaFreeGeomColl (geo);
    return 0;
}

GAIAGEO_DECLARE int
gaiaIsSimple (gaiaGeomCollPtr geom)
{
/* checks if this GEOMETRYCOLLECTION is a simple one */
    int ret;
    GEOSGeometry *g;
    gaiaResetGeosMsg ();
    if (!geom)
	return -1;
    if (gaiaIsToxic (geom))
	return 0;
    g = gaiaToGeos (geom);
    ret = GEOSisSimple (g);
    GEOSGeom_destroy (g);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaIsSimple_r (const void *p_cache, gaiaGeomCollPtr geom)
{
/* checks if this GEOMETRYCOLLECTION is a simple one */
    int ret;
    GEOSGeometry *g;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return -1;
    if (gaiaIsToxic_r (cache, geom))
	return -1;
    g = gaiaToGeos_r (cache, geom);
    ret = GEOSisSimple_r (handle, g);
    GEOSGeom_destroy_r (handle, g);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaIsRing (gaiaLinestringPtr line)
{
/* checks if this LINESTRING can be a valid RING */
    gaiaGeomCollPtr geo;
    gaiaLinestringPtr line2;
    int ret;
    int iv;
    double x;
    double y;
    double z;
    double m;
    GEOSGeometry *g;
    gaiaResetGeosMsg ();
    if (!line)
	return -1;
    if (line->DimensionModel == GAIA_XY_Z)
	geo = gaiaAllocGeomCollXYZ ();
    else if (line->DimensionModel == GAIA_XY_M)
	geo = gaiaAllocGeomCollXYM ();
    else if (line->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaAllocGeomCollXYZM ();
    else
	geo = gaiaAllocGeomColl ();
    line2 = gaiaAddLinestringToGeomColl (geo, line->Points);
    for (iv = 0; iv < line2->Points; iv++)
      {
	  z = 0.0;
	  m = 0.0;
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
	  if (line2->DimensionModel == GAIA_XY_Z)
	    {
		gaiaSetPointXYZ (line2->Coords, iv, x, y, z);
	    }
	  else if (line2->DimensionModel == GAIA_XY_M)
	    {
		gaiaSetPointXYM (line2->Coords, iv, x, y, m);
	    }
	  else if (line2->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaSetPointXYZM (line2->Coords, iv, x, y, z, m);
	    }
	  else
	    {
		gaiaSetPoint (line2->Coords, iv, x, y);
	    }
      }
    if (gaiaIsToxic (geo))
      {
	  gaiaFreeGeomColl (geo);
	  return -1;
      }
    g = gaiaToGeos (geo);
    gaiaFreeGeomColl (geo);
    ret = GEOSisRing (g);
    GEOSGeom_destroy (g);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaIsRing_r (const void *p_cache, gaiaLinestringPtr line)
{
/* checks if this LINESTRING can be a valid RING */
    gaiaGeomCollPtr geo;
    gaiaLinestringPtr line2;
    int ret;
    int iv;
    double x;
    double y;
    double z;
    double m;
    GEOSGeometry *g;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!line)
	return -1;
    if (line->DimensionModel == GAIA_XY_Z)
	geo = gaiaAllocGeomCollXYZ ();
    else if (line->DimensionModel == GAIA_XY_M)
	geo = gaiaAllocGeomCollXYM ();
    else if (line->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaAllocGeomCollXYZM ();
    else
	geo = gaiaAllocGeomColl ();
    line2 = gaiaAddLinestringToGeomColl (geo, line->Points);
    for (iv = 0; iv < line2->Points; iv++)
      {
	  z = 0.0;
	  m = 0.0;
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
	  if (line2->DimensionModel == GAIA_XY_Z)
	    {
		gaiaSetPointXYZ (line2->Coords, iv, x, y, z);
	    }
	  else if (line2->DimensionModel == GAIA_XY_M)
	    {
		gaiaSetPointXYM (line2->Coords, iv, x, y, m);
	    }
	  else if (line2->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaSetPointXYZM (line2->Coords, iv, x, y, z, m);
	    }
	  else
	    {
		gaiaSetPoint (line2->Coords, iv, x, y);
	    }
      }
    if (gaiaIsToxic_r (cache, geo))
      {
	  gaiaFreeGeomColl (geo);
	  return -1;
      }
    g = gaiaToGeos_r (cache, geo);
    gaiaFreeGeomColl (geo);
    ret = GEOSisRing_r (handle, g);
    GEOSGeom_destroy_r (handle, g);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaIsValid (gaiaGeomCollPtr geom)
{
/* checks if this GEOMETRYCOLLECTION is a valid one */
    int ret;
    GEOSGeometry *g;
    gaiaResetGeosMsg ();
    if (!geom)
	return -1;
    if (gaiaIsToxic (geom))
	return 0;
    if (gaiaIsNotClosedGeomColl (geom))
	return 0;
    g = gaiaToGeos (geom);
    ret = GEOSisValid (g);
    GEOSGeom_destroy (g);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaIsValid_r (const void *p_cache, gaiaGeomCollPtr geom)
{
/* checks if this GEOMETRYCOLLECTION is a valid one */
    int ret;
    GEOSGeometry *g;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return -1;
    if (gaiaIsToxic_r (cache, geom))
	return 0;
    if (gaiaIsNotClosedGeomColl_r (cache, geom))
	return 0;
    g = gaiaToGeos_r (cache, geom);
    ret = GEOSisValid_r (handle, g);
    GEOSGeom_destroy_r (handle, g);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE char *
gaiaIsValidReason (gaiaGeomCollPtr geom)
{
/* return a TEXT string stating if a Geometry is valid
/ and if not valid, a reason why */
    char *text;
    int len;
    const char *str;
    char *gstr;
    GEOSGeometry *g;
    gaiaResetGeosMsg ();
    if (!geom)
      {
	  str = "Invalid: NULL Geometry";
	  len = strlen (str);
	  text = malloc (len + 1);
	  strcpy (text, str);
	  return text;
      }
    if (gaiaIsToxic (geom))
      {
	  str = "Invalid: Toxic Geometry ... too few points";
	  len = strlen (str);
	  text = malloc (len + 1);
	  strcpy (text, str);
	  return text;
      }
    if (gaiaIsNotClosedGeomColl (geom))
      {
	  str = "Invalid: Unclosed Rings were detected";
	  len = strlen (str);
	  text = malloc (len + 1);
	  strcpy (text, str);
	  return text;
      }
    g = gaiaToGeos (geom);
    gstr = GEOSisValidReason (g);
    GEOSGeom_destroy (g);
    if (gstr == NULL)
	return NULL;
    len = strlen (gstr);
    text = malloc (len + 1);
    strcpy (text, gstr);
    GEOSFree (gstr);
    return text;
}

GAIAGEO_DECLARE char *
gaiaIsValidReason_r (const void *p_cache, gaiaGeomCollPtr geom)
{
/* return a TEXT string stating if a Geometry is valid
/ and if not valid, a reason why */
    char *text;
    int len;
    const char *str;
    char *gstr;
    GEOSGeometry *g;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
      {
	  str = "Invalid: NULL Geometry";
	  len = strlen (str);
	  text = malloc (len + 1);
	  strcpy (text, str);
	  return text;
      }
    if (gaiaIsToxic (geom))
      {
	  str = "Invalid: Toxic Geometry ... too few points";
	  len = strlen (str);
	  text = malloc (len + 1);
	  strcpy (text, str);
	  return text;
      }
    if (gaiaIsNotClosedGeomColl (geom))
      {
	  str = "Invalid: Unclosed Rings were detected";
	  len = strlen (str);
	  text = malloc (len + 1);
	  strcpy (text, str);
	  return text;
      }
    g = gaiaToGeos_r (cache, geom);
    gstr = GEOSisValidReason_r (handle, g);
    GEOSGeom_destroy_r (handle, g);
    if (gstr == NULL)
	return NULL;
    len = strlen (gstr);
    text = malloc (len + 1);
    strcpy (text, gstr);
    GEOSFree_r (handle, gstr);
    return text;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaIsValidDetail (gaiaGeomCollPtr geom)
{
/* return a Geometry detail causing a Geometry to be invalid */
    char *reason = NULL;
    GEOSGeometry *g;
    GEOSGeometry *d = NULL;
    gaiaGeomCollPtr detail;
    gaiaResetGeosMsg ();
    if (!geom)
	return NULL;
    if (gaiaIsToxic (geom))
	return NULL;
    if (gaiaIsNotClosedGeomColl (geom))
	return NULL;
    g = gaiaToGeos (geom);
    GEOSisValidDetail (g, 0, &reason, &d);
    GEOSGeom_destroy (g);
    if (reason != NULL)
	GEOSFree (reason);
    if (d == NULL)
	return NULL;
    detail = gaiaFromGeos_XY (d);
    GEOSGeom_destroy (d);
    return detail;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaIsValidDetail_r (const void *p_cache, gaiaGeomCollPtr geom)
{
/* return a Geometry detail causing a Geometry to be invalid */
    char *reason = NULL;
    GEOSGeometry *g;
    GEOSGeometry *d = NULL;
    gaiaGeomCollPtr detail;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return NULL;
    if (gaiaIsToxic (geom))
	return NULL;
    if (gaiaIsNotClosedGeomColl (geom))
	return NULL;
    g = gaiaToGeos_r (cache, geom);
    GEOSisValidDetail_r (handle, g, 0, &reason, &d);
    GEOSGeom_destroy_r (handle, g);
    if (reason != NULL)
	GEOSFree_r (handle, reason);
    if (d == NULL)
	return NULL;
    detail = gaiaFromGeos_XY_r (cache, d);
    GEOSGeom_destroy_r (handle, d);
    return detail;
}

GAIAGEO_DECLARE int
gaiaIsClosedGeom_r (const void *cache, gaiaGeomCollPtr geom)
{
/* checks if this geometry is a closed linestring (or multilinestring) */
    int ret = 0;
    gaiaLinestringPtr ln;
    if (cache != NULL)
	gaiaResetGeosMsg_r (cache);
    if (!geom)
	return -1;
    if (cache != NULL)
	ret = gaiaIsToxic_r (cache, geom);
    else
	ret = gaiaIsToxic (geom);
    if (ret)
	return 0;
    ln = geom->FirstLinestring;
    while (ln)
      {
	  /* unhappily GEOS v3.2.2 [system package on Debian Lenny and Ubuntu 12.04]
	   * doesn't exposes the GEOSisClosed() API at all !!!!
	   *
	   GEOSGeometry *g;
	   gaiaGeomCollPtr geoColl = gaiaAllocGeomColl();
	   gaiaInsertLinestringInGeomColl(geoColl, gaiaCloneLinestring(ln));
	   g = gaiaToGeos (geoColl);
	   ret = GEOSisClosed (g);
	   GEOSGeom_destroy (g);
	   gaiaFreeGeomColl(geoColl);
	   */

	  /* so we'll use this internal default in order to circumvent the above issue */
	  double x1;
	  double y1;
	  double z1;
	  double m1;
	  double x2;
	  double y2;
	  double z2;
	  double m2;
	  gaiaLineGetPoint (ln, 0, &x1, &y1, &z1, &m1);
	  gaiaLineGetPoint (ln, ln->Points - 1, &x2, &y2, &z2, &m2);
	  if (x1 == x2 && y1 == y2 && z1 == z2)
	      ret = 1;
	  else
	      ret = 0;
	  if (ret == 0)
	    {
		/* this line isn't closed, so we don't need to continue */
		break;
	    }
	  ln = ln->Next;
      }
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaIsClosedGeom (gaiaGeomCollPtr geom)
{
    gaiaResetGeosMsg ();
    return gaiaIsClosedGeom_r (NULL, geom);
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollSimplify (gaiaGeomCollPtr geom, double tolerance)
{
/* builds a simplified geometry using the Douglas-Peuker algorihtm */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom)
	return NULL;
    if (gaiaIsToxic (geom))
	return NULL;
    g1 = gaiaToGeos (geom);
    g2 = GEOSSimplify (g1, tolerance);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g2);
    else
	geo = gaiaFromGeos_XY (g2);
    GEOSGeom_destroy (g2);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollSimplify_r (const void *p_cache, gaiaGeomCollPtr geom,
			double tolerance)
{
/* builds a simplified geometry using the Douglas-Peuker algorihtm */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return NULL;
    if (gaiaIsToxic_r (cache, geom))
	return NULL;
    g1 = gaiaToGeos_r (cache, geom);
    g2 = GEOSSimplify_r (handle, g1, tolerance);
    GEOSGeom_destroy_r (handle, g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g2);
    else
	geo = gaiaFromGeos_XY_r (cache, g2);
    GEOSGeom_destroy_r (handle, g2);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollSimplifyPreserveTopology (gaiaGeomCollPtr geom, double tolerance)
{
/* builds a simplified geometry using the Douglas-Peuker algorihtm [preserving topology] */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom)
	return NULL;
    if (gaiaIsToxic (geom))
	return NULL;
    g1 = gaiaToGeos (geom);
    g2 = GEOSTopologyPreserveSimplify (g1, tolerance);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g2);
    else
	geo = gaiaFromGeos_XY (g2);
    GEOSGeom_destroy (g2);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollSimplifyPreserveTopology_r (const void *p_cache,
					gaiaGeomCollPtr geom, double tolerance)
{
/* builds a simplified geometry using the Douglas-Peuker algorihtm [preserving topology] */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return NULL;
    if (gaiaIsToxic_r (cache, geom))
	return NULL;
    g1 = gaiaToGeos_r (cache, geom);
    g2 = GEOSTopologyPreserveSimplify_r (handle, g1, tolerance);
    GEOSGeom_destroy_r (handle, g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g2);
    else
	geo = gaiaFromGeos_XY_r (cache, g2);
    GEOSGeom_destroy_r (handle, g2);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaConvexHull (gaiaGeomCollPtr geom)
{
/* builds a geometry that is the convex hull of GEOM */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom)
	return NULL;
    if (gaiaIsToxic (geom))
	return NULL;
    g1 = gaiaToGeos (geom);
    g2 = GEOSConvexHull (g1);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g2);
    else
	geo = gaiaFromGeos_XY (g2);
    GEOSGeom_destroy (g2);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaConvexHull_r (const void *p_cache, gaiaGeomCollPtr geom)
{
/* builds a geometry that is the convex hull of GEOM */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return NULL;
    if (gaiaIsToxic_r (cache, geom))
	return NULL;
    g1 = gaiaToGeos_r (cache, geom);
    g2 = GEOSConvexHull_r (handle, g1);
    GEOSGeom_destroy_r (handle, g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g2);
    else
	geo = gaiaFromGeos_XY_r (cache, g2);
    GEOSGeom_destroy_r (handle, g2);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollBuffer (gaiaGeomCollPtr geom, double radius, int points)
{
/* builds a geometry that is the GIS buffer of GEOM */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom)
	return NULL;
    if (gaiaIsToxic (geom))
	return NULL;
    g1 = gaiaToGeos (geom);
    g2 = GEOSBuffer (g1, radius, points);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ (g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM (g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM (g2);
    else
	geo = gaiaFromGeos_XY (g2);
    GEOSGeom_destroy (g2);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom->Srid;
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollBuffer_r (const void *p_cache, gaiaGeomCollPtr geom, double radius,
		      int points)
{
/* builds a geometry that is the GIS buffer of GEOM */
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    if (!geom)
	return NULL;
    if (gaiaIsToxic_r (cache, geom))
	return NULL;
    g1 = gaiaToGeos_r (cache, geom);
    g2 = GEOSBuffer_r (handle, g1, radius, points);
    GEOSGeom_destroy_r (handle, g1);
    if (!g2)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z)
	geo = gaiaFromGeos_XYZ_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_M)
	geo = gaiaFromGeos_XYM_r (cache, g2);
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	geo = gaiaFromGeos_XYZM_r (cache, g2);
    else
	geo = gaiaFromGeos_XY_r (cache, g2);
    GEOSGeom_destroy_r (handle, g2);
    if (geo == NULL)
	return NULL;
    geo->Srid = geom->Srid;
    return geo;
}

static void
auxFromGeosPolygon (GEOSContextHandle_t handle, const GEOSGeometry * geos,
		    gaiaGeomCollPtr result)
{
/* converting a Polygon from GEOS to SpatiaLite */
    const GEOSGeometry *geos_ring;
    const GEOSCoordSequence *coords;
    unsigned int pts;
    unsigned int geos_dims;
    int interiors;
    int iv;
    int ib;
    double x;
    double y;
    double z;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;

    if (handle != NULL)
      {
	  geos_ring = GEOSGetExteriorRing_r (handle, geos);
	  interiors = GEOSGetNumInteriorRings_r (handle, geos);
	  coords = GEOSGeom_getCoordSeq_r (handle, geos_ring);
	  GEOSCoordSeq_getDimensions_r (handle, coords, &geos_dims);
	  GEOSCoordSeq_getSize_r (handle, coords, &pts);
      }
    else
      {
	  geos_ring = GEOSGetExteriorRing (geos);
	  interiors = GEOSGetNumInteriorRings (geos);
	  coords = GEOSGeom_getCoordSeq (geos_ring);
	  GEOSCoordSeq_getDimensions (coords, &geos_dims);
	  GEOSCoordSeq_getSize (coords, &pts);
      }

    pg = gaiaAddPolygonToGeomColl (result, pts, interiors);
/* setting up the Exterior ring */
    rng = pg->Exterior;
    for (iv = 0; iv < (int) pts; iv++)
      {
	  if (geos_dims == 3)
	    {
		if (handle != NULL)
		  {
		      GEOSCoordSeq_getX_r (handle, coords, iv, &x);
		      GEOSCoordSeq_getY_r (handle, coords, iv, &y);
		      GEOSCoordSeq_getZ_r (handle, coords, iv, &z);
		  }
		else
		  {
		      GEOSCoordSeq_getX (coords, iv, &x);
		      GEOSCoordSeq_getY (coords, iv, &y);
		      GEOSCoordSeq_getZ (coords, iv, &z);
		  }
	    }
	  else
	    {
		if (handle != NULL)
		  {
		      GEOSCoordSeq_getX_r (handle, coords, iv, &x);
		      GEOSCoordSeq_getY_r (handle, coords, iv, &y);
		  }
		else
		  {
		      GEOSCoordSeq_getX (coords, iv, &x);
		      GEOSCoordSeq_getY (coords, iv, &y);
		  }
		z = 0.0;
	    }
	  if (rng->DimensionModel == GAIA_XY_Z)
	    {
		gaiaSetPointXYZ (rng->Coords, iv, x, y, z);
	    }
	  else if (rng->DimensionModel == GAIA_XY_M)
	    {
		gaiaSetPointXYM (rng->Coords, iv, x, y, 0.0);
	    }
	  else if (rng->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaSetPointXYZM (rng->Coords, iv, x, y, z, 0.0);
	    }
	  else
	    {
		gaiaSetPoint (rng->Coords, iv, x, y);
	    }
      }

    for (ib = 0; ib < interiors; ib++)
      {
	  /* setting up any interior ring */
	  if (handle != NULL)
	    {
		geos_ring = GEOSGetInteriorRingN_r (handle, geos, ib);
		coords = GEOSGeom_getCoordSeq_r (handle, geos_ring);
		GEOSCoordSeq_getDimensions_r (handle, coords, &geos_dims);
		GEOSCoordSeq_getSize_r (handle, coords, &pts);
	    }
	  else
	    {
		geos_ring = GEOSGetInteriorRingN (geos, ib);
		coords = GEOSGeom_getCoordSeq (geos_ring);
		GEOSCoordSeq_getDimensions (coords, &geos_dims);
		GEOSCoordSeq_getSize (coords, &pts);
	    }
	  rng = gaiaAddInteriorRing (pg, ib, pts);
	  for (iv = 0; iv < (int) pts; iv++)
	    {
		if (geos_dims == 3)
		  {
		      if (handle != NULL)
			{
			    GEOSCoordSeq_getX_r (handle, coords, iv, &x);
			    GEOSCoordSeq_getY_r (handle, coords, iv, &y);
			    GEOSCoordSeq_getZ_r (handle, coords, iv, &z);
			}
		      else
			{
			    GEOSCoordSeq_getX (coords, iv, &x);
			    GEOSCoordSeq_getY (coords, iv, &y);
			    GEOSCoordSeq_getZ (coords, iv, &z);
			}
		  }
		else
		  {
		      if (handle != NULL)
			{
			    GEOSCoordSeq_getX_r (handle, coords, iv, &x);
			    GEOSCoordSeq_getY_r (handle, coords, iv, &y);
			}
		      else
			{
			    GEOSCoordSeq_getX (coords, iv, &x);
			    GEOSCoordSeq_getY (coords, iv, &y);
			}
		      z = 0.0;
		  }
		if (rng->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (rng->Coords, iv, x, y, z);
		  }
		else if (rng->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (rng->Coords, iv, x, y, 0.0);
		  }
		else if (rng->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (rng->Coords, iv, x, y, z, 0.0);
		  }
		else
		  {
		      gaiaSetPoint (rng->Coords, iv, x, y);
		  }
	    }
      }
}

static void
auxGeosMbr (GEOSContextHandle_t handle, const GEOSCoordSequence * cs,
	    unsigned int pts, double *min_x, double *min_y, double *max_x,
	    double *max_y)
{
/* computing the MBR */
    int iv;
    double x;
    double y;
    *min_x = DBL_MAX;
    *min_y = DBL_MAX;
    *max_x = 0 - DBL_MAX;
    *max_y = 0 - DBL_MAX;
    for (iv = 0; iv < (int) pts; iv++)
      {
	  if (handle != NULL)
	    {
		GEOSCoordSeq_getX_r (handle, cs, iv, &x);
		GEOSCoordSeq_getY_r (handle, cs, iv, &y);
	    }
	  else
	    {
		GEOSCoordSeq_getX (cs, iv, &x);
		GEOSCoordSeq_getY (cs, iv, &y);
	    }
	  if (x < *min_x)
	      *min_x = x;
	  if (x > *max_x)
	      *max_x = x;
	  if (y < *min_y)
	      *min_y = y;
	  if (y > *max_y)
	      *max_y = y;
      }
}

static gaiaGeomCollPtr
gaiaPolygonizeCommon (const void *cache, GEOSContextHandle_t handle,
		      gaiaGeomCollPtr geom, int force_multi)
{
/* attempts to rearrange a generic Geometry into a (multi)polygon */
    int ig;
    int ib;
    int iv;
    int interiors;
    int geos_dims = 2;
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    int items;
    int error;
    double x;
    double y;
    double z;
    double m;
    gaiaGeomCollPtr result = NULL;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    GEOSCoordSequence *cs;
    const GEOSGeometry *const *geos_list = NULL;
    GEOSGeometry **p_item;
    GEOSGeometry *geos;
    const GEOSGeometry *geos_item;
    const GEOSGeometry *geos_item2;
    const GEOSGeometry *geos_ring;
    char *valid_polygons = NULL;
    const GEOSCoordSequence *coords;
    unsigned int pts1;
    unsigned int pts2;
    double min_x1;
    double max_x1;
    double min_y1;
    double max_y1;
    double min_x2;
    double max_x2;
    double min_y2;
    double max_y2;
    int ret;

    if (!geom)
	return NULL;
    if (cache != NULL)
	ret = gaiaIsToxic_r (cache, geom);
    else
	ret = gaiaIsToxic (geom);
    if (ret)
	return NULL;
    pt = geom->FirstPoint;
    while (pt)
      {
	  pts++;
	  pt = pt->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  pgs++;
	  pg = pg->Next;
      }
    if (pts || pgs)
	return NULL;
    ln = geom->FirstLinestring;
    while (ln)
      {
	  lns++;
	  ln = ln->Next;
      }
    if (!lns)
	return NULL;
    if (geom->DimensionModel == GAIA_XY_Z
	|| geom->DimensionModel == GAIA_XY_Z_M)
	geos_dims = 3;

/* allocating GEOS linestrings */
    geos_list = malloc (sizeof (const GEOSGeometry * const *) * lns);
    p_item = (GEOSGeometry **) geos_list;
    for (iv = 0; iv < lns; iv++)
      {
	  /* initializing to NULL */
	  *p_item++ = NULL;
      }
    p_item = (GEOSGeometry **) geos_list;

/* initializing GEOS linestrings */
    ln = geom->FirstLinestring;
    while (ln)
      {
	  if (handle != NULL)
	      cs = GEOSCoordSeq_create_r (handle, ln->Points, geos_dims);
	  else
	      cs = GEOSCoordSeq_create (ln->Points, geos_dims);
	  for (iv = 0; iv < ln->Points; iv++)
	    {
		/* exterior ring segments */
		z = 0.0;
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
		if (geos_dims == 3)
		  {
		      if (handle != NULL)
			{
			    GEOSCoordSeq_setX_r (handle, cs, iv, x);
			    GEOSCoordSeq_setY_r (handle, cs, iv, y);
			    GEOSCoordSeq_setZ_r (handle, cs, iv, z);
			}
		      else
			{
			    GEOSCoordSeq_setX (cs, iv, x);
			    GEOSCoordSeq_setY (cs, iv, y);
			    GEOSCoordSeq_setZ (cs, iv, z);
			}
		  }
		else
		  {
		      if (handle != NULL)
			{
			    GEOSCoordSeq_setX_r (handle, cs, iv, x);
			    GEOSCoordSeq_setY_r (handle, cs, iv, y);
			}
		      else
			{
			    GEOSCoordSeq_setX (cs, iv, x);
			    GEOSCoordSeq_setY (cs, iv, y);
			}
		  }
	    }
	  if (handle != NULL)
	      *p_item++ = GEOSGeom_createLineString_r (handle, cs);
	  else
	      *p_item++ = GEOSGeom_createLineString (cs);
	  ln = ln->Next;
      }

/* calling GEOSPolygonize */
    if (handle != NULL)
	geos = GEOSPolygonize_r (handle, geos_list, lns);
    else
	geos = GEOSPolygonize (geos_list, lns);
    if (geos == NULL)
	goto cleanup;

/*
/ 
/ GEOSPolygonize is expected to return a collection of Polygons
/
/ CAVEAT: internal holes are returned as such (interior rings in
/         some Polygon), but are returned as distinct Polygons too
/
/ we must check this, so to *not* return Polygons representing holes
/
*/
    error = 0;
    if (handle != NULL)
	items = GEOSGetNumGeometries_r (handle, geos);
    else
	items = GEOSGetNumGeometries (geos);
    for (ig = 0; ig < items; ig++)
      {
	  /* looping on elementaty GEOS geometries */
	  if (handle != NULL)
	    {
		geos_item = GEOSGetGeometryN_r (handle, geos, ig);
		if (GEOSGeomTypeId_r (handle, geos_item) != GEOS_POLYGON)
		  {
		      /* not a Polygon ... ouch ... */
		      error = 1;
		      goto cleanup;
		  }
	    }
	  else
	    {
		geos_item = GEOSGetGeometryN (geos, ig);
		if (GEOSGeomTypeId (geos_item) != GEOS_POLYGON)
		  {
		      /* not a Polygon ... ouch ... */
		      error = 1;
		      goto cleanup;
		  }
	    }
      }

/* identifying valid Polygons [excluding holes] */
    valid_polygons = malloc (items);
    for (ig = 0; ig < items; ig++)
	valid_polygons[ig] = 'Y';
    for (ig = 0; ig < items; ig++)
      {
	  /* looping on elementaty GEOS Polygons */
	  if (handle != NULL)
	    {
		geos_item = GEOSGetGeometryN_r (handle, geos, ig);
		interiors = GEOSGetNumInteriorRings_r (handle, geos_item);
	    }
	  else
	    {
		geos_item = GEOSGetGeometryN (geos, ig);
		interiors = GEOSGetNumInteriorRings (geos_item);
	    }
	  for (ib = 0; ib < interiors; ib++)
	    {
		/* looping on any interior ring */
		if (handle != NULL)
		  {
		      geos_ring =
			  GEOSGetInteriorRingN_r (handle, geos_item, ib);
		      coords = GEOSGeom_getCoordSeq_r (handle, geos_ring);
		      GEOSCoordSeq_getSize_r (handle, coords, &pts1);
		  }
		else
		  {
		      geos_ring = GEOSGetInteriorRingN (geos_item, ib);
		      coords = GEOSGeom_getCoordSeq (geos_ring);
		      GEOSCoordSeq_getSize (coords, &pts1);
		  }
		auxGeosMbr (handle, coords, pts1, &min_x1, &min_y1, &max_x1,
			    &max_y1);
		for (iv = 0; iv < items; iv++)
		  {
		      if (iv == ig)
			{
			    /* skipping the Polygon itself */
			    continue;
			}
		      if (valid_polygons[iv] == 'N')
			{
			    /* skipping any already invalid Polygon */
			    continue;
			}
		      if (handle != NULL)
			{
			    geos_item2 = GEOSGetGeometryN_r (handle, geos, iv);
			    if (GEOSGetNumInteriorRings_r (handle, geos_item2) >
				0)
			      {
				  /* this Polygon contains holes [surely valid] */
				  continue;
			      }
			    geos_ring =
				GEOSGetExteriorRing_r (handle, geos_item2);
			    coords = GEOSGeom_getCoordSeq_r (handle, geos_ring);
			    GEOSCoordSeq_getSize_r (handle, coords, &pts2);
			}
		      else
			{
			    geos_item2 = GEOSGetGeometryN (geos, iv);
			    if (GEOSGetNumInteriorRings (geos_item2) > 0)
			      {
				  /* this Polygon contains holes [surely valid] */
				  continue;
			      }
			    geos_ring = GEOSGetExteriorRing (geos_item2);
			    coords = GEOSGeom_getCoordSeq (geos_ring);
			    GEOSCoordSeq_getSize (coords, &pts2);
			}
		      if (pts1 == pts2)
			{
			    auxGeosMbr (handle, coords, pts2, &min_x2, &min_y2,
					&max_x2, &max_y2);
			    if (min_x1 == min_x2 && min_y1 == min_y2
				&& max_x1 == max_x2 && max_y1 == max_y2)
			      {
				  /* same #points, same MBRs: invalidating */
				  valid_polygons[iv] = 'N';
			      }
			}
		  }
	    }
      }

/* creating the Geometry to be returned */
    if (geom->DimensionModel == GAIA_XY_Z)
	result = gaiaAllocGeomCollXYZ ();
    else if (geom->DimensionModel == GAIA_XY_M)
	result = gaiaAllocGeomCollXYM ();
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	result = gaiaAllocGeomCollXYZM ();
    else
	result = gaiaAllocGeomColl ();
    if (result == NULL)
	return NULL;
    result->Srid = geom->Srid;
    if (force_multi)
	result->DeclaredType = GAIA_MULTIPOLYGON;

    for (ig = 0; ig < items; ig++)
      {
	  /* looping on GEOS Polygons */
	  if (handle != NULL)
	      geos_item = GEOSGetGeometryN_r (handle, geos, ig);
	  else
	      geos_item = GEOSGetGeometryN (geos, ig);
	  if (valid_polygons[ig] == 'Y')
	      auxFromGeosPolygon (handle, geos_item, result);
      }

  cleanup:
    if (valid_polygons != NULL)
	free (valid_polygons);
    if (geos_list != NULL)
      {
	  /* memory cleanup */
	  p_item = (GEOSGeometry **) geos_list;
	  for (iv = 0; iv < lns; iv++)
	    {
		if (*p_item != NULL)
		  {
		      if (handle != NULL)
			  GEOSGeom_destroy_r (handle, *p_item);
		      else
			  GEOSGeom_destroy (*p_item);
		  }
		p_item++;
	    }
	  p_item = (GEOSGeometry **) geos_list;
	  free (p_item);
      }
    if (geos != NULL)
      {
	  if (handle != NULL)
	      GEOSGeom_destroy_r (handle, geos);
	  else
	      GEOSGeom_destroy (geos);
      }
    if (error || result->FirstPolygon == NULL)
      {
	  gaiaFreeGeomColl (result);
	  return NULL;
      }
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaPolygonize (gaiaGeomCollPtr geom, int force_multi)
{
/* attempts to rearrange a generic Geometry into a (multi)polygon */
    gaiaResetGeosMsg ();
    return gaiaPolygonizeCommon (NULL, NULL, geom, force_multi);
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaPolygonize_r (const void *p_cache, gaiaGeomCollPtr geom, int force_multi)
{
/* attempts to rearrange a generic Geometry into a (multi)polygon */
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return NULL;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return NULL;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return NULL;
    gaiaResetGeosMsg_r (cache);
    return gaiaPolygonizeCommon (cache, handle, geom, force_multi);
}

GAIAGEO_DECLARE int
gaiaGeomCollCovers (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if geom1 "spatially covers" geom2 */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_contains (geom1, geom2))
	return 0;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSCovers (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollCovers_r (const void *p_cache, gaiaGeomCollPtr geom1,
		      gaiaGeomCollPtr geom2)
{
/* checks if geom1 "spatially covers" geom2 */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_contains (geom1, geom2))
	return 0;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSCovers_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollPreparedCovers (const void *p_cache, gaiaGeomCollPtr geom1,
			    unsigned char *blob1, int size1,
			    gaiaGeomCollPtr geom2, unsigned char *blob2,
			    int size2)
{
/* checks if geom1 "spatially covers" geom2 */
    int ret;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSPreparedGeometry *gPrep;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaGeomCollPtr geom;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_contains (geom1, geom2))
	return 0;

/* handling the internal GEOS cache */
    if (evalGeosCache
	(cache, geom1, blob1, size1, geom2, blob2, size2, &gPrep, &geom))
      {
	  g2 = gaiaToGeos_r (cache, geom);
	  if (geom == geom2)
	      ret = GEOSPreparedCovers_r (handle, gPrep, g2);
	  else
	      ret = GEOSPreparedCoveredBy_r (handle, gPrep, g2);
	  GEOSGeom_destroy_r (handle, g2);
	  if (ret == 2)
	      return -1;
	  return ret;
      }

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSCovers_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollCoveredBy (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if geom1 is "spatially covered by" geom2 */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    gaiaResetGeosMsg ();
    if (!geom1 || !geom2)
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_within (geom1, geom2))
	return 0;

    g1 = gaiaToGeos (geom1);
    g2 = gaiaToGeos (geom2);
    ret = GEOSCoveredBy (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollCoveredBy_r (const void *p_cache, gaiaGeomCollPtr geom1,
			 gaiaGeomCollPtr geom2)
{
/* checks if geom1 is "spatially covered by" geom2 */
    int ret;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSContextHandle_t handle = NULL;
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_within (geom1, geom2))
	return 0;

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSCoveredBy_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollPreparedCoveredBy (const void *p_cache, gaiaGeomCollPtr geom1,
			       unsigned char *blob1, int size1,
			       gaiaGeomCollPtr geom2, unsigned char *blob2,
			       int size2)
{
/* checks if geom1 is "spatially covered by" geom2 */
    int ret;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    GEOSPreparedGeometry *gPrep;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSContextHandle_t handle = NULL;
    gaiaGeomCollPtr geom;
    gaiaResetGeosMsg ();
    if (cache == NULL)
	return -1;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return -1;
    handle = cache->GEOS_handle;
    if (handle == NULL)
	return -1;
    gaiaResetGeosMsg_r (cache);
    if (!geom1 || !geom2)
	return -1;

/* quick check based on MBRs comparison */
    if (!splite_mbr_within (geom1, geom2))
	return 0;

/* handling the internal GEOS cache */
    if (evalGeosCache
	(cache, geom1, blob1, size1, geom2, blob2, size2, &gPrep, &geom))
      {
	  g2 = gaiaToGeos_r (cache, geom);
	  if (geom == geom2)
	      ret = GEOSPreparedCoveredBy_r (handle, gPrep, g2);
	  else
	      ret = GEOSPreparedCovers_r (handle, gPrep, g2);
	  GEOSGeom_destroy_r (handle, g2);
	  if (ret == 2)
	      return -1;
	  return ret;
      }

    g1 = gaiaToGeos_r (cache, geom1);
    g2 = gaiaToGeos_r (cache, geom2);
    ret = GEOSCoveredBy_r (handle, g1, g2);
    GEOSGeom_destroy_r (handle, g1);
    GEOSGeom_destroy_r (handle, g2);
    if (ret == 2)
	return -1;
    return ret;
}

#endif /* end including GEOS */
