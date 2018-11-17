/*

 gg_vanuatu.c -- WKT parser/lexer 
  
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

The Vanuatu Team - University of Toronto - Supervisor:
Greg Wilson	gvwilson@cs.toronto.ca

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

#include <assert.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>
#include <spatialite/debug.h>

#include <spatialite/gaiageo.h>

#if defined(_WIN32) || defined(WIN32)
#include <io.h>
#ifndef isatty
#define isatty	_isatty
#endif
#ifndef fileno
#define fileno	_fileno
#endif
#endif

#define VANUATU_DYN_NONE	0
#define VANUATU_DYN_POINT	1
#define VANUATU_DYN_LINESTRING	2
#define VANUATU_DYN_POLYGON	3
#define VANUATU_DYN_RING	4
#define VANUATU_DYN_GEOMETRY	5

#define VANUATU_DYN_BLOCK 1024



/*
** CAVEAT: we must redefine any Lemon/Flex own macro
*/
#define YYMINORTYPE		VANUATU_MINORTYPE
#define YY_CHAR			VANUATU_YY_CHAR
#define	input			vanuatu_input
#define ParseAlloc		vanuatuParseAlloc
#define ParseFree		vanuatuParseFree
#define ParseStackPeak		vanuatuParseStackPeak
#define Parse			vanuatuParse
#define yyStackEntry		vanuatu_yyStackEntry
#define yyzerominor		vanuatu_yyzerominor
#define yy_accept		vanuatu_yy_accept
#define yy_action		vanuatu_yy_action
#define yy_base			vanuatu_yy_base
#define yy_buffer_stack		vanuatu_yy_buffer_stack
#define yy_buffer_stack_max	vanuatu_yy_buffer_stack_max
#define yy_buffer_stack_top	vanuatu_yy_buffer_stack_top
#define yy_c_buf_p		vanuatu_yy_c_buf_p
#define yy_chk			vanuatu_yy_chk
#define yy_def			vanuatu_yy_def
#define yy_default		vanuatu_yy_default
#define yy_destructor		vanuatu_yy_destructor
#define yy_ec			vanuatu_yy_ec
#define yy_fatal_error		vanuatu_yy_fatal_error
#define yy_find_reduce_action	vanuatu_yy_find_reduce_action
#define yy_find_shift_action	vanuatu_yy_find_shift_action
#define yy_get_next_buffer	vanuatu_yy_get_next_buffer
#define yy_get_previous_state	vanuatu_yy_get_previous_state
#define yy_init			vanuatu_yy_init
#define yy_init_globals		vanuatu_yy_init_globals
#define yy_lookahead		vanuatu_yy_lookahead
#define yy_meta			vanuatu_yy_meta
#define yy_nxt			vanuatu_yy_nxt
#define yy_parse_failed		vanuatu_yy_parse_failed
#define yy_pop_parser_stack	vanuatu_yy_pop_parser_stack
#define yy_reduce		vanuatu_yy_reduce
#define yy_reduce_ofst		vanuatu_yy_reduce_ofst
#define yy_shift		vanuatu_yy_shift
#define yy_shift_ofst		vanuatu_yy_shift_ofst
#define yy_start		vanuatu_yy_start
#define yy_state_type		vanuatu_yy_state_type
#define yy_syntax_error		vanuatu_yy_syntax_error
#define yy_trans_info		vanuatu_yy_trans_info
#define yy_try_NUL_trans	vanuatu_yy_try_NUL_trans
#define yyParser		vanuatu_yyParser
#define yyStackEntry		vanuatu_yyStackEntry
#define yyStackOverflow		vanuatu_yyStackOverflow
#define yyRuleInfo		vanuatu_yyRuleInfo
#define yyunput			vanuatu_yyunput
#define yyzerominor		vanuatu_yyzerominor
#define yyTraceFILE		vanuatu_yyTraceFILE
#define yyTracePrompt		vanuatu_yyTracePrompt
#define yyTokenName		vanuatu_yyTokenName
#define yyRuleName		vanuatu_yyRuleName
#define ParseTrace		vanuatu_ParseTrace

#define yylex			vanuatu_yylex
#define YY_DECL int yylex (yyscan_t yyscanner)


/* including LEMON generated header */
#include "vanuatuWkt.h"



typedef union
{
    double dval;
    struct symtab *symp;
} vanuatu_yystype;
#define YYSTYPE vanuatu_yystype


struct vanuatu_dyn_block
{
/* a struct taking trace of dynamic allocations */
    int type[VANUATU_DYN_BLOCK];
    void *ptr[VANUATU_DYN_BLOCK];
    int index;
    struct vanuatu_dyn_block *next;
};

struct vanuatu_data
{
/* a struct used to make the lexer-parser reentrant and thread-safe */
    int vanuatu_parse_error;
    int vanuatu_line;
    int vanuatu_col;
    struct vanuatu_dyn_block *vanuatu_first_dyn_block;
    struct vanuatu_dyn_block *vanuatu_last_dyn_block;
    gaiaGeomCollPtr result;
    YYSTYPE VanuatuWktlval;
};

static struct vanuatu_dyn_block *
vanuatuCreateDynBlock (void)
{
/* allocating a new block to trace dynamic allocations */
    int i;
    struct vanuatu_dyn_block *p = malloc (sizeof (struct vanuatu_dyn_block));
    for (i = 0; i < VANUATU_DYN_BLOCK; i++)
      {
	  /* initializing map entries */
	  p->type[i] = VANUATU_DYN_NONE;
	  p->ptr[i] = NULL;
      }
    p->index = 0;
    p->next = NULL;
    return p;
}

static void
vanuatuMapDynAlloc (struct vanuatu_data *p_data, int type, void *ptr)
{
/* appending a dynamic allocation into the map */
    struct vanuatu_dyn_block *p;
    if (p_data->vanuatu_first_dyn_block == NULL)
      {
	  /* inserting the first block of the map */
	  p = vanuatuCreateDynBlock ();
	  p_data->vanuatu_first_dyn_block = p;
	  p_data->vanuatu_last_dyn_block = p;
      }
    if (p_data->vanuatu_last_dyn_block->index >= VANUATU_DYN_BLOCK)
      {
	  /* adding a further block to the map */
	  p = vanuatuCreateDynBlock ();
	  p_data->vanuatu_last_dyn_block->next = p;
	  p_data->vanuatu_last_dyn_block = p;
      }
    p_data->vanuatu_last_dyn_block->type[p_data->
					 vanuatu_last_dyn_block->index] = type;
    p_data->vanuatu_last_dyn_block->ptr[p_data->vanuatu_last_dyn_block->index] =
	ptr;
    p_data->vanuatu_last_dyn_block->index++;
}

static void
vanuatuMapDynClean (struct vanuatu_data *p_data, void *ptr)
{
/* deleting a dynamic allocation from the map */
    int i;
    struct vanuatu_dyn_block *p = p_data->vanuatu_first_dyn_block;
    while (p)
      {
	  for (i = 0; i < VANUATU_DYN_BLOCK; i++)
	    {
		switch (p->type[i])
		  {
		  case VANUATU_DYN_POINT:
		  case VANUATU_DYN_LINESTRING:
		  case VANUATU_DYN_POLYGON:
		  case VANUATU_DYN_RING:
		  case VANUATU_DYN_GEOMETRY:
		      if (p->ptr[i] == ptr)
			{
			    p->type[i] = VANUATU_DYN_NONE;
			    return;
			}
		      break;
		  };
	    }
	  p = p->next;
      }
}

