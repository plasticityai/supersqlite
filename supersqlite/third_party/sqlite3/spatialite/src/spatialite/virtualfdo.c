/*

 virtualfdo.c -- SQLite3 extension [VIRTUAL TABLE accessing FDO-OGR tables]

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
#include <string.h>
#include <limits.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>

#include <spatialite/spatialite.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/gaiageo.h>

/* constants definining FDO-OGR Geometry formats */
#define FDO_OGR_NONE	0
#define FDO_OGR_WKT	1
#define FDO_OGR_WKB	2
#define FDO_OGR_FGF	3
#define FDO_OGR_SPLITE	4

#ifdef _WIN32
#define strcasecmp	_stricmp
#define strncasecmp	_strnicmp
#endif /* not WIN32 */

/* 64 bit integer: portable format for printf() */
#if defined(_WIN32) && !defined(__MINGW32__)
#define FRMT64 "%I64d"
#else
#define FRMT64 "%lld"
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#define LONG64_MAX	_I64_MAX
#define LONG64_MIN	_I64_MIN
#else
#define LONG64_MAX	9223372036854775807LL
#define LONG64_MIN	(-LONG64_MAX + 1)
#endif

struct sqlite3_module my_fdo_module;

typedef struct SqliteValue
{
/* a multitype storing a column value */
    int Type;
    sqlite3_int64 IntValue;
    double DoubleValue;
    char *Text;
    unsigned char *Blob;
    int Size;
} SqliteValue;
typedef SqliteValue *SqliteValuePtr;

typedef struct VirtualFDOStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    char *table;		/* the real-table name */
    int nColumns;		/* the # columns into the table */
    char **Column;		/* the name for each column */
    char **Type;		/* the type for each column */
    int *NotNull;		/* NotNull clause for each column */
    SqliteValuePtr *Value;	/* the current-row value for each column */
    int nGeometries;		/* # Geometry columns into the table */
    char **GeoColumn;		/* the name for each Geometry column */
    int *Srid;			/* the SRID for each Geometry column */
    int *GeoType;		/* the Type for each Geometry column */
    int *Format;		/* the Format for each Geometry column */
    int *CoordDimensions;	/* # Dimensions for each Geometry column */
} VirtualFDO;
typedef VirtualFDO *VirtualFDOPtr;

typedef struct VirtualFDOCursorStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualFDOPtr pVtab;	/* Virtual table of this cursor */
    sqlite3_stmt *stmt;
    sqlite3_int64 current_row;	/* the current row ID */
    int eof;			/* the EOF marker */
} VirtualFDOCursor;
typedef VirtualFDOCursor *VirtualFDOCursorPtr;

static SqliteValuePtr
value_alloc (void)
{
/* allocates and initialites a Value multitype */
    SqliteValuePtr p = malloc (sizeof (SqliteValue));
    p->Type = SQLITE_NULL;
    p->Text = NULL;
    p->Blob = NULL;
    return p;
}

static void
value_free (SqliteValuePtr p)
{
/* freeing a Value multitype */
    if (!p)
	return;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    free (p);
}

static void
value_set_null (SqliteValuePtr p)
{
/* setting a NULL value to the multitype */
    if (!p)
	return;
    p->Type = SQLITE_NULL;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    p->Text = NULL;
    p->Blob = NULL;
}

static void
value_set_int (SqliteValuePtr p, sqlite3_int64 value)
{
/* setting an INT value to the multitype */
    if (!p)
	return;
    p->Type = SQLITE_INTEGER;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    p->Text = NULL;
    p->Blob = NULL;
    p->IntValue = value;
}

static void
value_set_double (SqliteValuePtr p, double value)
{
/* setting a DOUBLE value to the multitype */
    if (!p)
	return;
    p->Type = SQLITE_FLOAT;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    p->Text = NULL;
    p->Blob = NULL;
    p->DoubleValue = value;
}

static void
value_set_text (SqliteValuePtr p, const char *value, int size)
{
/* setting a TEXT value to the multitype */
    if (!p)
	return;
    p->Type = SQLITE_TEXT;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    p->Blob = NULL;
    p->Text = malloc (size);
    memcpy (p->Text, value, size);
    p->Size = size;
}

static void
value_set_blob (SqliteValuePtr p, const unsigned char *value, int size)
{
/* setting a BLOB value to the multitype */
    if (!p)
	return;
    p->Type = SQLITE_BLOB;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    p->Text = NULL;
    p->Blob = malloc (size);
    memcpy (p->Blob, value, size);
    p->Size = size;
}

static void
free_table (VirtualFDOPtr p_vt)
{
/* memory cleanup; freeing the virtual table struct */
    int i;
    if (!p_vt)
	return;
    if (p_vt->table)
	sqlite3_free (p_vt->table);
    if (p_vt->Column)
      {
	  for (i = 0; i < p_vt->nColumns; i++)
	    {
		if (*(p_vt->Column + i))
		    sqlite3_free (*(p_vt->Column + i));
	    }
	  sqlite3_free (p_vt->Column);
      }
    if (p_vt->Type)
      {
	  for (i = 0; i < p_vt->nColumns; i++)
	    {
		if (*(p_vt->Type + i))
		    sqlite3_free (*(p_vt->Type + i));
	    }
	  sqlite3_free (p_vt->Type);
      }
    if (p_vt->NotNull)
	sqlite3_free (p_vt->NotNull);
    if (p_vt->Value)
      {
	  for (i = 0; i < p_vt->nColumns; i++)
	    {
		if (*(p_vt->Value + i))
		    value_free (*(p_vt->Value + i));
	    }
	  sqlite3_free (p_vt->Value);
      }
    if (p_vt->GeoColumn)
      {
	  for (i = 0; i < p_vt->nGeometries; i++)
	    {
		if (*(p_vt->GeoColumn + i))
		    sqlite3_free (*(p_vt->GeoColumn + i));
	    }
	  sqlite3_free (p_vt->GeoColumn);
      }
    if (p_vt->Srid)
	sqlite3_free (p_vt->Srid);
    if (p_vt->GeoType)
	sqlite3_free (p_vt->GeoType);
    if (p_vt->Format)
	sqlite3_free (p_vt->Format);
    if (p_vt->CoordDimensions)
	sqlite3_free (p_vt->CoordDimensions);
    sqlite3_free (p_vt);
}


