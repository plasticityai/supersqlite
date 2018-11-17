/*

 gg_kml.c -- KML parser/lexer 
  
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
 
Portions created by the Initial Developer are Copyright (C) 2011-2015
the Initial Developer. All Rights Reserved.

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

#define KML_PARSER_OPEN_NODE		1
#define KML_PARSER_SELF_CLOSED_NODE	2
#define KML_PARSER_CLOSED_NODE		3

#define GAIA_KML_UNKNOWN		0
#define GAIA_KML_POINT			1
#define GAIA_KML_LINESTRING		2
#define GAIA_KML_POLYGON		3
#define GAIA_KML_MULTIGEOMETRY		4

#define KML_DYN_NONE	0
#define KML_DYN_DYNLINE	1
#define KML_DYN_GEOM	2
#define KML_DYN_DYNPG	3
#define KML_DYN_NODE	4
#define KML_DYN_COORD	5
#define KML_DYN_ATTRIB	6

#define KML_DYN_BLOCK 1024



/*
** CAVEAT: we must redefine any Lemon/Flex own macro
*/
#define YYMINORTYPE		KML_MINORTYPE
#define YY_CHAR			KML_YY_CHAR
#define	input			kml_input
#define ParseAlloc		kmlParseAlloc
#define ParseFree		kmlParseFree
#define ParseStackPeak		kmlParseStackPeak
#define Parse			kmlParse
#define yyStackEntry		kml_yyStackEntry
#define yyzerominor		kml_yyzerominor
#define yy_accept		kml_yy_accept
#define yy_action		kml_yy_action
#define yy_base			kml_yy_base
#define yy_buffer_stack		kml_yy_buffer_stack
#define yy_buffer_stack_max	kml_yy_buffer_stack_max
#define yy_buffer_stack_top	kml_yy_buffer_stack_top
#define yy_c_buf_p		kml_yy_c_buf_p
#define yy_chk			kml_yy_chk
#define yy_def			kml_yy_def
#define yy_default		kml_yy_default
#define yy_destructor		kml_yy_destructor
#define yy_ec			kml_yy_ec
#define yy_fatal_error		kml_yy_fatal_error
#define yy_find_reduce_action	kml_yy_find_reduce_action
#define yy_find_shift_action	kml_yy_find_shift_action
#define yy_get_next_buffer	kml_yy_get_next_buffer
#define yy_get_previous_state	kml_yy_get_previous_state
#define yy_init			kml_yy_init
#define yy_init_globals		kml_yy_init_globals
#define yy_lookahead		kml_yy_lookahead
#define yy_meta			kml_yy_meta
#define yy_nxt			kml_yy_nxt
#define yy_parse_failed		kml_yy_parse_failed
#define yy_pop_parser_stack	kml_yy_pop_parser_stack
#define yy_reduce		kml_yy_reduce
#define yy_reduce_ofst		kml_yy_reduce_ofst
#define yy_shift		kml_yy_shift
#define yy_shift_ofst		kml_yy_shift_ofst
#define yy_start		kml_yy_start
#define yy_state_type		kml_yy_state_type
#define yy_syntax_error		kml_yy_syntax_error
#define yy_trans_info		kml_yy_trans_info
#define yy_try_NUL_trans	kml_yy_try_NUL_trans
#define yyParser		kml_yyParser
#define yyStackEntry		kml_yyStackEntry
#define yyStackOverflow		kml_yyStackOverflow
#define yyRuleInfo		kml_yyRuleInfo
#define yyunput			kml_yyunput
#define yyzerominor		kml_yyzerominor
#define yyTraceFILE		kml_yyTraceFILE
#define yyTracePrompt		kml_yyTracePrompt
#define yyTokenName		kml_yyTokenName
#define yyRuleName		kml_yyRuleName
#define ParseTrace		kml_ParseTrace

#define yylex			kml_yylex
#define YY_DECL int yylex (yyscan_t yyscanner)


/* including LEMON generated header */
#include "Kml.h"


typedef union
{
    char *pval;
    struct symtab *symp;
} kml_yystype;
#define YYSTYPE kml_yystype


/*
** This is a linked-list struct to store all the values for each token.
*/
typedef struct kmlFlexTokenStruct
{
    char *value;
    struct kmlFlexTokenStruct *Next;
} kmlFlexToken;

typedef struct kml_coord
{
    char *Value;
    struct kml_coord *Next;
} kmlCoord;

typedef kmlCoord *kmlCoordPtr;

typedef struct kml_attr
{
    char *Key;
    char *Value;
    struct kml_attr *Next;
} kmlAttr;
typedef kmlAttr *kmlAttrPtr;

typedef struct kml_node
{
    char *Tag;
    int Type;
    int Error;
    struct kml_attr *Attributes;
    struct kml_coord *Coordinates;
    struct kml_node *Next;
} kmlNode;
typedef kmlNode *kmlNodePtr;

typedef struct kml_dynamic_ring
{
    gaiaDynamicLinePtr ring;
    int interior;
    int has_z;
    struct kml_dynamic_ring *next;
} kmlDynamicRing;
typedef kmlDynamicRing *kmlDynamicRingPtr;

typedef struct kml_dynamic_polygon
{
    struct kml_dynamic_ring *first;
    struct kml_dynamic_ring *last;
} kmlDynamicPolygon;
typedef kmlDynamicPolygon *kmlDynamicPolygonPtr;

struct kml_dyn_block
{
/* a struct taking trace of dynamic allocations */
    int type[KML_DYN_BLOCK];
    void *ptr[KML_DYN_BLOCK];
    int index;
    struct kml_dyn_block *next;
};

struct kml_data
{
/* a struct used to make the lexer-parser reentrant and thread-safe */
    int kml_parse_error;
    int kml_line;
    int kml_col;
    struct kml_dyn_block *kml_first_dyn_block;
    struct kml_dyn_block *kml_last_dyn_block;
    kmlNodePtr result;
    YYSTYPE KmlLval;
};

static struct kml_dyn_block *
kmlCreateDynBlock (void)
{
/* allocating a new block to trace dynamic allocations */
    int i;
    struct kml_dyn_block *p = malloc (sizeof (struct kml_dyn_block));
    for (i = 0; i < KML_DYN_BLOCK; i++)
      {
	  /* initializing map entries */
	  p->type[i] = KML_DYN_NONE;
	  p->ptr[i] = NULL;
      }
    p->index = 0;
    p->next = NULL;
    return p;
}