static void
vanuatuCleanMapDynAlloc (struct vanuatu_data *p_data, int clean_all)
{
/* cleaning the dynamic allocations map */
    int i;
    struct vanuatu_dyn_block *pn;
    struct vanuatu_dyn_block *p = p_data->vanuatu_first_dyn_block;
    while (p)
      {
	  if (clean_all)
	    {
		for (i = 0; i < VANUATU_DYN_BLOCK; i++)
		  {
		      /* deleting Geometry objects */
		      switch (p->type[i])
			{
			case VANUATU_DYN_POINT:
			    gaiaFreePoint ((gaiaPointPtr) (p->ptr[i]));
			    break;
			case VANUATU_DYN_LINESTRING:
			    gaiaFreeLinestring ((gaiaLinestringPtr)
						(p->ptr[i]));
			    break;
			case VANUATU_DYN_POLYGON:
			    gaiaFreePolygon ((gaiaPolygonPtr) (p->ptr[i]));
			    break;
			case VANUATU_DYN_RING:
			    gaiaFreeRing ((gaiaRingPtr) (p->ptr[i]));
			    break;
			case VANUATU_DYN_GEOMETRY:
			    gaiaFreeGeomColl ((gaiaGeomCollPtr) (p->ptr[i]));
			    break;
			};
		  }
	    }
	  /* deleting the map block */
	  pn = p->next;
	  free (p);
	  p = pn;
      }
}

static int
vanuatuCheckValidity (gaiaGeomCollPtr geom)
{
/* checks if this one is a degenerated geometry */
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    int ib;
    int entities = 0;
    pt = geom->FirstPoint;
    while (pt)
      {
	  /* checking points */
	  entities++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  /* checking linestrings */
	  if (ln->Points < 2)
	      return 0;
	  entities++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  /* checking polygons */
	  rng = pg->Exterior;
	  if (rng->Points < 4)
	      return 0;
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		rng = pg->Interiors + ib;
		if (rng->Points < 4)
		    return 0;
	    }
	  entities++;
	  pg = pg->Next;
      }
    if (!entities)
	return 0;
    return 1;
}

static gaiaGeomCollPtr
gaiaGeometryFromPoint (struct vanuatu_data *p_data, gaiaPointPtr point)
{
/* builds a GEOMETRY containing a POINT */
    gaiaGeomCollPtr geom = NULL;
    geom = gaiaAllocGeomColl ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_POINT;
    gaiaAddPointToGeomColl (geom, point->X, point->Y);
    vanuatuMapDynClean (p_data, point);
    gaiaFreePoint (point);
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromPointZ (struct vanuatu_data *p_data, gaiaPointPtr point)
{
/* builds a GEOMETRY containing a POINTZ */
    gaiaGeomCollPtr geom = NULL;
    geom = gaiaAllocGeomCollXYZ ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_POINTZ;
    gaiaAddPointToGeomCollXYZ (geom, point->X, point->Y, point->Z);
    vanuatuMapDynClean (p_data, point);
    gaiaFreePoint (point);
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromPointM (struct vanuatu_data *p_data, gaiaPointPtr point)
{
/* builds a GEOMETRY containing a POINTM */
    gaiaGeomCollPtr geom = NULL;
    geom = gaiaAllocGeomCollXYM ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_POINTM;
    gaiaAddPointToGeomCollXYM (geom, point->X, point->Y, point->M);
    vanuatuMapDynClean (p_data, point);
    gaiaFreePoint (point);
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromPointZM (struct vanuatu_data *p_data, gaiaPointPtr point)
{
/* builds a GEOMETRY containing a POINTZM */
    gaiaGeomCollPtr geom = NULL;
    geom = gaiaAllocGeomCollXYZM ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_POINTZM;
    gaiaAddPointToGeomCollXYZM (geom, point->X, point->Y, point->Z, point->M);
    vanuatuMapDynClean (p_data, point);
    gaiaFreePoint (point);
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromLinestring (struct vanuatu_data *p_data, gaiaLinestringPtr line)
{
/* builds a GEOMETRY containing a LINESTRING */
    gaiaGeomCollPtr geom = NULL;
    gaiaLinestringPtr line2;
    int iv;
    double x;
    double y;
    geom = gaiaAllocGeomColl ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_LINESTRING;
    line2 = gaiaAddLinestringToGeomColl (geom, line->Points);
    for (iv = 0; iv < line2->Points; iv++)
      {
	  /* sets the POINTS for the exterior ring */
	  gaiaGetPoint (line->Coords, iv, &x, &y);
	  gaiaSetPoint (line2->Coords, iv, x, y);
      }
    vanuatuMapDynClean (p_data, line);
    gaiaFreeLinestring (line);
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromLinestringZ (struct vanuatu_data *p_data,
			     gaiaLinestringPtr line)
{
/* builds a GEOMETRY containing a LINESTRINGZ */
    gaiaGeomCollPtr geom = NULL;
    gaiaLinestringPtr line2;
    int iv;
    double x;
    double y;
    double z;
    geom = gaiaAllocGeomCollXYZ ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_LINESTRING;
    line2 = gaiaAddLinestringToGeomColl (geom, line->Points);
    for (iv = 0; iv < line2->Points; iv++)
      {
	  /* sets the POINTS for the exterior ring */
	  gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
	  gaiaSetPointXYZ (line2->Coords, iv, x, y, z);
      }
    vanuatuMapDynClean (p_data, line);
    gaiaFreeLinestring (line);
    return geom;
}


static gaiaGeomCollPtr
gaiaGeometryFromLinestringM (struct vanuatu_data *p_data,
			     gaiaLinestringPtr line)
{
/* builds a GEOMETRY containing a LINESTRINGM */
    gaiaGeomCollPtr geom = NULL;
    gaiaLinestringPtr line2;
    int iv;
    double x;
    double y;
    double m;
    geom = gaiaAllocGeomCollXYM ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_LINESTRING;
    line2 = gaiaAddLinestringToGeomColl (geom, line->Points);
    for (iv = 0; iv < line2->Points; iv++)
      {
	  /* sets the POINTS for the exterior ring */
	  gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
	  gaiaSetPointXYM (line2->Coords, iv, x, y, m);
      }
    vanuatuMapDynClean (p_data, line);
    gaiaFreeLinestring (line);
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromLinestringZM (struct vanuatu_data *p_data,
			      gaiaLinestringPtr line)
{
/* builds a GEOMETRY containing a LINESTRINGZM */
    gaiaGeomCollPtr geom = NULL;
    gaiaLinestringPtr line2;
    int iv;
    double x;
    double y;
    double z;
    double m;
    geom = gaiaAllocGeomCollXYZM ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_LINESTRING;
    line2 = gaiaAddLinestringToGeomColl (geom, line->Points);
    for (iv = 0; iv < line2->Points; iv++)
      {
	  /* sets the POINTS for the exterior ring */
	  gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
	  gaiaSetPointXYZM (line2->Coords, iv, x, y, z, m);
      }
    vanuatuMapDynClean (p_data, line);
    gaiaFreeLinestring (line);
    return geom;
}

/******************************************************************************
** The following code was created by Team Vanuatu of The University of Toronto.
** It is responsible for handling the parsing of wkt expressions.  The parser
** is built using LEMON and the cooresponding methods were written by the 
** students.

Authors:
Ruppi Rana			ruppi.rana@gmail.com
Dev Tanna			dev.tanna@gmail.com
Elias Adum			elias.adum@gmail.com
Benton Hui			benton.hui@gmail.com
Abhayan Sundararajan		abhayan@gmail.com
Chee-Lun Michael Stephen Cho	cheelun.cho@gmail.com
Nikola Banovic			nikola.banovic@gmail.com
Yong Jian			yong.jian@utoronto.ca

Supervisor:
Greg Wilson			gvwilson@cs.toronto.ca

-------------------------------------------------------------------------------
*/

/* 
 * Creates a 2D (xy) point in SpatiaLite
 * x and y are pointers to doubles which represent the x and y coordinates of the point to be created.
 * Returns a gaiaPointPtr representing the created point.
 *
 * Creates a 2D (xy) point. This is a parser helper function which is called when 2D coordinates are encountered.
 * Parameters x and y are pointers to doubles which represent the x and y coordinates of the point to be created.
 * Returns a gaiaPointPtr pointing to the created 2D point.
 */
static gaiaPointPtr
vanuatu_point_xy (struct vanuatu_data *p_data, double *x, double *y)
{
    gaiaPointPtr pt = gaiaAllocPoint (*x, *y);
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_POINT, pt);
    return pt;
}

/* 
 * Creates a 3D (xyz) point in SpatiaLite
 * x, y, and z are pointers to doubles which represent the x, y, and z coordinates of the point to be created.
 * Returns a gaiaPointPtr representing the created point.
 *
 * Creates a 3D (xyz) point. This is a parser helper function which is called when 3D coordinates are encountered.
 * Parameters x, y, and z are pointers to doubles which represent the x, y, and z coordinates of the point to be created.
 * Returns a gaiaPointPtr pointing to the 3D created point.
 */
static gaiaPointPtr
vanuatu_point_xyz (struct vanuatu_data *p_data, double *x, double *y, double *z)
{
    gaiaPointPtr pt = gaiaAllocPointXYZ (*x, *y, *z);
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_POINT, pt);
    return pt;
}

/* 
 * Creates a 2D (xy) point with an m value which is a part of the linear reference system. This is a parser helper
 * function which is called when 2D   *coordinates with an m value are encountered.
 * Parameters x and y are pointers to doubles which represent the x and y coordinates of the point to be created.
 * Parameter m is a pointer to a double which represents the part of the linear reference system.
 * Returns a gaiaPointPtr pointing to the created 2D point with an m value.
 */
static gaiaPointPtr
vanuatu_point_xym (struct vanuatu_data *p_data, double *x, double *y, double *m)
{
    gaiaPointPtr pt = gaiaAllocPointXYM (*x, *y, *m);
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_POINT, pt);
    return pt;
}

/* 
 * Creates a 4D (xyz) point with an m value which is a part of the linear reference system. This is a parser helper
 * function which is called when  *4Dcoordinates with an m value are encountered
 * Parameters x, y, and z are pointers to doubles which represent the x, y, and z coordinates of the point to be created. 
 * Parameter m is a pointer to a double which represents the part of the linear reference system.
 * Returns a gaiaPointPtr pointing the created 4D point with an m value.
 */
gaiaPointPtr
vanuatu_point_xyzm (struct vanuatu_data * p_data, double *x, double *y,
		    double *z, double *m)
{
    gaiaPointPtr pt = gaiaAllocPointXYZM (*x, *y, *z, *m);
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_POINT, pt);
    return pt;
}

/*
 * Builds a geometry collection from a point. The geometry collection should contain only one element ? the point. 
 * The correct geometry type must be *decided based on the point type. The parser should call this function when the 
 * ?POINT? WKT expression is encountered.
 * Parameter point is a pointer to a 2D, 3D, 2D with an m value, or 4D with an m value point.
 * Returns a geometry collection containing the point. The geometry must have FirstPoint and LastPoint  pointing to the
 * same place as point.  *DimensionModel must be the same as the model of the point and DimensionType must be GAIA_TYPE_POINT.
 */
static gaiaGeomCollPtr
vanuatu_buildGeomFromPoint (struct vanuatu_data *p_data, gaiaPointPtr point)
{
    switch (point->DimensionModel)
      {
      case GAIA_XY:
	  return gaiaGeometryFromPoint (p_data, point);
      case GAIA_XY_Z:
	  return gaiaGeometryFromPointZ (p_data, point);
      case GAIA_XY_M:
	  return gaiaGeometryFromPointM (p_data, point);
      case GAIA_XY_Z_M:
	  return gaiaGeometryFromPointZM (p_data, point);
      }
    return NULL;
}

/* 
 * Creates a 2D (xy) linestring from a list of 2D points.
 *
 * Parameter first is a gaiaPointPtr to the first point in a linked list of points which define the linestring.
 * All of the points in the list must be 2D (xy) points. There must be at least 2 points in the list.
 *
 * Returns a pointer to linestring containing all of the points in the list.
 */
static gaiaLinestringPtr
vanuatu_linestring_xy (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    int points = 0;
    int i = 0;
    gaiaLinestringPtr linestring;

    while (p != NULL)
      {
	  p = p->Next;
	  points++;
      }

    linestring = gaiaAllocLinestring (points);
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_LINESTRING, linestring);

    p = first;
    while (p != NULL)
      {
	  gaiaSetPoint (linestring->Coords, i, p->X, p->Y);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
	  i++;
      }

    return linestring;
}

/* 
 * Creates a 3D (xyz) linestring from a list of 3D points.
 *
 * Parameter first is a gaiaPointPtr to the first point in a linked list of points which define the linestring.
 * All of the points in the list must be 3D (xyz) points. There must be at least 2 points in the list.
 *
 * Returns a pointer to linestring containing all of the points in the list.
 */
static gaiaLinestringPtr
vanuatu_linestring_xyz (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    int points = 0;
    int i = 0;
    gaiaLinestringPtr linestring;

    while (p != NULL)
      {
	  p = p->Next;
	  points++;
      }

    linestring = gaiaAllocLinestringXYZ (points);
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_LINESTRING, linestring);

    p = first;
    while (p != NULL)
      {
	  gaiaSetPointXYZ (linestring->Coords, i, p->X, p->Y, p->Z);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
	  i++;
      }

    return linestring;
}

