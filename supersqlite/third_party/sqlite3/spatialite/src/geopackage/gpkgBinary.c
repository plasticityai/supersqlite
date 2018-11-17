/*

    GeoPackage extensions for SpatiaLite / SQLite
 
Version: MPL 1.1/GPL 2.0/LGPL 2.1

The contents of this file are subject to the Mozilla Public License Version
1.1 (the "License"); you may not use this file except in compliance with
the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/
 
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is GeoPackage Extensions

The Initial Developer of the Original Code is Brad Hards (bradh@frogmouth.net)
 
Portions created by the Initial Developer are Copyright (C) 2014-2015
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

#include "spatialite/geopackage.h"
#include "config.h"
#include "geopackage_internal.h"

#ifdef ENABLE_GEOPACKAGE

#define GEOPACKAGE_UNUSED() if (argc || argv) argc = argc;

GEOPACKAGE_DECLARE void
gpkgSetHeader2DLittleEndian (unsigned char *ptr, int srid, int endian_arch)
{
    *ptr = GEOPACKAGE_MAGIC1;
    *(ptr + 1) = GEOPACKAGE_MAGIC2;
    *(ptr + 2) = GEOPACKAGE_VERSION;
    *(ptr + 3) = GEOPACKAGE_FLAGS_2D_LITTLEENDIAN;
    gaiaExport32 (ptr + 4, srid, 1, endian_arch);	/* the SRID */
}

GEOPACKAGE_DECLARE void
gpkgSetHeader2DMbr (unsigned char *ptr, double min_x, double min_y,
		    double max_x, double max_y, int endian_arch)
{
    gaiaExport64 (ptr, min_x, 1, endian_arch);
    gaiaExport64 (ptr + sizeof (double), max_x, 1, endian_arch);
    gaiaExport64 (ptr + 2 * sizeof (double), min_y, 1, endian_arch);
    gaiaExport64 (ptr + 3 * sizeof (double), max_y, 1, endian_arch);
}

GEOPACKAGE_DECLARE void
gaiaToGPB (gaiaGeomCollPtr geom, unsigned char **result, int *size)
{
    int wkbOnlyLength;
    unsigned char *wkbOnlyGeometry = NULL;
    unsigned char *ptr;
    int endian_arch = gaiaEndianArch ();

    gaiaToWkb (geom, &wkbOnlyGeometry, &wkbOnlyLength);
    /* Calculate output size */
    /* We only do 2D envelopes (MBR) irrespective of the input geometry dimensions */
    *size = GEOPACKAGE_HEADER_LEN + GEOPACKAGE_2D_ENVELOPE_LEN;
    *size += wkbOnlyLength;

    /* allocate result and fill with "canary" value */
    *result = malloc (*size);
    if (*result == NULL)
      {
	  return;
      }
    memset (*result, 0xD9, *size);
    ptr = *result;

    /* build header */
    gpkgSetHeader2DLittleEndian (ptr, geom->Srid, endian_arch);

    /* build MBR */
    gpkgSetHeader2DMbr (ptr + GEOPACKAGE_HEADER_LEN, geom->MinX, geom->MinY,
			geom->MaxX, geom->MaxY, endian_arch);

    /* copy wkbonly results to result */
    memcpy (ptr + GEOPACKAGE_HEADER_LEN + GEOPACKAGE_2D_ENVELOPE_LEN,
	    wkbOnlyGeometry, wkbOnlyLength);

    free (wkbOnlyGeometry);
}