static void
vfdoOutWkt3D (gaiaOutBufferPtr out_buf, gaiaGeomCollPtr geom)
{
/* prints the GEOS-WKT-3D representation of current geometry */
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* counting how many POINTs are there */
	  pts++;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* counting how many LINESTRINGs are there */
	  lns++;
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* counting how many POLYGONs are there */
	  pgs++;
	  polyg = polyg->Next;
      }
    if ((pts + lns + pgs) == 1
	&& (geom->DeclaredType == GAIA_POINT
	    || geom->DeclaredType == GAIA_LINESTRING
	    || geom->DeclaredType == GAIA_POLYGON))
      {
	  /* we have only one elementary geometry */
	  point = geom->FirstPoint;
	  while (point)
	    {
		/* processing POINT */
		gaiaAppendToOutBuffer (out_buf, "POINT (");
		gaiaOutPointZ (out_buf, point);
		gaiaAppendToOutBuffer (out_buf, ")");
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		/* processing LINESTRING */
		gaiaAppendToOutBuffer (out_buf, "LINESTRING (");
		gaiaOutLinestringZ (out_buf, line);
		gaiaAppendToOutBuffer (out_buf, ")");
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		/* processing POLYGONZ */
		gaiaAppendToOutBuffer (out_buf, "POLYGON (");
		gaiaOutPolygonZ (out_buf, polyg);
		gaiaAppendToOutBuffer (out_buf, ")");
		polyg = polyg->Next;
	    }
      }
    else
      {
	  /* we have some kind of complex geometry */
	  if (pts > 0 && lns == 0 && pgs == 0
	      && geom->DeclaredType == GAIA_MULTIPOINT)
	    {
		/* some kind of MULTIPOINT */
		gaiaAppendToOutBuffer (out_buf, "MULTIPOINT (");
		point = geom->FirstPoint;
		while (point)
		  {
		      if (point != geom->FirstPoint)
			  gaiaAppendToOutBuffer (out_buf, ", ");
		      gaiaOutPointZ (out_buf, point);
		      point = point->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else if (pts == 0 && lns > 0 && pgs == 0
		   && geom->DeclaredType == GAIA_MULTILINESTRING)
	    {
		/* some kind of MULTILINESTRING */
		gaiaAppendToOutBuffer (out_buf, "MULTILINESTRING (");
		line = geom->FirstLinestring;
		while (line)
		  {
		      if (line != geom->FirstLinestring)
			  gaiaAppendToOutBuffer (out_buf, ", (");
		      else
			  gaiaAppendToOutBuffer (out_buf, "(");
		      gaiaOutLinestringZ (out_buf, line);
		      gaiaAppendToOutBuffer (out_buf, ")");
		      line = line->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else if (pts == 0 && lns == 0 && pgs > 0
		   && geom->DeclaredType == GAIA_MULTIPOLYGON)
	    {
		/* some kind of MULTIPOLYGON */
		gaiaAppendToOutBuffer (out_buf, "MULTIPOLYGON (");
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      if (polyg != geom->FirstPolygon)
			  gaiaAppendToOutBuffer (out_buf, ", (");
		      else
			  gaiaAppendToOutBuffer (out_buf, "(");
		      gaiaOutPolygonZ (out_buf, polyg);
		      gaiaAppendToOutBuffer (out_buf, ")");
		      polyg = polyg->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else
	    {
		/* some kind of GEOMETRYCOLLECTION */
		int ie = 0;
		gaiaAppendToOutBuffer (out_buf, "GEOMETRYCOLLECTION (");
		point = geom->FirstPoint;
		while (point)
		  {
		      /* processing POINTs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ", ");
		      ie++;
		      gaiaAppendToOutBuffer (out_buf, "POINT (");
		      gaiaOutPointZ (out_buf, point);
		      gaiaAppendToOutBuffer (out_buf, ")");
		      point = point->Next;
		  }
		line = geom->FirstLinestring;
		while (line)
		  {
		      /* processing LINESTRINGs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ", ");
		      ie++;
		      gaiaAppendToOutBuffer (out_buf, "LINESTRING (");
		      gaiaOutLinestringZ (out_buf, line);
		      gaiaAppendToOutBuffer (out_buf, ")");
		      line = line->Next;
		  }
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      /* processing POLYGONs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ", ");
		      ie++;
		      gaiaAppendToOutBuffer (out_buf, "POLYGON (");
		      gaiaOutPolygonZ (out_buf, polyg);
		      gaiaAppendToOutBuffer (out_buf, ")");
		      polyg = polyg->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
      }
}

static void
vfdoToWkb3D (gaiaGeomCollPtr geom, unsigned char **result, int *size)
{
/* builds the GEOS-WKB-3D representation for this GEOMETRY */
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
	      type = GAIA_GEOSWKB_MULTIPOINTZ;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ;
	  else
	      type = GAIA_GEOSWKB_POINTZ;
      }
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ;
	  else
	      type = GAIA_GEOSWKB_MULTIPOINTZ;
      }
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	      type = GAIA_GEOSWKB_MULTILINESTRINGZ;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ;
	  else
	      type = GAIA_GEOSWKB_LINESTRINGZ;
      }
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ;
	  else
	      type = GAIA_GEOSWKB_MULTILINESTRINGZ;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	      type = GAIA_GEOSWKB_MULTIPOLYGONZ;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ;
	  else
	      type = GAIA_GEOSWKB_POLYGONZ;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ;
	  else
	      type = GAIA_GEOSWKB_MULTIPOLYGONZ;
      }
    else
	type = GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ;
/* and now we compute the size of WKB */
    *size = 5;			/* header size */
    if (type == GAIA_GEOSWKB_MULTIPOINTZ
	|| type == GAIA_GEOSWKB_MULTILINESTRINGZ
	|| type == GAIA_GEOSWKB_MULTIPOLYGONZ
	|| type == GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ)
	*size += 4;
    point = geom->FirstPoint;
    while (point)
      {
	  if (type == GAIA_GEOSWKB_MULTIPOINTZ
	      || type == GAIA_GEOSWKB_MULTILINESTRINGZ
	      || type == GAIA_GEOSWKB_MULTIPOLYGONZ
	      || type == GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ)
	      *size += 5;
	  *size += (sizeof (double) * 3);	/* three doubles for each POINT */
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  if (type == GAIA_GEOSWKB_MULTIPOINTZ
	      || type == GAIA_GEOSWKB_MULTILINESTRINGZ
	      || type == GAIA_GEOSWKB_MULTIPOLYGONZ
	      || type == GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ)
	      *size += 5;
	  *size += (4 + ((sizeof (double) * 3) * line->Points));	/* # points + [x,y,z] for each vertex */
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  if (type == GAIA_GEOSWKB_MULTIPOINTZ
	      || type == GAIA_GEOSWKB_MULTILINESTRINGZ
	      || type == GAIA_GEOSWKB_MULTIPOLYGONZ
	      || type == GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ)
	      *size += 5;
	  rng = polyg->Exterior;
	  *size += (8 + ((sizeof (double) * 3) * rng->Points));	/* # rings + # points + [x,y,z] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + ((sizeof (double) * 3) * rng->Points));	/* # points + [x,y,z] array - interior ring */
	    }
	  polyg = polyg->Next;
      }
    *result = malloc (*size);
    ptr = *result;
/* and finally we build the WKB */
    *ptr = 0x01;		/* little endian byte order */
    gaiaExport32 (ptr + 1, type, 1, endian_arch);	/* the main CLASS TYPE */
    ptr += 5;
    if (type == GAIA_GEOSWKB_MULTIPOINTZ
	|| type == GAIA_GEOSWKB_MULTILINESTRINGZ
	|| type == GAIA_GEOSWKB_MULTIPOLYGONZ
	|| type == GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ)
      {
	  gaiaExport32 (ptr, entities, 1, endian_arch);	/* it's a collection; # entities */
	  ptr += 4;
      }
    point = geom->FirstPoint;
    while (point)
      {
	  if (type == GAIA_GEOSWKB_MULTIPOINTZ
	      || type == GAIA_GEOMETRYCOLLECTIONZ)
	    {
		*ptr = 0x01;
		/* it's a collection: the CLASS TYPE for this element */
		gaiaExport32 (ptr + 1, GAIA_GEOSWKB_POINTZ, 1, endian_arch);
		ptr += 5;
	    }
	  gaiaExport64 (ptr, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 8, point->Y, 1, endian_arch);	/* Y */
	  gaiaExport64 (ptr + 16, point->Z, 1, endian_arch);	/* Z */
	  ptr += 24;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  if (type == GAIA_GEOSWKB_MULTILINESTRINGZ
	      || type == GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ)
	    {
		*ptr = 0x01;
		/* it's a collection: the CLASS TYPE for this element */
		gaiaExport32 (ptr + 1, GAIA_GEOSWKB_LINESTRINGZ, 1,
			      endian_arch);
		ptr += 5;
	    }
	  gaiaExport32 (ptr, line->Points, 1, endian_arch);	/* # points */
	  ptr += 4;
	  for (iv = 0; iv < line->Points; iv++)
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
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y */
		gaiaExport64 (ptr + 16, z, 1, endian_arch);	/* Z */
		ptr += 24;
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  if (type == GAIA_GEOSWKB_MULTIPOLYGONZ
	      || type == GAIA_GEOSWKB_GEOMETRYCOLLECTIONZ)
	    {
		*ptr = 0x01;
		/* it's a collection: the CLASS TYPE for this element */
		gaiaExport32 (ptr + 1, GAIA_GEOSWKB_POLYGONZ, 1, endian_arch);
		ptr += 5;
	    }
	  gaiaExport32 (ptr, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 4, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 8;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		z = 0.0;
		m = 0.0;
		if (rng->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		  }
		else if (rng->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		  }
		else if (rng->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		  }
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
		      z = 0.0;
		      m = 0.0;
		      if (rng->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
			}
		      else if (rng->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
			}
		      else if (rng->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (rng->Coords, iv, &x, &y);
			}
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
		      gaiaExport64 (ptr + 16, z, 1, endian_arch);	/* Z - exterior ring */
		      ptr += 24;
		  }
	    }
	  polyg = polyg->Next;
      }
}

static int
vfdoGeometryType (gaiaGeomCollPtr geom)
{
/* determinates the Class for this geometry */
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    if (!geom)
	return GAIA_UNKNOWN;
    point = geom->FirstPoint;
    while (point)
      {
	  /* counts how many points are there */
	  n_points++;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* counts how many linestrings are there */
	  n_linestrings++;
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* counts how many polygons are there */
	  n_polygons++;
	  polyg = polyg->Next;
      }
    if (n_points == 0 && n_linestrings == 0 && n_polygons == 0)
	return GAIA_UNKNOWN;
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT)
	      return GAIA_MULTIPOINT;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_POINT;
      }
    if (n_points > 0 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_MULTIPOINT;
      }
    if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	      return GAIA_MULTILINESTRING;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_LINESTRING;
      }
    if (n_points == 0 && n_linestrings > 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_MULTILINESTRING;
      }
    if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	      return GAIA_MULTIPOLYGON;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_POLYGON;
      }
    if (n_points == 0 && n_linestrings == 0 && n_polygons > 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_MULTIPOLYGON;
      }
    return GAIA_GEOMETRYCOLLECTION;
}