/* 
 * Creates a 2D (xy) with m value linestring from a list of 2D with m value points.
 *
 * Parameter first is a gaiaPointPtr to the first point in a linked list of points which define the linestring.
 * All of the points in the list must be 2D (xy) with m value points. There must be at least 2 points in the list.
 *
 * Returns a pointer to linestring containing all of the points in the list.
 */
static gaiaLinestringPtr
vanuatu_linestring_xym (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    int points = 0;
    int i = 0;
    gaiaLinestringPtr linestring;

    while (p != NULL)
      {
	  p = p->Next;
	  points++;
      }

    linestring = gaiaAllocLinestringXYM (points);
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_LINESTRING, linestring);

    p = first;
    while (p != NULL)
      {
	  gaiaSetPointXYM (linestring->Coords, i, p->X, p->Y, p->M);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
	  i++;
      }

    return linestring;
}

/* 
 * Creates a 4D (xyz) with m value linestring from a list of 4D with m value points.
 *
 * Parameter first is a gaiaPointPtr to the first point in a linked list of points which define the linestring.
 * All of the points in the list must be 4D (xyz) with m value points. There must be at least 2 points in the list.
 *
 * Returns a pointer to linestring containing all of the points in the list.
 */
static gaiaLinestringPtr
vanuatu_linestring_xyzm (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    int points = 0;
    int i = 0;
    gaiaLinestringPtr linestring;

    while (p != NULL)
      {
	  p = p->Next;
	  points++;
      }

    linestring = gaiaAllocLinestringXYZM (points);
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_LINESTRING, linestring);

    p = first;
    while (p != NULL)
      {
	  gaiaSetPointXYZM (linestring->Coords, i, p->X, p->Y, p->Z, p->M);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
	  i++;
      }

    return linestring;
}

/*
 * Builds a geometry collection from a linestring.
 */
static gaiaGeomCollPtr
vanuatu_buildGeomFromLinestring (struct vanuatu_data *p_data,
				 gaiaLinestringPtr line)
{
    switch (line->DimensionModel)
      {
      case GAIA_XY:
	  return gaiaGeometryFromLinestring (p_data, line);
      case GAIA_XY_Z:
	  return gaiaGeometryFromLinestringZ (p_data, line);
      case GAIA_XY_M:
	  return gaiaGeometryFromLinestringM (p_data, line);
      case GAIA_XY_Z_M:
	  return gaiaGeometryFromLinestringZM (p_data, line);
      }
    return NULL;
}

/*
 * Helper function that determines the number of points in the linked list.
 */
static int
vanuatu_count_points (gaiaPointPtr first)
{
    /* Counts the number of points in the ring. */
    gaiaPointPtr p = first;
    int numpoints = 0;
    while (p != NULL)
      {
	  numpoints++;
	  p = p->Next;
      }
    return numpoints;
}

/*
 * Creates a 2D (xy) ring in SpatiaLite
 *
 * first is a gaiaPointPtr to the first point in a linked list of points which define the polygon.
 * All of the points given to the function are 2D (xy) points. There will be at least 4 points in the list.
 *
 * Returns the ring defined by the points given to the function.
 */
