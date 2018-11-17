/*

 virtualbbox.c -- SQLite3 extension [VIRTUAL TABLE accessing BoundingBox tables]

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
#include <float.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>
#include <spatialite/debug.h>

#include <spatialite/spatialite.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/gaiageo.h>

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

static struct sqlite3_module my_bbox_module;

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

typedef struct VirtualBBoxStruct
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
    char *Visible;		/* true / false */
    SqliteValuePtr *Value;	/* the current-row value for each column */
    const void *p_cache;	/* pointer to the internal cache */
    int Srid;			/* the SRID for each Geometry column */
    char *ColSrid;		/* the column containing a SRID definition */
    int ForceWGS84;		/* always force WGS84 long-lat */
    char *MinX;			/* name of the MinX input column */
    char *MinY;			/* name of the MinY input column */
    char *MaxX;			/* name of the MaxX input column */
    char *MaxY;			/* name of the MaxY input column */
    gaiaGeomCollPtr BBoxGeom;	/* current BBOX geometry */
} VirtualBBox;
typedef VirtualBBox *VirtualBBoxPtr;

typedef struct VirtualBBoxCursorStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualBBoxPtr pVtab;	/* Virtual table of this cursor */
    sqlite3_stmt *stmt;
    sqlite3_int64 current_row;	/* the current row ID */
    int eof;			/* the EOF marker */
} VirtualBBoxCursor;
typedef VirtualBBoxCursor *VirtualBBoxCursorPtr;

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
free_table (VirtualBBoxPtr p_vt)
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
    if (p_vt->Visible)
	sqlite3_free (p_vt->Visible);
    if (p_vt->Value)
      {
	  for (i = 0; i < p_vt->nColumns; i++)
	    {
		if (*(p_vt->Value + i))
		    value_free (*(p_vt->Value + i));
	    }
	  sqlite3_free (p_vt->Value);
      }
    if (p_vt->MinX)
	sqlite3_free (p_vt->MinX);
    if (p_vt->MinY)
	sqlite3_free (p_vt->MinY);
    if (p_vt->MaxX)
	sqlite3_free (p_vt->MaxX);
    if (p_vt->MaxY)
	sqlite3_free (p_vt->MaxY);
    if (p_vt->ColSrid)
	sqlite3_free (p_vt->ColSrid);
    if (p_vt->BBoxGeom)
	gaiaFreeGeomColl (p_vt->BBoxGeom);
    sqlite3_free (p_vt);
}