static int
vfdo_insert_row (VirtualFDOPtr p_vt, sqlite3_int64 * rowid, int argc,
		 sqlite3_value ** argv)
{
/* trying to insert a row into FDO-OGR real-table */
    sqlite3_stmt *stmt;
    int ret;
    int i;
    int ic;
    int ig;
    int geom_done;
    int err_geom = 0;
    int geom_constraint_err = 0;
    char prefix[16];
    const char *text;
    const unsigned char *blob;
    gaiaOutBuffer out_buf;
    gaiaOutBuffer sql_statement;
    unsigned char *blob_wkb;
    int size;
    char *sql;
    char buf[256];
    char *xname;
    gaiaGeomCollPtr geom = NULL;
    gaiaOutBufferInitialize (&sql_statement);
    xname = gaiaDoubleQuotedSql (p_vt->table);
    sql = sqlite3_mprintf ("INSERT INTO \"%s\" ", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
    for (ic = 0; ic < p_vt->nColumns; ic++)
      {
	  if (ic == 0)
	      strcpy (prefix, "(");
	  else
	      strcpy (prefix, ", ");
	  xname = gaiaDoubleQuotedSql (*(p_vt->Column + ic));
	  sql = sqlite3_mprintf ("%s%s", prefix, xname);
	  free (xname);
	  gaiaAppendToOutBuffer (&sql_statement, sql);
	  sqlite3_free (sql);
      }
    gaiaAppendToOutBuffer (&sql_statement, ") VALUES ");
    for (ic = 0; ic < p_vt->nColumns; ic++)
      {
	  if (ic == 0)
	      strcpy (prefix, "(");
	  else
	      strcpy (prefix, ", ");
	  sprintf (buf, "%s?", prefix);
	  gaiaAppendToOutBuffer (&sql_statement, buf);
      }
    gaiaAppendToOutBuffer (&sql_statement, ")");
    if (sql_statement.Error == 0 && sql_statement.Buffer != NULL)
	ret =
	    sqlite3_prepare_v2 (p_vt->db, sql_statement.Buffer,
				strlen (sql_statement.Buffer), &stmt, NULL);
    else
	ret = SQLITE_ERROR;
    gaiaOutBufferReset (&sql_statement);
    if (ret != SQLITE_OK)
	return SQLITE_ERROR;
    for (i = 2; i < argc; i++)
      {
	  geom_done = 0;
	  for (ig = 0; ig < p_vt->nGeometries; ig++)
	    {
		if (strcasecmp
		    (*(p_vt->Column + i - 2), *(p_vt->GeoColumn + ig)) == 0)
		  {
		      /* this one is a Geometry column */
		      if (sqlite3_value_type (argv[i]) == SQLITE_BLOB)
			{
			    blob = sqlite3_value_blob (argv[i]);
			    size = sqlite3_value_bytes (argv[i]);
			    geom = gaiaFromSpatiaLiteBlobWkb (blob, size);
			    if (geom)
			      {
				  if (geom->Srid != *(p_vt->Srid + ig))
				    {
					/* SRID constraint violation */
					geom_constraint_err = 1;
					goto error;
				    }
				  /* checking for TYPE constraint violation */
				  if (vfdoGeometryType (geom) !=
				      *(p_vt->GeoType + ig))
				    {
					/* Geometry TYPE constraint violation */
					geom_constraint_err = 1;
					goto error;
				    }
				  switch (*(p_vt->Format + ig))
				    {
				    case FDO_OGR_WKT:
					gaiaOutBufferInitialize (&out_buf);
					if (*(p_vt->CoordDimensions + ig) ==
					    GAIA_XY_Z)
					    vfdoOutWkt3D (&out_buf, geom);
					else
					    gaiaOutWkt (&out_buf, geom);
					if (out_buf.Error == 0
					    && out_buf.Buffer != NULL)
					  {
					      sqlite3_bind_text (stmt, i - 1,
								 out_buf.Buffer,
								 out_buf.WriteOffset,
								 free);
					      out_buf.Buffer = NULL;
					      gaiaOutBufferReset (&out_buf);
					  }
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    case FDO_OGR_WKB:
					if (*(p_vt->CoordDimensions + ig) ==
					    GAIA_XY_Z)
					    vfdoToWkb3D (geom, &blob_wkb,
							 &size);
					else
					    gaiaToWkb (geom, &blob_wkb, &size);
					if (blob_wkb)
					    sqlite3_bind_blob (stmt, i - 1,
							       blob_wkb, size,
							       free);
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    case FDO_OGR_FGF:
					gaiaToFgf (geom, &blob_wkb, &size,
						   *(p_vt->CoordDimensions +
						     ig));
					if (blob_wkb)
					    sqlite3_bind_blob (stmt, i - 1,
							       blob_wkb, size,
							       free);
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    case FDO_OGR_SPLITE:
					gaiaToSpatiaLiteBlobWkb (geom,
								 &blob_wkb,
								 &size);
					if (blob_wkb)
					    sqlite3_bind_blob (stmt, i - 1,
							       blob_wkb, size,
							       free);
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    default:
					err_geom = 1;
					goto error;
					break;
				    };
			      }
			    else
			      {
				  err_geom = 1;
				  goto error;
			      }
			}
		      else if (sqlite3_value_type (argv[i]) == SQLITE_NULL)
			  sqlite3_bind_null (stmt, i - 1);
		      else
			{
			    err_geom = 1;
			    goto error;
			}
		      geom_done = 1;
		  }
	    }
	  if (geom_done)
	    {
		if (geom)
		  {
		      /* memory cleanup: Kashif Rasul 14 Jan 2010 */
		      gaiaFreeGeomColl (geom);
		      geom = NULL;
		  }
		continue;
	    }
	  switch (sqlite3_value_type (argv[i]))
	    {
	    case SQLITE_INTEGER:
		sqlite3_bind_int64 (stmt, i - 1, sqlite3_value_int64 (argv[i]));
		break;
	    case SQLITE_FLOAT:
		sqlite3_bind_double (stmt, i - 1,
				     sqlite3_value_double (argv[i]));
		break;
	    case SQLITE_TEXT:
		text = (char *) sqlite3_value_text (argv[i]);
		size = sqlite3_value_bytes (argv[i]);
		sqlite3_bind_text (stmt, i - 1, text, size, SQLITE_STATIC);
		break;
	    case SQLITE_BLOB:
		blob = sqlite3_value_blob (argv[i]);
		size = sqlite3_value_bytes (argv[i]);
		sqlite3_bind_blob (stmt, i - 1, blob, size, SQLITE_STATIC);
		break;
	    case SQLITE_NULL:
	    default:
		sqlite3_bind_null (stmt, i - 1);
		break;
	    };
      }
  error:
    if (geom)
      {
	  /* memory cleanup: Kashif Rasul 14 Jan 2010 */
	  gaiaFreeGeomColl (geom);
      }
    if (err_geom || geom_constraint_err)
      {
	  sqlite3_finalize (stmt);
	  return SQLITE_CONSTRAINT;
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  sqlite3_finalize (stmt);
	  return ret;
      }
    sqlite3_finalize (stmt);
    *rowid = sqlite3_last_insert_rowid (p_vt->db);
    return SQLITE_OK;
}

static int
vfdo_update_row (VirtualFDOPtr p_vt, sqlite3_int64 rowid, int argc,
		 sqlite3_value ** argv)
{
/* trying to update a row in FDO-OGR real-table */
    sqlite3_stmt *stmt;
    int ret;
    int i;
    int ic;
    int ig;
    int geom_done;
    int err_geom = 0;
    int geom_constraint_err = 0;
    char prefix[16];
    const char *text;
    const unsigned char *blob;
    gaiaOutBuffer out_buf;
    gaiaOutBuffer sql_statement;
    unsigned char *blob_wkb;
    int size;
    char *sql;
    char buf[256];
    char *xname;
    gaiaGeomCollPtr geom = NULL;
    gaiaOutBufferInitialize (&sql_statement);
    xname = gaiaDoubleQuotedSql (p_vt->table);
    sql = sqlite3_mprintf ("UPDATE \"%s\" SET", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
    for (ic = 0; ic < p_vt->nColumns; ic++)
      {
	  if (ic == 0)
	      strcpy (prefix, " ");
	  else
	      strcpy (prefix, ", ");
	  xname = gaiaDoubleQuotedSql (*(p_vt->Column + ic));
	  sql = sqlite3_mprintf ("%s%s = ?", prefix, xname);
	  free (xname);
	  gaiaAppendToOutBuffer (&sql_statement, sql);
	  sqlite3_free (sql);
      }
    sprintf (buf, " WHERE ROWID = " FRMT64, rowid);
    gaiaAppendToOutBuffer (&sql_statement, buf);
    if (sql_statement.Error == 0 && sql_statement.Buffer != NULL)
	ret =
	    sqlite3_prepare_v2 (p_vt->db, sql_statement.Buffer,
				strlen (sql_statement.Buffer), &stmt, NULL);
    else
	ret = SQLITE_ERROR;
    gaiaOutBufferReset (&sql_statement);
    if (ret != SQLITE_OK)
	return SQLITE_ERROR;
    for (i = 2; i < argc; i++)
      {
	  geom_done = 0;
	  for (ig = 0; ig < p_vt->nGeometries; ig++)
	    {
		if (strcasecmp
		    (*(p_vt->Column + i - 2), *(p_vt->GeoColumn + ig)) == 0)
		  {
		      /* this one is a Geometry column */
		      if (sqlite3_value_type (argv[i]) == SQLITE_BLOB)
			{
			    blob = sqlite3_value_blob (argv[i]);
			    size = sqlite3_value_bytes (argv[i]);
			    geom = gaiaFromSpatiaLiteBlobWkb (blob, size);
			    if (geom)
			      {
				  if (geom->Srid != *(p_vt->Srid + ig))
				    {
					/* SRID constraint violation */
					geom_constraint_err = 1;
					goto error;
				    }
				  /* checking for TYPE constraint violation */
				  if (vfdoGeometryType (geom) !=
				      *(p_vt->GeoType + ig))
				    {
					/* Geometry TYPE constraint violation */
					geom_constraint_err = 1;
					goto error;
				    }
				  switch (*(p_vt->Format + ig))
				    {
				    case FDO_OGR_WKT:
					gaiaOutBufferInitialize (&out_buf);
					if (*(p_vt->CoordDimensions + ig) ==
					    GAIA_XY_Z)
					    vfdoOutWkt3D (&out_buf, geom);
					else
					    gaiaOutWkt (&out_buf, geom);
					if (out_buf.Error == 0
					    && out_buf.Buffer != NULL)
					  {
					      sqlite3_bind_text (stmt, i - 1,
								 out_buf.Buffer,
								 out_buf.WriteOffset,
								 free);
					      out_buf.Buffer = NULL;
					      gaiaOutBufferReset (&out_buf);
					  }
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    case FDO_OGR_WKB:
					if (*(p_vt->CoordDimensions + ig) ==
					    GAIA_XY_Z)
					    vfdoToWkb3D (geom, &blob_wkb,
							 &size);
					else
					    gaiaToWkb (geom, &blob_wkb, &size);
					if (blob_wkb)
					    sqlite3_bind_blob (stmt, i - 1,
							       blob_wkb, size,
							       free);
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    case FDO_OGR_FGF:
					gaiaToFgf (geom, &blob_wkb, &size,
						   *(p_vt->CoordDimensions +
						     ig));
					if (blob_wkb)
					    sqlite3_bind_blob (stmt, i - 1,
							       blob_wkb, size,
							       free);
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    case FDO_OGR_SPLITE:
					gaiaToSpatiaLiteBlobWkb (geom,
								 &blob_wkb,
								 &size);
					if (blob_wkb)
					    sqlite3_bind_blob (stmt, i - 1,
							       blob_wkb, size,
							       free);
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    default:
					err_geom = 1;
					goto error;
					break;
				    };
			      }
			    else
			      {
				  err_geom = 1;
				  goto error;
			      }
			}
		      else if (sqlite3_value_type (argv[i]) == SQLITE_NULL)
			  sqlite3_bind_null (stmt, i - 1);
		      else
			{
			    err_geom = 1;
			    goto error;
			}
		      geom_done = 1;
		  }
	    }
	  if (geom_done)
	    {
		gaiaFreeGeomColl (geom);
		geom = NULL;
		continue;
	    }
	  switch (sqlite3_value_type (argv[i]))
	    {
	    case SQLITE_INTEGER:
		sqlite3_bind_int64 (stmt, i - 1, sqlite3_value_int64 (argv[i]));
		break;
	    case SQLITE_FLOAT:
		sqlite3_bind_double (stmt, i - 1,
				     sqlite3_value_double (argv[i]));
		break;
	    case SQLITE_TEXT:
		text = (char *) sqlite3_value_text (argv[i]);
		size = sqlite3_value_bytes (argv[i]);
		sqlite3_bind_text (stmt, i - 1, text, size, SQLITE_STATIC);
		break;
	    case SQLITE_BLOB:
		blob = sqlite3_value_blob (argv[i]);
		size = sqlite3_value_bytes (argv[i]);
		sqlite3_bind_blob (stmt, i - 1, blob, size, SQLITE_STATIC);
		break;
	    case SQLITE_NULL:
	    default:
		sqlite3_bind_null (stmt, i - 1);
		break;
	    };
      }
  error:
    gaiaFreeGeomColl (geom);
    if (err_geom || geom_constraint_err)
      {
	  sqlite3_finalize (stmt);
	  return SQLITE_CONSTRAINT;
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  sqlite3_finalize (stmt);
	  return ret;
      }
    sqlite3_finalize (stmt);
    return SQLITE_OK;
}

static int
vfdo_delete_row (VirtualFDOPtr p_vt, sqlite3_int64 rowid)
{
/* trying to delete a row from FDO-OGR real-table */
    char *sql_statement;
    char dummy[256];
    int ret;
    char *xname;
    xname = gaiaDoubleQuotedSql (p_vt->table);
    sprintf (dummy, FRMT64, rowid);
    sql_statement =
	sqlite3_mprintf ("DELETE FROM \"%s\" WHERE ROWID = %s", xname, dummy);
    free (xname);
    ret = sqlite3_exec (p_vt->db, sql_statement, NULL, NULL, NULL);
    sqlite3_free (sql_statement);
    return ret;
}

char *
vfdo_convertWKT3D (const char *wkt)
{
/* converting WKT 3D */
    char *out;
    char *o;
    const char *p = wkt;
    int len = strlen (wkt);
    int extra = 0;
    while (*p != '\0')
      {
	  if (strncasecmp (p, "POINT", 5) == 0)
	    {
		p += 5;
		extra++;
		continue;
	    }
	  if (strncasecmp (p, "LINESTRING", 10) == 0)
	    {
		p += 10;
		extra++;
		continue;
	    }
	  if (strncasecmp (p, "POLYGON", 7) == 0)
	    {
		p += 7;
		extra++;
		continue;
	    }
	  if (strncasecmp (p, "MULTIPOINT", 10) == 0)
	    {
		p += 10;
		extra++;
		continue;
	    }
	  if (strncasecmp (p, "MULTILINESTRING", 15) == 0)
	    {
		p += 15;
		extra++;
		continue;
	    }
	  if (strncasecmp (p, "MULTIPOLYGON", 12) == 0)
	    {
		p += 12;
		extra++;
		continue;
	    }
	  if (strncasecmp (p, "GEOMETRYCOLLECTION", 18) == 0)
	    {
		p += 18;
		extra++;
		continue;
	    }
	  p++;
      }
    out = malloc (len + extra + 1);
    o = out;
    p = wkt;
    while (*p != '\0')
      {
	  if (strncasecmp (p, "POINT", 5) == 0)
	    {
		strcpy (o, "POINTZ");
		o += 6;
		p += 5;
		continue;
	    }
	  if (strncasecmp (p, "LINESTRING", 10) == 0)
	    {
		strcpy (o, "LINESTRINGZ");
		o += 11;
		p += 10;
		continue;
	    }
	  if (strncasecmp (p, "POLYGON", 7) == 0)
	    {
		strcpy (o, "POLYGONZ");
		o += 8;
		p += 7;
		continue;
	    }
	  if (strncasecmp (p, "MULTIPOINT", 10) == 0)
	    {
		strcpy (o, "MULTIPOINTZ");
		o += 11;
		p += 10;
		continue;
	    }
	  if (strncasecmp (p, "MULTILINESTRING", 15) == 0)
	    {
		strcpy (o, "MULTILINESTRINGZ");
		o += 16;
		p += 15;
		continue;
	    }
	  if (strncasecmp (p, "MULTIPOLYGON", 12) == 0)
	    {
		strcpy (o, "MULTIPOLYGONZ");
		o += 13;
		p += 12;
		continue;
	    }
	  if (strncasecmp (p, "GEOMETRYCOLLECTION", 18) == 0)
	    {
		strcpy (o, "GEOMETRYCOLLECTIONZ");
		o += 19;
		p += 18;
		continue;
	    }
	  *o++ = *p++;
      }
    *o = '\0';
    return out;
}

static void
vfdo_read_row (VirtualFDOCursorPtr cursor)
{
/* trying to read a row from FDO-OGR real-table */
    sqlite3_stmt *stmt;
    int ret;
    int ic;
    int ig;
    unsigned char *wkt;
    const char *text;
    const unsigned char *blob;
    unsigned char *xblob;
    int size;
    sqlite3_int64 pk;
    int geom_done;
    gaiaGeomCollPtr geom;
    stmt = cursor->stmt;
    sqlite3_bind_int64 (stmt, 1, cursor->current_row);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_ROW)
      {
	  pk = sqlite3_column_int64 (stmt, 0);
	  for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
	    {
		/* fetching column values */
		geom_done = 0;
		for (ig = 0; ig < cursor->pVtab->nGeometries; ig++)
		  {
		      if (strcasecmp
			  (*(cursor->pVtab->Column + ic),
			   *(cursor->pVtab->GeoColumn + ig)) == 0)
			{
			    /* this one is a Geometry column */
			    switch (*(cursor->pVtab->Format + ig))
			      {
			      case FDO_OGR_WKT:
				  if (sqlite3_column_type (stmt, ic + 1) ==
				      SQLITE_TEXT)
				    {
					/* trying to parse a WKT Geometry */
					char delete_wkt = 0;
					wkt = (unsigned char *)
					    sqlite3_column_text (stmt, ic + 1);
					if (*
					    (cursor->pVtab->CoordDimensions +
					     ig) == GAIA_XY_Z)
					  {
					      wkt = (unsigned char *)
						  vfdo_convertWKT3D ((char *)
								     wkt);
					      if (wkt == NULL)
						{
						    value_set_null (*
								    (cursor->
								     pVtab->
								     Value +
								     ic));
						    continue;
						}
					      delete_wkt = 1;
					  }
					geom = gaiaParseWkt (wkt, -1);
					if (delete_wkt)
					    free ((void *) wkt);
					if (!geom)
					    value_set_null (*
							    (cursor->pVtab->
							     Value + ic));
					else
					  {
					      geom->Srid =
						  *(cursor->pVtab->Srid + ig);
					      gaiaToSpatiaLiteBlobWkb (geom,
								       &xblob,
								       &size);
					      if (xblob)
						{
						    value_set_blob (*
								    (cursor->
								     pVtab->
								     Value +
								     ic), xblob,
								    size);
						    free (xblob);
						}
					      else
						  value_set_null (*
								  (cursor->
								   pVtab->
								   Value + ic));
					      gaiaFreeGeomColl (geom);
					  }
				    }
				  else
				      value_set_null (*
						      (cursor->pVtab->Value +
						       ic));
				  break;
			      case FDO_OGR_WKB:
				  if (sqlite3_column_type (stmt, ic + 1) ==
				      SQLITE_BLOB)
				    {
					/* trying to parse a WKB Geometry */
					blob =
					    sqlite3_column_blob (stmt, ic + 1);
					size =
					    sqlite3_column_bytes (stmt, ic + 1);
					geom = gaiaFromWkb (blob, size);
					if (!geom)
					    value_set_null (*
							    (cursor->pVtab->
							     Value + ic));
					else
					  {
					      geom->Srid =
						  *(cursor->pVtab->Srid + ig);
					      gaiaToSpatiaLiteBlobWkb (geom,
								       &xblob,
								       &size);
					      if (xblob)
						{
						    value_set_blob (*
								    (cursor->
								     pVtab->
								     Value +
								     ic), xblob,
								    size);
						    free (xblob);
						}
					      else
						  value_set_null (*
								  (cursor->
								   pVtab->
								   Value + ic));
					      gaiaFreeGeomColl (geom);
					  }
				    }
				  else
				      value_set_null (*
						      (cursor->pVtab->Value +
						       ic));
				  break;
			      case FDO_OGR_FGF:
				  if (sqlite3_column_type (stmt, ic + 1) ==
				      SQLITE_BLOB)
				    {
					/* trying to parse an FGF Geometry */
					blob =
					    sqlite3_column_blob (stmt, ic + 1);
					size =
					    sqlite3_column_bytes (stmt, ic + 1);
					geom = gaiaFromFgf (blob, size);
					if (!geom)
					    value_set_null (*
							    (cursor->pVtab->
							     Value + ic));
					else
					  {
					      geom->Srid =
						  *(cursor->pVtab->Srid + ig);
					      gaiaToSpatiaLiteBlobWkb (geom,
								       &xblob,
								       &size);
					      if (xblob)
						{
						    value_set_blob (*
								    (cursor->
								     pVtab->
								     Value +
								     ic), xblob,
								    size);
						    free (xblob);
						}
					      else
						  value_set_null (*
								  (cursor->
								   pVtab->
								   Value + ic));
					      gaiaFreeGeomColl (geom);
					  }
				    }
				  else
				      value_set_null (*
						      (cursor->pVtab->Value +
						       ic));
				  break;
			      case FDO_OGR_SPLITE:
				  if (sqlite3_column_type (stmt, ic + 1) ==
				      SQLITE_BLOB)
				    {
					/* trying to parse a SPATIALITE Geometry */
					blob =
					    sqlite3_column_blob (stmt, ic + 1);
					size =
					    sqlite3_column_bytes (stmt, ic + 1);
					value_set_blob (*
							(cursor->pVtab->Value +
							 ic), blob, size);
				    }
				  else
				      value_set_null (*
						      (cursor->pVtab->Value +
						       ic));
				  break;
			      default:
				  value_set_null (*(cursor->pVtab->Value + ic));
				  break;
			      };
			    geom_done = 1;
			}
		  }
		if (geom_done)
		    continue;
		switch (sqlite3_column_type (stmt, ic + 1))
		  {
		  case SQLITE_INTEGER:
		      value_set_int (*(cursor->pVtab->Value + ic),
				     sqlite3_column_int64 (stmt, ic + 1));
		      break;
		  case SQLITE_FLOAT:
		      value_set_double (*(cursor->pVtab->Value + ic),
					sqlite3_column_double (stmt, ic + 1));
		      break;
		  case SQLITE_TEXT:
		      text = (char *) sqlite3_column_text (stmt, ic + 1);
		      size = sqlite3_column_bytes (stmt, ic + 1);
		      value_set_text (*(cursor->pVtab->Value + ic), text, size);
		      break;
		  case SQLITE_BLOB:
		      blob = sqlite3_column_blob (stmt, ic + 1);
		      size = sqlite3_column_bytes (stmt, ic + 1);
		      value_set_blob (*(cursor->pVtab->Value + ic), blob, size);
		      break;
		  case SQLITE_NULL:
		  default:
		      value_set_null (*(cursor->pVtab->Value + ic));
		      break;
		  };
	    }
      }
    else
      {
	  /* an error occurred */
	  cursor->eof = 1;
	  return;
      }
    cursor->eof = 0;
    cursor->current_row = pk;
}

static int
vfdo_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some FDO-OGR table */
    char *vtable = NULL;
    char *table = NULL;
    int ret;
    int i;
    int len;
    int n_rows;
    int n_columns;
    const char *col_name;
    const char *col_type;
    const char *format;
    int coord_dimension;
    int not_null;
    int srid;
    int type;
    char **results;
    char *sql;
    char prefix[16];
    char *xname;
    gaiaOutBuffer sql_statement;
    VirtualFDOPtr p_vt = NULL;
    if (pAux)
	pAux = pAux;		/* unused arg warning suppression */
    gaiaOutBufferInitialize (&sql_statement);
/* checking for table_name */
    if (argc == 4)
      {
	  vtable = gaiaDequotedSql ((char *) argv[2]);
	  table = gaiaDequotedSql ((char *) argv[3]);
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualFDO module] CREATE VIRTUAL: illegal arg list {table_name}\n");
	  goto error;
      }