static gaiaRingPtr
vanuatu_ring_xy (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    gaiaRingPtr ring = NULL;
    int numpoints;
    int index;

    /* If no pointers are given, return. */
    if (first == NULL)
	return NULL;

    /* Counts the number of points in the ring. */
    numpoints = vanuatu_count_points (first);
    if (numpoints < 4)
	return NULL;

    /* Creates and allocates a ring structure. */
    ring = gaiaAllocRing (numpoints);
    if (ring == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_RING, ring);

    /* Adds every point into the ring structure. */
    p = first;
    for (index = 0; index < numpoints; index++)
      {
	  gaiaSetPoint (ring->Coords, index, p->X, p->Y);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
      }

    return ring;
}

/*
 * Creates a 3D (xyz) ring in SpatiaLite
 *
 * first is a gaiaPointPtr to the first point in a linked list of points which define the polygon.
 * All of the points given to the function are 3D (xyz) points. There will be at least 4 points in the list.
 *
 * Returns the ring defined by the points given to the function.
 */
static gaiaRingPtr
vanuatu_ring_xyz (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    gaiaRingPtr ring = NULL;
    int numpoints;
    int index;

    /* If no pointers are given, return. */
    if (first == NULL)
	return NULL;

    /* Counts the number of points in the ring. */
    numpoints = vanuatu_count_points (first);
    if (numpoints < 4)
	return NULL;

    /* Creates and allocates a ring structure. */
    ring = gaiaAllocRingXYZ (numpoints);
    if (ring == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_RING, ring);

    /* Adds every point into the ring structure. */
    p = first;
    for (index = 0; index < numpoints; index++)
      {
	  gaiaSetPointXYZ (ring->Coords, index, p->X, p->Y, p->Z);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
      }

    return ring;
}

/*
 * Creates a 2D (xym) ring in SpatiaLite
 *
 * first is a gaiaPointPtr to the first point in a linked list of points which define the polygon.
 * All of the points given to the function are 2D (xym) points. There will be at least 4 points in the list.
 *
 * Returns the ring defined by the points given to the function.
 */
static gaiaRingPtr
vanuatu_ring_xym (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    gaiaRingPtr ring = NULL;
    int numpoints;
    int index;

    /* If no pointers are given, return. */
    if (first == NULL)
	return NULL;

    /* Counts the number of points in the ring. */
    numpoints = vanuatu_count_points (first);
    if (numpoints < 4)
	return NULL;

    /* Creates and allocates a ring structure. */
    ring = gaiaAllocRingXYM (numpoints);
    if (ring == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_RING, ring);

    /* Adds every point into the ring structure. */
    p = first;
    for (index = 0; index < numpoints; index++)
      {
	  gaiaSetPointXYM (ring->Coords, index, p->X, p->Y, p->M);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
      }

    return ring;
}

/*
 * Creates a 3D (xyzm) ring in SpatiaLite
 *
 * first is a gaiaPointPtr to the first point in a linked list of points which define the polygon.
 * All of the points given to the function are 3D (xyzm) points. There will be at least 4 points in the list.
 *
 * Returns the ring defined by the points given to the function.
 */
static gaiaRingPtr
vanuatu_ring_xyzm (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    gaiaRingPtr ring = NULL;
    int numpoints;
    int index;

    /* If no pointers are given, return. */
    if (first == NULL)
	return NULL;

    /* Counts the number of points in the ring. */
    numpoints = vanuatu_count_points (first);
    if (numpoints < 4)
	return NULL;

    /* Creates and allocates a ring structure. */
    ring = gaiaAllocRingXYZM (numpoints);
    if (ring == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_RING, ring);

    /* Adds every point into the ring structure. */
    p = first;
    for (index = 0; index < numpoints; index++)
      {
	  gaiaSetPointXYZM (ring->Coords, index, p->X, p->Y, p->Z, p->M);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
      }

    return ring;
}

/*
 * Helper function that will create any type of polygon (xy, xym, xyz, xyzm) in SpatiaLite.
 * 
 * first is a gaiaRingPtr to the first ring in a linked list of rings which define the polygon.
 * The first ring in the linked list is the external ring while the rest (if any) are internal rings.
 * All of the rings given to the function are of the same type. There will be at least 1 ring in the list.
 *
 * Returns the polygon defined by the rings given to the function.
 */
static gaiaPolygonPtr
vanuatu_polygon_any_type (struct vanuatu_data *p_data, gaiaRingPtr first)
{
    gaiaRingPtr p;
    gaiaRingPtr p_n;
    gaiaPolygonPtr polygon;
    /* If no pointers are given, return. */
    if (first == NULL)
	return NULL;

    /* Creates and allocates a polygon structure with the exterior ring. */
    polygon = gaiaCreatePolygon (first);
    if (polygon == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_POLYGON, polygon);

    /* Adds all interior rings into the polygon structure. */
    p = first;
    while (p != NULL)
      {
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  if (p == first)
	      gaiaFreeRing (p);
	  else
	      gaiaAddRingToPolyg (polygon, p);
	  p = p_n;
      }

    return polygon;
}

/* 
 * Creates a 2D (xy) polygon in SpatiaLite
 *
 * first is a gaiaRingPtr to the first ring in a linked list of rings which define the polygon.
 * The first ring in the linked list is the external ring while the rest (if any) are internal rings.
 * All of the rings given to the function are 2D (xy) rings. There will be at least 1 ring in the list.
 *
 * Returns the polygon defined by the rings given to the function.
 */
static gaiaPolygonPtr
vanuatu_polygon_xy (struct vanuatu_data *p_data, gaiaRingPtr first)
{
    return vanuatu_polygon_any_type (p_data, first);
}

/* 
 * Creates a 3D (xyz) polygon in SpatiaLite
 *
 * first is a gaiaRingPtr to the first ring in a linked list of rings which define the polygon.
 * The first ring in the linked list is the external ring while the rest (if any) are internal rings.
 * All of the rings given to the function are 3D (xyz) rings. There will be at least 1 ring in the list.
 *
 * Returns the polygon defined by the rings given to the function.
 */
static gaiaPolygonPtr
vanuatu_polygon_xyz (struct vanuatu_data *p_data, gaiaRingPtr first)
{
    return vanuatu_polygon_any_type (p_data, first);
}

/* 
 * Creates a 2D (xym) polygon in SpatiaLite
 *
 * first is a gaiaRingPtr to the first ring in a linked list of rings which define the polygon.
 * The first ring in the linked list is the external ring while the rest (if any) are internal rings.
 * All of the rings given to the function are 2D (xym) rings. There will be at least 1 ring in the list.
 *
 * Returns the polygon defined by the rings given to the function.
 */
static gaiaPolygonPtr
vanuatu_polygon_xym (struct vanuatu_data *p_data, gaiaRingPtr first)
{
    return vanuatu_polygon_any_type (p_data, first);
}

/* 
 * Creates a 3D (xyzm) polygon in SpatiaLite
 *
 * first is a gaiaRingPtr to the first ring in a linked list of rings which define the polygon.
 * The first ring in the linked list is the external ring while the rest (if any) are internal rings.
 * All of the rings given to the function are 3D (xyzm) rings. There will be at least 1 ring in the list.
 *
 * Returns the polygon defined by the rings given to the function.
 */
static gaiaPolygonPtr
vanuatu_polygon_xyzm (struct vanuatu_data *p_data, gaiaRingPtr first)
{
    return vanuatu_polygon_any_type (p_data, first);
}

/*
 * Builds a geometry collection from a polygon.
 * NOTE: This function may already be implemented in the SpatiaLite code base. If it is, make sure that we
 *              can use it (ie. it doesn't use any other variables or anything else set by Sandro's parser). If you find
 *              that we can use an existing function then ignore this one.
 */
