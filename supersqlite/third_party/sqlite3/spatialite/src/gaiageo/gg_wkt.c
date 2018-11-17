/*

 gg_wkt.c -- Gaia common support for WKT encoded geometries
  
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
Klaus Foerster klaus.foerster@svg.cc

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

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>

#include <spatialite/gaiageo.h>

static void
gaiaOutClean (char *buffer)
{
/* cleans unneeded trailing zeros */
    int i;
    for (i = strlen (buffer) - 1; i > 0; i--)
      {
	  if (buffer[i] == '0')
	      buffer[i] = '\0';
	  else
	      break;
      }
    if (buffer[i] == '.')
	buffer[i] = '\0';
    if (strcmp (buffer, "-0") == 0)
      {
	  /* avoiding to return embarassing NEGATIVE ZEROes */
	  strcpy (buffer, "0");
      }

    if (strcmp (buffer, "-1.#QNAN") == 0 || strcmp (buffer, "NaN") == 0
	|| strcmp (buffer, "1.#QNAN") == 0
	|| strcmp (buffer, "-1.#IND") == 0 || strcmp (buffer, "1.#IND") == 0)
      {
	  /* on Windows a NaN could be represented in "odd" ways */
	  /* this is intended to restore a consistent behaviour  */
	  strcpy (buffer, "nan");
      }
}

GAIAGEO_DECLARE void
gaiaOutBufferInitialize (gaiaOutBufferPtr buf)
{
/* initializing a dynamically growing output buffer */
    buf->Buffer = NULL;
    buf->WriteOffset = 0;
    buf->BufferSize = 0;
    buf->Error = 0;
}

GAIAGEO_DECLARE void
gaiaOutBufferReset (gaiaOutBufferPtr buf)
{
/* cleaning a dynamically growing output buffer */
    if (buf->Buffer)
	free (buf->Buffer);
    buf->Buffer = NULL;
    buf->WriteOffset = 0;
    buf->BufferSize = 0;
    buf->Error = 0;
}

GAIAGEO_DECLARE void
gaiaAppendToOutBuffer (gaiaOutBufferPtr buf, const char *text)
{
/* appending a text string */
    int len = strlen (text);
    int free_size = buf->BufferSize - buf->WriteOffset;
    if ((len + 1) > free_size)
      {
	  /* we must allocate a bigger buffer */
	  int new_size;
	  char *new_buf;
	  if (buf->BufferSize == 0)
	      new_size = (len + 1) + 1024;
	  else if (buf->BufferSize <= 4196)
	      new_size = buf->BufferSize + (len + 1) + 4196;
	  else if (buf->BufferSize <= 65536)
	      new_size = buf->BufferSize + (len + 1) + 65536;
	  else
	      new_size = buf->BufferSize + (len + 1) + (1024 * 1024);
	  new_buf = malloc (new_size);
	  if (!new_buf)
	    {
		buf->Error = 1;
		return;
	    }
	  memcpy (new_buf, buf->Buffer, buf->WriteOffset);
	  if (buf->Buffer)
	      free (buf->Buffer);
	  buf->Buffer = new_buf;
	  buf->BufferSize = new_size;
      }
    strcpy (buf->Buffer + buf->WriteOffset, text);
    buf->WriteOffset += len;
}