/* retrieving the base table columns */
    xname = gaiaDoubleQuotedSql (table);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (db, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto illegal;
    if (n_rows >= 1)
      {
	  p_vt = (VirtualFDOPtr) sqlite3_malloc (sizeof (VirtualFDO));
	  if (!p_vt)
	      return SQLITE_NOMEM;
	  p_vt->db = db;
	  p_vt->nRef = 0;
	  p_vt->zErrMsg = NULL;
	  len = strlen (table);
	  p_vt->table = sqlite3_malloc (len + 1);
	  strcpy (p_vt->table, table);
	  p_vt->nColumns = n_rows;
	  p_vt->Column = sqlite3_malloc (sizeof (char *) * n_rows);
	  p_vt->Type = sqlite3_malloc (sizeof (char *) * n_rows);
	  p_vt->NotNull = sqlite3_malloc (sizeof (int) * n_rows);
	  p_vt->Value = sqlite3_malloc (sizeof (SqliteValuePtr) * n_rows);
	  for (i = 0; i < n_rows; i++)
	    {
		*(p_vt->Column + i) = NULL;
		*(p_vt->Type + i) = NULL;
		*(p_vt->NotNull + i) = -1;
		*(p_vt->Value + i) = value_alloc ();
	    }
	  p_vt->nGeometries = 0;
	  p_vt->GeoColumn = NULL;
	  p_vt->Srid = NULL;
	  p_vt->GeoType = NULL;
	  p_vt->Format = NULL;
	  p_vt->CoordDimensions = NULL;
	  for (i = 1; i <= n_rows; i++)
	    {
		col_name = results[(i * n_columns) + 1];
		col_type = results[(i * n_columns) + 2];
		if (atoi (results[(i * n_columns) + 3]) == 0)
		    not_null = 0;
		else
		    not_null = 1;
		len = strlen (col_name);
		*(p_vt->Column + (i - 1)) = sqlite3_malloc (len + 1);
		strcpy (*(p_vt->Column + (i - 1)), col_name);
		len = strlen (col_type);
		*(p_vt->Type + (i - 1)) = sqlite3_malloc (len + 1);
		strcpy (*(p_vt->Type + (i - 1)), col_type);
		*(p_vt->NotNull + (i - 1)) = not_null;
	    }
	  sqlite3_free_table (results);
      }
    else
	goto illegal;
/* retrieving the base table columns */
    sql = sqlite3_mprintf ("SELECT f_geometry_column, geometry_type, srid, "
			   "geometry_format, coord_dimension\n"
			   "FROM geometry_columns WHERE Upper(f_table_name) = Upper(%Q)",
			   table);
    ret = sqlite3_get_table (db, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto illegal;
    if (n_rows >= 1)
      {
	  p_vt->nGeometries = n_rows;
	  p_vt->GeoColumn = sqlite3_malloc (sizeof (char *) * n_rows);
	  p_vt->Srid = sqlite3_malloc (sizeof (char *) * n_rows);
	  p_vt->GeoType = sqlite3_malloc (sizeof (int) * n_rows);
	  p_vt->Format = sqlite3_malloc (sizeof (int) * n_rows);
	  p_vt->CoordDimensions = sqlite3_malloc (sizeof (int) * n_rows);
	  for (i = 0; i < n_rows; i++)
	    {
		*(p_vt->GeoColumn + i) = NULL;
		*(p_vt->Srid + i) = -1;
		*(p_vt->GeoType + i) = -1;
		*(p_vt->Format + i) = FDO_OGR_NONE;
		*(p_vt->CoordDimensions + i) = GAIA_XY;
	    }
	  for (i = 1; i <= n_rows; i++)
	    {
		col_name = results[(i * n_columns) + 0];
		type = atoi (results[(i * n_columns) + 1]);
		srid = atoi (results[(i * n_columns) + 2]);
		format = results[(i * n_columns) + 3];
		coord_dimension = atoi (results[(i * n_columns) + 4]);
		len = strlen (col_name);
		*(p_vt->GeoColumn + (i - 1)) = sqlite3_malloc (len + 1);
		strcpy (*(p_vt->GeoColumn + (i - 1)), col_name);
		*(p_vt->GeoType + (i - 1)) = type;
		*(p_vt->Srid + (i - 1)) = srid;
		if (strcasecmp (format, "WKT") == 0)
		    *(p_vt->Format + (i - 1)) = FDO_OGR_WKT;
		if (strcasecmp (format, "WKB") == 0)
		    *(p_vt->Format + (i - 1)) = FDO_OGR_WKB;
		if (strcasecmp (format, "FGF") == 0)
		    *(p_vt->Format + (i - 1)) = FDO_OGR_FGF;
		if (strcasecmp (format, "SPATIALITE") == 0)
		    *(p_vt->Format + (i - 1)) = FDO_OGR_SPLITE;
		if (coord_dimension == 3)
		    *(p_vt->CoordDimensions + (i - 1)) = GAIA_XY_Z;
		else if (coord_dimension == 4)
		    *(p_vt->CoordDimensions + (i - 1)) = GAIA_XY_Z_M;
		else
		    *(p_vt->CoordDimensions + (i - 1)) = GAIA_XY;
	    }
	  sqlite3_free_table (results);
      }
    else
	goto illegal;
/* preparing the COLUMNs for this VIRTUAL TABLE */
    xname = gaiaDoubleQuotedSql (vtable);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" ", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
    for (i = 0; i < p_vt->nColumns; i++)
      {
	  if (i == 0)
	      strcpy (prefix, "(");
	  else
	      strcpy (prefix, ", ");
	  xname = gaiaDoubleQuotedSql (*(p_vt->Column + i));
	  if (*(p_vt->NotNull + i))
	      sql =
		  sqlite3_mprintf ("%s%s %s NOT NULL", prefix, xname,
				   *(p_vt->Type + i));
	  else
	      sql =
		  sqlite3_mprintf ("%s%s %s", prefix, xname, *(p_vt->Type + i));
	  free (xname);
	  gaiaAppendToOutBuffer (&sql_statement, sql);
	  sqlite3_free (sql);
      }
    gaiaAppendToOutBuffer (&sql_statement, ")");
    if (sql_statement.Error == 0 && sql_statement.Buffer != NULL)
      {
	  if (sqlite3_declare_vtab (db, sql_statement.Buffer) != SQLITE_OK)
	    {
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualFDO module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
		     sql);
		goto error;
	    }
	  gaiaOutBufferReset (&sql_statement);
      }
    else
	goto error;
    *ppVTab = (sqlite3_vtab *) p_vt;
    free (vtable);
    free (table);
    return SQLITE_OK;
  illegal:
/* something is going the wrong way */
    gaiaOutBufferReset (&sql_statement);
    if (p_vt)
	free_table (p_vt);
    *pzErr =
	sqlite3_mprintf
	("[VirtualFDO module] '%s' isn't a valid FDO-OGR Geometry table\n",
	 table);
  error:
    if (vtable)
	free (vtable);
    if (table)
	free (table);
    gaiaOutBufferReset (&sql_statement);
    return SQLITE_ERROR;
}