GEOPACKAGE_PRIVATE void
fnct_ToGPB (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ AsGPB(BLOB encoded geometry)
/
/ converts the (spatialite) geometry blob into a GeoPackage format geometry blob
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    int len;
    unsigned char *p_result = NULL;
    GEOPACKAGE_UNUSED ();	/* LCOV_EXCL_LINE */
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
      {
	  sqlite3_result_null (context);
	  return;
      }
    else
      {
	  gaiaToGPB (geo, &p_result, &len);
	  if (!p_result)
	    {
		sqlite3_result_null (context);
	    }
	  else
	    {
		sqlite3_result_blob (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static int
sanity_check_gpb (const unsigned char *wkb, int size, int *srid,
		  unsigned int *envelope_length)
{
    /* checking type coherency for GeoPackageBinary encoded GEOMETRY */
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    char flags;
    int envelope_code;

    if (size < GEOPACKAGE_HEADER_LEN)
      {
	  return 0;		/* too short to be GPB */
      }
    if ((*(wkb + 0) != GEOPACKAGE_MAGIC1) || (*(wkb + 1) != GEOPACKAGE_MAGIC2))
      {
	  return 0;		/* doesn't match required signature */
      }
    if (*(wkb + 2) != GEOPACKAGE_VERSION)
      {
	  return 0;		/* we only know how to parse version 1 */
      }

    flags = *(wkb + 3);

    little_endian = flags & GEOPACKAGE_WKB_LITTLEENDIAN;

    envelope_code = ((flags >> 1) & 0x07);
    switch (envelope_code)
      {
      case 0:
	  *envelope_length = 0;
	  break;
      case 1:
	  *envelope_length = 32;
	  break;
      case 2:
      case 3:
	  *envelope_length = 48;
	  break;
      case 4:
	  *envelope_length = 64;
	  break;
      default:
	  /* illegal value - not valid GeoPackageBinary */
	  fprintf (stderr, "Unsupported geopackage envelope value: 0x%x\n",
		   envelope_code);
	  return 0;
      }

    if (flags & GEOPACKAGE_WKB_EXTENDEDGEOMETRY_FLAG)
      {
	  fprintf (stderr,
		   "unsupported geopackage binary type (extended geopackage binary)\n");
	  return 0;
      }

    *srid = gaiaImport32 (wkb + 4, little_endian, endian_arch);

    return 1;
}

GEOPACKAGE_DECLARE gaiaGeomCollPtr
gaiaFromGeoPackageGeometryBlob (const unsigned char *gpb, unsigned int gpb_len)
{
    gaiaGeomCollPtr geo = NULL;
    int srid = GEOPACKAGE_DEFAULT_UNDEFINED_SRID;
    unsigned int envelope_length = 0;
    const unsigned char *wkb;
    unsigned int wkb_len;

    if (!sanity_check_gpb (gpb, gpb_len, &srid, &envelope_length))
      {
	  return NULL;
      }

    wkb = gpb + GEOPACKAGE_HEADER_LEN + envelope_length;
    wkb_len = gpb_len - (GEOPACKAGE_HEADER_LEN + envelope_length);

    geo = gaiaFromWkb (wkb, wkb_len);
    if (geo == NULL)
      {
	  return NULL;
      }
    geo->Srid = srid;

    return geo;
}

GEOPACKAGE_PRIVATE void
fnct_GeomFromGPB (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ GeomFromGPB(GPB encoded geometry)
/
/ returns a geometry created by parsing a GeoPackageBinary encoded blob 
/ or NULL if any error is encountered
*/
    int len;
    unsigned char *p_result = NULL;
    const unsigned char *gpb;
    unsigned int gpb_len;
    gaiaGeomCollPtr geo = NULL;


    GEOPACKAGE_UNUSED ();	/* LCOV_EXCL_LINE */
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    gpb = sqlite3_value_blob (argv[0]);
    gpb_len = sqlite3_value_bytes (argv[0]);

    geo = gaiaFromGeoPackageGeometryBlob (gpb, gpb_len);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }

    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

/* Sandro Furieri - 2014-05-19 */

GEOPACKAGE_DECLARE int
gaiaIsValidGPB (const unsigned char *gpb, int gpb_len)
{
/* checks for a valid GPB */
    int srid;
    unsigned int envelope_length;

    if (gpb == NULL)
	return 0;
    return sanity_check_gpb (gpb, gpb_len, &srid, &envelope_length);
}

GEOPACKAGE_DECLARE int
gaiaGetSridFromGPB (const unsigned char *gpb, int gpb_len)
{
/* extracts the SRID from a valid GPB */
    int srid;
    unsigned int envelope_length;

    if (gpb == NULL)
	return -1;
    if (!sanity_check_gpb (gpb, gpb_len, &srid, &envelope_length))
	return -1;
    return srid;
}

GEOPACKAGE_DECLARE int
gaiaIsEmptyGPB (const unsigned char *gpb, int gpb_len)
{
/* checks for an empty GPB */
    int srid;
    unsigned int envelope_length;
    int is_empty;

    if (gpb == NULL)
	return -1;
    if (!sanity_check_gpb (gpb, gpb_len, &srid, &envelope_length))
	return -1;
    is_empty = *(gpb + 3) & GEOPACKAGE_WKB_EMPTY_FLAG;
    return is_empty;
}

GEOPACKAGE_DECLARE int
gaiaGetEnvelopeFromGPB (const unsigned char *gpb, int gpb_len, double *min_x,
			double *max_x, double *min_y, double *max_y, int *has_z,
			double *min_z, double *max_z, int *has_m, double *min_m,
			double *max_m)
{
/* attempts to retrieve a full Envelope from a GPB */
    gaiaGeomCollPtr geo;
    double min;
    double max;
    if (gpb == NULL)
	return 0;
    geo = gaiaFromGeoPackageGeometryBlob (gpb, gpb_len);
    if (geo == NULL)
	return 0;
/*
/ defensive programming
/
/ the GPKG seems to be a rather sparse and inconsistent standard
/ so we'll always ignore the Envelope declared by GPB
/ and we'll instead recompute 'our' Envelope from scratch
*/
    gaiaMbrGeometry (geo);
    *min_x = geo->MinX;
    *max_x = geo->MaxX;
    *min_y = geo->MinY;
    *max_y = geo->MaxY;
    if (geo->DimensionModel == GAIA_XY_Z || geo->DimensionModel == GAIA_XY_Z_M)
      {
	  *has_z = 1;
	  gaiaZRangeGeometry (geo, &min, &max);
	  *min_z = min;
	  *max_z = max;
      }
    else
	*has_z = 0;
    if (geo->DimensionModel == GAIA_XY_M || geo->DimensionModel == GAIA_XY_Z_M)
      {
	  *has_m = 1;
	  gaiaMRangeGeometry (geo, &min, &max);
	  *min_m = min;
	  *max_m = max;
      }
    else
	*has_m = 0;
    gaiaFreeGeomColl (geo);
    return 1;
}

GEOPACKAGE_DECLARE char *
gaiaGetGeometryTypeFromGPB (const unsigned char *gpb, int gpb_len)
{
/* attempts to retrieve the Geometry Type from a GPB */
    gaiaGeomCollPtr geo;
    const char *type = NULL;
    int len;
    char *gtype;

    if (gpb == NULL)
	return NULL;
    geo = gaiaFromGeoPackageGeometryBlob (gpb, gpb_len);
    if (geo == NULL)
	return NULL;
/*
/ defensive programming
/
/ the GPKG seems to be a rather sparse and inconsistent standard
/ so we'll always fetch the Geometry Type from 'our' Geometry Type
*/
    switch (gaiaGeometryType (geo))
      {
      case GAIA_POINT:
      case GAIA_POINTZ:
      case GAIA_POINTM:
      case GAIA_POINTZM:
	  type = "POINT";
	  break;
      case GAIA_LINESTRING:
      case GAIA_LINESTRINGZ:
      case GAIA_LINESTRINGM:
      case GAIA_LINESTRINGZM:
	  type = "LINESTRING";
	  break;
      case GAIA_POLYGON:
      case GAIA_POLYGONZ:
      case GAIA_POLYGONM:
      case GAIA_POLYGONZM:
	  type = "POLYGON";
	  break;
      case GAIA_MULTIPOINT:
      case GAIA_MULTIPOINTZ:
      case GAIA_MULTIPOINTM:
      case GAIA_MULTIPOINTZM:
	  type = "MULTIPOINT";
	  break;
      case GAIA_MULTILINESTRING:
      case GAIA_MULTILINESTRINGZ:
      case GAIA_MULTILINESTRINGM:
      case GAIA_MULTILINESTRINGZM:
	  type = "MULTILINESTRING";
	  break;
      case GAIA_MULTIPOLYGON:
      case GAIA_MULTIPOLYGONZ:
      case GAIA_MULTIPOLYGONM:
      case GAIA_MULTIPOLYGONZM:
	  type = "MULTIPOLYGON";
	  break;
      case GAIA_GEOMETRYCOLLECTION:
      case GAIA_GEOMETRYCOLLECTIONZ:
      case GAIA_GEOMETRYCOLLECTIONM:
      case GAIA_GEOMETRYCOLLECTIONZM:
	  type = "GEOMCOLLECTION";
	  break;
      };
    gaiaFreeGeomColl (geo);

    if (type == NULL)
	return NULL;
    len = strlen (type);
    gtype = malloc (len + 1);
    strcpy (gtype, type);
    return gtype;
}

GEOPACKAGE_PRIVATE void
fnct_GPKG_IsAssignable (sqlite3_context * context, int argc,
			sqlite3_value ** argv)
{
/* SQL function:
/ GPKG_IsAssignale(expected_type_name TEXT, actual_type_name TEXT)
/
/ returns:
/ 1 if the expected type is the same or a super-type of actual type
/ 0 otherwise
*/
    const char *expected;
    const char *actual;
    int ret = 0;

    GEOPACKAGE_UNUSED ();	/* LCOV_EXCL_LINE */
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_int (context, ret);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  sqlite3_result_int (context, ret);
	  return;
      }
    expected = (const char *) sqlite3_value_text (argv[0]);
    actual = (const char *) sqlite3_value_text (argv[1]);
    if (strcasecmp (expected, actual) == 0)
	ret = 1;
    if (strcasecmp (expected, "GEOMETRY") == 0)
	ret = 1;
    if (strcasecmp (expected, "MULTIPOINT") == 0
	&& strcasecmp (actual, "POINT") == 0)
	ret = 1;
    if (strcasecmp (expected, "MULTILINESTRING") == 0
	&& strcasecmp (actual, "LINESTRING") == 0)
	ret = 1;
    if (strcasecmp (expected, "MULTIPOLYGON") == 0
	&& strcasecmp (actual, "POLYGON") == 0)
	ret = 1;
    sqlite3_result_int (context, ret);
}

GEOPACKAGE_PRIVATE void
fnct_IsValidGPB (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsValidGPB(GPB encoded geometry)
/
/ check for a valid GPB encoded geometry
*/
    const unsigned char *gpb;
    unsigned int gpb_len;


    GEOPACKAGE_UNUSED ();	/* LCOV_EXCL_LINE */
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, 0);
	  return;
      }
    gpb = sqlite3_value_blob (argv[0]);
    gpb_len = sqlite3_value_bytes (argv[0]);
    sqlite3_result_int (context, gaiaIsValidGPB (gpb, gpb_len));
}

/* end Sandro Furieri - 2014-05-19 */

#endif