static void
gaiaOutPointStrict (gaiaOutBufferPtr out_buf, gaiaPointPtr point, int precision)
{
/* formats a WKT POINT [Strict 2D] */
    char *buf_x;
    char *buf_y;
    char *buf;
    buf_x = sqlite3_mprintf ("%.*f", precision, point->X);
    gaiaOutClean (buf_x);
    buf_y = sqlite3_mprintf ("%.*f", precision, point->Y);
    gaiaOutClean (buf_y);
    buf = sqlite3_mprintf ("%s %s", buf_x, buf_y);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

static void
gaiaOutPoint (gaiaOutBufferPtr out_buf, gaiaPointPtr point, int precision)
{
/* formats a WKT POINT */
    char *buf_x;
    char *buf_y;
    char *buf;
    if (precision < 0)
	buf_x = sqlite3_mprintf ("%1.6f", point->X);
    else
	buf_x = sqlite3_mprintf ("%.*f", precision, point->X);
    gaiaOutClean (buf_x);
    if (precision < 0)
	buf_y = sqlite3_mprintf ("%1.6f", point->Y);
    else
	buf_y = sqlite3_mprintf ("%.*f", precision, point->Y);
    gaiaOutClean (buf_y);
    buf = sqlite3_mprintf ("%s %s", buf_x, buf_y);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

GAIAGEO_DECLARE void
gaiaOutPointZex (gaiaOutBufferPtr out_buf, gaiaPointPtr point, int precision)
{
/* formats a WKT POINTZ */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf;
    if (precision < 0)
	buf_x = sqlite3_mprintf ("%1.6f", point->X);
    else
	buf_x = sqlite3_mprintf ("%.*f", precision, point->X);
    gaiaOutClean (buf_x);
    if (precision < 0)
	buf_y = sqlite3_mprintf ("%1.6f", point->Y);
    else
	buf_y = sqlite3_mprintf ("%.*f", precision, point->Y);
    gaiaOutClean (buf_y);
    if (precision < 0)
	buf_z = sqlite3_mprintf ("%1.6f", point->Z);
    else
	buf_z = sqlite3_mprintf ("%.*f", precision, point->Z);
    gaiaOutClean (buf_z);
    buf = sqlite3_mprintf ("%s %s %s", buf_x, buf_y, buf_z);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    sqlite3_free (buf_z);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

GAIAGEO_DECLARE void
gaiaOutPointZ (gaiaOutBufferPtr out_buf, gaiaPointPtr point)
{
/* 
* formats a WKT POINTZ 
* convenience method - default decimal precision
*/
    gaiaOutPointZex (out_buf, point, -1);
}

static void
gaiaOutPointM (gaiaOutBufferPtr out_buf, gaiaPointPtr point, int precision)
{
/* formats a WKT POINTM */
    char *buf_x;
    char *buf_y;
    char *buf_m;
    char *buf;
    if (precision < 0)
	buf_x = sqlite3_mprintf ("%1.6f", point->X);
    else
	buf_x = sqlite3_mprintf ("%.*f", precision, point->X);
    gaiaOutClean (buf_x);
    if (precision < 0)
	buf_y = sqlite3_mprintf ("%1.6f", point->Y);
    else
	buf_y = sqlite3_mprintf ("%.*f", precision, point->Y);
    gaiaOutClean (buf_y);
    if (precision < 0)
	buf_m = sqlite3_mprintf ("%1.6f", point->M);
    else
	buf_m = sqlite3_mprintf ("%.*f", precision, point->M);
    gaiaOutClean (buf_m);
    buf = sqlite3_mprintf ("%s %s %s", buf_x, buf_y, buf_m);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    sqlite3_free (buf_m);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

static void
gaiaOutPointZM (gaiaOutBufferPtr out_buf, gaiaPointPtr point, int precision)
{
/* formats a WKT POINTZM */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf_m;
    char *buf;
    if (precision < 0)
	buf_x = sqlite3_mprintf ("%1.6f", point->X);
    else
	buf_x = sqlite3_mprintf ("%.*f", precision, point->X);
    gaiaOutClean (buf_x);
    if (precision < 0)
	buf_y = sqlite3_mprintf ("%1.6f", point->Y);
    else
	buf_y = sqlite3_mprintf ("%.*f", precision, point->Y);
    gaiaOutClean (buf_y);
    if (precision < 0)
	buf_z = sqlite3_mprintf ("%1.6f", point->Z);
    else
	buf_z = sqlite3_mprintf ("%.*f", precision, point->Z);
    gaiaOutClean (buf_z);
    if (precision < 0)
	buf_m = sqlite3_mprintf ("%1.6f", point->M);
    else
	buf_m = sqlite3_mprintf ("%.*f", precision, point->M);
    gaiaOutClean (buf_m);
    buf = sqlite3_mprintf ("%s %s %s %s", buf_x, buf_y, buf_z, buf_m);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    sqlite3_free (buf_z);
    sqlite3_free (buf_m);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

static void
gaiaOutEwktPoint (gaiaOutBufferPtr out_buf, gaiaPointPtr point)
{
/* formats an EWKT POINT */
    char *buf_x;
    char *buf_y;
    char *buf;
    buf_x = sqlite3_mprintf ("%1.15f", point->X);
    gaiaOutClean (buf_x);
    buf_y = sqlite3_mprintf ("%1.15f", point->Y);
    gaiaOutClean (buf_y);
    buf = sqlite3_mprintf ("%s %s", buf_x, buf_y);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

GAIAGEO_DECLARE void
gaiaOutEwktPointZ (gaiaOutBufferPtr out_buf, gaiaPointPtr point)
{
/* formats an EWKT POINTZ */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf;
    buf_x = sqlite3_mprintf ("%1.15f", point->X);
    gaiaOutClean (buf_x);
    buf_y = sqlite3_mprintf ("%1.15f", point->Y);
    gaiaOutClean (buf_y);
    buf_z = sqlite3_mprintf ("%1.15f", point->Z);
    gaiaOutClean (buf_z);
    buf = sqlite3_mprintf ("%s %s %s", buf_x, buf_y, buf_z);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    sqlite3_free (buf_z);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

static void
gaiaOutEwktPointM (gaiaOutBufferPtr out_buf, gaiaPointPtr point)
{
/* formats an EWKT POINTM */
    char *buf_x;
    char *buf_y;
    char *buf_m;
    char *buf;
    buf_x = sqlite3_mprintf ("%1.15f", point->X);
    gaiaOutClean (buf_x);
    buf_y = sqlite3_mprintf ("%1.15f", point->Y);
    gaiaOutClean (buf_y);
    buf_m = sqlite3_mprintf ("%1.15f", point->M);
    gaiaOutClean (buf_m);
    buf = sqlite3_mprintf ("%s %s %s", buf_x, buf_y, buf_m);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    sqlite3_free (buf_m);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

static void
gaiaOutEwktPointZM (gaiaOutBufferPtr out_buf, gaiaPointPtr point)
{
/* formats an EWKT POINTZM */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf_m;
    char *buf;
    buf_x = sqlite3_mprintf ("%1.15f", point->X);
    gaiaOutClean (buf_x);
    buf_y = sqlite3_mprintf ("%1.15f", point->Y);
    gaiaOutClean (buf_y);
    buf_z = sqlite3_mprintf ("%1.15f", point->Z);
    gaiaOutClean (buf_z);
    buf_m = sqlite3_mprintf ("%1.15f", point->M);
    gaiaOutClean (buf_m);
    buf = sqlite3_mprintf ("%s %s %s %s", buf_x, buf_y, buf_z, buf_m);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    sqlite3_free (buf_z);
    sqlite3_free (buf_m);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

static void
gaiaOutLinestringStrict (gaiaOutBufferPtr out_buf, gaiaLinestringPtr line,
			 int precision)
{
/* formats a WKT LINESTRING [Strict 2D] */
    char *buf_x;
    char *buf_y;
    char *buf;
    double x;
    double y;
    double z;
    double m;
    int iv;
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
	  buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (iv > 0)
	      buf = sqlite3_mprintf (",%s %s", buf_x, buf_y);
	  else
	      buf = sqlite3_mprintf ("%s %s", buf_x, buf_y);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

static void
gaiaOutLinestring (gaiaOutBufferPtr out_buf, gaiaLinestringPtr line,
		   int precision)
{
/* formats a WKT LINESTRING */
    char *buf_x;
    char *buf_y;
    char *buf;
    double x;
    double y;
    int iv;
    for (iv = 0; iv < line->Points; iv++)
      {
	  gaiaGetPoint (line->Coords, iv, &x, &y);
	  if (precision < 0)
	      buf_x = sqlite3_mprintf ("%1.6f", x);
	  else
	      buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  if (precision < 0)
	      buf_y = sqlite3_mprintf ("%1.6f", y);
	  else
	      buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (iv > 0)
	      buf = sqlite3_mprintf (", %s %s", buf_x, buf_y);
	  else
	      buf = sqlite3_mprintf ("%s %s", buf_x, buf_y);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

GAIAGEO_DECLARE void
gaiaOutLinestringZex (gaiaOutBufferPtr out_buf, gaiaLinestringPtr line,
		      int precision)
{
/* formats a WKT LINESTRINGZ */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf;
    double x;
    double y;
    double z;
    int iv;
    for (iv = 0; iv < line->Points; iv++)
      {
	  gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
	  if (precision < 0)
	      buf_x = sqlite3_mprintf ("%1.6f", x);
	  else
	      buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  if (precision < 0)
	      buf_y = sqlite3_mprintf ("%1.6f", y);
	  else
	      buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (precision < 0)
	      buf_z = sqlite3_mprintf ("%1.6f", z);
	  else
	      buf_z = sqlite3_mprintf ("%.*f", precision, z);
	  gaiaOutClean (buf_z);
	  if (iv > 0)
	      buf = sqlite3_mprintf (", %s %s %s", buf_x, buf_y, buf_z);
	  else
	      buf = sqlite3_mprintf ("%s %s %s", buf_x, buf_y, buf_z);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_z);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

GAIAGEO_DECLARE void
gaiaOutLinestringZ (gaiaOutBufferPtr out_buf, gaiaLinestringPtr line)
{
/*
* formats a WKT LINESTRINGZ 
* convenience method - default decimal precision
*/
    gaiaOutLinestringZex (out_buf, line, -1);
}

static void
gaiaOutLinestringM (gaiaOutBufferPtr out_buf, gaiaLinestringPtr line,
		    int precision)
{
/* formats a WKT LINESTRINGM */
    char *buf_x;
    char *buf_y;
    char *buf_m;
    char *buf;
    double x;
    double y;
    double m;
    int iv;
    for (iv = 0; iv < line->Points; iv++)
      {
	  gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
	  if (precision < 0)
	      buf_x = sqlite3_mprintf ("%1.6f", x);
	  else
	      buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  if (precision < 0)
	      buf_y = sqlite3_mprintf ("%1.6f", y);
	  else
	      buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (precision < 0)
	      buf_m = sqlite3_mprintf ("%1.6f", m);
	  else
	      buf_m = sqlite3_mprintf ("%.*f", precision, m);
	  gaiaOutClean (buf_m);
	  if (iv > 0)
	      buf = sqlite3_mprintf (", %s %s %s", buf_x, buf_y, buf_m);
	  else
	      buf = sqlite3_mprintf ("%s %s %s", buf_x, buf_y, buf_m);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_m);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

static void
gaiaOutLinestringZM (gaiaOutBufferPtr out_buf, gaiaLinestringPtr line,
		     int precision)
{
/* formats a WKT LINESTRINGZM */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf_m;
    char *buf;
    double x;
    double y;
    double z;
    double m;
    int iv;
    for (iv = 0; iv < line->Points; iv++)
      {
	  gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
	  if (precision < 0)
	      buf_x = sqlite3_mprintf ("%1.6f", x);
	  else
	      buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  if (precision < 0)
	      buf_y = sqlite3_mprintf ("%1.6f", y);
	  else
	      buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (precision < 0)
	      buf_z = sqlite3_mprintf ("%1.6f", z);
	  else
	      buf_z = sqlite3_mprintf ("%.*f", precision, z);
	  gaiaOutClean (buf_z);
	  if (precision < 0)
	      buf_m = sqlite3_mprintf ("%1.6f", m);
	  else
	      buf_m = sqlite3_mprintf ("%.*f", precision, m);
	  gaiaOutClean (buf_m);
	  if (iv > 0)
	      buf =
		  sqlite3_mprintf (", %s %s %s %s", buf_x, buf_y, buf_z, buf_m);
	  else
	      buf = sqlite3_mprintf ("%s %s %s %s", buf_x, buf_y, buf_z, buf_m);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_z);
	  sqlite3_free (buf_m);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

static void
gaiaOutEwktLinestring (gaiaOutBufferPtr out_buf, gaiaLinestringPtr line)
{
/* formats an EWKT LINESTRING */
    char *buf_x;
    char *buf_y;
    char *buf;
    double x;
    double y;
    int iv;
    for (iv = 0; iv < line->Points; iv++)
      {
	  gaiaGetPoint (line->Coords, iv, &x, &y);
	  buf_x = sqlite3_mprintf ("%1.15f", x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%1.15f", y);
	  gaiaOutClean (buf_y);
	  if (iv > 0)
	      buf = sqlite3_mprintf (",%s %s", buf_x, buf_y);
	  else
	      buf = sqlite3_mprintf ("%s %s", buf_x, buf_y);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

GAIAGEO_DECLARE void
gaiaOutEwktLinestringZ (gaiaOutBufferPtr out_buf, gaiaLinestringPtr line)
{
/* formats an EWKT LINESTRINGZ */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf;
    double x;
    double y;
    double z;
    int iv;
    for (iv = 0; iv < line->Points; iv++)
      {
	  gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
	  buf_x = sqlite3_mprintf ("%1.15f", x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%1.15f", y);
	  gaiaOutClean (buf_y);
	  buf_z = sqlite3_mprintf ("%1.15f", z);
	  gaiaOutClean (buf_z);
	  if (iv > 0)
	      buf = sqlite3_mprintf (",%s %s %s", buf_x, buf_y, buf_z);
	  else
	      buf = sqlite3_mprintf ("%s %s %s", buf_x, buf_y, buf_z);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_z);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

static void
gaiaOutEwktLinestringM (gaiaOutBufferPtr out_buf, gaiaLinestringPtr line)
{
/* formats an EWKT LINESTRINGM */
    char *buf_x;
    char *buf_y;
    char *buf_m;
    char *buf;
    double x;
    double y;
    double m;
    int iv;
    for (iv = 0; iv < line->Points; iv++)
      {
	  gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
	  buf_x = sqlite3_mprintf ("%1.15f", x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%1.15f", y);
	  gaiaOutClean (buf_y);
	  buf_m = sqlite3_mprintf ("%1.15f", m);
	  gaiaOutClean (buf_m);
	  if (iv > 0)
	      buf = sqlite3_mprintf (",%s %s %s", buf_x, buf_y, buf_m);
	  else
	      buf = sqlite3_mprintf ("%s %s %s", buf_x, buf_y, buf_m);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_m);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

static void
gaiaOutEwktLinestringZM (gaiaOutBufferPtr out_buf, gaiaLinestringPtr line)
{
/* formats an EWKT LINESTRINGZM */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf_m;
    char *buf;
    double x;
    double y;
    double z;
    double m;
    int iv;
    for (iv = 0; iv < line->Points; iv++)
      {
	  gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
	  buf_x = sqlite3_mprintf ("%1.15f", x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%1.15f", y);
	  gaiaOutClean (buf_y);
	  buf_z = sqlite3_mprintf ("%1.15f", z);
	  gaiaOutClean (buf_z);
	  buf_m = sqlite3_mprintf ("%1.15f", m);
	  gaiaOutClean (buf_m);
	  if (iv > 0)
	      buf =
		  sqlite3_mprintf (",%s %s %s %s", buf_x, buf_y, buf_z, buf_m);
	  else
	      buf = sqlite3_mprintf ("%s %s %s %s", buf_x, buf_y, buf_z, buf_m);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_z);
	  sqlite3_free (buf_m);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

static void
gaiaOutPolygonStrict (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polyg,
		      int precision)
{
/* formats a WKT POLYGON [Strict 2D] */
    char *buf_x;
    char *buf_y;
    char *buf;
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    gaiaRingPtr ring = polyg->Exterior;
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
	  buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (iv == 0)
	      buf = sqlite3_mprintf ("(%s %s", buf_x, buf_y);
	  else if (iv == (ring->Points - 1))
	      buf = sqlite3_mprintf (",%s %s)", buf_x, buf_y);
	  else
	      buf = sqlite3_mprintf (",%s %s", buf_x, buf_y);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
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
		buf_x = sqlite3_mprintf ("%.*f", precision, x);
		gaiaOutClean (buf_x);
		buf_y = sqlite3_mprintf ("%.*f", precision, y);
		gaiaOutClean (buf_y);
		if (iv == 0)
		    buf = sqlite3_mprintf (",(%s %s", buf_x, buf_y);
		else if (iv == (ring->Points - 1))
		    buf = sqlite3_mprintf (",%s %s)", buf_x, buf_y);
		else
		    buf = sqlite3_mprintf (",%s %s", buf_x, buf_y);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
      }
}

static void
gaiaOutPolygon (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polyg, int precision)
{
/* formats a WKT POLYGON */
    char *buf_x;
    char *buf_y;
    char *buf;
    int ib;
    int iv;
    double x;
    double y;
    gaiaRingPtr ring = polyg->Exterior;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPoint (ring->Coords, iv, &x, &y);
	  if (precision < 0)
	      buf_x = sqlite3_mprintf ("%1.6f", x);
	  else
	      buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  if (precision < 0)
	      buf_y = sqlite3_mprintf ("%1.6f", y);
	  else
	      buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (iv == 0)
	      buf = sqlite3_mprintf ("(%s %s", buf_x, buf_y);
	  else if (iv == (ring->Points - 1))
	      buf = sqlite3_mprintf (", %s %s)", buf_x, buf_y);
	  else
	      buf = sqlite3_mprintf (", %s %s", buf_x, buf_y);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		gaiaGetPoint (ring->Coords, iv, &x, &y);
		if (precision < 0)
		    buf_x = sqlite3_mprintf ("%1.6f", x);
		else
		    buf_x = sqlite3_mprintf ("%.*f", precision, x);
		gaiaOutClean (buf_x);
		if (precision < 0)
		    buf_y = sqlite3_mprintf ("%1.6f", y);
		else
		    buf_y = sqlite3_mprintf ("%.*f", precision, y);
		gaiaOutClean (buf_y);
		if (iv == 0)
		    buf = sqlite3_mprintf (", (%s %s", buf_x, buf_y);
		else if (iv == (ring->Points - 1))
		    buf = sqlite3_mprintf (", %s %s)", buf_x, buf_y);
		else
		    buf = sqlite3_mprintf (", %s %s", buf_x, buf_y);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
      }
}

GAIAGEO_DECLARE void
gaiaOutPolygonZex (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polyg,
		   int precision)
{
/* formats a WKT POLYGONZ */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf;
    int ib;
    int iv;
    double x;
    double y;
    double z;
    gaiaRingPtr ring = polyg->Exterior;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
	  if (precision < 0)
	      buf_x = sqlite3_mprintf ("%1.6f", x);
	  else
	      buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  if (precision < 0)
	      buf_y = sqlite3_mprintf ("%1.6f", y);
	  else
	      buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (precision < 0)
	      buf_z = sqlite3_mprintf ("%1.6f", z);
	  else
	      buf_z = sqlite3_mprintf ("%.*f", precision, z);
	  gaiaOutClean (buf_z);
	  if (iv == 0)
	      buf = sqlite3_mprintf ("(%s %s %s", buf_x, buf_y, buf_z);
	  else if (iv == (ring->Points - 1))
	      buf = sqlite3_mprintf (", %s %s %s)", buf_x, buf_y, buf_z);
	  else
	      buf = sqlite3_mprintf (", %s %s %s", buf_x, buf_y, buf_z);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_z);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		if (precision < 0)
		    buf_x = sqlite3_mprintf ("%1.6f", x);
		else
		    buf_x = sqlite3_mprintf ("%.*f", precision, x);
		gaiaOutClean (buf_x);
		if (precision < 0)
		    buf_y = sqlite3_mprintf ("%1.6f", y);
		else
		    buf_y = sqlite3_mprintf ("%.*f", precision, y);
		gaiaOutClean (buf_y);
		if (precision < 0)
		    buf_z = sqlite3_mprintf ("%1.6f", z);
		else
		    buf_z = sqlite3_mprintf ("%.*f", precision, z);
		gaiaOutClean (buf_z);
		if (iv == 0)
		    buf = sqlite3_mprintf (", (%s %s %s", buf_x, buf_y, buf_z);
		else if (iv == (ring->Points - 1))
		    buf = sqlite3_mprintf (", %s %s %s)", buf_x, buf_y, buf_z);
		else
		    buf = sqlite3_mprintf (", %s %s %s", buf_x, buf_y, buf_z);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		sqlite3_free (buf_z);
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
      }
}

GAIAGEO_DECLARE void
gaiaOutPolygonZ (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polyg)
{
/* 
* formats a WKT POLYGONZ
* convenience method - default decimal precision
*/
    gaiaOutPolygonZex (out_buf, polyg, -1);
}

static void
gaiaOutPolygonM (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polyg, int precision)
{
/* formats a WKT POLYGONM */
    char *buf_x;
    char *buf_y;
    char *buf_m;
    char *buf;
    int ib;
    int iv;
    double x;
    double y;
    double m;
    gaiaRingPtr ring = polyg->Exterior;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
	  if (precision < 0)
	      buf_x = sqlite3_mprintf ("%1.6f", x);
	  else
	      buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  if (precision < 0)
	      buf_y = sqlite3_mprintf ("%1.6f", y);
	  else
	      buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (precision < 0)
	      buf_m = sqlite3_mprintf ("%1.6f", m);
	  else
	      buf_m = sqlite3_mprintf ("%.*f", precision, m);
	  gaiaOutClean (buf_m);
	  if (iv == 0)
	      buf = sqlite3_mprintf ("(%s %s %s", buf_x, buf_y, buf_m);
	  else if (iv == (ring->Points - 1))
	      buf = sqlite3_mprintf (", %s %s %s)", buf_x, buf_y, buf_m);
	  else
	      buf = sqlite3_mprintf (", %s %s %s", buf_x, buf_y, buf_m);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_m);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		if (precision < 0)
		    buf_x = sqlite3_mprintf ("%1.6f", x);
		else
		    buf_x = sqlite3_mprintf ("%.*f", precision, x);
		gaiaOutClean (buf_x);
		if (precision < 0)
		    buf_y = sqlite3_mprintf ("%1.6f", y);
		else
		    buf_y = sqlite3_mprintf ("%.*f", precision, y);
		gaiaOutClean (buf_y);
		if (precision < 0)
		    buf_m = sqlite3_mprintf ("%1.6f", m);
		else
		    buf_m = sqlite3_mprintf ("%.*f", precision, m);
		gaiaOutClean (buf_m);
		if (iv == 0)
		    buf = sqlite3_mprintf (", (%s %s %s", buf_x, buf_y, buf_m);
		else if (iv == (ring->Points - 1))
		    buf = sqlite3_mprintf (", %s %s %s)", buf_x, buf_y, buf_m);
		else
		    buf = sqlite3_mprintf (", %s %s %s", buf_x, buf_y, buf_m);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		sqlite3_free (buf_m);
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
      }
}

static void
gaiaOutPolygonZM (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polyg, int precision)
{
/* formats a WKT POLYGONZM */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf_m;
    char *buf;
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    gaiaRingPtr ring = polyg->Exterior;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
	  if (precision < 0)
	      buf_x = sqlite3_mprintf ("%1.6f", x);
	  else
	      buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  if (precision < 0)
	      buf_y = sqlite3_mprintf ("%1.6f", y);
	  else
	      buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (precision < 0)
	      buf_z = sqlite3_mprintf ("%1.6f", z);
	  else
	      buf_z = sqlite3_mprintf ("%.*f", precision, z);
	  gaiaOutClean (buf_z);
	  if (precision < 0)
	      buf_m = sqlite3_mprintf ("%1.6f", m);
	  else
	      buf_m = sqlite3_mprintf ("%.*f", precision, m);
	  gaiaOutClean (buf_m);
	  if (iv == 0)
	      buf =
		  sqlite3_mprintf ("(%s %s %s %s", buf_x, buf_y, buf_z, buf_m);
	  else if (iv == (ring->Points - 1))
	      buf =
		  sqlite3_mprintf (", %s %s %s %s)", buf_x, buf_y, buf_z,
				   buf_m);
	  else
	      buf =
		  sqlite3_mprintf (", %s %s %s %s", buf_x, buf_y, buf_z, buf_m);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_z);
	  sqlite3_free (buf_m);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		if (precision < 0)
		    buf_x = sqlite3_mprintf ("%1.6f", x);
		else
		    buf_x = sqlite3_mprintf ("%.*f", precision, x);
		gaiaOutClean (buf_x);
		if (precision < 0)
		    buf_y = sqlite3_mprintf ("%1.6f", y);
		else
		    buf_y = sqlite3_mprintf ("%.*f", precision, y);
		gaiaOutClean (buf_y);
		if (precision < 0)
		    buf_z = sqlite3_mprintf ("%1.6f", z);
		else
		    buf_z = sqlite3_mprintf ("%.*f", precision, z);
		gaiaOutClean (buf_z);
		if (precision < 0)
		    buf_m = sqlite3_mprintf ("%1.6f", m);
		else
		    buf_m = sqlite3_mprintf ("%.*f", precision, m);
		gaiaOutClean (buf_m);
		if (iv == 0)
		    buf =
			sqlite3_mprintf (", (%s %s %s %s", buf_x, buf_y, buf_z,
					 buf_m);
		else if (iv == (ring->Points - 1))
		    buf =
			sqlite3_mprintf (", %s %s %s %s)", buf_x, buf_y, buf_z,
					 buf_m);
		else
		    buf =
			sqlite3_mprintf (", %s %s %s %s", buf_x, buf_y, buf_z,
					 buf_m);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		sqlite3_free (buf_z);
		sqlite3_free (buf_m);
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
      }
}

static void
gaiaOutEwktPolygon (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polyg)
{
/* formats an EWKT POLYGON */
    char *buf_x;
    char *buf_y;
    char *buf;
    int ib;
    int iv;
    double x;
    double y;
    gaiaRingPtr ring = polyg->Exterior;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPoint (ring->Coords, iv, &x, &y);
	  buf_x = sqlite3_mprintf ("%1.15f", x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%1.15f", y);
	  gaiaOutClean (buf_y);
	  if (iv == 0)
	      buf = sqlite3_mprintf ("(%s %s", buf_x, buf_y);
	  else if (iv == (ring->Points - 1))
	      buf = sqlite3_mprintf (",%s %s)", buf_x, buf_y);
	  else
	      buf = sqlite3_mprintf (",%s %s", buf_x, buf_y);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		gaiaGetPoint (ring->Coords, iv, &x, &y);
		buf_x = sqlite3_mprintf ("%1.15f", x);
		gaiaOutClean (buf_x);
		buf_y = sqlite3_mprintf ("%1.15f", y);
		gaiaOutClean (buf_y);
		if (iv == 0)
		    buf = sqlite3_mprintf (",(%s %s", buf_x, buf_y);
		else if (iv == (ring->Points - 1))
		    buf = sqlite3_mprintf (",%s %s)", buf_x, buf_y);
		else
		    buf = sqlite3_mprintf (",%s %s", buf_x, buf_y);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
      }
}

GAIAGEO_DECLARE void
gaiaOutEwktPolygonZ (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polyg)
{
/* formats an EWKT POLYGONZ */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf;
    int ib;
    int iv;
    double x;
    double y;
    double z;
    gaiaRingPtr ring = polyg->Exterior;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
	  buf_x = sqlite3_mprintf ("%1.15f", x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%1.15f", y);
	  gaiaOutClean (buf_y);
	  buf_z = sqlite3_mprintf ("%1.15f", z);
	  gaiaOutClean (buf_z);
	  if (iv == 0)
	      buf = sqlite3_mprintf ("(%s %s %s", buf_x, buf_y, buf_z);
	  else if (iv == (ring->Points - 1))
	      buf = sqlite3_mprintf (",%s %s %s)", buf_x, buf_y, buf_z);
	  else
	      buf = sqlite3_mprintf (",%s %s %s", buf_x, buf_y, buf_z);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_z);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		buf_x = sqlite3_mprintf ("%1.15f", x);
		gaiaOutClean (buf_x);
		buf_y = sqlite3_mprintf ("%1.15f", y);
		gaiaOutClean (buf_y);
		buf_z = sqlite3_mprintf ("%1.15f", z);
		gaiaOutClean (buf_z);
		if (iv == 0)
		    buf = sqlite3_mprintf (",(%s %s %s", buf_x, buf_y, buf_z);
		else if (iv == (ring->Points - 1))
		    buf = sqlite3_mprintf (",%s %s %s)", buf_x, buf_y, buf_z);
		else
		    buf = sqlite3_mprintf (",%s %s %s", buf_x, buf_y, buf_z);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		sqlite3_free (buf_z);
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
      }
}

static void
gaiaOutEwktPolygonM (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polyg)
{
/* formats an EWKT POLYGONM */
    char *buf_x;
    char *buf_y;
    char *buf_m;
    char *buf;
    int ib;
    int iv;
    double x;
    double y;
    double m;
    gaiaRingPtr ring = polyg->Exterior;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
	  buf_x = sqlite3_mprintf ("%1.15f", x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%1.15f", y);
	  gaiaOutClean (buf_y);
	  buf_m = sqlite3_mprintf ("%1.15f", m);
	  gaiaOutClean (buf_m);
	  if (iv == 0)
	      buf = sqlite3_mprintf ("(%s %s %s", buf_x, buf_y, buf_m);
	  else if (iv == (ring->Points - 1))
	      buf = sqlite3_mprintf (",%s %s %s)", buf_x, buf_y, buf_m);
	  else
	      buf = sqlite3_mprintf (",%s %s %s", buf_x, buf_y, buf_m);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_m);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		buf_x = sqlite3_mprintf ("%1.15f", x);
		gaiaOutClean (buf_x);
		buf_y = sqlite3_mprintf ("%1.15f", y);
		gaiaOutClean (buf_y);
		buf_m = sqlite3_mprintf ("%1.15f", m);
		gaiaOutClean (buf_m);
		if (iv == 0)
		    buf = sqlite3_mprintf (",(%s %s %s", buf_x, buf_y, buf_m);
		else if (iv == (ring->Points - 1))
		    buf = sqlite3_mprintf (",%s %s %s)", buf_x, buf_y, buf_m);
		else
		    buf = sqlite3_mprintf (",%s %s %s", buf_x, buf_y, buf_m);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		sqlite3_free (buf_m);
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
      }
}

static void
gaiaOutEwktPolygonZM (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polyg)
{
/* formats an EWKT POLYGONZM */
    char *buf_x;
    char *buf_y;
    char *buf_z;
    char *buf_m;
    char *buf;
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    gaiaRingPtr ring = polyg->Exterior;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
	  buf_x = sqlite3_mprintf ("%1.15f", x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%1.15f", y);
	  gaiaOutClean (buf_y);
	  buf_z = sqlite3_mprintf ("%1.15f", z);
	  gaiaOutClean (buf_z);
	  buf_m = sqlite3_mprintf ("%1.15f", m);
	  gaiaOutClean (buf_m);
	  if (iv == 0)
	      buf =
		  sqlite3_mprintf ("(%s %s %s %s", buf_x, buf_y, buf_z, buf_m);
	  else if (iv == (ring->Points - 1))
	      buf =
		  sqlite3_mprintf (",%s %s %s %s)", buf_x, buf_y, buf_z, buf_m);
	  else
	      buf =
		  sqlite3_mprintf (",%s %s %s %s", buf_x, buf_y, buf_z, buf_m);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  sqlite3_free (buf_z);
	  sqlite3_free (buf_m);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		buf_x = sqlite3_mprintf ("%1.15f", x);
		gaiaOutClean (buf_x);
		buf_y = sqlite3_mprintf ("%1.15f", y);
		gaiaOutClean (buf_y);
		buf_z = sqlite3_mprintf ("%1.15f", z);
		gaiaOutClean (buf_z);
		buf_m = sqlite3_mprintf ("%1.15f", m);
		gaiaOutClean (buf_m);
		if (iv == 0)
		    buf =
			sqlite3_mprintf (",(%s %s %s %s", buf_x, buf_y, buf_z,
					 buf_m);
		else if (iv == (ring->Points - 1))
		    buf =
			sqlite3_mprintf (",%s %s %s %s)", buf_x, buf_y, buf_z,
					 buf_m);
		else
		    buf =
			sqlite3_mprintf (",%s %s %s %s", buf_x, buf_y, buf_z,
					 buf_m);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		sqlite3_free (buf_z);
		sqlite3_free (buf_m);
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
      }
}

GAIAGEO_DECLARE void
gaiaOutWktEx (gaiaOutBufferPtr out_buf, gaiaGeomCollPtr geom, int precision)
{
/* prints the WKT representation of current geometry */
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
		if (point->DimensionModel == GAIA_XY_Z)
		  {
		      /* processing POINTZ */
		      gaiaAppendToOutBuffer (out_buf, "POINT Z(");
		      gaiaOutPointZex (out_buf, point, precision);
		  }
		else if (point->DimensionModel == GAIA_XY_M)
		  {
		      /* processing POINTM */
		      gaiaAppendToOutBuffer (out_buf, "POINT M(");
		      gaiaOutPointM (out_buf, point, precision);
		  }
		else if (point->DimensionModel == GAIA_XY_Z_M)
		  {
		      /* processing POINTZM */
		      gaiaAppendToOutBuffer (out_buf, "POINT ZM(");
		      gaiaOutPointZM (out_buf, point, precision);
		  }
		else
		  {
		      /* processing POINT */
		      gaiaAppendToOutBuffer (out_buf, "POINT(");
		      gaiaOutPoint (out_buf, point, precision);
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      /* processing LINESTRINGZ */
		      gaiaAppendToOutBuffer (out_buf, "LINESTRING Z(");
		      gaiaOutLinestringZex (out_buf, line, precision);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      /* processing LINESTRINGM */
		      gaiaAppendToOutBuffer (out_buf, "LINESTRING M(");
		      gaiaOutLinestringM (out_buf, line, precision);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      /* processing LINESTRINGZM */
		      gaiaAppendToOutBuffer (out_buf, "LINESTRING ZM(");
		      gaiaOutLinestringZM (out_buf, line, precision);
		  }
		else
		  {
		      /* processing LINESTRING */
		      gaiaAppendToOutBuffer (out_buf, "LINESTRING(");
		      gaiaOutLinestring (out_buf, line, precision);
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		if (polyg->DimensionModel == GAIA_XY_Z)
		  {
		      /* processing POLYGONZ */
		      gaiaAppendToOutBuffer (out_buf, "POLYGON Z(");
		      gaiaOutPolygonZex (out_buf, polyg, precision);
		  }
		else if (polyg->DimensionModel == GAIA_XY_M)
		  {
		      /* processing POLYGONM */
		      gaiaAppendToOutBuffer (out_buf, "POLYGON M(");
		      gaiaOutPolygonM (out_buf, polyg, precision);
		  }
		else if (polyg->DimensionModel == GAIA_XY_Z_M)
		  {
		      /* processing POLYGONZM */
		      gaiaAppendToOutBuffer (out_buf, "POLYGON ZM(");
		      gaiaOutPolygonZM (out_buf, polyg, precision);
		  }
		else
		  {
		      /* processing POLYGON */
		      gaiaAppendToOutBuffer (out_buf, "POLYGON(");
		      gaiaOutPolygon (out_buf, polyg, precision);
		  }
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
		if (geom->DimensionModel == GAIA_XY_Z)
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOINT Z(");
		else if (geom->DimensionModel == GAIA_XY_M)
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOINT M(");
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOINT ZM(");
		else
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOINT(");
		point = geom->FirstPoint;
		while (point)
		  {
		      if (point->DimensionModel == GAIA_XY_Z)
			{
			    if (point != geom->FirstPoint)
				gaiaAppendToOutBuffer (out_buf, ", ");
			    gaiaOutPointZex (out_buf, point, precision);
			}
		      else if (point->DimensionModel == GAIA_XY_M)
			{
			    if (point != geom->FirstPoint)
				gaiaAppendToOutBuffer (out_buf, ", ");
			    gaiaOutPointM (out_buf, point, precision);
			}
		      else if (point->DimensionModel == GAIA_XY_Z_M)
			{
			    if (point != geom->FirstPoint)
				gaiaAppendToOutBuffer (out_buf, ", ");
			    gaiaOutPointZM (out_buf, point, precision);
			}
		      else
			{
			    if (point != geom->FirstPoint)
				gaiaAppendToOutBuffer (out_buf, ", ");
			    gaiaOutPoint (out_buf, point, precision);
			}
		      point = point->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else if (pts == 0 && lns > 0 && pgs == 0
		   && geom->DeclaredType == GAIA_MULTILINESTRING)
	    {
		/* some kind of MULTILINESTRING */
		if (geom->DimensionModel == GAIA_XY_Z)
		    gaiaAppendToOutBuffer (out_buf, "MULTILINESTRING Z(");
		else if (geom->DimensionModel == GAIA_XY_M)
		    gaiaAppendToOutBuffer (out_buf, "MULTILINESTRING M(");
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    gaiaAppendToOutBuffer (out_buf, "MULTILINESTRING ZM(");
		else
		    gaiaAppendToOutBuffer (out_buf, "MULTILINESTRING(");
		line = geom->FirstLinestring;
		while (line)
		  {
		      if (line != geom->FirstLinestring)
			  gaiaAppendToOutBuffer (out_buf, ", (");
		      else
			  gaiaAppendToOutBuffer (out_buf, "(");
		      if (line->DimensionModel == GAIA_XY_Z)
			{
			    gaiaOutLinestringZex (out_buf, line, precision);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else if (line->DimensionModel == GAIA_XY_M)
			{
			    gaiaOutLinestringM (out_buf, line, precision);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else if (line->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaOutLinestringZM (out_buf, line, precision);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else
			{
			    gaiaOutLinestring (out_buf, line, precision);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      line = line->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else if (pts == 0 && lns == 0 && pgs > 0
		   && geom->DeclaredType == GAIA_MULTIPOLYGON)
	    {
		/* some kind of MULTIPOLYGON */
		if (geom->DimensionModel == GAIA_XY_Z)
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOLYGON Z(");
		else if (geom->DimensionModel == GAIA_XY_M)
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOLYGON M(");
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOLYGON ZM(");
		else
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOLYGON(");
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      if (polyg != geom->FirstPolygon)
			  gaiaAppendToOutBuffer (out_buf, ", (");
		      else
			  gaiaAppendToOutBuffer (out_buf, "(");
		      if (polyg->DimensionModel == GAIA_XY_Z)
			{
			    gaiaOutPolygonZex (out_buf, polyg, precision);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else if (polyg->DimensionModel == GAIA_XY_M)
			{
			    gaiaOutPolygonM (out_buf, polyg, precision);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else if (polyg->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaOutPolygonZM (out_buf, polyg, precision);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else
			{
			    gaiaOutPolygon (out_buf, polyg, precision);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      polyg = polyg->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else
	    {
		/* some kind of GEOMETRYCOLLECTION */
		int ie = 0;
		if (geom->DimensionModel == GAIA_XY_Z)
		    gaiaAppendToOutBuffer (out_buf, "GEOMETRYCOLLECTION Z(");
		else if (geom->DimensionModel == GAIA_XY_M)
		    gaiaAppendToOutBuffer (out_buf, "GEOMETRYCOLLECTION M(");
		else if (geom->DimensionModel == GAIA_XY_Z_M)
		    gaiaAppendToOutBuffer (out_buf, "GEOMETRYCOLLECTION ZM(");
		else
		    gaiaAppendToOutBuffer (out_buf, "GEOMETRYCOLLECTION(");
		point = geom->FirstPoint;
		while (point)
		  {
		      /* processing POINTs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ", ");
		      ie++;
		      if (point->DimensionModel == GAIA_XY_Z)
			{
			    gaiaAppendToOutBuffer (out_buf, "POINT Z(");
			    gaiaOutPointZex (out_buf, point, precision);
			}
		      else if (point->DimensionModel == GAIA_XY_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "POINT M(");
			    gaiaOutPointM (out_buf, point, precision);
			}
		      else if (point->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "POINT ZM(");
			    gaiaOutPointZM (out_buf, point, precision);
			}
		      else
			{
			    gaiaAppendToOutBuffer (out_buf, "POINT(");
			    gaiaOutPoint (out_buf, point, precision);
			}
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
		      if (line->DimensionModel == GAIA_XY_Z)
			{
			    gaiaAppendToOutBuffer (out_buf, "LINESTRING Z(");
			    gaiaOutLinestringZex (out_buf, line, precision);
			}
		      else if (line->DimensionModel == GAIA_XY_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "LINESTRING M(");
			    gaiaOutLinestringM (out_buf, line, precision);
			}
		      else if (line->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "LINESTRING ZM(");
			    gaiaOutLinestringZM (out_buf, line, precision);
			}
		      else
			{
			    gaiaAppendToOutBuffer (out_buf, "LINESTRING(");
			    gaiaOutLinestring (out_buf, line, precision);
			}
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
		      if (polyg->DimensionModel == GAIA_XY_Z)
			{
			    gaiaAppendToOutBuffer (out_buf, "POLYGON Z(");
			    gaiaOutPolygonZex (out_buf, polyg, precision);
			}
		      else if (polyg->DimensionModel == GAIA_XY_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "POLYGON M(");
			    gaiaOutPolygonM (out_buf, polyg, precision);
			}
		      else if (polyg->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "POLYGON ZM(");
			    gaiaOutPolygonZM (out_buf, polyg, precision);
			}
		      else
			{
			    gaiaAppendToOutBuffer (out_buf, "POLYGON(");
			    gaiaOutPolygon (out_buf, polyg, precision);
			}
		      gaiaAppendToOutBuffer (out_buf, ")");
		      polyg = polyg->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
      }
}

GAIAGEO_DECLARE void
gaiaOutWkt (gaiaOutBufferPtr out_buf, gaiaGeomCollPtr geom)
{
/* 
* prints the WKT representation of current geometry 
* convenience method - default decimal precision
*/
    gaiaOutWktEx (out_buf, geom, -1);
}

GAIAGEO_DECLARE void
gaiaOutWktStrict (gaiaOutBufferPtr out_buf, gaiaGeomCollPtr geom, int precision)
{
/* 
 * prints the WKT representation of current geometry 
 * strictly conformant 2D WKT implementation
*/
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    if (precision > 18)
	precision = 18;
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
		gaiaAppendToOutBuffer (out_buf, "POINT(");
		gaiaOutPointStrict (out_buf, point, precision);
		gaiaAppendToOutBuffer (out_buf, ")");
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		/* processing LINESTRING */
		gaiaAppendToOutBuffer (out_buf, "LINESTRING(");
		gaiaOutLinestringStrict (out_buf, line, precision);
		gaiaAppendToOutBuffer (out_buf, ")");
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		/* processing POLYGON */
		gaiaAppendToOutBuffer (out_buf, "POLYGON(");
		gaiaOutPolygonStrict (out_buf, polyg, precision);
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
		gaiaAppendToOutBuffer (out_buf, "MULTIPOINT(");
		point = geom->FirstPoint;
		while (point)
		  {
		      if (point != geom->FirstPoint)
			  gaiaAppendToOutBuffer (out_buf, ",");
		      gaiaOutPointStrict (out_buf, point, precision);
		      point = point->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else if (pts == 0 && lns > 0 && pgs == 0
		   && geom->DeclaredType == GAIA_MULTILINESTRING)
	    {
		/* some kind of MULTILINESTRING */
		gaiaAppendToOutBuffer (out_buf, "MULTILINESTRING(");
		line = geom->FirstLinestring;
		while (line)
		  {
		      if (line != geom->FirstLinestring)
			  gaiaAppendToOutBuffer (out_buf, ",(");
		      else
			  gaiaAppendToOutBuffer (out_buf, "(");
		      gaiaOutLinestringStrict (out_buf, line, precision);
		      gaiaAppendToOutBuffer (out_buf, ")");
		      line = line->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else if (pts == 0 && lns == 0 && pgs > 0
		   && geom->DeclaredType == GAIA_MULTIPOLYGON)
	    {
		/* some kind of MULTIPOLYGON */
		gaiaAppendToOutBuffer (out_buf, "MULTIPOLYGON(");
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      if (polyg != geom->FirstPolygon)
			  gaiaAppendToOutBuffer (out_buf, ",(");
		      else
			  gaiaAppendToOutBuffer (out_buf, "(");
		      gaiaOutPolygonStrict (out_buf, polyg, precision);
		      gaiaAppendToOutBuffer (out_buf, ")");
		      polyg = polyg->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else
	    {
		/* some kind of GEOMETRYCOLLECTION */
		int ie = 0;
		gaiaAppendToOutBuffer (out_buf, "GEOMETRYCOLLECTION(");
		point = geom->FirstPoint;
		while (point)
		  {
		      /* processing POINTs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ",");
		      ie++;
		      gaiaAppendToOutBuffer (out_buf, "POINT(");
		      gaiaOutPointStrict (out_buf, point, precision);
		      gaiaAppendToOutBuffer (out_buf, ")");
		      point = point->Next;
		  }
		line = geom->FirstLinestring;
		while (line)
		  {
		      /* processing LINESTRINGs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ",");
		      ie++;
		      gaiaAppendToOutBuffer (out_buf, "LINESTRING(");
		      gaiaOutLinestringStrict (out_buf, line, precision);
		      gaiaAppendToOutBuffer (out_buf, ")");
		      line = line->Next;
		  }
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      /* processing POLYGONs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ",");
		      ie++;
		      gaiaAppendToOutBuffer (out_buf, "POLYGON(");
		      gaiaOutPolygonStrict (out_buf, polyg, precision);
		      gaiaAppendToOutBuffer (out_buf, ")");
		      polyg = polyg->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
      }
}

GAIAGEO_DECLARE void
gaiaToEWKT (gaiaOutBufferPtr out_buf, gaiaGeomCollPtr geom)
{
/* prints the EWKT representation of current geometry */
    char buf[128];
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    if (!geom)
	return;
    sprintf (buf, "SRID=%d;", geom->Srid);
    gaiaAppendToOutBuffer (out_buf, buf);
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
		if (point->DimensionModel == GAIA_XY_Z)
		  {
		      /* processing POINTZ */
		      gaiaAppendToOutBuffer (out_buf, "POINT(");
		      gaiaOutEwktPointZ (out_buf, point);
		  }
		else if (point->DimensionModel == GAIA_XY_M)
		  {
		      /* processing POINTM */
		      gaiaAppendToOutBuffer (out_buf, "POINTM(");
		      gaiaOutEwktPointM (out_buf, point);
		  }
		else if (point->DimensionModel == GAIA_XY_Z_M)
		  {
		      /* processing POINTZM */
		      gaiaAppendToOutBuffer (out_buf, "POINT(");
		      gaiaOutEwktPointZM (out_buf, point);
		  }
		else
		  {
		      /* processing POINT */
		      gaiaAppendToOutBuffer (out_buf, "POINT(");
		      gaiaOutEwktPoint (out_buf, point);
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      /* processing LINESTRINGZ */
		      gaiaAppendToOutBuffer (out_buf, "LINESTRING(");
		      gaiaOutEwktLinestringZ (out_buf, line);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      /* processing LINESTRINGM */
		      gaiaAppendToOutBuffer (out_buf, "LINESTRINGM(");
		      gaiaOutEwktLinestringM (out_buf, line);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      /* processing LINESTRINGZM */
		      gaiaAppendToOutBuffer (out_buf, "LINESTRING(");
		      gaiaOutEwktLinestringZM (out_buf, line);
		  }
		else
		  {
		      /* processing LINESTRING */
		      gaiaAppendToOutBuffer (out_buf, "LINESTRING(");
		      gaiaOutEwktLinestring (out_buf, line);
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		if (polyg->DimensionModel == GAIA_XY_Z)
		  {
		      /* processing POLYGONZ */
		      gaiaAppendToOutBuffer (out_buf, "POLYGON(");
		      gaiaOutEwktPolygonZ (out_buf, polyg);
		  }
		else if (polyg->DimensionModel == GAIA_XY_M)
		  {
		      /* processing POLYGONM */
		      gaiaAppendToOutBuffer (out_buf, "POLYGONM(");
		      gaiaOutEwktPolygonM (out_buf, polyg);
		  }
		else if (polyg->DimensionModel == GAIA_XY_Z_M)
		  {
		      /* processing POLYGONZM */
		      gaiaAppendToOutBuffer (out_buf, "POLYGON(");
		      gaiaOutEwktPolygonZM (out_buf, polyg);
		  }
		else
		  {
		      /* processing POLYGON */
		      gaiaAppendToOutBuffer (out_buf, "POLYGON(");
		      gaiaOutEwktPolygon (out_buf, polyg);
		  }
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
		if (geom->DimensionModel == GAIA_XY_M)
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOINTM(");
		else
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOINT(");
		point = geom->FirstPoint;
		while (point)
		  {
		      if (point->DimensionModel == GAIA_XY_Z)
			{
			    if (point != geom->FirstPoint)
				gaiaAppendToOutBuffer (out_buf, ",");
			    gaiaOutEwktPointZ (out_buf, point);
			}
		      else if (point->DimensionModel == GAIA_XY_M)
			{
			    if (point != geom->FirstPoint)
				gaiaAppendToOutBuffer (out_buf, ",");
			    gaiaOutEwktPointM (out_buf, point);
			}
		      else if (point->DimensionModel == GAIA_XY_Z_M)
			{
			    if (point != geom->FirstPoint)
				gaiaAppendToOutBuffer (out_buf, ",");
			    gaiaOutEwktPointZM (out_buf, point);
			}
		      else
			{
			    if (point != geom->FirstPoint)
				gaiaAppendToOutBuffer (out_buf, ",");
			    gaiaOutEwktPoint (out_buf, point);
			}
		      point = point->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else if (pts == 0 && lns > 0 && pgs == 0
		   && geom->DeclaredType == GAIA_MULTILINESTRING)
	    {
		/* some kind of MULTILINESTRING */
		if (geom->DimensionModel == GAIA_XY_M)
		    gaiaAppendToOutBuffer (out_buf, "MULTILINESTRINGM(");
		else
		    gaiaAppendToOutBuffer (out_buf, "MULTILINESTRING(");
		line = geom->FirstLinestring;
		while (line)
		  {
		      if (line != geom->FirstLinestring)
			  gaiaAppendToOutBuffer (out_buf, ",(");
		      else
			  gaiaAppendToOutBuffer (out_buf, "(");
		      if (line->DimensionModel == GAIA_XY_Z)
			{
			    gaiaOutEwktLinestringZ (out_buf, line);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else if (line->DimensionModel == GAIA_XY_M)
			{
			    gaiaOutEwktLinestringM (out_buf, line);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else if (line->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaOutEwktLinestringZM (out_buf, line);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else
			{
			    gaiaOutEwktLinestring (out_buf, line);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      line = line->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else if (pts == 0 && lns == 0 && pgs > 0
		   && geom->DeclaredType == GAIA_MULTIPOLYGON)
	    {
		/* some kind of MULTIPOLYGON */
		if (geom->DimensionModel == GAIA_XY_M)
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOLYGONM(");
		else
		    gaiaAppendToOutBuffer (out_buf, "MULTIPOLYGON(");
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      if (polyg != geom->FirstPolygon)
			  gaiaAppendToOutBuffer (out_buf, ",(");
		      else
			  gaiaAppendToOutBuffer (out_buf, "(");
		      if (polyg->DimensionModel == GAIA_XY_Z)
			{
			    gaiaOutEwktPolygonZ (out_buf, polyg);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else if (polyg->DimensionModel == GAIA_XY_M)
			{
			    gaiaOutEwktPolygonM (out_buf, polyg);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else if (polyg->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaOutEwktPolygonZM (out_buf, polyg);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      else
			{
			    gaiaOutEwktPolygon (out_buf, polyg);
			    gaiaAppendToOutBuffer (out_buf, ")");
			}
		      polyg = polyg->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
	  else
	    {
		/* some kind of GEOMETRYCOLLECTION */
		int ie = 0;
		if (geom->DimensionModel == GAIA_XY_M)
		    gaiaAppendToOutBuffer (out_buf, "GEOMETRYCOLLECTIONM(");
		else
		    gaiaAppendToOutBuffer (out_buf, "GEOMETRYCOLLECTION(");
		point = geom->FirstPoint;
		while (point)
		  {
		      /* processing POINTs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ",");
		      ie++;
		      if (point->DimensionModel == GAIA_XY_Z)
			{
			    gaiaAppendToOutBuffer (out_buf, "POINT(");
			    gaiaOutEwktPointZ (out_buf, point);
			}
		      else if (point->DimensionModel == GAIA_XY_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "POINTM(");
			    gaiaOutEwktPointM (out_buf, point);
			}
		      else if (point->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "POINT(");
			    gaiaOutEwktPointZM (out_buf, point);
			}
		      else
			{
			    gaiaAppendToOutBuffer (out_buf, "POINT(");
			    gaiaOutEwktPoint (out_buf, point);
			}
		      gaiaAppendToOutBuffer (out_buf, ")");
		      point = point->Next;
		  }
		line = geom->FirstLinestring;
		while (line)
		  {
		      /* processing LINESTRINGs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ",");
		      ie++;
		      if (line->DimensionModel == GAIA_XY_Z)
			{
			    gaiaAppendToOutBuffer (out_buf, "LINESTRING(");
			    gaiaOutEwktLinestringZ (out_buf, line);
			}
		      else if (line->DimensionModel == GAIA_XY_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "LINESTRINGM(");
			    gaiaOutEwktLinestringM (out_buf, line);
			}
		      else if (line->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "LINESTRING(");
			    gaiaOutEwktLinestringZM (out_buf, line);
			}
		      else
			{
			    gaiaAppendToOutBuffer (out_buf, "LINESTRING(");
			    gaiaOutEwktLinestring (out_buf, line);
			}
		      gaiaAppendToOutBuffer (out_buf, ")");
		      line = line->Next;
		  }
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      /* processing POLYGONs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ",");
		      ie++;
		      if (polyg->DimensionModel == GAIA_XY_Z)
			{
			    gaiaAppendToOutBuffer (out_buf, "POLYGON(");
			    gaiaOutEwktPolygonZ (out_buf, polyg);
			}
		      else if (polyg->DimensionModel == GAIA_XY_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "POLYGONM(");
			    gaiaOutEwktPolygonM (out_buf, polyg);
			}
		      else if (polyg->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaAppendToOutBuffer (out_buf, "POLYGON(");
			    gaiaOutEwktPolygonZM (out_buf, polyg);
			}
		      else
			{
			    gaiaAppendToOutBuffer (out_buf, "POLYGON(");
			    gaiaOutEwktPolygon (out_buf, polyg);
			}
		      gaiaAppendToOutBuffer (out_buf, ")");
		      polyg = polyg->Next;
		  }
		gaiaAppendToOutBuffer (out_buf, ")");
	    }
      }
}

/*
/
/  Gaia common support for SVG encoded geometries
/
////////////////////////////////////////////////////////////
/
/ Author: Klaus Foerster klaus.foerster@svg.cc
/ version 0.9. 2008 September 21
 /
 */

static void
SvgCoords (gaiaOutBufferPtr out_buf, gaiaPointPtr point, int precision)
{
/* formats POINT as SVG-attributes x,y */
    char *buf_x;
    char *buf_y;
    char *buf;
    buf_x = sqlite3_mprintf ("%.*f", precision, point->X);
    gaiaOutClean (buf_x);
    buf_y = sqlite3_mprintf ("%.*f", precision, point->Y * -1);
    gaiaOutClean (buf_y);
    buf = sqlite3_mprintf ("x=\"%s\" y=\"%s\"", buf_x, buf_y);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

static void
SvgCircle (gaiaOutBufferPtr out_buf, gaiaPointPtr point, int precision)
{
/* formats POINT as SVG-attributes cx,cy */
    char *buf_x;
    char *buf_y;
    char *buf;
    buf_x = sqlite3_mprintf ("%.*f", precision, point->X);
    gaiaOutClean (buf_x);
    buf_y = sqlite3_mprintf ("%.*f", precision, point->Y * -1);
    gaiaOutClean (buf_y);
    buf = sqlite3_mprintf ("cx=\"%s\" cy=\"%s\"", buf_x, buf_y);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
}

static void
SvgPathRelative (gaiaOutBufferPtr out_buf, int dims, int points, double *coords,
		 int precision, int closePath)
{
/* formats LINESTRING as SVG-path d-attribute with relative coordinate moves */
    char *buf_x;
    char *buf_y;
    char *buf;
    double x;
    double y;
    double z;
    double m;
    double lastX = 0.0;
    double lastY = 0.0;
    int iv;
    for (iv = 0; iv < points; iv++)
      {
	  if (dims == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (coords, iv, &x, &y, &z);
	    }
	  else if (dims == GAIA_XY_M)
	    {
		gaiaGetPointXYM (coords, iv, &x, &y, &m);
	    }
	  else if (dims == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (coords, iv, &x, &y, &z, &m);
	    }
	  else
	    {
		gaiaGetPoint (coords, iv, &x, &y);
	    }
	  buf_x = sqlite3_mprintf ("%.*f", precision, x - lastX);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%.*f", precision, (y - lastY) * -1);
	  gaiaOutClean (buf_y);
	  if (iv == 0)
	      buf = sqlite3_mprintf ("M %s %s l ", buf_x, buf_y);
	  else
	      buf = sqlite3_mprintf ("%s %s ", buf_x, buf_y);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  lastX = x;
	  lastY = y;
	  if (iv == points - 1 && closePath == 1)
	      gaiaAppendToOutBuffer (out_buf, "z ");
	  else
	      gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

static void
SvgPathAbsolute (gaiaOutBufferPtr out_buf, int dims, int points, double *coords,
		 int precision, int closePath)
{
/* formats LINESTRING as SVG-path d-attribute with relative coordinate moves */
    char *buf_x;
    char *buf_y;
    char *buf;
    double x;
    double y;
    double z;
    double m;
    int iv;
    for (iv = 0; iv < points; iv++)
      {
	  if (dims == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (coords, iv, &x, &y, &z);
	    }
	  else if (dims == GAIA_XY_M)
	    {
		gaiaGetPointXYM (coords, iv, &x, &y, &m);
	    }
	  else if (dims == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (coords, iv, &x, &y, &z, &m);
	    }
	  else
	    {
		gaiaGetPoint (coords, iv, &x, &y);
	    }
	  buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%.*f", precision, y * -1);
	  gaiaOutClean (buf_y);
	  if (iv == 0)
	      buf = sqlite3_mprintf ("M %s %s L ", buf_x, buf_y);
	  else
	      buf = sqlite3_mprintf ("%s %s ", buf_x, buf_y);
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  if (iv == points - 1 && closePath == 1)
	      gaiaAppendToOutBuffer (out_buf, "z ");
	  else
	      gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
}

GAIAGEO_DECLARE void
gaiaOutSvg (gaiaOutBufferPtr out_buf, gaiaGeomCollPtr geom, int relative,
	    int precision)
{
/* prints the SVG representation of current geometry */
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    int ib;
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    gaiaRingPtr ring;
    if (precision > 18)
	precision = 18;
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

    if ((pts + lns + pgs) == 1)
      {
	  /* we have only one elementary geometry */
	  point = geom->FirstPoint;
	  while (point)
	    {
		/* processing POINT */
		if (relative == 1)
		    SvgCoords (out_buf, point, precision);
		else
		    SvgCircle (out_buf, point, precision);
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		/* processing LINESTRING */
		if (relative == 1)
		    SvgPathRelative (out_buf, line->DimensionModel,
				     line->Points, line->Coords, precision, 0);
		else
		    SvgPathAbsolute (out_buf, line->DimensionModel,
				     line->Points, line->Coords, precision, 0);
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		/* process exterior and interior rings */
		ring = polyg->Exterior;
		if (relative == 1)
		  {
		      SvgPathRelative (out_buf, ring->DimensionModel,
				       ring->Points, ring->Coords, precision,
				       1);
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    ring = polyg->Interiors + ib;
			    SvgPathRelative (out_buf, ring->DimensionModel,
					     ring->Points, ring->Coords,
					     precision, 1);
			}
		  }
		else
		  {
		      SvgPathAbsolute (out_buf, ring->DimensionModel,
				       ring->Points, ring->Coords, precision,
				       1);
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    ring = polyg->Interiors + ib;
			    SvgPathAbsolute (out_buf, ring->DimensionModel,
					     ring->Points, ring->Coords,
					     precision, 1);
			}
		  }
		polyg = polyg->Next;
	    }
      }
    else
      {
	  /* we have some kind of complex geometry */
	  if (pts > 0 && lns == 0 && pgs == 0)
	    {
		/* this one is a MULTIPOINT */
		point = geom->FirstPoint;
		while (point)
		  {
		      /* processing POINTs */
		      if (point != geom->FirstPoint)
			  gaiaAppendToOutBuffer (out_buf, ",");
		      if (relative == 1)
			  SvgCoords (out_buf, point, precision);
		      else
			  SvgCircle (out_buf, point, precision);
		      point = point->Next;
		  }
	    }
	  else if (pts == 0 && lns > 0 && pgs == 0)
	    {
		/* this one is a MULTILINESTRING */
		line = geom->FirstLinestring;
		while (line)
		  {
		      /* processing LINESTRINGs */
		      if (relative == 1)
			  SvgPathRelative (out_buf, line->DimensionModel,
					   line->Points, line->Coords,
					   precision, 0);
		      else
			  SvgPathAbsolute (out_buf, line->DimensionModel,
					   line->Points, line->Coords,
					   precision, 0);
		      line = line->Next;
		  }
	    }
	  else if (pts == 0 && lns == 0 && pgs > 0)
	    {
		/* this one is a MULTIPOLYGON */
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      /* processing POLYGONs */
		      ring = polyg->Exterior;
		      if (relative == 1)
			{
			    SvgPathRelative (out_buf, ring->DimensionModel,
					     ring->Points, ring->Coords,
					     precision, 1);
			    for (ib = 0; ib < polyg->NumInteriors; ib++)
			      {
				  ring = polyg->Interiors + ib;
				  SvgPathRelative (out_buf,
						   ring->DimensionModel,
						   ring->Points, ring->Coords,
						   precision, 1);
			      }
			}
		      else
			{
			    SvgPathAbsolute (out_buf, ring->DimensionModel,
					     ring->Points, ring->Coords,
					     precision, 1);
			    for (ib = 0; ib < polyg->NumInteriors; ib++)
			      {
				  ring = polyg->Interiors + ib;
				  SvgPathAbsolute (out_buf,
						   ring->DimensionModel,
						   ring->Points, ring->Coords,
						   precision, 1);
			      }
			}
		      polyg = polyg->Next;
		  }
	    }
	  else
	    {
		/* this one is a GEOMETRYCOLLECTION */
		int ie = 0;
		point = geom->FirstPoint;
		while (point)
		  {
		      /* processing POINTs */
		      if (ie > 0)
			{
			    gaiaAppendToOutBuffer (out_buf, ";");
			}
		      ie++;
		      if (relative == 1)
			  SvgCoords (out_buf, point, precision);
		      else
			  SvgCircle (out_buf, point, precision);
		      point = point->Next;
		  }
		line = geom->FirstLinestring;
		while (line)
		  {
		      /* processing LINESTRINGs */
		      if (ie > 0)
			  gaiaAppendToOutBuffer (out_buf, ";");
		      ie++;
		      if (relative == 1)
			  SvgPathRelative (out_buf, line->DimensionModel,
					   line->Points, line->Coords,
					   precision, 0);
		      else
			  SvgPathAbsolute (out_buf, line->DimensionModel,
					   line->Points, line->Coords,
					   precision, 0);
		      line = line->Next;
		  }
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      /* processing POLYGONs */
		      ie++;
		      /* process exterior and interior rings */
		      ring = polyg->Exterior;
		      if (relative == 1)
			{
			    SvgPathRelative (out_buf, ring->DimensionModel,
					     ring->Points, ring->Coords,
					     precision, 1);
			    for (ib = 0; ib < polyg->NumInteriors; ib++)
			      {
				  ring = polyg->Interiors + ib;
				  SvgPathRelative (out_buf,
						   ring->DimensionModel,
						   ring->Points, ring->Coords,
						   precision, 1);
			      }
			}
		      else
			{
			    SvgPathAbsolute (out_buf, ring->DimensionModel,
					     ring->Points, ring->Coords,
					     precision, 1);
			    for (ib = 0; ib < polyg->NumInteriors; ib++)
			      {
				  ring = polyg->Interiors + ib;
				  SvgPathAbsolute (out_buf,
						   ring->DimensionModel,
						   ring->Points, ring->Coords,
						   precision, 1);
			      }
			}
		      polyg = polyg->Next;
		  }
	    }
      }

    if (out_buf->Error == 0 && out_buf->WriteOffset > 0)
      {
	  /* sandro 2012-02-23 cleaning extra trailing spaces */
	  int i;
	  for (i = out_buf->WriteOffset - 1; i >= 0; i--)
	    {
		if (*(out_buf->Buffer + i) == ' ')
		  {
		      *(out_buf->Buffer + i) = '\0';
		      out_buf->WriteOffset -= 1;
		  }
		else
		    break;
	    }
      }
}

/* END of Klaus Foerster SVG implementation */


static char *
XmlClean (const char *string)
{
/* well formatting a text string for XML */
    int ind;
    char *clean;
    char *p_out;
    int len = strlen (string);
    clean = malloc (len * 3);
    if (!clean)
	return NULL;
    p_out = clean;
    for (ind = 0; ind < len; ind++)
      {
	  /* masking XML special chars */
	  switch (string[ind])
	    {
	    case '&':
		*p_out++ = '&';
		*p_out++ = 'a';
		*p_out++ = 'm';
		*p_out++ = 'p';
		*p_out++ = ';';
		break;
	    case '<':
		*p_out++ = '&';
		*p_out++ = 'l';
		*p_out++ = 't';
		*p_out++ = ';';
		break;
	    case '>':
		*p_out++ = '&';
		*p_out++ = 'g';
		*p_out++ = 't';
		*p_out++ = ';';
		break;
	    case '"':
		*p_out++ = '&';
		*p_out++ = 'q';
		*p_out++ = 'u';
		*p_out++ = 'o';
		*p_out++ = 't';
		*p_out++ = ';';
		break;
	    default:
		*p_out++ = string[ind];
		break;
	    };
      }
    *p_out = '\0';
    return clean;
}

static void
out_kml_point (gaiaOutBufferPtr out_buf, gaiaPointPtr point, int precision)
{
/* formats POINT as KML [x,y] */
    char *buf_x = NULL;
    char *buf_y = NULL;
    char *buf_z = NULL;
    char *buf = NULL;
    buf_x = sqlite3_mprintf ("%.*f", precision, point->X);
    gaiaOutClean (buf_x);
    buf_y = sqlite3_mprintf ("%.*f", precision, point->Y);
    gaiaOutClean (buf_y);
    if (point->DimensionModel == GAIA_XY_Z
	|| point->DimensionModel == GAIA_XY_Z_M)
      {
	  buf_z = sqlite3_mprintf ("%.*f", precision, point->Z);
	  gaiaOutClean (buf_z);
      }
    gaiaAppendToOutBuffer (out_buf, "<Point><coordinates>");
    if (point->DimensionModel == GAIA_XY_Z
	|| point->DimensionModel == GAIA_XY_Z_M)
      {
	  buf = sqlite3_mprintf ("%s,%s,%s", buf_x, buf_y, buf_z);
	  sqlite3_free (buf_z);
      }
    else
	buf = sqlite3_mprintf ("%s,%s", buf_x, buf_y);
    sqlite3_free (buf_x);
    sqlite3_free (buf_y);
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
    gaiaAppendToOutBuffer (out_buf, "</coordinates></Point>");
}

static void
out_kml_linestring (gaiaOutBuffer * out_buf, int dims, int points,
		    double *coords, int precision)
{
/* formats LINESTRING as KML [x,y] */
    char *buf_x = NULL;
    char *buf_y = NULL;
    char *buf_z = NULL;
    char *buf = NULL;
    int iv;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double m = 0.0;
    gaiaAppendToOutBuffer (out_buf, "<LineString><coordinates>");
    for (iv = 0; iv < points; iv++)
      {
	  /* exporting vertices */
	  if (dims == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (coords, iv, &x, &y, &z);
	    }
	  else if (dims == GAIA_XY_M)
	    {
		gaiaGetPointXYM (coords, iv, &x, &y, &m);
	    }
	  else if (dims == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (coords, iv, &x, &y, &z, &m);
	    }
	  else
	    {
		gaiaGetPoint (coords, iv, &x, &y);
	    }
	  buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (dims == GAIA_XY_Z || dims == GAIA_XY_Z_M)
	    {
		buf_z = sqlite3_mprintf ("%.*f", precision, z);
		gaiaOutClean (buf_z);
		if (iv == 0)
		    buf = sqlite3_mprintf ("%s,%s,%s", buf_x, buf_y, buf_z);
		else
		    buf = sqlite3_mprintf (" %s,%s,%s", buf_x, buf_y, buf_z);
		sqlite3_free (buf_z);
	    }
	  else
	    {
		if (iv == 0)
		    buf = sqlite3_mprintf ("%s,%s", buf_x, buf_y);
		else
		    buf = sqlite3_mprintf (" %s,%s", buf_x, buf_y);
	    }
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    gaiaAppendToOutBuffer (out_buf, "</coordinates></LineString>");
}

static void
out_kml_polygon (gaiaOutBufferPtr out_buf, gaiaPolygonPtr polygon,
		 int precision)
{
/* formats POLYGON as KML [x,y] */
    char *buf_x = NULL;
    char *buf_y = NULL;
    char *buf_z = NULL;
    char *buf = NULL;
    gaiaRingPtr ring;
    int iv;
    int ib;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double m = 0.0;
    gaiaAppendToOutBuffer (out_buf, "<Polygon>");
    gaiaAppendToOutBuffer (out_buf,
			   "<outerBoundaryIs><LinearRing><coordinates>");
    ring = polygon->Exterior;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  /* exporting vertices [Exterior Ring] */
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
	  buf_x = sqlite3_mprintf ("%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%.*f", precision, y);
	  gaiaOutClean (buf_y);
	  if (ring->DimensionModel == GAIA_XY_Z
	      || ring->DimensionModel == GAIA_XY_Z_M)
	    {
		buf_z = sqlite3_mprintf ("%.*f", precision, z);
		gaiaOutClean (buf_z);
		if (iv == 0)
		    buf = sqlite3_mprintf ("%s,%s,%s", buf_x, buf_y, buf_z);
		else
		    buf = sqlite3_mprintf (" %s,%s,%s", buf_x, buf_y, buf_z);
		sqlite3_free (buf_z);
	    }
	  else
	    {
		if (iv == 0)
		    buf = sqlite3_mprintf ("%s,%s", buf_x, buf_y);
		else
		    buf = sqlite3_mprintf (" %s,%s", buf_x, buf_y);
	    }
	  sqlite3_free (buf_x);
	  sqlite3_free (buf_y);
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
      }
    gaiaAppendToOutBuffer (out_buf,
			   "</coordinates></LinearRing></outerBoundaryIs>");
    for (ib = 0; ib < polygon->NumInteriors; ib++)
      {
	  /* interior rings */
	  ring = polygon->Interiors + ib;
	  gaiaAppendToOutBuffer (out_buf,
				 "<innerBoundaryIs><LinearRing><coordinates>");
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* exporting vertices [Interior Ring] */
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
		buf_x = sqlite3_mprintf ("%.*f", precision, x);
		gaiaOutClean (buf_x);
		buf_y = sqlite3_mprintf ("%.*f", precision, y);
		gaiaOutClean (buf_y);
		if (ring->DimensionModel == GAIA_XY_Z
		    || ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      buf_z = sqlite3_mprintf ("%.*f", precision, z);
		      gaiaOutClean (buf_z);
		      if (iv == 0)
			  buf =
			      sqlite3_mprintf ("%s,%s,%s", buf_x, buf_y, buf_z);
		      else
			  buf =
			      sqlite3_mprintf (" %s,%s,%s", buf_x, buf_y,
					       buf_z);
		      sqlite3_free (buf_z);
		  }
		else
		  {
		      if (iv == 0)
			  buf = sqlite3_mprintf ("%s,%s", buf_x, buf_y);
		      else
			  buf = sqlite3_mprintf (" %s,%s", buf_x, buf_y);
		  }
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
	  gaiaAppendToOutBuffer (out_buf,
				 "</coordinates></LinearRing></innerBoundaryIs>");
      }
    gaiaAppendToOutBuffer (out_buf, "</Polygon>");
}

GAIAGEO_DECLARE void
gaiaOutFullKml (gaiaOutBufferPtr out_buf, const char *name, const char *desc,
		gaiaGeomCollPtr geom, int precision)
{
/* prints the 'full' KML representation of current geometry */
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    int count = 0;
    char *xml_clean;
    if (!geom)
	return;
    if (precision > 18)
	precision = 18;

/* counting how many elementary geometries are there */
    point = geom->FirstPoint;
    while (point)
      {
	  count++;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  count++;
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  count++;
	  polyg = polyg->Next;
      }
    if (count == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT ||
	      geom->DeclaredType == GAIA_MULTILINESTRING ||
	      geom->DeclaredType == GAIA_MULTIPOLYGON ||
	      geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      count = 2;
      }

    gaiaAppendToOutBuffer (out_buf, "<Placemark><name>");
    xml_clean = XmlClean (name);
    if (xml_clean)
      {
	  gaiaAppendToOutBuffer (out_buf, xml_clean);
	  free (xml_clean);
      }
    else
	gaiaAppendToOutBuffer (out_buf, " ");
    gaiaAppendToOutBuffer (out_buf, "</name><description>");
    xml_clean = XmlClean (desc);
    if (xml_clean)
      {
	  gaiaAppendToOutBuffer (out_buf, xml_clean);
	  free (xml_clean);
      }
    else
	gaiaAppendToOutBuffer (out_buf, " ");
    gaiaAppendToOutBuffer (out_buf, "</description>");

    if (count > 1)
      {
	  /* MultiGeometry start */
	  gaiaAppendToOutBuffer (out_buf, "<MultiGeometry>");
      }

    point = geom->FirstPoint;
    while (point)
      {
	  /* processing POINT */
	  out_kml_point (out_buf, point, precision);
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* processing LINESTRING */
	  out_kml_linestring (out_buf, line->DimensionModel,
			      line->Points, line->Coords, precision);
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* processing POLYGON */
	  out_kml_polygon (out_buf, polyg, precision);
	  polyg = polyg->Next;
      }

    if (count > 1)
      {
	  /* MultiGeometry end */
	  gaiaAppendToOutBuffer (out_buf, "</MultiGeometry>");
      }
    gaiaAppendToOutBuffer (out_buf, "</Placemark>");
}

GAIAGEO_DECLARE void
gaiaOutBareKml (gaiaOutBufferPtr out_buf, gaiaGeomCollPtr geom, int precision)
{
/* prints the 'bare' KML representation of current geometry */
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    int count = 0;
    if (!geom)
	return;
    if (precision > 18)
	precision = 18;

/* counting how many elementary geometries are there */
    point = geom->FirstPoint;
    while (point)
      {
	  count++;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  count++;
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  count++;
	  polyg = polyg->Next;
      }
    if (count == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT ||
	      geom->DeclaredType == GAIA_MULTILINESTRING ||
	      geom->DeclaredType == GAIA_MULTIPOLYGON ||
	      geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      count = 2;
      }

    if (count > 1)
      {
	  /* MultiGeometry start */
	  gaiaAppendToOutBuffer (out_buf, "<MultiGeometry>");
      }

    point = geom->FirstPoint;
    while (point)
      {
	  /* processing POINT */
	  out_kml_point (out_buf, point, precision);
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* processing LINESTRING */
	  out_kml_linestring (out_buf, line->DimensionModel, line->Points,
			      line->Coords, precision);
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* processing POLYGON */
	  out_kml_polygon (out_buf, polyg, precision);
	  polyg = polyg->Next;
      }

    if (count > 1)
      {
	  /* MultiGeometry end */
	  gaiaAppendToOutBuffer (out_buf, "</MultiGeometry>");
      }
}

GAIAGEO_DECLARE void
gaiaOutGml (gaiaOutBufferPtr out_buf, int version, int precision,
	    gaiaGeomCollPtr geom)
{
/*
/ prints the GML representation of current geometry
/ *result* returns the encoded GML or NULL if any error is encountered
*/
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    gaiaRingPtr ring;
    int iv;
    int ib;
    double x;
    double y;
    double z;
    double m;
    int has_z;
    int is_multi = 1;
    int is_coll = 0;
    char buf[2048];
    char *xbuf;
    char *buf_x;
    char *buf_y;
    char *buf_z;
    if (!geom)
	return;
    if (precision > 18)
	precision = 18;

    switch (geom->DeclaredType)
      {
      case GAIA_POINT:
      case GAIA_LINESTRING:
      case GAIA_POLYGON:
	  *buf = '\0';
	  is_multi = 0;
	  break;
      case GAIA_MULTIPOINT:
	  if (geom->Srid <= 0)
	      strcpy (buf, "<gml:MultiPoint>");
	  else
	      sprintf (buf, "<gml:MultiPoint srsName=\"EPSG:%d\">", geom->Srid);
	  break;
      case GAIA_MULTILINESTRING:
	  if (version == 3)
	    {
		if (geom->Srid <= 0)
		    strcpy (buf, "<gml:MultiCurve>");
		else
		    sprintf (buf, "<gml:MultiCurve srsName=\"EPSG:%d\">",
			     geom->Srid);
	    }
	  else
	    {
		if (geom->Srid <= 0)
		    strcpy (buf, "<gml:MultiLineString>");
		else
		    sprintf (buf,
			     "<gml:MultiLineString srsName=\"EPSG:%d\">",
			     geom->Srid);
	    }
	  break;
      case GAIA_MULTIPOLYGON:
	  if (version == 3)
	    {
		if (geom->Srid <= 0)
		    strcpy (buf, "<gml:MultiSurface>");
		else
		    sprintf (buf, "<gml:MultiSurface srsName=\"EPSG:%d\">",
			     geom->Srid);
	    }
	  else
	    {
		if (geom->Srid <= 0)
		    strcpy (buf, "<gml:MultiPolygon>");
		else
		    sprintf (buf, "<gml:MultiPolygon srsName=\"EPSG:%d\">",
			     geom->Srid);
	    }
	  break;
      default:
	  if (geom->Srid <= 0)
	      strcpy (buf, "<gml:MultiGeometry>");
	  else
	      sprintf (buf, "<gml:MultiGeometry srsName=\"EPSG:%d\">",
		       geom->Srid);
	  is_coll = 1;
	  break;
      };
    gaiaAppendToOutBuffer (out_buf, buf);
    point = geom->FirstPoint;
    while (point)
      {
	  /* processing POINT */
	  if (is_multi)
	    {
		if (is_coll)
		    strcpy (buf, "<gml:geometryMember>");
		else
		    strcpy (buf, "<gml:pointMember>");
		strcat (buf, "<gml:Point>");
	    }
	  else
	    {
		if (geom->Srid <= 0)
		    strcpy (buf, "<gml:Point>");
		else
		    sprintf (buf, "<gml:Point srsName=\"EPSG:%d\">",
			     geom->Srid);
	    }
	  if (version == 3)
	    {
		if (point->DimensionModel == GAIA_XY_Z
		    || point->DimensionModel == GAIA_XY_Z_M)
		    strcat (buf, "<gml:pos srsDimension=\"3\">");
		else
		    strcat (buf, "<gml:pos srsDimension=\"2\">");
	    }
	  else
	      strcat (buf, "<gml:coordinates>");
	  gaiaAppendToOutBuffer (out_buf, buf);
	  buf_x = sqlite3_mprintf ("%.*f", precision, point->X);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%.*f", precision, point->Y);
	  gaiaOutClean (buf_y);
	  if (point->DimensionModel == GAIA_XY_Z
	      || point->DimensionModel == GAIA_XY_Z_M)
	    {
		buf_z = sqlite3_mprintf ("%.*f", precision, point->Z);
		gaiaOutClean (buf_z);
		if (version == 3)
		  {
		      xbuf = sqlite3_mprintf ("%s %s %s", buf_x, buf_y, buf_z);
		      sqlite3_free (buf_x);
		      sqlite3_free (buf_y);
		      sqlite3_free (buf_z);
		  }
		else
		  {
		      xbuf = sqlite3_mprintf ("%s,%s,%s", buf_x, buf_y, buf_z);
		      sqlite3_free (buf_x);
		      sqlite3_free (buf_y);
		      sqlite3_free (buf_z);
		  }
	    }
	  else
	    {
		if (version == 3)
		  {
		      xbuf = sqlite3_mprintf ("%s %s", buf_x, buf_y);
		      sqlite3_free (buf_x);
		      sqlite3_free (buf_y);
		  }
		else
		  {
		      xbuf = sqlite3_mprintf ("%s,%s", buf_x, buf_y);
		      sqlite3_free (buf_x);
		      sqlite3_free (buf_y);
		  }
	    }
	  gaiaAppendToOutBuffer (out_buf, xbuf);
	  sqlite3_free (xbuf);
	  if (version == 3)
	      strcpy (buf, "</gml:pos>");
	  else
	      strcpy (buf, "</gml:coordinates>");
	  if (is_multi)
	    {
		strcat (buf, "</gml:Point>");
		if (is_coll)
		    strcat (buf, "</gml:geometryMember>");
		else
		    strcat (buf, "</gml:pointMember>");
	    }
	  else
	      strcat (buf, "</gml:Point>");
	  gaiaAppendToOutBuffer (out_buf, buf);
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* processing LINESTRING */
	  if (is_multi)
	    {
		if (version == 3)
		  {
		      if (is_coll)
			  strcpy (buf, "<gml:geometryMember>");
		      else
			  strcpy (buf, "<gml:curveMember>");
		      strcat (buf, "<gml:Curve>");
		      strcat (buf, "<gml:segments>");
		      strcat (buf, "<gml:LineStringSegment>");
		      if (line->DimensionModel == GAIA_XY_Z
			  || line->DimensionModel == GAIA_XY_Z_M)
			  strcat (buf, "<gml:posList srsDimension=\"3\">");
		      else
			  strcat (buf, "<gml:posList srsDimension=\"2\">");
		  }
		else
		  {
		      if (is_coll)
			  strcpy (buf, "<gml:geometryMember>");
		      else
			  strcpy (buf, "<gml:lineStringMember>");
		      strcat (buf, "<gml:LineString>");
		      strcat (buf, "<gml:coordinates>");
		  }
	    }
	  else
	    {
		if (version == 3)
		  {
		      if (geom->Srid <= 0)
			  strcpy (buf, "<gml:Curve>");
		      else
			  sprintf (buf, "<gml:Curve srsName=\"EPSG:%d\">",
				   geom->Srid);
		      strcat (buf, "<gml:segments>");
		      strcat (buf, "<gml:LineStringSegment>");
		      if (line->DimensionModel == GAIA_XY_Z
			  || line->DimensionModel == GAIA_XY_Z_M)
			  strcat (buf, "<gml:posList srsDimension=\"3\">");
		      else
			  strcat (buf, "<gml:posList srsDimension=\"2\">");
		  }
		else
		  {
		      if (geom->Srid <= 0)
			  strcpy (buf, "<gml:LineString>");
		      else
			  sprintf (buf, "<gml:LineString srsName=\"EPSG:%d\">",
				   geom->Srid);
		      strcat (buf, "<gml:coordinates>");
		  }
	    }
	  gaiaAppendToOutBuffer (out_buf, buf);
	  for (iv = 0; iv < line->Points; iv++)
	    {
		/* exporting vertices */
		has_z = 0;
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      has_z = 1;
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      has_z = 1;
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		if (iv == 0)
		    *buf = '\0';
		else
		    strcpy (buf, " ");
		if (has_z)
		  {
		      buf_x = sqlite3_mprintf ("%.*f", precision, x);
		      gaiaOutClean (buf_x);
		      buf_y = sqlite3_mprintf ("%.*f", precision, y);
		      gaiaOutClean (buf_y);
		      buf_z = sqlite3_mprintf ("%.*f", precision, z);
		      gaiaOutClean (buf_z);
		      if (version == 3)
			{
			    xbuf =
				sqlite3_mprintf ("%s%s %s %s", buf, buf_x,
						 buf_y, buf_z);
			    sqlite3_free (buf_x);
			    sqlite3_free (buf_y);
			    sqlite3_free (buf_z);
			}
		      else
			{
			    xbuf =
				sqlite3_mprintf ("%s%s,%s,%s", buf, buf_x,
						 buf_y, buf_z);
			    sqlite3_free (buf_x);
			    sqlite3_free (buf_y);
			    sqlite3_free (buf_z);
			}
		  }
		else
		  {
		      buf_x = sqlite3_mprintf ("%.*f", precision, x);
		      gaiaOutClean (buf_x);
		      buf_y = sqlite3_mprintf ("%.*f", precision, y);
		      gaiaOutClean (buf_y);
		      if (version == 3)
			{
			    xbuf =
				sqlite3_mprintf ("%s%s %s", buf, buf_x, buf_y);
			    sqlite3_free (buf_x);
			    sqlite3_free (buf_y);
			}
		      else
			{
			    xbuf =
				sqlite3_mprintf ("%s%s,%s", buf, buf_x, buf_y);
			    sqlite3_free (buf_x);
			    sqlite3_free (buf_y);
			}
		  }
		gaiaAppendToOutBuffer (out_buf, xbuf);
		sqlite3_free (xbuf);
	    }
	  if (is_multi)
	    {
		if (version == 3)
		  {
		      strcpy (buf, "</gml:posList>");
		      strcat (buf, "</gml:LineStringSegment>");
		      strcat (buf, "</gml:segments>");
		      strcat (buf, "</gml:Curve>");
		      if (is_coll)
			  strcat (buf, "</gml:geometryMember>");
		      else
			  strcat (buf, "</gml:curveMember>");
		  }
		else
		  {
		      strcpy (buf, "</gml:coordinates>");
		      strcat (buf, "</gml:LineString>");
		      if (is_coll)
			  strcat (buf, "</gml:geometryMember>");
		      else
			  strcat (buf, "</gml:lineStringMember>");
		  }
	    }
	  else
	    {
		if (version == 3)
		  {
		      strcpy (buf, "</gml:posList>");
		      strcat (buf, "</gml:LineStringSegment>");
		      strcat (buf, "</gml:segments>");
		      strcat (buf, "</gml:Curve>");
		  }
		else
		  {
		      strcpy (buf, "</gml:coordinates>");
		      strcat (buf, "</gml:LineString>");
		  }
	    }
	  gaiaAppendToOutBuffer (out_buf, buf);
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* processing POLYGON */
	  ring = polyg->Exterior;
	  if (is_multi)
	    {
		if (version == 3)
		  {
		      if (is_coll)
			  strcpy (buf, "<gml:geometryMember>");
		      else
			  strcpy (buf, "<gml:surfaceMember>");
		      strcat (buf, "<gml:Polygon>");
		      strcat (buf, "<gml:exterior>");
		      strcat (buf, "<gml:LinearRing>");
		      if (ring->DimensionModel == GAIA_XY_Z
			  || ring->DimensionModel == GAIA_XY_Z_M)
			  strcat (buf, "<gml:posList srsDimension=\"3\">");
		      else
			  strcat (buf, "<gml:posList srsDimension=\"2\">");
		  }
		else
		  {
		      if (is_coll)
			  strcpy (buf, "<gml:geometryMember>");
		      else
			  strcpy (buf, "<gml:polygonMember>");
		      strcat (buf, "<gml:Polygon>");
		      strcat (buf, "<gml:outerBoundaryIs>");
		      strcat (buf, "<gml:LinearRing>");
		      strcat (buf, "<gml:coordinates>");
		  }
	    }
	  else
	    {
		if (geom->Srid <= 0)
		    strcpy (buf, "<gml:Polygon>");
		else
		    sprintf (buf, "<gml:Polygon srsName=\"EPSG:%d\">",
			     geom->Srid);
		if (version == 3)
		  {
		      strcat (buf, "<gml:exterior>");
		      strcat (buf, "<gml:LinearRing>");
		      if (ring->DimensionModel == GAIA_XY_Z
			  || ring->DimensionModel == GAIA_XY_Z_M)
			  strcat (buf, "<gml:posList srsDimension=\"3\">");
		      else
			  strcat (buf, "<gml:posList srsDimension=\"2\">");
		  }
		else
		  {
		      strcat (buf, "<gml:outerBoundaryIs>");
		      strcat (buf, "<gml:LinearRing>");
		      strcat (buf, "<gml:coordinates>");
		  }
	    }
	  gaiaAppendToOutBuffer (out_buf, buf);
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* exporting vertices [Interior Ring] */
		has_z = 0;
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      has_z = 1;
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      has_z = 1;
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		  }
		if (iv == 0)
		    *buf = '\0';
		else
		    strcpy (buf, " ");
		if (has_z)
		  {
		      buf_x = sqlite3_mprintf ("%.*f", precision, x);
		      gaiaOutClean (buf_x);
		      buf_y = sqlite3_mprintf ("%.*f", precision, y);
		      gaiaOutClean (buf_y);
		      buf_z = sqlite3_mprintf ("%.*f", precision, z);
		      gaiaOutClean (buf_z);
		      if (version == 3)
			{
			    xbuf =
				sqlite3_mprintf ("%s%s %s %s", buf, buf_x,
						 buf_y, buf_z);
			    sqlite3_free (buf_x);
			    sqlite3_free (buf_y);
			    sqlite3_free (buf_z);
			}
		      else
			{
			    xbuf =
				sqlite3_mprintf ("%s%s,%s,%s", buf, buf_x,
						 buf_y, buf_z);
			    sqlite3_free (buf_x);
			    sqlite3_free (buf_y);
			    sqlite3_free (buf_z);
			}
		  }
		else
		  {
		      buf_x = sqlite3_mprintf ("%.*f", precision, x);
		      gaiaOutClean (buf_x);
		      buf_y = sqlite3_mprintf ("%.*f", precision, y);
		      gaiaOutClean (buf_y);
		      if (version == 3)
			{
			    xbuf =
				sqlite3_mprintf ("%s%s %s", buf, buf_x, buf_y);
			    sqlite3_free (buf_x);
			    sqlite3_free (buf_y);
			}
		      else
			{
			    xbuf =
				sqlite3_mprintf ("%s%s,%s", buf, buf_x, buf_y);
			    sqlite3_free (buf_x);
			    sqlite3_free (buf_y);
			}
		  }
		gaiaAppendToOutBuffer (out_buf, xbuf);
		sqlite3_free (xbuf);
	    }
	  /* closing the Exterior Ring */
	  if (version == 3)
	    {
		strcpy (buf, "</gml:posList>");
		strcat (buf, "</gml:LinearRing>");
		strcat (buf, "</gml:exterior>");
	    }
	  else
	    {
		strcpy (buf, "</gml:coordinates>");
		strcat (buf, "</gml:LinearRing>");
		strcat (buf, "</gml:outerBoundaryIs>");
	    }
	  gaiaAppendToOutBuffer (out_buf, buf);
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* interior rings */
		ring = polyg->Interiors + ib;
		if (version == 3)
		  {
		      strcpy (buf, "<gml:interior>");
		      strcat (buf, "<gml:LinearRing>");
		      if (ring->DimensionModel == GAIA_XY_Z
			  || ring->DimensionModel == GAIA_XY_Z_M)
			  strcat (buf, "<gml:posList srsDimension=\"3\">");
		      else
			  strcat (buf, "<gml:posList srsDimension=\"2\">");
		  }
		else
		  {
		      strcpy (buf, "<gml:innerBoundaryIs>");
		      strcat (buf, "<gml:LinearRing>");
		      strcat (buf, "<gml:coordinates>");
		  }
		gaiaAppendToOutBuffer (out_buf, buf);
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      /* exporting vertices [Interior Ring] */
		      has_z = 0;
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    has_z = 1;
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    has_z = 1;
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			}
		      if (iv == 0)
			  *buf = '\0';
		      else
			  strcpy (buf, " ");
		      if (has_z)
			{
			    buf_x = sqlite3_mprintf ("%.*f", precision, x);
			    gaiaOutClean (buf_x);
			    buf_y = sqlite3_mprintf ("%.*f", precision, y);
			    gaiaOutClean (buf_y);
			    buf_z = sqlite3_mprintf ("%.*f", precision, z);
			    gaiaOutClean (buf_z);
			    if (version == 3)
			      {
				  xbuf =
				      sqlite3_mprintf ("%s%s %s %s", buf, buf_x,
						       buf_y, buf_z);
				  sqlite3_free (buf_x);
				  sqlite3_free (buf_y);
				  sqlite3_free (buf_z);
			      }
			    else
			      {
				  xbuf =
				      sqlite3_mprintf ("%s%s,%s,%s", buf, buf_x,
						       buf_y, buf_z);
				  sqlite3_free (buf_x);
				  sqlite3_free (buf_y);
				  sqlite3_free (buf_z);
			      }
			}
		      else
			{
			    buf_x = sqlite3_mprintf ("%.*f", precision, x);
			    gaiaOutClean (buf_x);
			    buf_y = sqlite3_mprintf ("%.*f", precision, y);
			    gaiaOutClean (buf_y);
			    if (version == 3)
			      {
				  xbuf =
				      sqlite3_mprintf ("%s%s %s", buf, buf_x,
						       buf_y);
				  sqlite3_free (buf_x);
				  sqlite3_free (buf_y);
			      }
			    else
			      {
				  xbuf =
				      sqlite3_mprintf ("%s%s,%s", buf, buf_x,
						       buf_y);
				  sqlite3_free (buf_x);
				  sqlite3_free (buf_y);
			      }
			}
		      gaiaAppendToOutBuffer (out_buf, xbuf);
		      sqlite3_free (xbuf);
		  }
		/* closing the Interior Ring */
		if (version == 3)
		  {
		      strcpy (buf, "</gml:posList>");
		      strcat (buf, "</gml:LinearRing>");
		      strcat (buf, "</gml:interior>");
		  }
		else
		  {
		      strcpy (buf, "</gml:coordinates>");
		      strcat (buf, "</gml:LinearRing>");
		      strcat (buf, "</gml:innerBoundaryIs>");
		  }
		gaiaAppendToOutBuffer (out_buf, buf);
	    }
	  /* closing the Polygon */
	  if (is_multi)
	    {
		if (version == 3)
		  {
		      strcpy (buf, "</gml:Polygon>");
		      if (is_coll)
			  strcat (buf, "</gml:geometryMember>");
		      else
			  strcat (buf, "</gml:surfaceMember>");
		  }
		else
		  {
		      strcpy (buf, "</gml:Polygon>");
		      if (is_coll)
			  strcat (buf, "</gml:geometryMember>");
		      else
			  strcat (buf, "</gml:polygonMember>");
		  }
	    }
	  else
	      strcpy (buf, "</gml:Polygon>");
	  gaiaAppendToOutBuffer (out_buf, buf);
	  polyg = polyg->Next;
      }
    switch (geom->DeclaredType)
      {
      case GAIA_POINT:
      case GAIA_LINESTRING:
      case GAIA_POLYGON:
	  *buf = '\0';
	  break;
      case GAIA_MULTIPOINT:
	  sprintf (buf, "</gml:MultiPoint>");
	  break;
      case GAIA_MULTILINESTRING:
	  if (version == 3)
	      sprintf (buf, "</gml:MultiCurve>");
	  else
	      sprintf (buf, "</gml:MultiLineString>");
	  break;
      case GAIA_MULTIPOLYGON:
	  if (version == 3)
	      sprintf (buf, "</gml:MultiSurface>");
	  else
	      sprintf (buf, "</gml:MultiPolygon>");
	  break;
      default:
	  sprintf (buf, "</gml:MultiGeometry>");
	  break;
      };
    gaiaAppendToOutBuffer (out_buf, buf);
}

GAIAGEO_DECLARE void
gaiaOutGeoJSON (gaiaOutBufferPtr out_buf, gaiaGeomCollPtr geom, int precision,
		int options)
{
/*
/ prints the GeoJSON representation of current geometry
/ *result* returns the encoded GeoJSON or NULL if any error is encountered
*/
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    gaiaRingPtr ring;
    int iv;
    int ib;
    double x;
    double y;
    double z;
    double m;
    int has_z;
    int is_multi = 0;
    int multi_count = 0;
    char *bbox;
    char crs[2048];
    char *buf;
    char *buf_x;
    char *buf_y;
    char *buf_m;
    char *buf_z;
    char endJson[16];
    if (!geom)
	return;
    if (precision > 18)
	precision = 18;

    if (options != 0)
      {
	  bbox = NULL;
	  *crs = '\0';
	  if (geom->Srid > 0)
	    {
		if (options == 2 || options == 3)
		  {
		      /* including short CRS */
		      sprintf (crs,
			       ",\"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"EPSG:%d\"}}",
			       geom->Srid);
		  }
		if (options == 4 || options == 5)
		  {
		      /* including long CRS */
		      sprintf (crs,
			       ",\"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"urn:ogc:def:crs:EPSG:%d\"}}",
			       geom->Srid);
		  }
	    }
	  if (options == 1 || options == 3 || options == 5)
	    {
		/* including BBOX */
		gaiaMbrGeometry (geom);
		buf_x = sqlite3_mprintf ("%.*f", precision, geom->MinX);
		gaiaOutClean (buf_x);
		buf_y = sqlite3_mprintf ("%.*f", precision, geom->MinY);
		gaiaOutClean (buf_y);
		buf_z = sqlite3_mprintf ("%.*f", precision, geom->MaxX);
		gaiaOutClean (buf_z);
		buf_m = sqlite3_mprintf ("%.*f", precision, geom->MaxY);
		gaiaOutClean (buf_m);
		bbox =
		    sqlite3_mprintf (",\"bbox\":[%s,%s,%s,%s]", buf_x, buf_y,
				     buf_z, buf_m);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		sqlite3_free (buf_z);
		sqlite3_free (buf_m);
	    }
	  switch (geom->DeclaredType)
	    {
	    case GAIA_POINT:
		buf =
		    sqlite3_mprintf ("{\"type\":\"Point\"%s%s,\"coordinates\":",
				     crs, bbox);
		strcpy (endJson, "}");
		break;
	    case GAIA_LINESTRING:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"LineString\"%s%s,\"coordinates\":[", crs,
		     bbox);
		strcpy (endJson, "}");
		break;
	    case GAIA_POLYGON:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"Polygon\"%s%s,\"coordinates\":[", crs, bbox);
		strcpy (endJson, "}");
		break;
	    case GAIA_MULTIPOINT:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"MultiPoint\"%s%s,\"coordinates\":[", crs,
		     bbox);
		strcpy (endJson, "]}");
		break;
	    case GAIA_MULTILINESTRING:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"MultiLineString\"%s%s,\"coordinates\":[[",
		     crs, bbox);
		strcpy (endJson, "]}");
		break;
	    case GAIA_MULTIPOLYGON:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"MultiPolygon\"%s%s,\"coordinates\":[[", crs,
		     bbox);
		strcpy (endJson, "]}");
		break;
	    default:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"GeometryCollection\"%s%s,\"geometries\":[",
		     crs, bbox);
		strcpy (endJson, "]}");
		is_multi = 1;
		break;
	    };
	  if (bbox)
	      sqlite3_free (bbox);
      }
    else
      {
	  /* omitting BBOX */
	  switch (geom->DeclaredType)
	    {
	    case GAIA_POINT:
		buf = sqlite3_mprintf ("{\"type\":\"Point\",\"coordinates\":");
		strcpy (endJson, "}");
		break;
	    case GAIA_LINESTRING:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"LineString\",\"coordinates\":[");
		strcpy (endJson, "}");
		break;
	    case GAIA_POLYGON:
		buf =
		    sqlite3_mprintf ("{\"type\":\"Polygon\",\"coordinates\":[");
		strcpy (endJson, "}");
		break;
	    case GAIA_MULTIPOINT:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"MultiPoint\",\"coordinates\":[");
		strcpy (endJson, "]}");
		break;
	    case GAIA_MULTILINESTRING:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"MultiLineString\",\"coordinates\":[[");
		strcpy (endJson, "]}");
		break;
	    case GAIA_MULTIPOLYGON:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"MultiPolygon\",\"coordinates\":[[");
		strcpy (endJson, "]}");
		break;
	    default:
		buf =
		    sqlite3_mprintf
		    ("{\"type\":\"GeometryCollection\",\"geometries\":[");
		strcpy (endJson, "]}");
		is_multi = 1;
		break;
	    };
      }
    gaiaAppendToOutBuffer (out_buf, buf);
    sqlite3_free (buf);
    point = geom->FirstPoint;
    while (point)
      {
	  /* processing POINT */
	  if (is_multi)
	    {
		if (multi_count > 0)
		    buf = ",{\"type\":\"Point\",\"coordinates\":";
		else
		    buf = "{\"type\":\"Point\",\"coordinates\":";
		gaiaAppendToOutBuffer (out_buf, buf);
	    }
	  else if (point != geom->FirstPoint)
	    {
		/* adding a further Point */
		gaiaAppendToOutBuffer (out_buf, ",");
	    }
	  buf_x = sqlite3_mprintf ("%.*f", precision, point->X);
	  gaiaOutClean (buf_x);
	  buf_y = sqlite3_mprintf ("%.*f", precision, point->Y);
	  gaiaOutClean (buf_y);
	  has_z = 0;
	  if (point->DimensionModel == GAIA_XY_Z
	      || point->DimensionModel == GAIA_XY_Z_M)
	    {
		buf_z = sqlite3_mprintf ("%.*f", precision, point->Z);
		gaiaOutClean (buf_z);
		has_z = 1;
	    }
	  if (has_z)
	    {
		buf = sqlite3_mprintf ("[%s,%s,%s]", buf_x, buf_y, buf_z);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
		sqlite3_free (buf_z);
	    }
	  else
	    {
		buf = sqlite3_mprintf ("[%s,%s]", buf_x, buf_y);
		sqlite3_free (buf_x);
		sqlite3_free (buf_y);
	    }
	  gaiaAppendToOutBuffer (out_buf, buf);
	  sqlite3_free (buf);
	  if (is_multi)
	    {
		gaiaAppendToOutBuffer (out_buf, "}");
		multi_count++;
	    }
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* processing LINESTRING */
	  if (is_multi)
	    {
		if (multi_count > 0)
		    buf = ",{\"type\":\"LineString\",\"coordinates\":[";
		else
		    buf = "{\"type\":\"LineString\",\"coordinates\":[";
		gaiaAppendToOutBuffer (out_buf, buf);
	    }
	  else if (line != geom->FirstLinestring)
	    {
		/* opening a further LineString */
		gaiaAppendToOutBuffer (out_buf, ",[");
	    }
	  for (iv = 0; iv < line->Points; iv++)
	    {
		/* exporting vertices */
		has_z = 0;
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      has_z = 1;
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      has_z = 1;
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		if (has_z)
		  {
		      buf_x = sqlite3_mprintf ("%.*f", precision, x);
		      gaiaOutClean (buf_x);
		      buf_y = sqlite3_mprintf ("%.*f", precision, y);
		      gaiaOutClean (buf_y);
		      buf_z = sqlite3_mprintf ("%.*f", precision, z);
		      gaiaOutClean (buf_z);
		      if (iv == 0)
			  buf =
			      sqlite3_mprintf ("[%s,%s,%s]", buf_x, buf_y,
					       buf_z);
		      else
			  buf =
			      sqlite3_mprintf (",[%s,%s,%s]", buf_x, buf_y,
					       buf_z);
		      sqlite3_free (buf_x);
		      sqlite3_free (buf_y);
		      sqlite3_free (buf_z);
		  }
		else
		  {
		      buf_x = sqlite3_mprintf ("%.*f", precision, x);
		      gaiaOutClean (buf_x);
		      buf_y = sqlite3_mprintf ("%.*f", precision, y);
		      gaiaOutClean (buf_y);
		      if (iv == 0)
			  buf = sqlite3_mprintf ("[%s,%s]", buf_x, buf_y);
		      else
			  buf = sqlite3_mprintf (",[%s,%s]", buf_x, buf_y);
		      sqlite3_free (buf_x);
		      sqlite3_free (buf_y);
		  }
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
	  /* closing the LineString */
	  gaiaAppendToOutBuffer (out_buf, "]");
	  if (is_multi)
	    {
		gaiaAppendToOutBuffer (out_buf, "}");
		multi_count++;
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* processing POLYGON */
	  if (is_multi)
	    {
		if (multi_count > 0)
		    buf = ",{\"type\":\"Polygon\",\"coordinates\":[";
		else
		    buf = "{\"type\":\"Polygon\",\"coordinates\":[";
		gaiaAppendToOutBuffer (out_buf, buf);
	    }
	  else if (polyg != geom->FirstPolygon)
	    {
		/* opening a further Polygon */
		gaiaAppendToOutBuffer (out_buf, ",[");
	    }
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* exporting vertices [Interior Ring] */
		has_z = 0;
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      has_z = 1;
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      has_z = 1;
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		  }
		if (has_z)
		  {
		      buf_x = sqlite3_mprintf ("%.*f", precision, x);
		      gaiaOutClean (buf_x);
		      buf_y = sqlite3_mprintf ("%.*f", precision, y);
		      gaiaOutClean (buf_y);
		      buf_z = sqlite3_mprintf ("%.*f", precision, z);
		      gaiaOutClean (buf_z);
		      if (iv == 0)
			  buf =
			      sqlite3_mprintf ("[[%s,%s,%s]", buf_x, buf_y,
					       buf_z);
		      else
			  buf =
			      sqlite3_mprintf (",[%s,%s,%s]", buf_x, buf_y,
					       buf_z);
		      sqlite3_free (buf_x);
		      sqlite3_free (buf_y);
		      sqlite3_free (buf_z);
		  }
		else
		  {
		      buf_x = sqlite3_mprintf ("%.*f", precision, x);
		      gaiaOutClean (buf_x);
		      buf_y = sqlite3_mprintf ("%.*f", precision, y);
		      gaiaOutClean (buf_y);
		      if (iv == 0)
			  buf = sqlite3_mprintf ("[[%s,%s]", buf_x, buf_y);
		      else
			  buf = sqlite3_mprintf (",[%s,%s]", buf_x, buf_y);
		      sqlite3_free (buf_x);
		      sqlite3_free (buf_y);
		  }
		gaiaAppendToOutBuffer (out_buf, buf);
		sqlite3_free (buf);
	    }
	  /* closing the Exterior Ring */
	  gaiaAppendToOutBuffer (out_buf, "]");
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* interior rings */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      /* exporting vertices [Interior Ring] */
		      has_z = 0;
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    has_z = 1;
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    has_z = 1;
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			}
		      if (has_z)
			{
			    buf_x = sqlite3_mprintf ("%.*f", precision, x);
			    gaiaOutClean (buf_x);
			    buf_y = sqlite3_mprintf ("%.*f", precision, y);
			    gaiaOutClean (buf_y);
			    buf_z = sqlite3_mprintf ("%.*f", precision, z);
			    gaiaOutClean (buf_z);
			    if (iv == 0)
				buf =
				    sqlite3_mprintf (",[[%s,%s,%s]", buf_x,
						     buf_y, buf_z);
			    else
				buf =
				    sqlite3_mprintf (",[%s,%s,%s]", buf_x,
						     buf_y, buf_z);
			    sqlite3_free (buf_x);
			    sqlite3_free (buf_y);
			    sqlite3_free (buf_z);
			}
		      else
			{
			    buf_x = sqlite3_mprintf ("%.*f", precision, x);
			    gaiaOutClean (buf_x);
			    buf_y = sqlite3_mprintf ("%.*f", precision, y);
			    gaiaOutClean (buf_y);
			    if (iv == 0)
				buf =
				    sqlite3_mprintf (",[[%s,%s]", buf_x, buf_y);
			    else
				buf =
				    sqlite3_mprintf (",[%s,%s]", buf_x, buf_y);
			    sqlite3_free (buf_x);
			    sqlite3_free (buf_y);
			}
		      gaiaAppendToOutBuffer (out_buf, buf);
		      sqlite3_free (buf);
		  }
		/* closing the Interior Ring */
		gaiaAppendToOutBuffer (out_buf, "]");
	    }
	  /* closing the Polygon */
	  gaiaAppendToOutBuffer (out_buf, "]");
	  if (is_multi)
	    {
		gaiaAppendToOutBuffer (out_buf, "}");
		multi_count++;
	    }
	  polyg = polyg->Next;
      }
    gaiaAppendToOutBuffer (out_buf, endJson);
}