static int
vfdo_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vfdo_create() */
    return vfdo_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vfdo_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
{
/* best index selection */
    if (pVTab || pIndex)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vfdo_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualFDOPtr p_vt = (VirtualFDOPtr) pVTab;
    free_table (p_vt);
    return SQLITE_OK;
}

static int
vfdo_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vfdo_disconnect() */
    return vfdo_disconnect (pVTab);
}

static int
vfdo_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    sqlite3_stmt *stmt;
    gaiaOutBuffer sql_statement;
    int ret;
    char *sql;
    int ic;
    char *xname;
    VirtualFDOCursorPtr cursor =
	(VirtualFDOCursorPtr) sqlite3_malloc (sizeof (VirtualFDOCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = (VirtualFDOPtr) pVTab;
    gaiaOutBufferInitialize (&sql_statement);
    gaiaAppendToOutBuffer (&sql_statement, "SELECT ROWID");
    for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
      {
	  value_set_null (*(cursor->pVtab->Value + ic));
	  xname = gaiaDoubleQuotedSql (*(cursor->pVtab->Column + ic));
	  sql = sqlite3_mprintf (",\"%s\"", xname);
	  free (xname);
	  gaiaAppendToOutBuffer (&sql_statement, sql);
	  sqlite3_free (sql);
      }
    xname = gaiaDoubleQuotedSql (cursor->pVtab->table);
    sql = sqlite3_mprintf (" FROM \"%s\" WHERE ROWID >= ?", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
    if (sql_statement.Error == 0 && sql_statement.Buffer != NULL)
	ret =
	    sqlite3_prepare_v2 (cursor->pVtab->db, sql_statement.Buffer,
				strlen (sql_statement.Buffer), &stmt, NULL);
    else
	ret = SQLITE_ERROR;
    gaiaOutBufferReset (&sql_statement);
    if (ret != SQLITE_OK)
      {
	  /* an error occurred */
	  cursor->eof = 1;
	  return SQLITE_ERROR;
      }
    cursor->stmt = stmt;
    cursor->current_row = LONG64_MIN;
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    vfdo_read_row (cursor);
    return SQLITE_OK;
}

static int
vfdo_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    int ic;
    VirtualFDOCursorPtr cursor = (VirtualFDOCursorPtr) pCursor;
    for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
	value_set_null (*(cursor->pVtab->Value + ic));
    if (cursor->stmt)
	sqlite3_finalize (cursor->stmt);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vfdo_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    if (pCursor || idxNum || idxStr || argc || argv)
	pCursor = pCursor;	/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vfdo_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching next row from cursor */
    VirtualFDOCursorPtr cursor = (VirtualFDOCursorPtr) pCursor;
    (cursor->current_row)++;
    vfdo_read_row (cursor);
    return SQLITE_OK;
}

static int
vfdo_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualFDOCursorPtr cursor = (VirtualFDOCursorPtr) pCursor;
    return cursor->eof;
}