static gaiaGeomCollPtr
vanuatu_buildGeomFromPolygon (struct vanuatu_data *p_data,
			      gaiaPolygonPtr polygon)
{
    gaiaGeomCollPtr geom = NULL;

    /* If no pointers are given, return. */
    if (polygon == NULL)
      {
	  return NULL;
      }

    /* Creates and allocates a geometry collection containing a multipoint. */
    switch (polygon->DimensionModel)
      {
      case GAIA_XY:
	  geom = gaiaAllocGeomColl ();
	  break;
      case GAIA_XY_Z:
	  geom = gaiaAllocGeomCollXYZ ();
	  break;
      case GAIA_XY_M:
	  geom = gaiaAllocGeomCollXYM ();
	  break;
      case GAIA_XY_Z_M:
	  geom = gaiaAllocGeomCollXYZM ();
	  break;
      }
    if (geom == NULL)
      {
	  return NULL;
      }
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_POLYGON;

    /* Stores the location of the first and last polygons in the linked list. */
    geom->FirstPolygon = polygon;
    while (polygon != NULL)
      {
	  vanuatuMapDynClean (p_data, polygon);
	  geom->LastPolygon = polygon;
	  polygon = polygon->Next;
      }
    return geom;
}

/* 
 * Creates a 2D (xy) multipoint object in SpatiaLite
 *
 * first is a gaiaPointPtr to the first point in a linked list of points.
 * All of the points given to the function are 2D (xy) points. There will be at least 1 point in the list.
 *
 * Returns a geometry collection containing the created multipoint object.
 */
static gaiaGeomCollPtr
vanuatu_multipoint_xy (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    gaiaGeomCollPtr geom = NULL;

    /* If no pointers are given, return. */
    if (first == NULL)
	return NULL;

    /* Creates and allocates a geometry collection containing a multipoint. */
    geom = gaiaAllocGeomColl ();
    if (geom == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_MULTIPOINT;

    /* For every 2D (xy) point, add it to the geometry collection. */
    while (p != NULL)
      {
	  gaiaAddPointToGeomColl (geom, p->X, p->Y);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
      }
    return geom;
}

/* 
 * Creates a 3D (xyz) multipoint object in SpatiaLite
 *
 * first is a gaiaPointPtr to the first point in a linked list of points.
 * All of the points given to the function are 3D (xyz) points. There will be at least 1 point in the list.
 *
 * Returns a geometry collection containing the created multipoint object.
 */
static gaiaGeomCollPtr
vanuatu_multipoint_xyz (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    gaiaGeomCollPtr geom = NULL;

    /* If no pointers are given, return. */
    if (first == NULL)
	return NULL;

    /* Creates and allocates a geometry collection containing a multipoint. */
    geom = gaiaAllocGeomCollXYZ ();
    if (geom == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_MULTIPOINT;

    /* For every 3D (xyz) point, add it to the geometry collection. */
    while (p != NULL)
      {
	  gaiaAddPointToGeomCollXYZ (geom, p->X, p->Y, p->Z);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
      }
    return geom;
}

/* 
 * Creates a 2D (xym) multipoint object in SpatiaLite
 *
 * first is a gaiaPointPtr to the first point in a linked list of points.
 * All of the points given to the function are 2D (xym) points. There will be at least 1 point in the list.
 *
 * Returns a geometry collection containing the created multipoint object.
 */
static gaiaGeomCollPtr
vanuatu_multipoint_xym (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    gaiaGeomCollPtr geom = NULL;

    /* If no pointers are given, return. */
    if (first == NULL)
	return NULL;

    /* Creates and allocates a geometry collection containing a multipoint. */
    geom = gaiaAllocGeomCollXYM ();
    if (geom == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_MULTIPOINT;

    /* For every 2D (xym) point, add it to the geometry collection. */
    while (p != NULL)
      {
	  gaiaAddPointToGeomCollXYM (geom, p->X, p->Y, p->M);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
      }
    return geom;
}

/* 
 * Creates a 3D (xyzm) multipoint object in SpatiaLite
 *
 * first is a gaiaPointPtr to the first point in a linked list of points which define the linestring.
 * All of the points given to the function are 3D (xyzm) points. There will be at least 1 point in the list.
 *
 * Returns a geometry collection containing the created multipoint object.
 */
static gaiaGeomCollPtr
vanuatu_multipoint_xyzm (struct vanuatu_data *p_data, gaiaPointPtr first)
{
    gaiaPointPtr p = first;
    gaiaPointPtr p_n;
    gaiaGeomCollPtr geom = NULL;

    /* If no pointers are given, return. */
    if (first == NULL)
	return NULL;

    /* Creates and allocates a geometry collection containing a multipoint. */
    geom = gaiaAllocGeomCollXYZM ();
    if (geom == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_MULTIPOINT;

    /* For every 3D (xyzm) point, add it to the geometry collection. */
    while (p != NULL)
      {
	  gaiaAddPointToGeomCollXYZM (geom, p->X, p->Y, p->Z, p->M);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePoint (p);
	  p = p_n;
      }
    return geom;
}

/*
 * Creates a geometry collection containing 2D (xy) linestrings.
 * Parameter first is a gaiaLinestringPtr to the first linestring in a linked list of linestrings which should be added to the
 * collection. All of the *linestrings in the list must be 2D (xy) linestrings. There must be at least 1 linestring in the list.
 * Returns a pointer to the created geometry collection of 2D linestrings. The geometry must have FirstLinestring pointing to the
 * first linestring in the list pointed by first and LastLinestring pointing to the last element of the same list. DimensionModel
 * must be GAIA_XY and DimensionType must be *GAIA_TYPE_LINESTRING.
 */

static gaiaGeomCollPtr
vanuatu_multilinestring_xy (struct vanuatu_data *p_data,
			    gaiaLinestringPtr first)
{
    gaiaLinestringPtr p = first;
    gaiaLinestringPtr p_n;
    gaiaLinestringPtr new_line;
    gaiaGeomCollPtr a = gaiaAllocGeomColl ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, a);
    a->DeclaredType = GAIA_MULTILINESTRING;
    a->DimensionModel = GAIA_XY;

    while (p)
      {
	  new_line = gaiaAddLinestringToGeomColl (a, p->Points);
	  gaiaCopyLinestringCoords (new_line, p);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreeLinestring (p);
	  p = p_n;
      }

    return a;
}

/* 
 * Returns a geometry collection containing the created multilinestring object (?).
 * Creates a geometry collection containing 3D (xyz) linestrings.
 * Parameter first is a gaiaLinestringPtr to the first linestring in a linked list of linestrings which should be added to the
 * collection. All of the *linestrings in the list must be 3D (xyz) linestrings. There must be at least 1 linestring in the list.
 * Returns a pointer to the created geometry collection of 3D linestrings. The geometry must have FirstLinestring pointing to the
 * first linestring in the *list pointed by first and LastLinestring pointing to the last element of the same list. DimensionModel
 * must be GAIA_XYZ and DimensionType must be *GAIA_TYPE_LINESTRING.
 */
static gaiaGeomCollPtr
vanuatu_multilinestring_xyz (struct vanuatu_data *p_data,
			     gaiaLinestringPtr first)
{
    gaiaLinestringPtr p = first;
    gaiaLinestringPtr p_n;
    gaiaLinestringPtr new_line;
    gaiaGeomCollPtr a = gaiaAllocGeomCollXYZ ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, a);
    a->DeclaredType = GAIA_MULTILINESTRING;
    a->DimensionModel = GAIA_XY_Z;

    while (p)
      {
	  new_line = gaiaAddLinestringToGeomColl (a, p->Points);
	  gaiaCopyLinestringCoords (new_line, p);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreeLinestring (p);
	  p = p_n;
      }
    return a;
}

/* 
 * Creates a geometry collection containing 2D (xy) with m value linestrings.
 * Parameter first is a gaiaLinestringPtr to the first linestring in a linked list of linestrings which should be added to the
 * collection. All of the  *linestrings in the list must be 2D (xy) with m value linestrings. There must be at least 1 linestring 
 * in the list.
 * Returns a pointer to the created geometry collection of 2D with m value linestrings. The geometry must have FirstLinestring
 * pointing to the first *linestring in the list pointed by first and LastLinestring pointing to the last element of the same list. 
 * DimensionModel must be GAIA_XYM and *DimensionType must be GAIA_TYPE_LINESTRING.
 */
static gaiaGeomCollPtr
vanuatu_multilinestring_xym (struct vanuatu_data *p_data,
			     gaiaLinestringPtr first)
{
    gaiaLinestringPtr p = first;
    gaiaLinestringPtr p_n;
    gaiaLinestringPtr new_line;
    gaiaGeomCollPtr a = gaiaAllocGeomCollXYM ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, a);
    a->DeclaredType = GAIA_MULTILINESTRING;
    a->DimensionModel = GAIA_XY_M;

    while (p)
      {
	  new_line = gaiaAddLinestringToGeomColl (a, p->Points);
	  gaiaCopyLinestringCoords (new_line, p);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreeLinestring (p);
	  p = p_n;
      }

    return a;
}

/* 
 * Creates a geometry collection containing 4D (xyz) with m value linestrings.
 * Parameter first is a gaiaLinestringPtr to the first linestring in a linked list of linestrings which should be added to the
 * collection. All of the *linestrings in the list must be 4D (xyz) with m value linestrings. There must be at least 1 linestring
 * in the list.
 * Returns a pointer to the created geometry collection of 4D with m value linestrings. The geometry must have FirstLinestring
 * pointing to the first *linestring in the list pointed by first and LastLinestring pointing to the last element of the same list. 
 * DimensionModel must be GAIA_XYZM and *DimensionType must be GAIA_TYPE_LINESTRING.
 */
static gaiaGeomCollPtr
vanuatu_multilinestring_xyzm (struct vanuatu_data *p_data,
			      gaiaLinestringPtr first)
{
    gaiaLinestringPtr p = first;
    gaiaLinestringPtr p_n;
    gaiaLinestringPtr new_line;
    gaiaGeomCollPtr a = gaiaAllocGeomCollXYZM ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, a);
    a->DeclaredType = GAIA_MULTILINESTRING;
    a->DimensionModel = GAIA_XY_Z_M;

    while (p)
      {
	  new_line = gaiaAddLinestringToGeomColl (a, p->Points);
	  gaiaCopyLinestringCoords (new_line, p);
	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreeLinestring (p);
	  p = p_n;
      }
    return a;
}