static void
kmlMapDynAlloc (struct kml_data *p_data, int type, void *ptr)
{
/* appending a dynamic allocation into the map */
    struct kml_dyn_block *p;
    if (p_data->kml_first_dyn_block == NULL)
      {
	  /* inserting the first block of the map */
	  p = kmlCreateDynBlock ();
	  p_data->kml_first_dyn_block = p;
	  p_data->kml_last_dyn_block = p;
      }
    if (p_data->kml_last_dyn_block->index >= KML_DYN_BLOCK)
      {
	  /* adding a further block to the map */
	  p = kmlCreateDynBlock ();
	  p_data->kml_last_dyn_block->next = p;
	  p_data->kml_last_dyn_block = p;
      }
    p_data->kml_last_dyn_block->type[p_data->kml_last_dyn_block->index] = type;
    p_data->kml_last_dyn_block->ptr[p_data->kml_last_dyn_block->index] = ptr;
    p_data->kml_last_dyn_block->index++;
}

static void
kmlMapDynClean (struct kml_data *p_data, void *ptr)
{
/* deleting a dynamic allocation from the map */
    int i;
    struct kml_dyn_block *p = p_data->kml_first_dyn_block;
    while (p)
      {
	  for (i = 0; i < KML_DYN_BLOCK; i++)
	    {
		switch (p->type[i])
		  {
		  case KML_DYN_DYNLINE:
		  case KML_DYN_GEOM:
		  case KML_DYN_DYNPG:
		  case KML_DYN_NODE:
		  case KML_DYN_COORD:
		  case KML_DYN_ATTRIB:
		      if (p->ptr[i] == ptr)
			{
			    p->type[i] = KML_DYN_NONE;
			    return;
			}
		      break;
		  };
	    }
	  p = p->next;
      }
}

static void
kml_free_dyn_polygon (kmlDynamicPolygonPtr dyn)
{
/* deleting a dynamic polygon (ring collection) */
    kmlDynamicRingPtr r;
    kmlDynamicRingPtr rn;
    if (!dyn)
	return;
    r = dyn->first;
    while (r)
      {
	  rn = r->next;
	  if (r->ring)
	      gaiaFreeDynamicLine (r->ring);
	  free (r);
	  r = rn;
      }
    free (dyn);
}

static void
kml_free_coord (kmlCoordPtr c)
{
/* deleting a KML coordinate */
    if (c == NULL)
	return;
    if (c->Value)
	free (c->Value);
    free (c);
}

static void
kml_free_attrib (kmlAttrPtr a)
{
/* deleting a KML attribute */
    if (a == NULL)
	return;
    if (a->Key)
	free (a->Key);
    if (a->Value)
	free (a->Value);
    free (a);
}

static void
kml_free_node (kmlNodePtr n)
{
/* deleting a KML node */
    kmlAttrPtr a;
    kmlAttrPtr an;
    kmlCoordPtr c;
    kmlCoordPtr cn;
    if (n == NULL)
	return;
    a = n->Attributes;
    while (a)
      {
	  an = a->Next;
	  kml_free_attrib (a);
	  a = an;
      }
    c = n->Coordinates;
    while (c)
      {
	  cn = c->Next;
	  kml_free_coord (c);
	  c = cn;
      }
    if (n->Tag)
	free (n->Tag);
    free (n);
}