static int
vfdo_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    VirtualFDOCursorPtr cursor = (VirtualFDOCursorPtr) pCursor;
    SqliteValuePtr value;
    if (column >= 0 && column < cursor->pVtab->nColumns)
      {
	  value = *(cursor->pVtab->Value + column);
	  switch (value->Type)
	    {
	    case SQLITE_INTEGER:
		sqlite3_result_int64 (pContext, value->IntValue);
		break;
	    case SQLITE_FLOAT:
		sqlite3_result_double (pContext, value->DoubleValue);
		break;
	    case SQLITE_TEXT:
		sqlite3_result_text (pContext, value->Text, value->Size,
				     SQLITE_STATIC);
		break;
	    case SQLITE_BLOB:
		sqlite3_result_blob (pContext, value->Blob, value->Size,
				     SQLITE_STATIC);
		break;
	    default:
		sqlite3_result_null (pContext);
		break;
	    };
      }
    else
	sqlite3_result_null (pContext);
    return SQLITE_OK;
}

static int
vfdo_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualFDOCursorPtr cursor = (VirtualFDOCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vfdo_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    sqlite3_int64 rowid = 0;
    int ret;
    VirtualFDOPtr p_vt = (VirtualFDOPtr) pVTab;
    if (argc == 1)
      {
	  /* performing a DELETE */
	  if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
	    {
		rowid = sqlite3_value_int64 (argv[0]);
		ret = vfdo_delete_row (p_vt, rowid);
	    }
	  else
	      ret = SQLITE_MISMATCH;
      }
    else
      {
	  if (sqlite3_value_type (argv[0]) == SQLITE_NULL)
	    {
		/* performing an INSERT */
		ret = vfdo_insert_row (p_vt, &rowid, argc, argv);
		if (ret == SQLITE_OK)
		    *pRowid = rowid;
	    }
	  else
	    {
		/* performing an UPDATE */
		rowid = sqlite3_value_int64 (argv[0]);
		ret = vfdo_update_row (p_vt, rowid, argc, argv);
	    }
      }
    return ret;
}