/* 
 * Creates a geometry collection containing 2D (xy) polygons.
 *
 * Parameter first is a gaiaPolygonPtr to the first polygon in a linked list of polygons which should 
 * be added to the collection. All of the polygons in the list must be 2D (xy) polygons. There must be 
 * at least 1 polygon in the list.
 *
 * Returns a pointer to the created geometry collection of 2D polygons. The geometry must have 
 * FirstPolygon pointing to the first polygon in the list pointed by first and LastPolygon pointing 
 * to the last element of the same list. DimensionModel must be GAIA_XY and DimensionType must 
 * be GAIA_TYPE_POLYGON.
 *
 */
static gaiaGeomCollPtr
vanuatu_multipolygon_xy (struct vanuatu_data *p_data, gaiaPolygonPtr first)
{
    gaiaPolygonPtr p = first;
    gaiaPolygonPtr p_n;
    int i = 0;
    gaiaPolygonPtr new_polyg;
    gaiaRingPtr i_ring;
    gaiaRingPtr o_ring;
    gaiaGeomCollPtr geom = gaiaAllocGeomColl ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);

    geom->DeclaredType = GAIA_MULTIPOLYGON;

    while (p)
      {
	  i_ring = p->Exterior;
	  new_polyg =
	      gaiaAddPolygonToGeomColl (geom, i_ring->Points, p->NumInteriors);
	  o_ring = new_polyg->Exterior;
	  gaiaCopyRingCoords (o_ring, i_ring);

	  for (i = 0; i < new_polyg->NumInteriors; i++)
	    {
		i_ring = p->Interiors + i;
		o_ring = gaiaAddInteriorRing (new_polyg, i, i_ring->Points);
		gaiaCopyRingCoords (o_ring, i_ring);
	    }

	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePolygon (p);
	  p = p_n;
      }

    return geom;
}

/* 
 * Creates a geometry collection containing 3D (xyz) polygons.
 *
 * Parameter first is a gaiaPolygonPtr to the first polygon in a linked list of polygons which should be 
 * added to the collection. All of the polygons in the list must be 3D (xyz) polygons. There must be at
 * least 1 polygon in the list.
 *
 * Returns a pointer to the created geometry collection of 3D polygons. The geometry must have 
 * FirstPolygon pointing to the first polygon in the list pointed by first and LastPolygon pointing to 
 * the last element of the same list. DimensionModel must be GAIA_XYZ and DimensionType must 
 * be GAIA_TYPE_POLYGON.
 *
 */
static gaiaGeomCollPtr
vanuatu_multipolygon_xyz (struct vanuatu_data *p_data, gaiaPolygonPtr first)
{
    gaiaPolygonPtr p = first;
    gaiaPolygonPtr p_n;
    int i = 0;
    gaiaPolygonPtr new_polyg;
    gaiaRingPtr i_ring;
    gaiaRingPtr o_ring;
    gaiaGeomCollPtr geom = gaiaAllocGeomCollXYZ ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);

    geom->DeclaredType = GAIA_MULTIPOLYGON;

    while (p)
      {
	  i_ring = p->Exterior;
	  new_polyg =
	      gaiaAddPolygonToGeomColl (geom, i_ring->Points, p->NumInteriors);
	  o_ring = new_polyg->Exterior;
	  gaiaCopyRingCoords (o_ring, i_ring);

	  for (i = 0; i < new_polyg->NumInteriors; i++)
	    {
		i_ring = p->Interiors + i;
		o_ring = gaiaAddInteriorRing (new_polyg, i, i_ring->Points);
		gaiaCopyRingCoords (o_ring, i_ring);
	    }

	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePolygon (p);
	  p = p_n;
      }

    return geom;
}

/* 
 * Creates a geometry collection containing 2D (xy) with m value polygons.
 *
 * Parameter first is a gaiaPolygonPtr to the first polygon in a linked list of polygons which should
 * be added to the collection. All of the polygons in the list must be 2D (xy) with m value polygons.
 * There must be at least 1 polygon in the list.
 *
 * Returns a pointer to the created geometry collection of 2D with m value polygons. The geometry 
 * must have FirstPolygon pointing to the first polygon in the list pointed by first and LastPolygon 
 * pointing to the last element of the same list. DimensionModel must be GAIA_XYM and DimensionType 
 * must be GAIA_TYPE_POLYGON.
 *
 */
static gaiaGeomCollPtr
vanuatu_multipolygon_xym (struct vanuatu_data *p_data, gaiaPolygonPtr first)
{
    gaiaPolygonPtr p = first;
    gaiaPolygonPtr p_n;
    int i = 0;
    gaiaPolygonPtr new_polyg;
    gaiaRingPtr i_ring;
    gaiaRingPtr o_ring;
    gaiaGeomCollPtr geom = gaiaAllocGeomCollXYM ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);

    geom->DeclaredType = GAIA_MULTIPOLYGON;

    while (p)
      {
	  i_ring = p->Exterior;
	  new_polyg =
	      gaiaAddPolygonToGeomColl (geom, i_ring->Points, p->NumInteriors);
	  o_ring = new_polyg->Exterior;
	  gaiaCopyRingCoords (o_ring, i_ring);

	  for (i = 0; i < new_polyg->NumInteriors; i++)
	    {
		i_ring = p->Interiors + i;
		o_ring = gaiaAddInteriorRing (new_polyg, i, i_ring->Points);
		gaiaCopyRingCoords (o_ring, i_ring);
	    }

	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePolygon (p);
	  p = p_n;
      }

    return geom;
}

/*
 * Creates a geometry collection containing 4D (xyz) with m value polygons.
 *
 * Parameter first is a gaiaPolygonPtr to the first polygon in a linked list of polygons which should be 
 * added to the collection. All of the polygons in the list must be 4D (xyz) with m value polygons. 
 * There must be at least 1 polygon in the list.
 *
 * Returns a pointer to the created geometry collection of 4D with m value polygons. The geometry must 
 * have FirstPolygon pointing to the first polygon in the list pointed by first and LastPolygon pointing 
//  * to the last element of the same list. DimensionModel must be GAIA_XYZM and DimensionType must 
 * be GAIA_TYPE_POLYGON.
 *
 */