static void
vbbox_read_row (VirtualBBoxCursorPtr cursor)
{
/* trying to read a row from the BoundingBox real-table */
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) cursor->pVtab->p_cache;
    sqlite3_stmt *stmt;
    int ret;
    int ic;
    int icx;
    const char *text;
    const unsigned char *blob;
    int size;
    sqlite3_int64 pk;
    double minx = DBL_MAX;
    double miny = DBL_MAX;
    double maxx = 0.0 - DBL_MAX;
    double maxy = 0.0 - DBL_MAX;
    int srid;
    char ok_minx = 'N';
    char ok_miny = 'N';
    char ok_maxx = 'N';
    char ok_maxy = 'N';
    char ok_srid = 'N';
    stmt = cursor->stmt;
    sqlite3_bind_int64 (stmt, 1, cursor->current_row);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_ROW)
      {
	  pk = sqlite3_column_int64 (stmt, 0);
	  if (sqlite3_column_type (stmt, 1) == SQLITE_FLOAT)
	    {
		minx = sqlite3_column_double (stmt, 1);
		ok_minx = 'Y';
	    }
	  if (sqlite3_column_type (stmt, 2) == SQLITE_FLOAT)
	    {
		miny = sqlite3_column_double (stmt, 2);
		ok_miny = 'Y';
	    }
	  if (sqlite3_column_type (stmt, 3) == SQLITE_FLOAT)
	    {
		maxx = sqlite3_column_double (stmt, 3);
		ok_maxx = 'Y';
	    }
	  if (sqlite3_column_type (stmt, 4) == SQLITE_FLOAT)
	    {
		maxy = sqlite3_column_double (stmt, 4);
		ok_maxy = 'Y';
	    }
	  if (sqlite3_column_type (stmt, 5) == SQLITE_INTEGER)
	    {
		srid = sqlite3_column_int (stmt, 5);
		ok_srid = 'Y';
	    }
	  if (cursor->pVtab->BBoxGeom)
	      gaiaFreeGeomColl (cursor->pVtab->BBoxGeom);
	  cursor->pVtab->BBoxGeom = NULL;
	  if (ok_minx == 'Y' && ok_miny == 'Y' && ok_maxx == 'Y'
	      && ok_maxy == 'Y')
	    {
		gaiaGeomCollPtr geom = gaiaAllocGeomColl ();
		gaiaPolygonPtr pg = gaiaAddPolygonToGeomColl (geom, 5, 0);
		gaiaRingPtr rng = pg->Exterior;
		gaiaSetPoint (rng->Coords, 0, minx, miny);
		gaiaSetPoint (rng->Coords, 1, maxx, miny);
		gaiaSetPoint (rng->Coords, 2, maxx, maxy);
		gaiaSetPoint (rng->Coords, 3, minx, maxy);
		gaiaSetPoint (rng->Coords, 4, minx, miny);
		if (ok_srid == 'Y')
		  {
		      if (cursor->pVtab->ForceWGS84)
			{
			    /* converting to WGS84 long-lat */
			    gaiaGeomCollPtr geom2 = NULL;
			    char *proj_from = NULL;
			    char *proj_to = NULL;
			    geom->Srid = srid;
			    getProjParams (cursor->pVtab->db, srid, &proj_from);
			    getProjParams (cursor->pVtab->db, 4326, &proj_to);
			    if (proj_to == NULL || proj_from == NULL)
				geom2 = NULL;
			    else
#ifndef OMIT_PROJ		/* including PROJ.4 */
			    if (cache != NULL)
				geom2 =
				    gaiaTransform_r (cache, geom, proj_from,
						     proj_to);
			    else
				geom2 =
				    gaiaTransform (geom, proj_from, proj_to);
#endif /* end including PROJ.4 */
			    geom2 = NULL;
			    if (geom2 != NULL)
				geom2->Srid = 4326;
			    cursor->pVtab->BBoxGeom = geom2;
			    gaiaFreeGeomColl (geom);
			    if (proj_from)
				free (proj_from);
			    if (proj_to)
				free (proj_to);
			}
		      else
			{
			    geom->Srid = srid;
			    cursor->pVtab->BBoxGeom = geom;
			}
		  }
		else
		  {
		      geom->Srid = cursor->pVtab->Srid;
		      cursor->pVtab->BBoxGeom = geom;
		  }
	    }
	  icx = 5;
	  for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
	    {
		if (*(cursor->pVtab->Visible + ic) != 'Y')
		    continue;
		icx++;
		switch (sqlite3_column_type (stmt, icx))
		  {
		  case SQLITE_INTEGER:
		      value_set_int (*(cursor->pVtab->Value + ic),
				     sqlite3_column_int64 (stmt, icx));
		      break;
		  case SQLITE_FLOAT:
		      value_set_double (*(cursor->pVtab->Value + ic),
					sqlite3_column_double (stmt, icx));
		      break;
		  case SQLITE_TEXT:
		      text = (char *) sqlite3_column_text (stmt, icx);
		      size = sqlite3_column_bytes (stmt, icx);
		      value_set_text (*(cursor->pVtab->Value + ic), text, size);
		      break;
		  case SQLITE_BLOB:
		      blob = sqlite3_column_blob (stmt, icx);
		      size = sqlite3_column_bytes (stmt, icx);
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
vbbox_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some BoundingBox table */
    char *vtable = NULL;
    char *table = NULL;
    char *col_minx = NULL;
    char *col_miny = NULL;
    char *col_maxx = NULL;
    char *col_maxy = NULL;
    char *col_srid = NULL;
    char *x_force_wgs84 = NULL;
    int ret;
    int i;
    int i2;
    int len;
    int n_rows;
    int n_columns;
    const char *col_name;
    const char *col_type;
    int force_wgs84;
    char **results;
    char *sql;
    char *xname;
    gaiaOutBuffer sql_statement;
    VirtualBBoxPtr p_vt = NULL;
    if (pAux)
	pAux = pAux;		/* unused arg warning suppression */
    gaiaOutBufferInitialize (&sql_statement);
/* checking for table_name */
    if (argc >= 10)
      {
	  vtable = gaiaDequotedSql ((char *) argv[2]);
	  table = gaiaDequotedSql ((char *) argv[3]);
	  col_minx = gaiaDequotedSql ((char *) argv[4]);
	  col_miny = gaiaDequotedSql ((char *) argv[5]);
	  col_maxx = gaiaDequotedSql ((char *) argv[6]);
	  col_maxy = gaiaDequotedSql ((char *) argv[7]);
	  col_srid = gaiaDequotedSql ((char *) argv[8]);
	  x_force_wgs84 = gaiaDequotedSql ((char *) argv[9]);
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualBBox module] CREATE VIRTUAL: illegal arg list {table_name, col_minx, col_miny, col_maxx, col_maxy, srid, longlat=1|0, columns}\n");
	  goto error;
      }
    if (strcmp (x_force_wgs84, "0") == 0)
	force_wgs84 = 0;
    else if (strcmp (x_force_wgs84, "1") == 0)
	force_wgs84 = 1;
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualBBox module] CREATE VIRTUAL: illegal arg list {table_name, col_minx, col_miny, col_maxx, col_maxy, srid, longlat=1|0, columns}\n");
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
	  p_vt = (VirtualBBoxPtr) sqlite3_malloc (sizeof (VirtualBBox));
	  if (!p_vt)
	      return SQLITE_NOMEM;
	  p_vt->db = db;
	  p_vt->p_cache = pAux;
	  p_vt->nRef = 0;
	  p_vt->zErrMsg = NULL;
	  len = strlen (table);
	  p_vt->table = sqlite3_malloc (len + 1);
	  strcpy (p_vt->table, table);
	  p_vt->nColumns = n_rows;
	  p_vt->Column = sqlite3_malloc (sizeof (char *) * n_rows);
	  p_vt->Type = sqlite3_malloc (sizeof (char *) * n_rows);
	  p_vt->Visible = sqlite3_malloc (sizeof (char *) * n_rows);
	  memset (p_vt->Visible, 'N', n_rows);
	  p_vt->Value = sqlite3_malloc (sizeof (SqliteValuePtr) * n_rows);
	  p_vt->Srid = atoi (col_srid);
	  p_vt->ForceWGS84 = force_wgs84;
#ifndef OMIT_PROJ		/* including PROJ.4 */
	  if (p_vt->ForceWGS84)
	      spatialite_e
		  ("VirtualBBOX WARNING - WGS84 is requested, but PROJ4 support is currently disabled\n");
#endif /* end including PROJ.4 */
	  p_vt->ColSrid = NULL;
	  p_vt->MinX = NULL;
	  p_vt->MinY = NULL;
	  p_vt->MaxX = NULL;
	  p_vt->MaxY = NULL;
	  p_vt->BBoxGeom = NULL;
	  for (i = 0; i < n_rows; i++)
	    {
		*(p_vt->Column + i) = NULL;
		*(p_vt->Type + i) = NULL;
		*(p_vt->Value + i) = value_alloc ();
	    }
	  for (i = 1; i <= n_rows; i++)
	    {
		col_name = results[(i * n_columns) + 1];
		col_type = results[(i * n_columns) + 2];
		len = strlen (col_name);
		if (strcasecmp (col_name, col_minx) == 0)
		  {
		      p_vt->MinX = sqlite3_malloc (len + 1);
		      strcpy (p_vt->MinX, col_name);
		  }
		if (strcasecmp (col_name, col_miny) == 0)
		  {
		      p_vt->MinY = sqlite3_malloc (len + 1);
		      strcpy (p_vt->MinY, col_name);
		  }
		if (strcasecmp (col_name, col_maxx) == 0)
		  {
		      p_vt->MaxX = sqlite3_malloc (len + 1);
		      strcpy (p_vt->MaxX, col_name);
		  }
		if (strcasecmp (col_name, col_maxy) == 0)
		  {
		      p_vt->MaxY = sqlite3_malloc (len + 1);
		      strcpy (p_vt->MaxY, col_name);
		  }
		if (strcasecmp (col_name, col_srid) == 0)
		  {
		      p_vt->ColSrid = sqlite3_malloc (len + 1);
		      strcpy (p_vt->ColSrid, col_name);
		  }
		*(p_vt->Column + (i - 1)) = sqlite3_malloc (len + 1);
		strcpy (*(p_vt->Column + (i - 1)), col_name);
		len = strlen (col_type);
		*(p_vt->Type + (i - 1)) = sqlite3_malloc (len + 1);
		strcpy (*(p_vt->Type + (i - 1)), col_type);
		for (i2 = 10; i2 < argc; i2++)
		  {
		      char *extra_col = gaiaDequotedSql ((char *) argv[i2]);
		      if (strcasecmp (extra_col, col_name) == 0)
			  *(p_vt->Visible + (i - 1)) = 'Y';
		      free (extra_col);
		  }
	    }
	  sqlite3_free_table (results);
      }
    else
      {
	  sqlite3_free_table (results);
	  goto illegal;
      }
    if (p_vt->MinX == NULL || p_vt->MinY == NULL || p_vt->MaxX == NULL
	|| p_vt->MaxY == NULL)
	goto illegal;