static void
kmlCleanMapDynAlloc (struct kml_data *p_data, int clean_all)
{
/* cleaning the dynamic allocations map */
    int i;
    struct kml_dyn_block *pn;
    struct kml_dyn_block *p = p_data->kml_first_dyn_block;
    while (p)
      {
	  if (clean_all)
	    {
		for (i = 0; i < KML_DYN_BLOCK; i++)
		  {
		      /* deleting Geometry objects */
		      switch (p->type[i])
			{
			case KML_DYN_DYNLINE:
			    gaiaFreeDynamicLine ((gaiaDynamicLinePtr)
						 (p->ptr[i]));
			    break;
			case KML_DYN_GEOM:
			    gaiaFreeGeomColl ((gaiaGeomCollPtr) (p->ptr[i]));
			    break;
			case KML_DYN_DYNPG:
			    kml_free_dyn_polygon ((kmlDynamicPolygonPtr)
						  (p->ptr[i]));
			    break;
			case KML_DYN_NODE:
			    kml_free_node ((kmlNodePtr) (p->ptr[i]));
			    break;
			case KML_DYN_COORD:
			    kml_free_coord ((kmlCoordPtr) (p->ptr[i]));
			    break;
			case KML_DYN_ATTRIB:
			    kml_free_attrib ((kmlAttrPtr) (p->ptr[i]));
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

static kmlDynamicPolygonPtr
kml_alloc_dyn_polygon (struct kml_data *p_data)
{
/* creating a dynamic polygon (ring collection) */
    kmlDynamicPolygonPtr p = malloc (sizeof (kmlDynamicPolygon));
    kmlMapDynAlloc (p_data, KML_DYN_DYNPG, p);
    p->first = NULL;
    p->last = NULL;
    return p;
}

static void
kml_add_polygon_ring (kmlDynamicPolygonPtr dyn_pg, gaiaDynamicLinePtr dyn,
		      int interior, int has_z)
{
/* inserting a further ring into the collection (dynamic polygon) */
    kmlDynamicRingPtr p = malloc (sizeof (kmlDynamicRing));
    p->ring = dyn;
    p->interior = interior;
    p->has_z = has_z;
    p->next = NULL;
    if (dyn_pg->first == NULL)
	dyn_pg->first = p;
    if (dyn_pg->last != NULL)
	dyn_pg->last->next = p;
    dyn_pg->last = p;
}

static void
kml_freeString (char **ptr)
{
/* releasing a string from the lexer */
    if (*ptr != NULL)
	free (*ptr);
    *ptr = NULL;
}

static void
kml_saveString (char **ptr, const char *str)
{
/* saving a string from the lexer */
    int len = strlen (str);
    kml_freeString (ptr);
    *ptr = malloc (len + 1);
    strcpy (*ptr, str);
}

static kmlCoordPtr
kml_coord (struct kml_data *p_data, void *value)
{
/* creating a coord Item */
    int len;
    kmlFlexToken *tok = (kmlFlexToken *) value;
    kmlCoordPtr c = malloc (sizeof (kmlCoord));
    kmlMapDynAlloc (p_data, KML_DYN_COORD, c);
    len = strlen (tok->value);
    c->Value = malloc (len + 1);
    strcpy (c->Value, tok->value);
    c->Next = NULL;
    return c;
}

static kmlAttrPtr
kml_attribute (struct kml_data *p_data, void *key, void *value)
{
/* creating an attribute */
    int len;
    kmlFlexToken *k_tok = (kmlFlexToken *) key;
    kmlFlexToken *v_tok = (kmlFlexToken *) value;
    kmlAttrPtr a = malloc (sizeof (kmlAttr));
    kmlMapDynAlloc (p_data, KML_DYN_ATTRIB, a);
    len = strlen (k_tok->value);
    a->Key = malloc (len + 1);
    strcpy (a->Key, k_tok->value);
    len = strlen (v_tok->value);
/* we need to de-quote the string, removing first and last ".." */
    if (*(v_tok->value + 0) == '"' && *(v_tok->value + len - 1) == '"')
      {
	  a->Value = malloc (len - 1);
	  memcpy (a->Value, v_tok->value + 1, len - 1);
	  *(a->Value + len - 1) = '\0';
      }
    else
      {
	  a->Value = malloc (len + 1);
	  strcpy (a->Value, v_tok->value);
      }
    a->Next = NULL;
    return a;
}

static void
kml_freeTree (kmlNodePtr t)
{
/* deleting a KML tree */
    kmlNodePtr n;
    kmlNodePtr nn;
    n = t;
    while (n)
      {
	  nn = n->Next;
	  kml_free_node (n);
	  n = nn;
      }
}

static kmlNodePtr
kml_createNode (struct kml_data *p_data, void *tag, void *attributes,
		void *coords)
{
/* creating a node */
    kmlAttrPtr a;
    kmlCoordPtr c;
    int len;
    kmlFlexToken *tok = (kmlFlexToken *) tag;
    kmlNodePtr n = malloc (sizeof (kmlNode));
    kmlMapDynAlloc (p_data, KML_DYN_NODE, n);
    len = strlen (tok->value);
    n->Tag = malloc (len + 1);
    strcpy (n->Tag, tok->value);
    n->Type = KML_PARSER_OPEN_NODE;
    n->Error = 0;
    a = (kmlAttrPtr) attributes;
    while (a)
      {
	  /* transferring ownership of attributes */
	  kmlMapDynClean (p_data, a);
	  a = a->Next;
      }
    n->Attributes = attributes;
    c = (kmlCoordPtr) coords;
    while (c)
      {
	  /* transferring ownership of attributes */
	  kmlMapDynClean (p_data, c);
	  c = c->Next;
      }
    n->Coordinates = coords;
    n->Next = NULL;
    return n;
}

static kmlNodePtr
kml_createSelfClosedNode (struct kml_data *p_data, void *tag, void *attributes)
{
/* creating a self-closed node */
    kmlAttrPtr a;
    int len;
    kmlFlexToken *tok = (kmlFlexToken *) tag;
    kmlNodePtr n = malloc (sizeof (kmlNode));
    kmlMapDynAlloc (p_data, KML_DYN_NODE, n);
    len = strlen (tok->value);
    n->Tag = malloc (len + 1);
    strcpy (n->Tag, tok->value);
    n->Type = KML_PARSER_SELF_CLOSED_NODE;
    n->Error = 0;
    a = (kmlAttrPtr) attributes;
    while (a)
      {
	  /* transferring ownership of attributes */
	  kmlMapDynClean (p_data, a);
	  a = a->Next;
      }
    n->Attributes = attributes;
    n->Coordinates = NULL;
    n->Next = NULL;
    return n;
}

static kmlNodePtr
kml_closingNode (struct kml_data *p_data, void *tag)
{
/* creating a closing node */
    int len;
    kmlFlexToken *tok = (kmlFlexToken *) tag;
    kmlNodePtr n = malloc (sizeof (kmlNode));
    kmlMapDynAlloc (p_data, KML_DYN_NODE, n);
    len = strlen (tok->value);
    n->Tag = malloc (len + 1);
    strcpy (n->Tag, tok->value);
    n->Type = KML_PARSER_CLOSED_NODE;
    n->Error = 0;
    n->Attributes = NULL;
    n->Coordinates = NULL;
    n->Next = NULL;
    return n;
}

static int
kml_cleanup (kmlFlexToken * token)
{
    kmlFlexToken *ptok;
    kmlFlexToken *ptok_n;
    if (token == NULL)
	return 0;
    ptok = token;
    while (ptok)
      {
	  ptok_n = ptok->Next;
	  if (ptok->value != NULL)
	      free (ptok->value);
	  free (ptok);
	  ptok = ptok_n;
      }
    return 0;
}

static void
kml_xferString (char **p, const char *str)
{
/* saving some token */
    int len;
    if (str == NULL)
      {
	  *p = NULL;
	  return;
      }
    len = strlen (str);
    *p = malloc (len + 1);
    strcpy (*p, str);
}

static int
guessKmlGeometryType (kmlNodePtr node)
{
/* attempting to guess the Geometry Type for a KML node */
    int type = GAIA_KML_UNKNOWN;
    if (strcmp (node->Tag, "Point") == 0)
	type = GAIA_KML_POINT;
    if (strcmp (node->Tag, "LineString") == 0)
	type = GAIA_KML_LINESTRING;
    if (strcmp (node->Tag, "Polygon") == 0)
	type = GAIA_KML_POLYGON;
    if (strcmp (node->Tag, "MultiGeometry") == 0)
	type = GAIA_KML_MULTIGEOMETRY;
    return type;
}

static int
kml_check_coord (const char *value)
{
/* checking a KML coordinate */
    int decimal = 0;
    const char *p = value;
    if (*p == '+' || *p == '-')
	p++;
    while (*p != '\0')
      {
	  if (*p == '.')
	    {
		if (!decimal)
		    decimal = 1;
		else
		    return 0;
	    }
	  else if (*p >= '0' && *p <= '9')
	      ;
	  else
	      return 0;
	  p++;
      }
    return 1;
}

static int
kml_extract_coords (const char *value, double *x, double *y, double *z,
		    int *count)
{
/* extracting KML coords from a comma-separated string */
    const char *in = value;
    char buf[1024];
    char *out = buf;
    *out = '\0';

    while (*in != '\0')
      {
	  if (*in == ',')
	    {
		*out = '\0';
		if (*buf != '\0')
		  {
		      if (!kml_check_coord (buf))
			  return 0;
		      switch (*count)
			{
			case 0:
			    *x = atof (buf);
			    *count += 1;
			    break;
			case 1:
			    *y = atof (buf);
			    *count += 1;
			    break;
			case 2:
			    *z = atof (buf);
			    *count += 1;
			    break;
			default:
			    *count += 1;
			    break;
			};
		  }
		in++;
		out = buf;
		*out = '\0';
		continue;
	    }
	  *out++ = *in++;
      }
    *out = '\0';
/* parsing the last item */
    if (*buf != '\0')
      {
	  if (!kml_check_coord (buf))
	      return 0;
	  switch (*count)
	    {
	    case 0:
		*x = atof (buf);
		*count += 1;
		break;
	    case 1:
		*y = atof (buf);
		*count += 1;
		break;
	    case 2:
		*z = atof (buf);
		*count += 1;
		break;
	    default:
		*count += 1;
		break;
	    };
      }
    return 1;
}

static int
kml_parse_point_v2 (kmlCoordPtr coord, double *x, double *y, double *z,
		    int *has_z)
{
/* parsing KML <coordinates> [Point] */
    int count = 0;
    kmlCoordPtr c = coord;
    while (c)
      {
	  if (!kml_extract_coords (c->Value, x, y, z, &count))
	      return 0;
	  c = c->Next;
      }
    if (count == 2)
      {
	  *has_z = 0;
	  return 1;
      }
    if (count == 3)
      {
	  *has_z = 1;
	  return 1;
      }
    return 0;
}

static int
kml_parse_point (struct kml_data *p_data, gaiaGeomCollPtr geom, kmlNodePtr node,
		 kmlNodePtr * next)
{
/* parsing a <Point> */
    double x;
    double y;
    double z;
    int has_z;
    gaiaGeomCollPtr pt;
    gaiaGeomCollPtr last;

    if (strcmp (node->Tag, "coordinates") == 0)
      {
	  /* parsing a KML <Point> */
	  if (!kml_parse_point_v2 (node->Coordinates, &x, &y, &z, &has_z))
	      return 0;
	  node = node->Next;
	  if (node == NULL)
	      return 0;
	  if (strcmp (node->Tag, "coordinates") == 0)
	      ;
	  else
	      return 0;
	  node = node->Next;
	  if (node == NULL)
	      return 0;
	  if (strcmp (node->Tag, "Point") == 0)
	      ;
	  else
	      return 0;
	  *next = node->Next;
	  goto ok;
      }
    return 0;

  ok:
/* ok, KML nodes match as expected */
    if (has_z)
      {
	  pt = gaiaAllocGeomCollXYZ ();
	  kmlMapDynAlloc (p_data, KML_DYN_GEOM, pt);
	  gaiaAddPointToGeomCollXYZ (pt, x, y, z);
      }
    else
      {
	  pt = gaiaAllocGeomColl ();
	  kmlMapDynAlloc (p_data, KML_DYN_GEOM, pt);
	  gaiaAddPointToGeomColl (pt, x, y);
      }
    last = geom;
    while (1)
      {
	  /* searching the last Geometry within chain */
	  if (last->Next == NULL)
	      break;
	  last = last->Next;
      }
    last->Next = pt;
    return 1;
}

static int
kml_extract_multi_coord (const char *value, double *x, double *y, double *z,
			 int *count, int *follow)
{
/* extracting KML coords from a comma-separated string */
    const char *in = value;
    char buf[1024];
    char *out = buf;
    int last = 0;
    *out = '\0';
    while (*in != '\0')
      {
	  last = *in;
	  if (*in == ',')
	    {
		*out = '\0';
		if (*buf != '\0')
		  {
		      if (!kml_check_coord (buf))
			  return 0;
		      switch (*count)
			{
			case 0:
			    *x = atof (buf);
			    *count += 1;
			    break;
			case 1:
			    *y = atof (buf);
			    *count += 1;
			    break;
			case 2:
			    *z = atof (buf);
			    *count += 1;
			    break;
			default:
			    *count += 1;
			    break;
			};
		  }
		in++;
		out = buf;
		*out = '\0';
		continue;
	    }
	  *out++ = *in++;
      }
    *out = '\0';
/* parsing the last item */
    if (*buf != '\0')
      {
	  if (!kml_check_coord (buf))
	      return 0;
	  switch (*count)
	    {
	    case 0:
		*x = atof (buf);
		*count += 1;
		break;
	    case 1:
		*y = atof (buf);
		*count += 1;
		break;
	    case 2:
		*z = atof (buf);
		*count += 1;
		break;
	    default:
		*count += 1;
		break;
	    };
      }
    if (last == ',')
	*follow = 1;
    else
	*follow = 0;
    return 1;
}

static int
kml_extract_multi_coords (kmlCoordPtr coord, double *x, double *y, double *z,
			  int *count, kmlCoordPtr * next)
{
/* extracting KML coords from a comma-separated string */
    int follow;
    kmlCoordPtr c = coord;
    while (c)
      {
	  if (!kml_extract_multi_coord (c->Value, x, y, z, count, &follow))
	      return 0;
	  if (!follow && c->Next != NULL)
	    {
		if (*(c->Next->Value) == ',')
		    follow = 1;
	    }
	  if (follow)
	      c = c->Next;
	  else
	    {
		*next = c->Next;
		break;
	    }
      }
    return 1;
}

static void
kml_add_point_to_line (gaiaDynamicLinePtr dyn, double x, double y)
{
/* appending a point */
    gaiaAppendPointToDynamicLine (dyn, x, y);
}

static void
kml_add_point_to_lineZ (gaiaDynamicLinePtr dyn, double x, double y, double z)
{
/* appending a point */
    gaiaAppendPointZToDynamicLine (dyn, x, y, z);
}

static int
kml_parse_coordinates (kmlCoordPtr coord, gaiaDynamicLinePtr dyn, int *has_z)
{
/* parsing KML <coordinates> [Linestring or Ring] */
    int count = 0;
    double x;
    double y;
    double z;
    kmlCoordPtr next = NULL;
    kmlCoordPtr c = coord;
    while (c)
      {
	  if (!kml_extract_multi_coords (c, &x, &y, &z, &count, &next))
	      return 0;
	  if (count == 2)
	    {
		*has_z = 0;
		kml_add_point_to_line (dyn, x, y);
		count = 0;
	    }
	  else if (count == 3)
	    {
		kml_add_point_to_lineZ (dyn, x, y, z);
		count = 0;
	    }
	  else
	      return 0;
	  c = next;
      }
    return 1;
}

static int
kml_count_dyn_points (gaiaDynamicLinePtr dyn)
{
/* count how many vertices are into sone linestring/ring */
    int iv = 0;
    gaiaPointPtr pt = dyn->First;
    while (pt)
      {
	  iv++;
	  pt = pt->Next;
      }
    return iv;
}

static int
kml_parse_linestring (struct kml_data *p_data, gaiaGeomCollPtr geom,
		      kmlNodePtr node, kmlNodePtr * next)
{
/* parsing a <LineString> */
    gaiaGeomCollPtr ln;
    gaiaGeomCollPtr last;
    gaiaLinestringPtr new_ln;
    gaiaPointPtr pt;
    int iv;
    int has_z = 1;
    int points = 0;
    gaiaDynamicLinePtr dyn = gaiaAllocDynamicLine ();
    kmlMapDynAlloc (p_data, KML_DYN_DYNLINE, dyn);

    if (strcmp (node->Tag, "coordinates") == 0)
      {
	  /* parsing a KML <LineString> */
	  if (!kml_parse_coordinates (node->Coordinates, dyn, &has_z))
	      goto error;
	  node = node->Next;
	  if (node == NULL)
	      goto error;
	  if (strcmp (node->Tag, "coordinates") == 0)
	      ;
	  else
	      goto error;
	  node = node->Next;
	  if (node == NULL)
	      goto error;
	  if (strcmp (node->Tag, "LineString") == 0)
	      ;
	  else
	      goto error;
	  *next = node->Next;
      }

/* ok, KML nodes match as expected */
    points = kml_count_dyn_points (dyn);
    if (points < 2)
	goto error;
    if (has_z)
      {
	  ln = gaiaAllocGeomCollXYZ ();
	  kmlMapDynAlloc (p_data, KML_DYN_GEOM, ln);
	  new_ln = gaiaAddLinestringToGeomColl (ln, points);
	  pt = dyn->First;
	  iv = 0;
	  while (pt)
	    {
		gaiaSetPointXYZ (new_ln->Coords, iv, pt->X, pt->Y, pt->Z);
		iv++;
		pt = pt->Next;
	    }
      }
    else
      {
	  ln = gaiaAllocGeomColl ();
	  kmlMapDynAlloc (p_data, KML_DYN_GEOM, ln);
	  new_ln = gaiaAddLinestringToGeomColl (ln, points);
	  pt = dyn->First;
	  iv = 0;
	  while (pt)
	    {
		gaiaSetPoint (new_ln->Coords, iv, pt->X, pt->Y);
		iv++;
		pt = pt->Next;
	    }
      }
    last = geom;
    while (1)
      {
	  /* searching the last Geometry within chain */
	  if (last->Next == NULL)
	      break;
	  last = last->Next;
      }
    last->Next = ln;
    gaiaFreeDynamicLine (dyn);
    return 1;

  error:
    gaiaFreeDynamicLine (dyn);
    return 0;
}

static gaiaDynamicLinePtr
kml_parse_ring (kmlNodePtr node, int *interior, int *has_z, kmlNodePtr * next)
{
/* parsing a generic KML ring */
    gaiaDynamicLinePtr dyn = gaiaAllocDynamicLine ();
    *has_z = 1;

    if (strcmp (node->Tag, "outerBoundaryIs") == 0)
      {
	  /* parsing a KML <outerBoundaryIs> */
	  node = node->Next;
	  if (node == NULL)
	      goto error;
	  if (strcmp (node->Tag, "LinearRing") == 0)
	      ;
	  else
	      goto error;
	  node = node->Next;
	  if (node == NULL)
	      goto error;
	  if (strcmp (node->Tag, "coordinates") == 0)
	    {
		/* parsing a KML <kml:coordinates> */
		if (!kml_parse_coordinates (node->Coordinates, dyn, has_z))
		    goto error;
		node = node->Next;
		if (node == NULL)
		    goto error;
		if (strcmp (node->Tag, "coordinates") == 0)
		    ;
		else
		    goto error;
	    }
	  else
	      goto error;
	  node = node->Next;
	  if (node == NULL)
	      goto error;
	  if (strcmp (node->Tag, "LinearRing") == 0)
	      ;
	  else
	      goto error;
	  node = node->Next;
	  if (node == NULL)
	      goto error;
	  if (strcmp (node->Tag, "outerBoundaryIs") == 0)
	      ;
	  else
	      goto error;
	  *interior = 0;
	  *next = node->Next;
	  return dyn;
      }
    if (strcmp (node->Tag, "innerBoundaryIs") == 0)
      {
	  /* parsing a KML <innerBoundaryIs> */
	  node = node->Next;
	  if (node == NULL)
	      goto error;
	  if (strcmp (node->Tag, "LinearRing") == 0)
	      ;
	  else
	      goto error;
	  node = node->Next;
	  if (node == NULL)
	      goto error;
	  if (strcmp (node->Tag, "coordinates") == 0)
	    {
		/* parsing a KML <coordinates> */
		if (!kml_parse_coordinates (node->Coordinates, dyn, has_z))
		    goto error;
		node = node->Next;
		if (node == NULL)
		    goto error;
		if (strcmp (node->Tag, "coordinates") == 0)
		    ;
		else
		    goto error;
	    }
	  else
	      goto error;
	  node = node->Next;
	  if (node == NULL)
	      goto error;
	  if (strcmp (node->Tag, "LinearRing") == 0)
	      ;
	  else
	      goto error;
	  node = node->Next;
	  if (node == NULL)
	      goto error;
	  if (strcmp (node->Tag, "innerBoundaryIs") == 0)
	      ;
	  else
	      goto error;
	  *interior = 1;
	  *next = node->Next;
	  return dyn;
      }

  error:
    gaiaFreeDynamicLine (dyn);
    return 0;
}

static int
kml_parse_polygon (struct kml_data *p_data, gaiaGeomCollPtr geom,
		   kmlNodePtr node, kmlNodePtr * next_n)
{
/* parsing a <Polygon> */
    int interior;
    int has_z;
    int inners;
    int outers;
    int points;
    int iv;
    int ib = 0;
    gaiaGeomCollPtr pg;
    gaiaGeomCollPtr last_g;
    gaiaPolygonPtr new_pg;
    gaiaRingPtr ring;
    gaiaDynamicLinePtr dyn;
    gaiaPointPtr pt;
    gaiaDynamicLinePtr exterior_ring;
    kmlNodePtr next;
    kmlDynamicRingPtr dyn_rng;
    kmlDynamicPolygonPtr dyn_pg = kml_alloc_dyn_polygon (p_data);
    kmlNodePtr n = node;
    while (n)
      {
	  /* looping on rings */
	  if (strcmp (n->Tag, "Polygon") == 0)
	    {
		*next_n = n->Next;
		break;
	    }
	  dyn = kml_parse_ring (n, &interior, &has_z, &next);
	  if (dyn == NULL)
	      goto error;
	  if (kml_count_dyn_points (dyn) < 4)
	    {
		/* cannot be a valid ring */
		goto error;
	    }
	  /* checking if the ring is closed */
	  if (has_z)
	    {
		if (dyn->First->X == dyn->Last->X
		    && dyn->First->Y == dyn->Last->Y
		    && dyn->First->Z == dyn->Last->Z)
		    ;
		else
		    goto error;
	    }
	  else
	    {
		if (dyn->First->X == dyn->Last->X
		    && dyn->First->Y == dyn->Last->Y)
		    ;
		else
		    goto error;
	    }
	  kml_add_polygon_ring (dyn_pg, dyn, interior, has_z);
	  n = next;
      }
/* ok, KML nodes match as expected */
    inners = 0;
    outers = 0;
    has_z = 1;
    dyn_rng = dyn_pg->first;
    while (dyn_rng)
      {
	  /* verifying the rings collection */
	  if (dyn_rng->has_z == 0)
	      has_z = 0;
	  if (dyn_rng->interior)
	      inners++;
	  else
	    {
		outers++;
		points = kml_count_dyn_points (dyn_rng->ring);
		exterior_ring = dyn_rng->ring;
	    }
	  dyn_rng = dyn_rng->next;
      }
    if (outers != 1)		/* no exterior ring declared */
	goto error;

    if (has_z)
      {
	  pg = gaiaAllocGeomCollXYZ ();
	  kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
	  new_pg = gaiaAddPolygonToGeomColl (pg, points, inners);
	  /* initializing the EXTERIOR RING */
	  ring = new_pg->Exterior;
	  pt = exterior_ring->First;
	  iv = 0;
	  while (pt)
	    {
		gaiaSetPointXYZ (ring->Coords, iv, pt->X, pt->Y, pt->Z);
		iv++;
		pt = pt->Next;
	    }
	  dyn_rng = dyn_pg->first;
	  while (dyn_rng)
	    {
		/* initializing any INTERIOR RING */
		if (dyn_rng->interior == 0)
		  {
		      dyn_rng = dyn_rng->next;
		      continue;
		  }
		points = kml_count_dyn_points (dyn_rng->ring);
		ring = gaiaAddInteriorRing (new_pg, ib, points);
		ib++;
		pt = dyn_rng->ring->First;
		iv = 0;
		while (pt)
		  {
		      gaiaSetPointXYZ (ring->Coords, iv, pt->X, pt->Y, pt->Z);
		      iv++;
		      pt = pt->Next;
		  }
		dyn_rng = dyn_rng->next;
	    }
      }
    else
      {
	  pg = gaiaAllocGeomColl ();
	  kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
	  new_pg = gaiaAddPolygonToGeomColl (pg, points, inners);
	  /* initializing the EXTERIOR RING */
	  ring = new_pg->Exterior;
	  pt = exterior_ring->First;
	  iv = 0;
	  while (pt)
	    {
		gaiaSetPoint (ring->Coords, iv, pt->X, pt->Y);
		iv++;
		pt = pt->Next;
	    }
	  dyn_rng = dyn_pg->first;
	  while (dyn_rng)
	    {
		/* initializing any INTERIOR RING */
		if (dyn_rng->interior == 0)
		  {
		      dyn_rng = dyn_rng->next;
		      continue;
		  }
		points = kml_count_dyn_points (dyn_rng->ring);
		ring = gaiaAddInteriorRing (new_pg, ib, points);
		ib++;
		pt = dyn_rng->ring->First;
		iv = 0;
		while (pt)
		  {
		      gaiaSetPoint (ring->Coords, iv, pt->X, pt->Y);
		      iv++;
		      pt = pt->Next;
		  }
		dyn_rng = dyn_rng->next;
	    }
      }

    last_g = geom;
    while (1)
      {
	  /* searching the last Geometry within chain */
	  if (last_g->Next == NULL)
	      break;
	  last_g = last_g->Next;
      }
    last_g->Next = pg;
    kml_free_dyn_polygon (dyn_pg);
    return 1;

  error:
    kml_free_dyn_polygon (dyn_pg);
    return 0;
}

static int
kml_parse_multi_geometry (struct kml_data *p_data, gaiaGeomCollPtr geom,
			  kmlNodePtr node)
{
/* parsing a <MultiGeometry> */
    kmlNodePtr next;
    kmlNodePtr n = node;
    while (n)
      {
	  /* looping on Geometry Members */
	  if (n->Next == NULL)
	    {
		/* verifying the last KML node */
		if (strcmp (n->Tag, "MultiGeometry") == 0)
		    break;
		else
		    return 0;
	    }
	  if (strcmp (n->Tag, "Point") == 0)
	    {
		n = n->Next;
		if (n == NULL)
		    return 0;
		if (!kml_parse_point (p_data, geom, n, &next))
		    return 0;
		n = next;
		continue;
	    }
	  else if (strcmp (n->Tag, "LineString") == 0)
	    {
		n = n->Next;
		if (n == NULL)
		    return 0;
		if (!kml_parse_linestring (p_data, geom, n, &next))
		    return 0;
		n = next;
		continue;
	    }
	  else if (strcmp (n->Tag, "Polygon") == 0)
	    {
		n = n->Next;
		if (n == NULL)
		    return 0;
		if (!kml_parse_polygon (p_data, geom, n, &next))
		    return 0;
		n = next;
		continue;
	    }
	  else
	      return 0;
      }
    return 1;
}

static gaiaGeomCollPtr
kml_validate_geometry (struct kml_data *p_data, gaiaGeomCollPtr chain)
{
    int xy = 0;
    int xyz = 0;
    int pts = 0;
    int lns = 0;
    int pgs = 0;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaPointPtr save_pt = NULL;
    gaiaLinestringPtr save_ln = NULL;
    gaiaPolygonPtr save_pg = NULL;
    gaiaRingPtr i_ring;
    gaiaRingPtr o_ring;
    int ib;
    gaiaGeomCollPtr g;
    gaiaGeomCollPtr geom;

    g = chain;
    while (g)
      {
	  if (g != chain)
	    {
		if (g->DimensionModel == GAIA_XY)
		    xy++;
		if (g->DimensionModel == GAIA_XY_Z)
		    xyz++;
	    }
	  pt = g->FirstPoint;
	  while (pt)
	    {
		pts++;
		save_pt = pt;
		pt = pt->Next;
	    }
	  ln = g->FirstLinestring;
	  while (ln)
	    {
		lns++;
		save_ln = ln;
		ln = ln->Next;
	    }
	  pg = g->FirstPolygon;
	  while (pg)
	    {
		pgs++;
		save_pg = pg;
		pg = pg->Next;
	    }
	  g = g->Next;
      }
    if (pts == 1 && lns == 0 && pgs == 0)
      {
	  /* POINT */
	  if (xy > 0)
	    {
		/* 2D [XY] */
		geom = gaiaAllocGeomColl ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
		if (chain->DeclaredType == GAIA_GEOMETRYCOLLECTION)
		    geom->DeclaredType = GAIA_MULTIPOINT;
		else
		    geom->DeclaredType = GAIA_POINT;
		gaiaAddPointToGeomColl (geom, save_pt->X, save_pt->Y);
		return geom;
	    }
	  else
	    {
		/* 3D [XYZ] */
		geom = gaiaAllocGeomCollXYZ ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
		if (chain->DeclaredType == GAIA_GEOMETRYCOLLECTION)
		    geom->DeclaredType = GAIA_MULTIPOINT;
		else
		    geom->DeclaredType = GAIA_POINT;
		gaiaAddPointToGeomCollXYZ (geom, save_pt->X, save_pt->Y,
					   save_pt->Z);
		return geom;
	    }
      }
    if (pts == 0 && lns == 1 && pgs == 0)
      {
	  /* LINESTRING */
	  if (xy > 0)
	    {
		/* 2D [XY] */
		geom = gaiaAllocGeomColl ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
	    }
	  else
	    {
		/* 3D [XYZ] */
		geom = gaiaAllocGeomCollXYZ ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
	    }
	  if (chain->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      geom->DeclaredType = GAIA_MULTILINESTRING;
	  else
	      geom->DeclaredType = GAIA_LINESTRING;
	  ln = gaiaAddLinestringToGeomColl (geom, save_ln->Points);
	  gaiaCopyLinestringCoords (ln, save_ln);
	  return geom;
      }
    if (pts == 0 && lns == 0 && pgs == 1)
      {
	  /* POLYGON */
	  if (xy > 0)
	    {
		/* 2D [XY] */
		geom = gaiaAllocGeomColl ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
	    }
	  else
	    {
		/* 3D [XYZ] */
		geom = gaiaAllocGeomCollXYZ ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
	    }
	  if (chain->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      geom->DeclaredType = GAIA_MULTIPOLYGON;
	  else
	      geom->DeclaredType = GAIA_POLYGON;
	  i_ring = save_pg->Exterior;
	  pg = gaiaAddPolygonToGeomColl (geom, i_ring->Points,
					 save_pg->NumInteriors);
	  o_ring = pg->Exterior;
	  gaiaCopyRingCoords (o_ring, i_ring);
	  for (ib = 0; ib < save_pg->NumInteriors; ib++)
	    {
		i_ring = save_pg->Interiors + ib;
		o_ring = gaiaAddInteriorRing (pg, ib, i_ring->Points);
		gaiaCopyRingCoords (o_ring, i_ring);
	    }
	  return geom;
      }
    if (pts >= 1 && lns == 0 && pgs == 0)
      {
	  /* MULTIPOINT */
	  if (xy > 0)
	    {
		/* 2D [XY] */
		geom = gaiaAllocGeomColl ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
		geom->DeclaredType = GAIA_MULTIPOINT;
		g = chain;
		while (g)
		  {
		      pt = g->FirstPoint;
		      while (pt)
			{
			    gaiaAddPointToGeomColl (geom, pt->X, pt->Y);
			    pt = pt->Next;
			}
		      g = g->Next;
		  }
		return geom;
	    }
	  else
	    {
		/* 3D [XYZ] */
		geom = gaiaAllocGeomCollXYZ ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
		geom->DeclaredType = GAIA_MULTIPOINT;
		g = chain;
		while (g)
		  {
		      pt = g->FirstPoint;
		      while (pt)
			{
			    gaiaAddPointToGeomCollXYZ (geom, pt->X, pt->Y,
						       pt->Z);
			    pt = pt->Next;
			}
		      g = g->Next;
		  }
		return geom;
	    }
      }
    if (pts == 0 && lns >= 1 && pgs == 0)
      {
	  /* MULTILINESTRING */
	  if (xy > 0)
	    {
		/* 2D [XY] */
		geom = gaiaAllocGeomColl ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
		geom->DeclaredType = GAIA_MULTILINESTRING;
		g = chain;
		while (g)
		  {
		      ln = g->FirstLinestring;
		      while (ln)
			{
			    save_ln =
				gaiaAddLinestringToGeomColl (geom, ln->Points);
			    gaiaCopyLinestringCoords (save_ln, ln);
			    ln = ln->Next;
			}
		      g = g->Next;
		  }
		return geom;
	    }
	  else
	    {
		/* 3D [XYZ] */
		geom = gaiaAllocGeomCollXYZ ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
		geom->DeclaredType = GAIA_MULTILINESTRING;
		g = chain;
		while (g)
		  {
		      ln = g->FirstLinestring;
		      while (ln)
			{
			    save_ln =
				gaiaAddLinestringToGeomColl (geom, ln->Points);
			    gaiaCopyLinestringCoords (save_ln, ln);
			    ln = ln->Next;
			}
		      g = g->Next;
		  }
		return geom;
	    }
      }
    if (pts == 0 && lns == 0 && pgs >= 1)
      {
	  /* MULTIPOLYGON */
	  if (xy > 0)
	    {
		/* 2D [XY] */
		geom = gaiaAllocGeomColl ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
		geom->DeclaredType = GAIA_MULTIPOLYGON;
		g = chain;
		while (g)
		  {
		      pg = g->FirstPolygon;
		      while (pg)
			{
			    i_ring = pg->Exterior;
			    save_pg =
				gaiaAddPolygonToGeomColl (geom, i_ring->Points,
							  pg->NumInteriors);
			    o_ring = save_pg->Exterior;
			    gaiaCopyRingCoords (o_ring, i_ring);
			    for (ib = 0; ib < pg->NumInteriors; ib++)
			      {
				  i_ring = pg->Interiors + ib;
				  o_ring =
				      gaiaAddInteriorRing (save_pg, ib,
							   i_ring->Points);
				  gaiaCopyRingCoords (o_ring, i_ring);
			      }
			    pg = pg->Next;
			}
		      g = g->Next;
		  }
		return geom;
	    }
	  else
	    {
		/* 3D [XYZ] */
		geom = gaiaAllocGeomCollXYZ ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
		geom->DeclaredType = GAIA_MULTIPOLYGON;
		g = chain;
		while (g)
		  {
		      pg = g->FirstPolygon;
		      while (pg)
			{
			    i_ring = pg->Exterior;
			    save_pg =
				gaiaAddPolygonToGeomColl (geom, i_ring->Points,
							  pg->NumInteriors);
			    o_ring = save_pg->Exterior;
			    gaiaCopyRingCoords (o_ring, i_ring);
			    for (ib = 0; ib < pg->NumInteriors; ib++)
			      {
				  i_ring = pg->Interiors + ib;
				  o_ring =
				      gaiaAddInteriorRing (save_pg, ib,
							   i_ring->Points);
				  gaiaCopyRingCoords (o_ring, i_ring);
			      }
			    pg = pg->Next;
			}
		      g = g->Next;
		  }
		return geom;
	    }
      }
    if ((pts + lns + pgs) > 0)
      {
	  /* GEOMETRYCOLLECTION */
	  if (xy > 0)
	    {
		/* 2D [XY] */
		geom = gaiaAllocGeomColl ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
		geom->DeclaredType = GAIA_GEOMETRYCOLLECTION;
		g = chain;
		while (g)
		  {
		      pt = g->FirstPoint;
		      while (pt)
			{
			    gaiaAddPointToGeomColl (geom, pt->X, pt->Y);
			    pt = pt->Next;
			}
		      ln = g->FirstLinestring;
		      while (ln)
			{
			    save_ln =
				gaiaAddLinestringToGeomColl (geom, ln->Points);
			    gaiaCopyLinestringCoords (save_ln, ln);
			    ln = ln->Next;
			}
		      pg = g->FirstPolygon;
		      while (pg)
			{
			    i_ring = pg->Exterior;
			    save_pg =
				gaiaAddPolygonToGeomColl (geom, i_ring->Points,
							  pg->NumInteriors);
			    o_ring = save_pg->Exterior;
			    gaiaCopyRingCoords (o_ring, i_ring);
			    for (ib = 0; ib < pg->NumInteriors; ib++)
			      {
				  i_ring = pg->Interiors + ib;
				  o_ring =
				      gaiaAddInteriorRing (save_pg, ib,
							   i_ring->Points);
				  gaiaCopyRingCoords (o_ring, i_ring);
			      }
			    pg = pg->Next;
			}
		      g = g->Next;
		  }
		return geom;
	    }
	  else
	    {
		/* 3D [XYZ] */
		geom = gaiaAllocGeomCollXYZ ();
		kmlMapDynAlloc (p_data, KML_DYN_GEOM, pg);
		geom->DeclaredType = GAIA_GEOMETRYCOLLECTION;
		g = chain;
		while (g)
		  {
		      pt = g->FirstPoint;
		      while (pt)
			{
			    gaiaAddPointToGeomCollXYZ (geom, pt->X, pt->Y,
						       pt->Z);
			    pt = pt->Next;
			}
		      ln = g->FirstLinestring;
		      while (ln)
			{
			    save_ln =
				gaiaAddLinestringToGeomColl (geom, ln->Points);
			    gaiaCopyLinestringCoords (save_ln, ln);
			    ln = ln->Next;
			}
		      pg = g->FirstPolygon;
		      while (pg)
			{
			    i_ring = pg->Exterior;
			    save_pg =
				gaiaAddPolygonToGeomColl (geom, i_ring->Points,
							  pg->NumInteriors);
			    o_ring = save_pg->Exterior;
			    gaiaCopyRingCoords (o_ring, i_ring);
			    for (ib = 0; ib < pg->NumInteriors; ib++)
			      {
				  i_ring = pg->Interiors + ib;
				  o_ring =
				      gaiaAddInteriorRing (save_pg, ib,
							   i_ring->Points);
				  gaiaCopyRingCoords (o_ring, i_ring);
			      }
			    pg = pg->Next;
			}
		      g = g->Next;
		  }
		return geom;
	    }
      }
    return NULL;
}

static void
kml_free_geom_chain (gaiaGeomCollPtr geom)
{
/* deleting a chain of preliminary geometries */
    gaiaGeomCollPtr gn;
    while (geom)
      {
	  gn = geom->Next;
	  gaiaFreeGeomColl (geom);
	  geom = gn;
      }
}

static gaiaGeomCollPtr
kml_build_geometry (struct kml_data *p_data, kmlNodePtr tree)
{
/* attempting to build a geometry from KML nodes */
    gaiaGeomCollPtr geom;
    gaiaGeomCollPtr result;
    int geom_type;
    kmlNodePtr next;

    if (tree == NULL)
	return NULL;
    geom_type = guessKmlGeometryType (tree);
    if (geom_type == GAIA_KML_UNKNOWN)
      {
	  /* unsupported main geometry type */
	  return NULL;
      }
/* creating the main geometry */
    geom = gaiaAllocGeomColl ();

    switch (geom_type)
      {
	  /* parsing KML nodes accordingly with declared KML type */
      case GAIA_KML_POINT:
	  geom->DeclaredType = GAIA_POINT;
	  if (!kml_parse_point (p_data, geom, tree->Next, &next))
	      goto error;
	  break;
      case GAIA_KML_LINESTRING:
	  geom->DeclaredType = GAIA_LINESTRING;
	  if (!kml_parse_linestring (p_data, geom, tree->Next, &next))
	      goto error;
	  break;
      case GAIA_KML_POLYGON:
	  geom->DeclaredType = GAIA_POLYGON;
	  if (!kml_parse_polygon (p_data, geom, tree->Next, &next))
	      goto error;
	  if (next != NULL)
	      goto error;
	  break;
      case GAIA_KML_MULTIGEOMETRY:
	  geom->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  if (!kml_parse_multi_geometry (p_data, geom, tree->Next))
	      goto error;
	  break;
      };

/* attempting to build the final geometry */
    result = kml_validate_geometry (p_data, geom);
    if (result == NULL)
	goto error;
    kml_free_geom_chain (geom);
    return result;

  error:
    kml_free_geom_chain (geom);
    return NULL;
}


/* including LEMON generated code */
#include "Kml.c"



/*
** CAVEAT: there is an incompatibility between LEMON and FLEX
** this macro resolves the issue
*/
#undef yy_accept
#define yy_accept	yy_kml_flex_accept



/* including FLEX generated code */
#include "lex.Kml.c"



gaiaGeomCollPtr
gaiaParseKml (const unsigned char *dirty_buffer)
{
    void *pParser = ParseAlloc (malloc);
    /* Linked-list of token values */
    kmlFlexToken *tokens = malloc (sizeof (kmlFlexToken));
    /* Pointer to the head of the list */
    kmlFlexToken *head = tokens;
    int yv;
    gaiaGeomCollPtr geom = NULL;
    yyscan_t scanner;
    struct kml_data str_data;

/* initializing the helper structs */
    str_data.kml_line = 1;
    str_data.kml_col = 1;
    str_data.kml_parse_error = 0;
    str_data.kml_first_dyn_block = NULL;
    str_data.kml_last_dyn_block = NULL;
    str_data.result = NULL;

/* initializing the scanner state */
    Kmllex_init_extra (&str_data, &scanner);

    str_data.KmlLval.pval = NULL;
    tokens->value = NULL;
    tokens->Next = NULL;
    Kml_scan_string ((char *) dirty_buffer, scanner);

    /*
       / Keep tokenizing until we reach the end
       / yylex() will return the next matching Token for us.
     */
    while ((yv = yylex (scanner)) != 0)
      {
	  if (yv == -1)
	    {
		str_data.kml_parse_error = 1;
		break;
	    }
	  tokens->Next = malloc (sizeof (kmlFlexToken));
	  tokens->Next->Next = NULL;
	  kml_xferString (&(tokens->Next->value), str_data.KmlLval.pval);
	  /* Pass the token to the wkt parser created from lemon */
	  Parse (pParser, yv, &(tokens->Next->value), &str_data);
	  tokens = tokens->Next;
      }
    /* This denotes the end of a line as well as the end of the parser */
    Parse (pParser, KML_NEWLINE, 0, &str_data);
    ParseFree (pParser, free);
    Kmllex_destroy (scanner);

    /* Assigning the token as the end to avoid seg faults while cleaning */
    tokens->Next = NULL;
    kml_cleanup (head);
    kml_freeString (&(str_data.KmlLval.pval));

    if (str_data.kml_parse_error)
      {
	  if (str_data.result)
	    {
		/* if a Geometry-result has been produced, the stack is already cleaned */
		kml_freeTree (str_data.result);
		kmlCleanMapDynAlloc (&str_data, 0);
	    }
	  else
	    {
		/* otherwise we are required to clean the stack */
		kmlCleanMapDynAlloc (&str_data, 1);
	    }
	  return NULL;
      }

    if (str_data.result == NULL)
      {
	  kmlCleanMapDynAlloc (&str_data, 0);
	  return NULL;
      }

    /* attempting to build a geometry from KML */
    geom = kml_build_geometry (&str_data, str_data.result);
    geom->Srid = 4326;
    kml_freeTree (str_data.result);
    kmlCleanMapDynAlloc (&str_data, 0);
    return geom;
}


/*
** CAVEAT: we must now undefine any Lemon/Flex own macro
*/
#undef YYNOCODE
#undef YYNSTATE
#undef YYNRULE
#undef YY_SHIFT_MAX
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