static gaiaGeomCollPtr
vanuatu_multipolygon_xyzm (struct vanuatu_data *p_data, gaiaPolygonPtr first)
{
    gaiaPolygonPtr p = first;
    gaiaPolygonPtr p_n;
    int i = 0;
    gaiaPolygonPtr new_polyg;
    gaiaRingPtr i_ring;
    gaiaRingPtr o_ring;
    gaiaGeomCollPtr geom = gaiaAllocGeomCollXYZM ();
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);

    geom->DeclaredType = GAIA_MULTIPOLYGON;

    while (p)
      {
	  i_ring = p->Exterior;
	  new_polyg =
	      gaiaAddPolygonToGeomColl (geom, i_ring->Points, p->NumInteriors);
	  o_ring = new_polyg->Exterior;
	  gaiaCopyRingCoords (o_ring, i_ring);

	  for (i = 0; i < new_polyg->NumInteriors; i++)
	    {
		i_ring = p->Interiors + i;
		o_ring = gaiaAddInteriorRing (new_polyg, i, i_ring->Points);
		gaiaCopyRingCoords (o_ring, i_ring);
	    }

	  p_n = p->Next;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreePolygon (p);
	  p = p_n;
      }

    return geom;
}

static void
vanuatu_geomColl_common (struct vanuatu_data *p_data, gaiaGeomCollPtr org,
			 gaiaGeomCollPtr dst)
{
/* 
/ helper function: xfers entities between the Origin and Destination 
/ Sandro Furieri: 2010 October 12
*/
    gaiaGeomCollPtr p = org;
    gaiaGeomCollPtr p_n;
    gaiaPointPtr pt;
    gaiaPointPtr pt_n;
    gaiaLinestringPtr ln;
    gaiaLinestringPtr ln_n;
    gaiaPolygonPtr pg;
    gaiaPolygonPtr pg_n;
    while (p)
      {
	  pt = p->FirstPoint;
	  while (pt)
	    {
		pt_n = pt->Next;
		pt->Next = NULL;
		if (dst->FirstPoint == NULL)
		    dst->FirstPoint = pt;
		if (dst->LastPoint != NULL)
		    dst->LastPoint->Next = pt;
		dst->LastPoint = pt;
		pt = pt_n;
	    }
	  ln = p->FirstLinestring;
	  while (ln)
	    {
		ln_n = ln->Next;
		ln->Next = NULL;
		if (dst->FirstLinestring == NULL)
		    dst->FirstLinestring = ln;
		if (dst->LastLinestring != NULL)
		    dst->LastLinestring->Next = ln;
		dst->LastLinestring = ln;
		ln = ln_n;
	    }
	  pg = p->FirstPolygon;
	  while (pg)
	    {
		pg_n = pg->Next;
		pg->Next = NULL;
		if (dst->FirstPolygon == NULL)
		    dst->FirstPolygon = pg;
		if (dst->LastPolygon != NULL)
		    dst->LastPolygon->Next = pg;
		dst->LastPolygon = pg;
		pg = pg_n;
	    }
	  p_n = p->Next;
	  p->FirstPoint = NULL;
	  p->LastPoint = NULL;
	  p->FirstLinestring = NULL;
	  p->LastLinestring = NULL;
	  p->FirstPolygon = NULL;
	  p->LastPolygon = NULL;
	  vanuatuMapDynClean (p_data, p);
	  gaiaFreeGeomColl (p);
	  p = p_n;
      }
}

/* Creates a 2D (xy) geometry collection in SpatiaLite
 *
 * first is the first geometry collection in a linked list of geometry collections.
 * Each geometry collection represents a single type of object (eg. one could be a POINT, 
 * another could be a LINESTRING, another could be a MULTILINESTRING, etc.).
 *
 * The type of object represented by any geometry collection is stored in the declaredType 
 * field of its struct. For example, if first->declaredType = GAIA_POINT, then first represents a point.
 * If first->declaredType = GAIA_MULTIPOINT, then first represents a multipoint.
 *
 * NOTE: geometry collections cannot contain other geometry collections (have to confirm this 
 * with Sandro).
 *
 * The goal of this function is to take the information from all of the structs in the linked list and 
 * return one geomColl struct containing all of that information.
 *
 * The integers used for 'declaredType' are defined in gaiageo.h. In this function, the only values 
 * contained in 'declaredType' that will be encountered will be:
 *
 *	GAIA_POINT, GAIA_LINESTRING, GAIA_POLYGON, 
 *	GAIA_MULTIPOINT, GAIA_MULTILINESTRING, GAIA_MULTIPOLYGON
 */
