/*

 dxf_writer.c -- implements DXF support
 [exporting a DXF]

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

/*
 
CREDITS:

inital development of the DXF module has been funded by:
Regione Toscana - Settore Sistema Informativo Territoriale ed Ambientale

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>
#include <spatialite/debug.h>

#include <spatialite/gaiageo.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/gg_dxf.h>
#include <spatialite.h>
#include <spatialite_private.h>

#include "dxf_private.h"

#if defined(_WIN32) && !defined(__MINGW32__)
#define strcasecmp	_stricmp
#endif /* not WIN32 */

GAIAGEO_DECLARE int
gaiaDxfWriterInit (gaiaDxfWriterPtr dxf, FILE * out, int precision, int version)
{
/* initializing a DXF Output object */
    if (dxf == NULL)
	return 0;
    dxf->error = 0;
    dxf->precision = precision;
    if (precision < 0)
	dxf->precision = 0;
    if (precision > 10)
	dxf->precision = 10;
    if (version != GAIA_DXF_V12)
	dxf->error = 1;
    else
	dxf->version = GAIA_DXF_V12;
    if (out == NULL)
	dxf->error = 1;
    else
	dxf->out = out;
    dxf->count = 0;
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWriteHeader (gaiaDxfWriterPtr dxf, double minx, double miny, double minz,
		    double maxx, double maxy, double maxz)
{
/* printing the DXF HEADER */
    char format[128];
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;

    fprintf (dxf->out, "%3d\r\nSECTION\r\n%3d\r\nHEADER\r\n", 0, 2);
    fprintf (dxf->out, "%3d\r\n$EXTMIN\r\n", 9);
    sprintf (format,
	     "%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n",
	     dxf->precision, dxf->precision, dxf->precision);
    fprintf (dxf->out, format, 10, minx, 20, miny, 30, minz);
    fprintf (dxf->out, "%3d\r\n$EXTMAX\r\n", 9);
    sprintf (format,
	     "%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n",
	     dxf->precision, dxf->precision, dxf->precision);
    fprintf (dxf->out, format, 10, maxx, 20, maxy, 30, maxz);
    fprintf (dxf->out, "%3d\r\nENDSEC\r\n", 0);
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWriteTables (gaiaDxfWriterPtr dxf)
{
/* printing the DXF TABLES section header */
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    fprintf (dxf->out, "%3d\r\nSECTION\r\n%3d\r\nTABLES\r\n", 0, 2);
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWriteLayer (gaiaDxfWriterPtr dxf, const char *layer)
{
/* printing a DXF TABLE/LAYER definition */
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    fprintf (dxf->out, "%3d\r\nTABLE\r\n%3d\r\nLAYER\r\n", 0, 2);
    fprintf (dxf->out, "%3d\r\n%3d\r\n%3d\r\nLAYER\r\n%3d\r\n%s\r\n", 70, 1, 0,
	     2, layer);
    fprintf (dxf->out, "%3d\r\n%d\r\n%3d\r\n%d\r\n%3d\r\nCONTINUOUS\r\n", 70,
	     64, 62, 7, 6);
    fprintf (dxf->out, "%3d\r\nENDTAB\r\n", 0);
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWriteFooter (gaiaDxfWriterPtr dxf)
{
/* printing the DXF footer */
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    fprintf (dxf->out, "%3d\r\nEOF\r\n", 0);
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWriteEntities (gaiaDxfWriterPtr dxf)
{
/* printing the DXF ENTITIES section header */
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    fprintf (dxf->out, "%3d\r\nSECTION\r\n%3d\r\nENTITIES\r\n", 0, 2);
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWriteEndSection (gaiaDxfWriterPtr dxf)
{
/* printing a DXF ENDSEC */
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    fprintf (dxf->out, "%3d\r\nENDSEC\r\n", 0);
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWritePoint (gaiaDxfWriterPtr dxf, const char *layer, double x, double y,
		   double z)
{
/* printing a DXF POINT */
    char format[128];
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    fprintf (dxf->out, "%3d\r\nPOINT\r\n%3d\r\n%s\r\n", 0, 8, layer);
    sprintf (format,
	     "%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n",
	     dxf->precision, dxf->precision, dxf->precision);
    fprintf (dxf->out, format, 10, x, 20, y, 30, z);
    dxf->count++;
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWriteText (gaiaDxfWriterPtr dxf, const char *layer, double x, double y,
		  double z, const char *label, double text_height, double angle)
{
/* printing a DXF TEXT */
    char format[128];
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    fprintf (dxf->out, "%3d\r\nTEXT\r\n%3d\r\n%s\r\n", 0, 8, layer);
    sprintf (format,
	     "%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n",
	     dxf->precision, dxf->precision, dxf->precision);
    fprintf (dxf->out, format, 10, x, 20, y, 30, z);
    sprintf (format, "%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n%%3d\r\n%%s\r\n",
	     dxf->precision, dxf->precision);
    fprintf (dxf->out, format, 40, text_height, 50, angle, 1, label);
    dxf->count++;
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWriteLine (gaiaDxfWriterPtr dxf, const char *layer,
		  gaiaLinestringPtr line)
{
/* printing a DXF POLYLINE (opened) */
    int iv;
    double x;
    double y;
    double z;
    double m;
    char format[128];
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    fprintf (dxf->out, "%3d\r\nPOLYLINE\r\n%3d\r\n%s\r\n%3d\r\n%6d\r\n", 0, 8,
	     layer, 66, 1);
    fprintf (dxf->out, "%3d\r\n%6d\r\n", 70, 0);
    for (iv = 0; iv < line->Points; iv++)
      {
	  /* exporting all vertices */
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
	  fprintf (dxf->out, "%3d\r\nVERTEX\r\n%3d\r\n%s\r\n", 0, 8, layer);
	  sprintf (format,
		   "%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n",
		   dxf->precision, dxf->precision, dxf->precision);
	  fprintf (dxf->out, format, 10, x, 20, y, 30, z);
      }
    fprintf (dxf->out, "%3d\r\nSEQEND\r\n%3d\r\n%s\r\n", 0, 8, layer);
    dxf->count++;
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWriteRing (gaiaDxfWriterPtr dxf, const char *layer, gaiaRingPtr ring)
{
/* printing a DXF POLYLINE (closed) */
    int iv;
    double x;
    double y;
    double z;
    double m;
    char format[128];
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    fprintf (dxf->out, "%3d\r\nPOLYLINE\r\n%3d\r\n%s\r\n%3d\r\n%6d\r\n", 0, 8,
	     layer, 66, 1);
    fprintf (dxf->out, "%3d\r\n%6d\r\n", 70, 1);
    for (iv = 0; iv < ring->Points - 1; iv++)
      {
	  /* sandro 2013-10-19 
	     exporting all vertices except the last one 
	     because accordingly to DXF specifications the
	     Ring closure is always implicitly assumed, so 
	     there is no need at all to explicitly export
	     a last vertex identical to the first one */
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
	  fprintf (dxf->out, "%3d\r\nVERTEX\r\n%3d\r\n%s\r\n", 0, 8, layer);
	  sprintf (format,
		   "%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n%%3d\r\n%%1.%df\r\n",
		   dxf->precision, dxf->precision, dxf->precision);
	  fprintf (dxf->out, format, 10, x, 20, y, 30, z);
      }
    fprintf (dxf->out, "%3d\r\nSEQEND\r\n%3d\r\n%s\r\n", 0, 8, layer);
    dxf->count++;
    return 1;
}

GAIAGEO_DECLARE int
gaiaDxfWriteGeometry (gaiaDxfWriterPtr dxf, const char *layer,
		      const char *label, double text_height,
		      double text_rotation, gaiaGeomCollPtr geom)
{
/* exporting a whole Geometry into the DXF */
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    int ib;
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    pt = geom->FirstPoint;
    while (pt != NULL)
      {
	  if (label == NULL)
	      gaiaDxfWritePoint (dxf, layer, pt->X, pt->Y, pt->Z);
	  else
	      gaiaDxfWriteText (dxf, layer, pt->X, pt->Y, pt->Z, label,
				text_height, text_rotation);
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln != NULL)
      {
	  gaiaDxfWriteLine (dxf, layer, ln);
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg != NULL)
      {
	  rng = pg->Exterior;
	  gaiaDxfWriteRing (dxf, layer, rng);
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		rng = pg->Interiors + ib;
		gaiaDxfWriteRing (dxf, layer, rng);
	    }
	  pg = pg->Next;
      }
    return 1;
}

static gaiaDxfExportLayerPtr
alloc_aux_layer (const char *layer, gaiaGeomCollPtr geom)
{
/* allocating and initializing an helper Layer */
    int len;
    gaiaDxfExportLayerPtr lyr = malloc (sizeof (gaiaDxfExportLayer));
    len = strlen (layer);
    lyr->layer_name = malloc (len + 1);
    strcpy (lyr->layer_name, layer), lyr->minx = geom->MinX;
    lyr->miny = geom->MinY;
    lyr->maxx = geom->MaxX;
    lyr->maxy = geom->MaxY;
    lyr->next = NULL;
    return lyr;
}

static void
destroy_aux_layer (gaiaDxfExportLayerPtr lyr)
{
/* destroying a DXF Helper Layer */
    if (lyr == NULL)
	return;
    if (lyr->layer_name != NULL)
	free (lyr->layer_name);
    free (lyr);
}

static gaiaDxfExportPtr
alloc_aux_exporter ()
{
/* allocating and initializing the helper Exporter */
    gaiaDxfExportPtr aux = malloc (sizeof (gaiaDxfExport));
    aux->first = NULL;
    aux->last = NULL;
    return aux;
}

static void
destroy_aux_exporter (gaiaDxfExportPtr aux)
{
/* memory cleanup - destroying the helper Exporter */
    gaiaDxfExportLayerPtr lyr;
    gaiaDxfExportLayerPtr n_lyr;
    if (aux == NULL)
	return;
    lyr = aux->first;
    while (lyr != NULL)
      {
	  n_lyr = lyr->next;
	  destroy_aux_layer (lyr);
	  lyr = n_lyr;
      }
    free (aux);
}

static void
update_aux_exporter (gaiaDxfExportPtr aux, const char *layer,
		     gaiaGeomCollPtr geom)
{
/* updating the Helper Exporter */
    gaiaDxfExportLayerPtr lyr = aux->first;
    while (lyr != NULL)
      {
	  if (strcasecmp (layer, lyr->layer_name) == 0)
	    {
		/* updating an already defined layer */
		if (geom->MinX < lyr->minx)
		    lyr->minx = geom->MinX;
		if (geom->MinY < lyr->miny)
		    lyr->miny = geom->MinY;
		if (geom->MaxX > lyr->maxx)
		    lyr->maxx = geom->MaxX;
		if (geom->MaxY > lyr->maxy)
		    lyr->maxy = geom->MaxY;
		if (geom->MinX < aux->minx)
		    aux->minx = geom->MinX;
		if (geom->MinY < aux->miny)
		    aux->miny = geom->MinY;
		if (geom->MaxX > aux->maxx)
		    aux->maxx = geom->MaxX;
		if (geom->MaxY > aux->maxy)
		    aux->maxy = geom->MaxY;
		return;
	    }
	  lyr = lyr->next;
      }
/* inserting a new Layer */
    lyr = alloc_aux_layer (layer, geom);
    if (aux->first == NULL)
      {
	  aux->first = lyr;
	  aux->minx = geom->MinX;
	  aux->miny = geom->MinY;
	  aux->maxx = geom->MaxX;
	  aux->maxy = geom->MaxY;
      }
    if (aux->last != NULL)
	aux->last->next = lyr;
    aux->last = lyr;
}

GAIAGEO_DECLARE int
gaiaExportDxf (gaiaDxfWriterPtr dxf, sqlite3 * db_handle,
	       const char *sql, const char *layer_col_name,
	       const char *geom_col_name, const char *label_col_name,
	       const char *text_height_col_name,
	       const char *text_rotation_col_name, gaiaGeomCollPtr geom_filter)
{
/* exporting a complex DXF by executing an arbitrary SQL query */
    sqlite3_stmt *stmt = NULL;
    int ret;
    int params;
    int first_row = 1;
    int layer_col = -1;
    int geom_col = -1;
    int label_col = -1;
    int text_height_col = -1;
    int text_rotation_col = -1;
    int i;
    unsigned char *p_blob;
    const unsigned char *blob;
    int len;
    const char *layer;
    const char *label = NULL;
    gaiaGeomCollPtr geom;
    gaiaDxfExportPtr aux = NULL;
    gaiaDxfExportLayerPtr lyr;
    if (dxf == NULL)
	return 0;
    if (dxf->error)
	return 0;
    if (dxf->out == NULL)
	return 0;
    if (db_handle == NULL)
	return 0;
    if (sql == NULL)
	return 0;
    if (layer_col_name == NULL)
	return 0;
    if (geom_col_name == NULL)
	return 0;

/* attempting to create the SQL prepared statement */
    ret = sqlite3_prepare_v2 (db_handle, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("exportDXF - CREATE STATEMENT error: %s\n",
			sqlite3_errmsg (db_handle));
	  goto stop;
      }
    params = sqlite3_bind_parameter_count (stmt);

    if (params > 0 && geom_filter != NULL)
      {
	  /* parameter binding - Spatial Filter */
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  for (i = 1; i <= params; i++)
	    {
		gaiaToSpatiaLiteBlobWkb (geom_filter, &p_blob, &len);
		ret = sqlite3_bind_blob (stmt, i, p_blob, len, free);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("exportDXF - parameter BIND error: %s\n",
				    sqlite3_errmsg (db_handle));
		      goto stop;
		  }
	    }
      }

/* pass #1 - sniffing the result set */
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		if (first_row)
		  {
		      /* this one is the first row of the resultset */
		      for (i = 0; i < sqlite3_column_count (stmt); i++)
			{
			    /* attempting to identify the resultset columns */
			    if (strcasecmp
				(layer_col_name,
				 sqlite3_column_name (stmt, i)) == 0)
				layer_col = i;
			    if (strcasecmp
				(geom_col_name,
				 sqlite3_column_name (stmt, i)) == 0)
				geom_col = i;
			    if (label_col_name != NULL)
			      {
				  if (strcasecmp
				      (label_col_name,
				       sqlite3_column_name (stmt, i)) == 0)
				      label_col = i;
			      }
			    if (text_height_col_name != NULL)
			      {
				  if (strcasecmp
				      (text_height_col_name,
				       sqlite3_column_name (stmt, i)) == 0)
				      text_height_col = i;
			      }
			    if (text_rotation_col_name != NULL)
			      {
				  if (strcasecmp
				      (text_rotation_col_name,
				       sqlite3_column_name (stmt, i)) == 0)
				      text_rotation_col = i;
			      }
			}
		      if (layer_col < 0)
			{
			    spatialite_e
				("exportDXF - Layer Column not found into the resultset\n");
			    goto stop;
			}
		      if (geom_col < 0)
			{
			    spatialite_e
				("exportDXF - Geometry Column not found into the resultset\n");
			    goto stop;
			}
		      first_row = 0;
		      aux = alloc_aux_exporter ();
		  }
		layer = (const char *) sqlite3_column_text (stmt, layer_col);
		blob = sqlite3_column_blob (stmt, geom_col);
		len = sqlite3_column_bytes (stmt, geom_col);
		geom = gaiaFromSpatiaLiteBlobWkb (blob, len);
		if (geom)
		  {
		      update_aux_exporter (aux, layer, geom);
		      gaiaFreeGeomColl (geom);
		  }
	    }
      }

/* pass #2 - exporting the DXF file */
    gaiaDxfWriteHeader (dxf, aux->minx, aux->miny, 0, aux->maxx, aux->maxy, 0);
    gaiaDxfWriteTables (dxf);
    lyr = aux->first;
    while (lyr != NULL)
      {
	  gaiaDxfWriteLayer (dxf, lyr->layer_name);
	  lyr = lyr->next;
      }
    gaiaDxfWriteEndSection (dxf);
    gaiaDxfWriteEntities (dxf);

    sqlite3_reset (stmt);
    while (1)
      {
	  /* scrolling the result set rows */
	  int ival;
	  double height = 10.0;
	  double rotation = 0.0;
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		layer = (const char *) sqlite3_column_text (stmt, layer_col);
		if (label_col >= 0)
		    label =
			(const char *) sqlite3_column_text (stmt, label_col);
		if (text_height_col >= 0)
		  {
		      if (sqlite3_column_type (stmt, text_height_col) ==
			  SQLITE_INTEGER)
			{
			    ival = sqlite3_column_int (stmt, text_height_col);
			    height = ival;
			}
		      if (sqlite3_column_type (stmt, text_height_col) ==
			  SQLITE_FLOAT)
			  height =
			      sqlite3_column_double (stmt, text_height_col);
		  }
		if (text_rotation_col >= 0)
		  {
		      if (sqlite3_column_type (stmt, text_rotation_col) ==
			  SQLITE_INTEGER)
			{
			    ival = sqlite3_column_int (stmt, text_rotation_col);
			    rotation = ival;
			}
		      if (sqlite3_column_type (stmt, text_height_col) ==
			  SQLITE_FLOAT)
			  rotation =
			      sqlite3_column_double (stmt, text_rotation_col);
		  }
		blob = sqlite3_column_blob (stmt, geom_col);
		len = sqlite3_column_bytes (stmt, geom_col);
		geom = gaiaFromSpatiaLiteBlobWkb (blob, len);
		if (geom)
		  {
		      gaiaDxfWriteGeometry (dxf, layer, label, height, rotation,
					    geom);
		      gaiaFreeGeomColl (geom);
		  }
	    }
      }
    gaiaDxfWriteEndSection (dxf);
    gaiaDxfWriteFooter (dxf);

    sqlite3_finalize (stmt);
    if (aux != NULL)
	destroy_aux_exporter (aux);
    return dxf->count;

  stop:
    if (stmt != NULL)
	sqlite3_finalize (stmt);
    if (aux != NULL)
	destroy_aux_exporter (aux);
    return 0;
}