/* preparing the COLUMNs for this VIRTUAL TABLE */
    xname = gaiaDoubleQuotedSql (vtable);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (Geometry Polygon", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
    for (i = 0; i < p_vt->nColumns; i++)
      {
	  if (*(p_vt->Visible + i) != 'Y')
	      continue;
	  xname = gaiaDoubleQuotedSql (*(p_vt->Column + i));
	  sql = sqlite3_mprintf (", \"%s\" %s", xname, *(p_vt->Type + i));
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
		    ("[VirtualBBox module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
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
    free (col_minx);
    free (col_miny);
    free (col_maxx);
    free (col_maxy);
    free (col_srid);
    free (x_force_wgs84);
    return SQLITE_OK;
  illegal:
/* something is going the wrong way */
    gaiaOutBufferReset (&sql_statement);
    if (p_vt)
	free_table (p_vt);
    *pzErr =
	sqlite3_mprintf
	("[VirtualBBox module] '%s' isn't a valid BoundingBox table\n", table);
  error:
    if (vtable)
	free (vtable);
    if (table)
	free (table);
    if (col_minx)
	free (col_minx);
    if (col_miny)
	free (col_miny);
    if (col_maxx)
	free (col_maxx);
    if (col_maxy)
	free (col_maxy);
    if (col_srid)
	free (col_srid);
    if (x_force_wgs84)
	free (x_force_wgs84);
    gaiaOutBufferReset (&sql_statement);
    return SQLITE_ERROR;
}

static int
vbbox_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	       sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vbbox_create() */
    return vbbox_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vbbox_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
{
/* best index selection */
    if (pVTab || pIndex)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vbbox_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualBBoxPtr p_vt = (VirtualBBoxPtr) pVTab;
    free_table (p_vt);
    return SQLITE_OK;
}

static int
vbbox_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vbbox_disconnect() */
    return vbbox_disconnect (pVTab);
}

static int
vbbox_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    sqlite3_stmt *stmt;
    gaiaOutBuffer sql_statement;
    int ret;
    char *sql;
    int ic;
    char *xname;
    VirtualBBoxCursorPtr cursor =
	(VirtualBBoxCursorPtr) sqlite3_malloc (sizeof (VirtualBBoxCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = (VirtualBBoxPtr) pVTab;
    gaiaOutBufferInitialize (&sql_statement);
    gaiaAppendToOutBuffer (&sql_statement, "SELECT ROWID");
    xname = gaiaDoubleQuotedSql (cursor->pVtab->MinX);
    sql = sqlite3_mprintf (",\"%s\"", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
    xname = gaiaDoubleQuotedSql (cursor->pVtab->MinY);
    sql = sqlite3_mprintf (",\"%s\"", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
    xname = gaiaDoubleQuotedSql (cursor->pVtab->MaxX);
    sql = sqlite3_mprintf (",\"%s\"", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
    xname = gaiaDoubleQuotedSql (cursor->pVtab->MaxY);
    sql = sqlite3_mprintf (",\"%s\"", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
    if (cursor->pVtab->ColSrid == NULL)
	gaiaAppendToOutBuffer (&sql_statement, ",NULL");
    else
      {
	  xname = gaiaDoubleQuotedSql (cursor->pVtab->ColSrid);
	  sql = sqlite3_mprintf (",\"%s\"", xname);
	  free (xname);
	  gaiaAppendToOutBuffer (&sql_statement, sql);
	  sqlite3_free (sql);
      }
    for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
      {
	  value_set_null (*(cursor->pVtab->Value + ic));
	  if (*(cursor->pVtab->Visible + ic) != 'Y')
	      continue;
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
    vbbox_read_row (cursor);
    return SQLITE_OK;
}

static int
vbbox_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    int ic;
    VirtualBBoxCursorPtr cursor = (VirtualBBoxCursorPtr) pCursor;
    for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
	value_set_null (*(cursor->pVtab->Value + ic));
    if (cursor->stmt)
	sqlite3_finalize (cursor->stmt);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vbbox_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	      int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    if (pCursor || idxNum || idxStr || argc || argv)
	pCursor = pCursor;	/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vbbox_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching next row from cursor */
    VirtualBBoxCursorPtr cursor = (VirtualBBoxCursorPtr) pCursor;
    (cursor->current_row)++;
    vbbox_read_row (cursor);
    return SQLITE_OK;
}

static int
vbbox_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualBBoxCursorPtr cursor = (VirtualBBoxCursorPtr) pCursor;
    return cursor->eof;
}

static int
vbbox_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	      int column)
{
/* fetching value for the Nth column */
    VirtualBBoxCursorPtr cursor = (VirtualBBoxCursorPtr) pCursor;
    SqliteValuePtr value;
    int icol = 0;
    int icol_ok = 0;
    if (column == 0)
      {
	  /* the BBox Geometry */
	  if (cursor->pVtab->BBoxGeom == NULL)
	      sqlite3_result_null (pContext);
	  else
	    {
		unsigned char *blob;
		int size;
		gaiaToSpatiaLiteBlobWkb (cursor->pVtab->BBoxGeom, &blob, &size);
		sqlite3_result_blob (pContext, blob, size, free);
	    }
	  return SQLITE_OK;
      }
    while (icol < cursor->pVtab->nColumns)
      {
	  if (*(cursor->pVtab->Visible + icol) != 'Y')
	    {
		icol++;
		continue;
	    }
	  icol_ok++;
	  if (icol_ok == column)
	    {
		value = *(cursor->pVtab->Value + icol);
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
		return SQLITE_OK;
	    }
	  icol++;
      }
    sqlite3_result_null (pContext);
    return SQLITE_OK;
}

static int
vbbox_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualBBoxCursorPtr cursor = (VirtualBBoxCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vbbox_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	      sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    if (pVTab || argc || argv || pRowid)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_READONLY;
}

static int
vbbox_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vbbox_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vbbox_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vbbox_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vbox_rename (sqlite3_vtab * pVTab, const char *zNew)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    if (zNew)
	zNew = zNew;		/* unused arg warning suppression */
    return SQLITE_ERROR;
}

static int
spliteVirtualBBoxInit (sqlite3 * db, void *p_cache)
{
    int rc = SQLITE_OK;
    my_bbox_module.iVersion = 1;
    my_bbox_module.xCreate = &vbbox_create;
    my_bbox_module.xConnect = &vbbox_connect;
    my_bbox_module.xBestIndex = &vbbox_best_index;
    my_bbox_module.xDisconnect = &vbbox_disconnect;
    my_bbox_module.xDestroy = &vbbox_destroy;
    my_bbox_module.xOpen = &vbbox_open;
    my_bbox_module.xClose = &vbbox_close;
    my_bbox_module.xFilter = &vbbox_filter;
    my_bbox_module.xNext = &vbbox_next;
    my_bbox_module.xEof = &vbbox_eof;
    my_bbox_module.xColumn = &vbbox_column;
    my_bbox_module.xRowid = &vbbox_rowid;
    my_bbox_module.xUpdate = &vbbox_update;
    my_bbox_module.xBegin = &vbbox_begin;
    my_bbox_module.xSync = &vbbox_sync;
    my_bbox_module.xCommit = &vbbox_commit;
    my_bbox_module.xRollback = &vbbox_rollback;
    my_bbox_module.xFindFunction = NULL;
    my_bbox_module.xRename = &vbox_rename;
    sqlite3_create_module_v2 (db, "VirtualBBox", &my_bbox_module, p_cache, 0);
    return rc;
}

SPATIALITE_PRIVATE int
virtualbbox_extension_init (void *xdb, const void *p_cache)
{
    sqlite3 *db = (sqlite3 *) xdb;
    return spliteVirtualBBoxInit (db, (void *) p_cache);
}