static gaiaGeomCollPtr
vanuatu_geomColl_xy (struct vanuatu_data *p_data, gaiaGeomCollPtr first)
{
    gaiaGeomCollPtr geom = gaiaAllocGeomColl ();
    if (geom == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_GEOMETRYCOLLECTION;
    geom->DimensionModel = GAIA_XY;
    vanuatu_geomColl_common (p_data, first, geom);
    return geom;
}

/*
 * See geomColl_xy for description.
 *
 * The only difference between this function and geomColl_xy is that the 'declaredType' field of the structs
 * in the linked list for this function will only contain the following types:
 *
 *	GAIA_POINTZ, GAIA_LINESTRINGZ, GAIA_POLYGONZ,
 * 	GAIA_MULTIPOINTZ, GAIA_MULTILINESTRINGZ, GAIA_MULTIPOLYGONZ
 */
static gaiaGeomCollPtr
vanuatu_geomColl_xyz (struct vanuatu_data *p_data, gaiaGeomCollPtr first)
{
    gaiaGeomCollPtr geom = gaiaAllocGeomColl ();
    if (geom == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_GEOMETRYCOLLECTION;
    geom->DimensionModel = GAIA_XY_Z;
    vanuatu_geomColl_common (p_data, first, geom);
    return geom;
}

/*
 * See geomColl_xy for description.
 *
 * The only difference between this function and geomColl_xy is that the 'declaredType' field of the structs
 * in the linked list for this function will only contain the following types:
 *
 *	GAIA_POINTM, GAIA_LINESTRINGM, GAIA_POLYGONM,
 * 	GAIA_MULTIPOINTM, GAIA_MULTILINESTRINGM, GAIA_MULTIPOLYGONM
 */
static gaiaGeomCollPtr
vanuatu_geomColl_xym (struct vanuatu_data *p_data, gaiaGeomCollPtr first)
{
    gaiaGeomCollPtr geom = gaiaAllocGeomColl ();
    if (geom == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_GEOMETRYCOLLECTION;
    geom->DimensionModel = GAIA_XY_M;
    vanuatu_geomColl_common (p_data, first, geom);
    return geom;
}

/*
 * See geomColl_xy for description.
 *
 * The only difference between this function and geomColl_xy is that the 'declaredType' field of the structs
 * in the linked list for this function will only contain the following types:
 *
 *	GAIA_POINTZM, GAIA_LINESTRINGZM, GAIA_POLYGONZM,
 * 	GAIA_MULTIPOINTZM, GAIA_MULTILINESTRINGZM, GAIA_MULTIPOLYGONZM
 */
static gaiaGeomCollPtr
vanuatu_geomColl_xyzm (struct vanuatu_data *p_data, gaiaGeomCollPtr first)
{
    gaiaGeomCollPtr geom = gaiaAllocGeomColl ();
    if (geom == NULL)
	return NULL;
    vanuatuMapDynAlloc (p_data, VANUATU_DYN_GEOMETRY, geom);
    geom->DeclaredType = GAIA_GEOMETRYCOLLECTION;
    geom->DimensionModel = GAIA_XY_Z_M;
    vanuatu_geomColl_common (p_data, first, geom);
    return geom;
}


/* including LEMON generated code */
#include "vanuatuWkt.c"



/*
** CAVEAT: there is an incompatibility between LEMON and FLEX
** this macro resolves the issue
*/
#undef yy_accept
#define yy_accept	yy_vanuatu_flex_accept



/* including FLEX generated code */
#include "lex.VanuatuWkt.c"



/*
** This is a linked-list struct to store all the values for each token.
** All tokens will have a value of 0, except tokens denoted as NUM.
** NUM tokens are geometry coordinates and will contain the floating
** point number.
*/
typedef struct vanuatuFlexTokenStruct
{
    double value;
    struct vanuatuFlexTokenStruct *Next;
} vanuatuFlexToken;

/*
** Function to clean up the linked-list of token values.
*/
static int
vanuatu_cleanup (vanuatuFlexToken * token)
{
    vanuatuFlexToken *ptok;
    vanuatuFlexToken *ptok_n;
    if (token == NULL)
	return 0;
    ptok = token;
    while (ptok)
      {
	  ptok_n = ptok->Next;
	  free (ptok);
	  ptok = ptok_n;
      }
    return 0;
}

gaiaGeomCollPtr
gaiaParseWkt (const unsigned char *dirty_buffer, short type)
{
    void *pParser = ParseAlloc (malloc);
    /* Linked-list of token values */
    vanuatuFlexToken *tokens = malloc (sizeof (vanuatuFlexToken));
    /* Pointer to the head of the list */
    vanuatuFlexToken *head = tokens;
    int yv;
    yyscan_t scanner;
    struct vanuatu_data str_data;

/* initializing the helper structs */
    str_data.vanuatu_line = 1;
    str_data.vanuatu_col = 1;
    str_data.vanuatu_parse_error = 0;
    str_data.vanuatu_first_dyn_block = NULL;
    str_data.vanuatu_last_dyn_block = NULL;
    str_data.result = NULL;

/* initializing the scanner state */
    VanuatuWktlex_init_extra (&str_data, &scanner);

    tokens->Next = NULL;

    VanuatuWkt_scan_string ((char *) dirty_buffer, scanner);

    /*
       / Keep tokenizing until we reach the end
       / yylex() will return the next matching Token for us.
     */
    while ((yv = yylex (scanner)) != 0)
      {
	  if (yv == -1)
	    {
		str_data.vanuatu_parse_error = 1;
		break;
	    }
	  tokens->Next = malloc (sizeof (vanuatuFlexToken));
	  tokens->Next->Next = NULL;
	  tokens->Next->value = str_data.VanuatuWktlval.dval;
	  /* Pass the token to the wkt parser created from lemon */
	  Parse (pParser, yv, &(tokens->Next->value), &str_data);
	  tokens = tokens->Next;
      }
    /* This denotes the end of a line as well as the end of the parser */
    Parse (pParser, VANUATU_NEWLINE, 0, &str_data);
    ParseFree (pParser, free);
    VanuatuWktlex_destroy (scanner);

    /* Assigning the token as the end to avoid seg faults while cleaning */
    tokens->Next = NULL;
    vanuatu_cleanup (head);

    /*
     ** Sandro Furieri 2010 Apr 4
     ** checking if any parsing error was encountered 
     */
    if (str_data.vanuatu_parse_error)
      {
	  if (str_data.result != NULL)
	    {
		/* if a Geometry-result has been produced, the stack is already cleaned */
		gaiaFreeGeomColl (str_data.result);
		vanuatuCleanMapDynAlloc (&str_data, 0);
	    }
	  else
	    {
		/* otherwise we are required to clean the stack */
		vanuatuCleanMapDynAlloc (&str_data, 1);
	    }
	  return NULL;
      }

    vanuatuCleanMapDynAlloc (&str_data, 0);
    /*
     ** Sandro Furieri 2010 Apr 4
     ** final checkup for validity
     */
    if (str_data.result == NULL)
	return NULL;
    if (!vanuatuCheckValidity (str_data.result))
      {
	  gaiaFreeGeomColl (str_data.result);
	  return NULL;
      }
    if (type < 0)
	;			/* no restrinction about GEOMETRY CLASS TYPE */
    else
      {
	  if (str_data.result->DeclaredType != type)
	    {
		/* invalid CLASS TYPE for request */
		gaiaFreeGeomColl (str_data.result);
		return NULL;
	    }
      }

    gaiaMbrGeometry (str_data.result);

    return str_data.result;
}

/******************************************************************************
** This is the end of the code that was created by Team Vanuatu 
** of The University of Toronto.

Authors:
Ruppi Rana			ruppi.rana@gmail.com
Dev Tanna			dev.tanna@gmail.com
Elias Adum			elias.adum@gmail.com
Benton Hui			benton.hui@gmail.com
Abhayan Sundararajan		abhayan@gmail.com
Chee-Lun Michael Stephen Cho	cheelun.cho@gmail.com
Nikola Banovic			nikola.banovic@gmail.com
Yong Jian			yong.jian@utoronto.ca

Supervisor:
Greg Wilson			gvwilson@cs.toronto.ca

-------------------------------------------------------------------------------

*/

/*
** CAVEAT: we must now undefine any Lemon/Flex own macro
*/
#undef YYNOCODE
#undef YYNSTATE
#undef YYNRULE
#undef YY_SHIFT_MAX
#undef YY_SHIFT_USE_DFLT
#undef YY_REDUCE_USE_DFLT
#undef YY_REDUCE_MAX
#undef YY_FLUSH_BUFFER
#undef YY_DO_BEFORE_ACTION
#undef YY_NUM_RULES
#undef YY_END_OF_BUFFER
#undef YY_END_FILE
#undef YYACTIONTYPE
#undef YY_SZ_ACTTAB
#undef YY_NEW_FILE
#undef BEGIN
#undef YY_START
#undef YY_CURRENT_BUFFER
#undef YY_CURRENT_BUFFER_LVALUE
#undef YY_STATE_BUF_SIZE
#undef YY_DECL
#undef YY_FATAL_ERROR
#undef YYMINORTYPE
#undef YY_CHAR
#undef YYSTYPE
#undef input
#undef ParseAlloc
#undef ParseFree
#undef ParseStackPeak
#undef Parse
#undef yyalloc
#undef yyfree
#undef yyin
#undef yyleng
#undef yyless
#undef yylex
#undef yylineno
#undef yyout
#undef yyrealloc
#undef yyrestart
#undef yyStackEntry
#undef yytext
#undef yywrap
#undef yyzerominor
#undef yy_accept
#undef yy_action
#undef yy_base
#undef yy_buffer_stack
#undef yy_buffer_stack_max
#undef yy_buffer_stack_top
#undef yy_c_buf_p
#undef yy_chk
#undef yy_create_buffer
#undef yy_def
#undef yy_default
#undef yy_delete_buffer
#undef yy_destructor
#undef yy_ec
#undef yy_fatal_error
#undef yy_find_reduce_action
#undef yy_find_shift_action
#undef yy_flex_debug
#undef yy_flush_buffer
#undef yy_get_next_buffer
#undef yy_get_previous_state
#undef yy_init
#undef yy_init_buffer
#undef yy_init_globals
#undef yy_load_buffer
#undef yy_load_buffer_state
#undef yy_lookahead
#undef yy_meta
#undef yy_new_buffer
#undef yy_nxt
#undef yy_parse_failed
#undef yy_pop_parser_stack
#undef yy_reduce
#undef yy_reduce_ofst
#undef yy_set_bol
#undef yy_set_interactive
#undef yy_shift
#undef yy_shift_ofst
#undef yy_start
#undef yy_state_type
#undef yy_switch_to_buffer
#undef yy_syntax_error
#undef yy_trans_info
#undef yy_try_NUL_trans
#undef yyParser
#undef yyStackEntry
#undef yyStackOverflow
#undef yyRuleInfo
#undef yytext_ptr
#undef yyunput
#undef yyzerominor
#undef ParseARG_SDECL
#undef ParseARG_PDECL
#undef ParseARG_FETCH
#undef ParseARG_STORE
#undef REJECT
#undef yymore
#undef YY_MORE_ADJ
#undef YY_RESTORE_YY_MORE_OFFSET
#undef YY_LESS_LINENO
#undef yyTracePrompt
#undef yyTraceFILE
#undef yyTokenName
#undef yyRuleName
#undef ParseTrace

#undef yylex
#undef YY_DECL