static int
vfdo_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vfdo_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vfdo_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vfdo_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vfdo_rename (sqlite3_vtab * pVTab, const char *zNew)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    if (zNew)
	zNew = zNew;		/* unused arg warning suppression */
    return SQLITE_ERROR;
}

static int
spliteVirtualFDOInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_fdo_module.iVersion = 1;
    my_fdo_module.xCreate = &vfdo_create;
    my_fdo_module.xConnect = &vfdo_connect;
    my_fdo_module.xBestIndex = &vfdo_best_index;
    my_fdo_module.xDisconnect = &vfdo_disconnect;
    my_fdo_module.xDestroy = &vfdo_destroy;
    my_fdo_module.xOpen = &vfdo_open;
    my_fdo_module.xClose = &vfdo_close;
    my_fdo_module.xFilter = &vfdo_filter;
    my_fdo_module.xNext = &vfdo_next;
    my_fdo_module.xEof = &vfdo_eof;
    my_fdo_module.xColumn = &vfdo_column;
    my_fdo_module.xRowid = &vfdo_rowid;
    my_fdo_module.xUpdate = &vfdo_update;
    my_fdo_module.xBegin = &vfdo_begin;
    my_fdo_module.xSync = &vfdo_sync;
    my_fdo_module.xCommit = &vfdo_commit;
    my_fdo_module.xRollback = &vfdo_rollback;
    my_fdo_module.xFindFunction = NULL;
    my_fdo_module.xRename = &vfdo_rename;
    sqlite3_create_module_v2 (db, "VirtualFDO", &my_fdo_module, NULL, 0);
    return rc;
}

SPATIALITE_PRIVATE int
virtualfdo_extension_init (void *xdb)
{
    sqlite3 *db = (sqlite3 *) xdb;
    return spliteVirtualFDOInit (db);
}
