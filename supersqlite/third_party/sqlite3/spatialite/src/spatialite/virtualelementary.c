/*

 virtualelementary.c -- SQLite3 extension [VIRTUAL TABLE Elementary Geometries]

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>

#include <spatialite/spatialite.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/gaiageo.h>

#ifdef _WIN32
#define strcasecmp	_stricmp
#define strncasecmp	_strnicmp
#endif /* not WIN32 */

static struct sqlite3_module my_elem_module;


/******************************************************************************
/
/ VirtualTable structs
/
******************************************************************************/

typedef struct VirtualElementaryStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
} VirtualElementary;
typedef VirtualElementary *VirtualElementaryPtr;

typedef struct VirtualElementaryCursorStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualElementaryPtr pVtab;	/* Virtual table of this cursor */
    int eof;			/* the EOF marker */
    char *db_prefix;
    char *f_table_name;
    char *f_geometry_column;
    sqlite3_int64 origin_rowid;
    gaiaGeomCollPtr *geometries;
    int count;
    int current;
} VirtualElementaryCursor;
typedef VirtualElementaryCursor *VirtualElementaryCursorPtr;

static int
velem_find_geometry (sqlite3 * sqlite, const char *db_prefix,
		     const char *table_name, const char *geom_column,
		     char **real_prefix, char **real_table, char **real_geom)
{
/* checks if the required Geometry actually defined */
    sqlite3_stmt *stmt;
    char *sql_statement;
    int ret;
    int count = 0;
    char *rt = NULL;
    char *rg = NULL;

    if (geom_column == NULL)
      {
	  if (db_prefix == NULL)
	    {
		sql_statement =
		    sqlite3_mprintf
		    ("SELECT f_table_name, f_geometry_column FROM geometry_columns "
		     "WHERE Upper(f_table_name) = Upper(%Q)", table_name);
	    }
	  else
	    {
		char *quoted_db = gaiaDoubleQuotedSql (db_prefix);
		sql_statement =
		    sqlite3_mprintf
		    ("SELECT f_table_name, f_geometry_column FROM \"%s\".geometry_columns "
		     "WHERE Upper(f_table_name) = Upper(%Q)",
		     quoted_db, table_name);
		free (quoted_db);
	    }
      }
    else
      {
	  if (db_prefix == NULL)
	    {
		sql_statement =
		    sqlite3_mprintf
		    ("SELECT f_table_name, f_geometry_column FROM geometry_columns "
		     "WHERE Upper(f_table_name) = Upper(%Q) AND "
		     "Upper(f_geometry_column) = Upper(%Q)",
		     table_name, geom_column);
	    }
	  else
	    {
		char *quoted_db = gaiaDoubleQuotedSql (db_prefix);
		sql_statement =
		    sqlite3_mprintf
		    ("SELECT f_table_name, f_geometry_column FROM \"%s\".geometry_columns "
		     "WHERE Upper(f_table_name) = Upper(%Q) AND "
		     "Upper(f_geometry_column) = Upper(%Q)",
		     quoted_db, table_name, geom_column);
		free (quoted_db);
	    }
      }
    ret =
	sqlite3_prepare_v2 (sqlite, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	return 0;
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		const char *v = (const char *) sqlite3_column_text (stmt, 0);
		int len = sqlite3_column_bytes (stmt, 0);
		if (rt)
		    free (rt);
		rt = malloc (len + 1);
		strcpy (rt, v);
		v = (const char *) sqlite3_column_text (stmt, 1);
		len = sqlite3_column_bytes (stmt, 1);
		if (rg)
		    free (rg);
		rg = malloc (len + 1);
		strcpy (rg, v);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count != 1)
	return 0;
    else
      {
	  int len;
	  if (db_prefix == NULL)
	      db_prefix = "main";
	  len = strlen (db_prefix);
	  *real_prefix = malloc (len + 1);
	  strcpy (*real_prefix, db_prefix);
	  *real_table = rt;
	  *real_geom = rg;
      }
    return 1;
}

static gaiaGeomCollPtr
velem_from_point (gaiaPointPtr pt, int srid)
{
/* creating a Geometry containing a single Point */
    gaiaGeomCollPtr g = NULL;
    switch (pt->DimensionModel)
      {
      case GAIA_XY_Z_M:
	  g = gaiaAllocGeomCollXYZM ();
	  break;
      case GAIA_XY_Z:
	  g = gaiaAllocGeomCollXYZ ();
	  break;
      case GAIA_XY_M:
	  g = gaiaAllocGeomCollXYM ();
	  break;
      default:
	  g = gaiaAllocGeomColl ();
	  break;
      };
    if (!g)
	return NULL;
    g->Srid = srid;
    g->DeclaredType = GAIA_POINT;
    switch (pt->DimensionModel)
      {
      case GAIA_XY_Z_M:
	  gaiaAddPointToGeomCollXYZM (g, pt->X, pt->Y, pt->Z, pt->M);
	  break;
      case GAIA_XY_Z:
	  gaiaAddPointToGeomCollXYZ (g, pt->X, pt->Y, pt->Z);
	  break;
      case GAIA_XY_M:
	  gaiaAddPointToGeomCollXYM (g, pt->X, pt->Y, pt->M);
	  break;
      default:
	  gaiaAddPointToGeomColl (g, pt->X, pt->Y);
	  break;
      };
    return g;
}

static gaiaGeomCollPtr
velem_from_linestring (gaiaLinestringPtr ln, int srid)
{
/* creating a Geometry containing a single Linestring */
    gaiaGeomCollPtr g = NULL;
    gaiaLinestringPtr ln2;
    int iv;
    double x;
    double y;
    double z;
    double m;
    switch (ln->DimensionModel)
      {
      case GAIA_XY_Z_M:
	  g = gaiaAllocGeomCollXYZM ();
	  break;
      case GAIA_XY_Z:
	  g = gaiaAllocGeomCollXYZ ();
	  break;
      case GAIA_XY_M:
	  g = gaiaAllocGeomCollXYM ();
	  break;
      default:
	  g = gaiaAllocGeomColl ();
	  break;
      };
    if (!g)
	return NULL;
    g->Srid = srid;
    g->DeclaredType = GAIA_LINESTRING;
    ln2 = gaiaAddLinestringToGeomColl (g, ln->Points);
    switch (ln->DimensionModel)
      {
      case GAIA_XY_Z_M:
	  for (iv = 0; iv < ln->Points; iv++)
	    {
		gaiaGetPointXYZM (ln->Coords, iv, &x, &y, &z, &m);
		gaiaSetPointXYZM (ln2->Coords, iv, x, y, z, m);
	    }
	  break;
      case GAIA_XY_Z:
	  for (iv = 0; iv < ln->Points; iv++)
	    {
		gaiaGetPointXYZ (ln->Coords, iv, &x, &y, &z);
		gaiaSetPointXYZ (ln2->Coords, iv, x, y, z);
	    }
	  break;
      case GAIA_XY_M:
	  for (iv = 0; iv < ln->Points; iv++)
	    {
		gaiaGetPointXYM (ln->Coords, iv, &x, &y, &m);
		gaiaSetPointXYM (ln2->Coords, iv, x, y, m);
	    }
	  break;
      default:
	  for (iv = 0; iv < ln->Points; iv++)
	    {
		gaiaGetPoint (ln->Coords, iv, &x, &y);
		gaiaSetPoint (ln2->Coords, iv, x, y);
	    }
	  break;
      };
    return g;
}

static gaiaGeomCollPtr
velem_from_polygon (gaiaPolygonPtr pg, int srid)
{
/* creating a Geometry containing a single Polygon */
    gaiaGeomCollPtr g = NULL;
    gaiaPolygonPtr pg2;
    gaiaRingPtr rng;
    gaiaRingPtr rng2;
    int ib;
    int iv;
    double x;
    double y;
    double z;
    double m;
    switch (pg->DimensionModel)
      {
      case GAIA_XY_Z_M:
	  g = gaiaAllocGeomCollXYZM ();
	  break;
      case GAIA_XY_Z:
	  g = gaiaAllocGeomCollXYZ ();
	  break;
      case GAIA_XY_M:
	  g = gaiaAllocGeomCollXYM ();
	  break;
      default:
	  g = gaiaAllocGeomColl ();
	  break;
      };
    if (!g)
	return NULL;
    g->Srid = srid;
    g->DeclaredType = GAIA_POLYGON;
    rng = pg->Exterior;
    pg2 = gaiaAddPolygonToGeomColl (g, rng->Points, pg->NumInteriors);
    rng2 = pg2->Exterior;
    switch (pg->DimensionModel)
      {
      case GAIA_XY_Z_M:
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		gaiaSetPointXYZM (rng2->Coords, iv, x, y, z, m);
	    }
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		rng = pg->Interiors + ib;
		rng2 = gaiaAddInteriorRing (pg2, ib, rng->Points);
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPointXYZM (rng->Coords, iv, &x, &y, &z, &m);
		      gaiaSetPointXYZM (rng2->Coords, iv, x, y, z, m);
		  }
	    }
	  break;
      case GAIA_XY_Z:
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		gaiaSetPointXYZ (rng2->Coords, iv, x, y, z);
	    }
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		rng = pg->Interiors + ib;
		rng2 = gaiaAddInteriorRing (pg2, ib, rng->Points);
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPointXYZ (rng->Coords, iv, &x, &y, &z);
		      gaiaSetPointXYZ (rng2->Coords, iv, x, y, z);
		  }
	    }
	  break;
      case GAIA_XY_M:
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		gaiaSetPointXYM (rng2->Coords, iv, x, y, m);
	    }
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		rng = pg->Interiors + ib;
		rng2 = gaiaAddInteriorRing (pg2, ib, rng->Points);
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPointXYM (rng->Coords, iv, &x, &y, &m);
		      gaiaSetPointXYM (rng2->Coords, iv, x, y, m);
		  }
	    }
	  break;
      default:
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPoint (rng->Coords, iv, &x, &y);
		gaiaSetPoint (rng2->Coords, iv, x, y);
	    }
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		rng = pg->Interiors + ib;
		rng2 = gaiaAddInteriorRing (pg2, ib, rng->Points);
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      gaiaSetPoint (rng2->Coords, iv, x, y);
		  }
	    }
	  break;
      };
    return g;
}

static int
velem_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table for Elementary Geometries metahandling */
    VirtualElementaryPtr p_vt;
    char *buf;
    char *vtable;
    char *xname;
    if (pAux)
	pAux = pAux;		/* unused arg warning suppression */
    if (argc == 3)
      {
	  vtable = gaiaDequotedSql ((char *) argv[2]);
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualElementary module] CREATE VIRTUAL: illegal arg list {void}\n");
	  return SQLITE_ERROR;
      }
    p_vt = (VirtualElementaryPtr) sqlite3_malloc (sizeof (VirtualElementary));
    if (!p_vt)
	return SQLITE_NOMEM;
    p_vt->db = db;
    p_vt->pModule = &my_elem_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
/* preparing the COLUMNs for this VIRTUAL TABLE */
    xname = gaiaDoubleQuotedSql (vtable);
    buf = sqlite3_mprintf ("CREATE TABLE \"%s\" (db_prefix TEXT, "
			   "f_table_name TEXT, f_geometry_column TEXT, "
			   "origin_rowid INTEGER, item_no INTEGER, geometry BLOB)",
			   xname);
    free (xname);
    free (vtable);
    if (sqlite3_declare_vtab (db, buf) != SQLITE_OK)
      {
	  sqlite3_free (buf);
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualElementary module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
	       buf);
	  return SQLITE_ERROR;
      }
    sqlite3_free (buf);
    *ppVTab = (sqlite3_vtab *) p_vt;
    return SQLITE_OK;
}

static void
velem_reset_cache (VirtualElementaryCursorPtr cursor)
{
/* cleaning the cursor's cache */
    if (cursor->db_prefix != NULL)
	free (cursor->db_prefix);
    if (cursor->f_table_name != NULL)
	free (cursor->f_table_name);
    if (cursor->f_geometry_column != NULL)
	free (cursor->f_geometry_column);
    if (cursor->geometries != NULL)
      {
	  int i;
	  for (i = 0; i < cursor->count; i++)
	    {
		gaiaGeomCollPtr geom = *(cursor->geometries + i);
		gaiaFreeGeomColl (geom);
	    }
	  free (cursor->geometries);
      }
    cursor->db_prefix = NULL;
    cursor->f_table_name = NULL;
    cursor->f_geometry_column = NULL;
    cursor->geometries = NULL;
    cursor->count = 0;
    cursor->current = 0;
}

static int
velem_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	       sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table - simply aliases velem_create() */
    return velem_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
velem_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIdxInfo)
{
/* best index selection */
    int i;
    int errors = 0;
    int err = 1;
    int db_prefix = 0;
    int table = 0;
    int geom = 0;
    int rowid = 0;
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    for (i = 0; i < pIdxInfo->nConstraint; i++)
      {
	  /* verifying the constraints */
	  struct sqlite3_index_constraint *p = &(pIdxInfo->aConstraint[i]);
	  if (p->usable)
	    {
		if (p->iColumn == 0 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		    db_prefix++;
		else if (p->iColumn == 1 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		    table++;
		else if (p->iColumn == 2 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		    geom++;
		else if (p->iColumn == 3 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		    rowid++;
		else
		    errors++;
	    }
      }
    if ((db_prefix == 0 || db_prefix == 1) && table == 1
	&& (geom == 0 || geom == 1) && rowid == 1 && errors == 0)
      {
	  /* this one is a valid ElementaryGeometries query */
	  if (db_prefix == 0)
	    {
		if (geom == 1)
		    pIdxInfo->idxNum = 1;
		else
		    pIdxInfo->idxNum = 2;
	    }
	  else
	    {
		if (geom == 1)
		    pIdxInfo->idxNum = 3;
		else
		    pIdxInfo->idxNum = 4;
	    }
	  pIdxInfo->estimatedCost = 1.0;
	  for (i = 0; i < pIdxInfo->nConstraint; i++)
	    {
		if (pIdxInfo->aConstraint[i].usable)
		  {
		      pIdxInfo->aConstraintUsage[i].argvIndex = i + 1;
		      pIdxInfo->aConstraintUsage[i].omit = 1;
		  }
	    }
	  err = 0;
      }
    if (err)
      {
	  /* illegal query */
	  pIdxInfo->idxNum = 0;
      }
    return SQLITE_OK;
}

static int
velem_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualElementaryPtr p_vt = (VirtualElementaryPtr) pVTab;
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
velem_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases velem_disconnect() */
    return velem_disconnect (pVTab);
}

static int
velem_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    VirtualElementaryCursorPtr cursor =
	(VirtualElementaryCursorPtr)
	sqlite3_malloc (sizeof (VirtualElementaryCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = (VirtualElementaryPtr) pVTab;
    cursor->eof = 1;
    cursor->db_prefix = NULL;
    cursor->f_table_name = NULL;
    cursor->f_geometry_column = NULL;
    cursor->geometries = NULL;
    cursor->count = 0;
    cursor->current = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    return SQLITE_OK;
}

static int
velem_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualElementaryCursorPtr cursor = (VirtualElementaryCursorPtr) pCursor;
    velem_reset_cache (cursor);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
velem_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	      int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    char *db_prefix = NULL;
    char *table_name = NULL;
    char *geom_column = NULL;
    char *xprefix = NULL;
    char *xtable = NULL;
    char *xgeom = NULL;
    char *table_nameQ;
    char *geo_nameQ;
    char *sql_statement;
    sqlite3_int64 rowid_value;
    int ok_prefix = 0;
    int ok_table = 0;
    int ok_geom = 0;
    int ok_rowid = 0;
    int exists;
    int ret;
    sqlite3_stmt *stmt = NULL;
    VirtualElementaryCursorPtr cursor = (VirtualElementaryCursorPtr) pCursor;
    VirtualElementaryPtr elem = (VirtualElementaryPtr) cursor->pVtab;
    if (idxStr)
	idxStr = idxStr;	/* unused arg warning suppression */
    cursor->eof = 1;
    if (idxNum == 1 && argc == 3)
      {
	  /* retrieving the Table/Column/ROWID params */
	  if (sqlite3_value_type (argv[0]) == SQLITE_TEXT)
	    {
		table_name = (char *) sqlite3_value_text (argv[0]);
		ok_table = 1;
	    }
	  if (sqlite3_value_type (argv[1]) == SQLITE_TEXT)
	    {
		geom_column = (char *) sqlite3_value_text (argv[1]);
		ok_geom = 1;
	    }
	  if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
	    {
		rowid_value = sqlite3_value_int64 (argv[2]);
		ok_rowid = 1;
	    }
	  if (ok_table && ok_geom && ok_rowid)
	      ;
	  else
	    {
		/* invalid args */
		goto stop;
	    }
      }
    if (idxNum == 2 && argc == 2)
      {
	  /* retrieving the Table/ROWID params */
	  if (sqlite3_value_type (argv[0]) == SQLITE_TEXT)
	    {
		table_name = (char *) sqlite3_value_text (argv[0]);
		ok_table = 1;
	    }
	  if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
	    {
		rowid_value = sqlite3_value_int64 (argv[1]);
		ok_rowid = 1;
	    }
	  if (ok_table && ok_rowid)
	      ;
	  else
	    {
		/* invalid args */
		goto stop;
	    }
      }
    if (idxNum == 3 && argc == 4)
      {
	  /* retrieving the DbPrefix/Table/Column/ROWID params */
	  if (sqlite3_value_type (argv[0]) == SQLITE_TEXT)
	    {
		db_prefix = (char *) sqlite3_value_text (argv[0]);
		ok_prefix = 1;
	    }
	  if (sqlite3_value_type (argv[1]) == SQLITE_TEXT)
	    {
		table_name = (char *) sqlite3_value_text (argv[1]);
		ok_table = 1;
	    }
	  if (sqlite3_value_type (argv[2]) == SQLITE_TEXT)
	    {
		geom_column = (char *) sqlite3_value_text (argv[2]);
		ok_geom = 1;
	    }
	  if (sqlite3_value_type (argv[3]) == SQLITE_INTEGER)
	    {
		rowid_value = sqlite3_value_int64 (argv[3]);
		ok_rowid = 1;
	    }
	  if (ok_prefix && ok_table && ok_geom && ok_rowid)
	      ;
	  else
	    {
		/* invalid args */
		goto stop;
	    }
      }
    if (idxNum == 4 && argc == 3)
      {
	  /* retrieving the DbPrefix/Table/ROWID params */
	  if (sqlite3_value_type (argv[0]) == SQLITE_TEXT)
	    {
		db_prefix = (char *) sqlite3_value_text (argv[0]);
		ok_prefix = 1;
	    }
	  if (sqlite3_value_type (argv[1]) == SQLITE_TEXT)
	    {
		table_name = (char *) sqlite3_value_text (argv[1]);
		ok_table = 1;
	    }
	  if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
	    {
		rowid_value = sqlite3_value_int64 (argv[2]);
		ok_rowid = 1;
	    }
	  if (ok_prefix && ok_table && ok_rowid)
	      ;
	  else
	    {
		/* invalid args */
		goto stop;
	    }
      }

/* checking if the corresponding Table/Geometry exists */
    exists =
	velem_find_geometry (elem->db, db_prefix, table_name, geom_column,
			     &xprefix, &xtable, &xgeom);
    if (!exists)
	goto stop;

/* building the basic query */
    table_nameQ = gaiaDoubleQuotedSql (xtable);
    geo_nameQ = gaiaDoubleQuotedSql (xgeom);
    if (db_prefix == NULL)
      {
	  sql_statement =
	      sqlite3_mprintf ("SELECT \"%s\" FROM \"%s\" WHERE ROWID = ?",
			       geo_nameQ, table_nameQ);
      }
    else
      {
	  char *quoted_db = gaiaDoubleQuotedSql (db_prefix);
	  sql_statement =
	      sqlite3_mprintf
	      ("SELECT \"%s\" FROM \"%s\".\"%s\" WHERE ROWID = ?", geo_nameQ,
	       quoted_db, table_nameQ);
	  free (quoted_db);
      }
    free (geo_nameQ);
    free (table_nameQ);
    ret =
	sqlite3_prepare_v2 (elem->db, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    sqlite3_free (sql_statement);
    if (ret != SQLITE_OK)
	goto stop;
/* binding stmt params [ROWID] */
    sqlite3_bind_int64 (stmt, 1, rowid_value);
    velem_reset_cache (cursor);
    cursor->eof = 0;
/* caching the ResultSet */
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_ROW)
      {
	  /* ok, found a corresponding row */
	  cursor->db_prefix = xprefix;
	  xprefix = NULL;
	  cursor->f_table_name = xtable;
	  xtable = NULL;
	  cursor->f_geometry_column = xgeom;
	  xgeom = NULL;
	  cursor->origin_rowid = rowid_value;
	  cursor->current = 0;
	  if (sqlite3_column_type (stmt, 0) == SQLITE_BLOB)
	    {
		/* preparing Elementary Geometries */
		gaiaGeomCollPtr geom;
		unsigned char *blob =
		    (unsigned char *) sqlite3_column_blob (stmt, 0);
		int blob_sz = sqlite3_column_bytes (stmt, 0);
		geom = gaiaFromSpatiaLiteBlobWkb (blob, blob_sz);
		if (geom == NULL)
		    cursor->eof = 1;
		else
		  {
		      int count = 0;
		      gaiaPointPtr pt;
		      gaiaLinestringPtr ln;
		      gaiaPolygonPtr pg;
		      pt = geom->FirstPoint;
		      while (pt)
			{
			    count++;
			    pt = pt->Next;
			}
		      ln = geom->FirstLinestring;
		      while (ln)
			{
			    count++;
			    ln = ln->Next;
			}
		      pg = geom->FirstPolygon;
		      while (pg)
			{
			    count++;
			    pg = pg->Next;
			}
		      cursor->count = count;
		      cursor->geometries =
			  malloc (sizeof (gaiaGeomCollPtr) * count);
		      for (count = 0; count < cursor->count; count++)
			  *(cursor->geometries + count) = NULL;
		      count = 0;
		      pt = geom->FirstPoint;
		      while (pt)
			{
			    gaiaGeomCollPtr elem =
				velem_from_point (pt, geom->Srid);
			    *(cursor->geometries + count++) = elem;
			    pt = pt->Next;
			}
		      ln = geom->FirstLinestring;
		      while (ln)
			{
			    gaiaGeomCollPtr elem =
				velem_from_linestring (ln, geom->Srid);
			    *(cursor->geometries + count++) = elem;
			    ln = ln->Next;
			}
		      pg = geom->FirstPolygon;
		      while (pg)
			{
			    gaiaGeomCollPtr elem =
				velem_from_polygon (pg, geom->Srid);
			    *(cursor->geometries + count++) = elem;
			    pg = pg->Next;
			}
		      gaiaFreeGeomColl (geom);
		  }
	    }
      }
    else
	cursor->eof = 1;
  stop:
    if (stmt != NULL)
	sqlite3_finalize (stmt);
    if (xtable)
	free (xtable);
    if (xgeom)
	free (xgeom);
    if (xprefix)
	free (xprefix);
    if (cursor->eof == 1)
	velem_reset_cache (cursor);
    return SQLITE_OK;
}

static int
velem_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching next row from cursor */
    VirtualElementaryCursorPtr cursor = (VirtualElementaryCursorPtr) pCursor;
    if (cursor->geometries == NULL)
	cursor->eof = 1;
    else
      {
	  cursor->current += 1;
	  if (cursor->current >= cursor->count)
	      cursor->eof = 1;
      }
    return SQLITE_OK;
}

static int
velem_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualElementaryCursorPtr cursor = (VirtualElementaryCursorPtr) pCursor;
    return cursor->eof;
}

static int
velem_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	      int column)
{
/* fetching value for the Nth column */
    VirtualElementaryCursorPtr cursor = (VirtualElementaryCursorPtr) pCursor;
    if (column == 0)
      {
	  /* the "db_prefix" column */
	  if (cursor->db_prefix == NULL)
	      sqlite3_result_null (pContext);
	  else
	      sqlite3_result_text (pContext, cursor->db_prefix,
				   strlen (cursor->db_prefix), SQLITE_STATIC);
      }
    if (column == 1)
      {
	  /* the "f_table_name" column */
	  if (cursor->f_table_name == NULL)
	      sqlite3_result_null (pContext);
	  else
	      sqlite3_result_text (pContext, cursor->f_table_name,
				   strlen (cursor->f_table_name),
				   SQLITE_STATIC);
      }
    if (column == 2)
      {
	  /* the "f_geometry_column" column */
	  if (cursor->f_geometry_column == NULL)
	      sqlite3_result_null (pContext);
	  else
	      sqlite3_result_text (pContext, cursor->f_geometry_column,
				   strlen (cursor->f_geometry_column),
				   SQLITE_STATIC);
      }
    if (column == 3)
      {
	  /* the "origin_rowid" column */
	  sqlite3_result_int64 (pContext, cursor->origin_rowid);
      }
    if (column == 4)
      {
	  /* the "item_no" column */
	  sqlite3_result_int (pContext, cursor->current);
      }
    if (column == 5)
      {
	  /* the "geometry" column */
	  gaiaGeomCollPtr geom = *(cursor->geometries + cursor->current);
	  if (geom == NULL)
	      sqlite3_result_null (pContext);
	  else
	    {
		unsigned char *blob;
		int size;
		gaiaToSpatiaLiteBlobWkb (geom, &blob, &size);
		sqlite3_result_blob (pContext, blob, size, free);
	    }
      }
    return SQLITE_OK;
}

static int
velem_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualElementaryCursorPtr cursor = (VirtualElementaryCursorPtr) pCursor;
    *pRowid = cursor->current;
    return SQLITE_OK;
}

static int
velem_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	      sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    if (pRowid || argc || argv || pVTab)
	pRowid = pRowid;	/* unused arg warning suppression */
/* read only datasource */
    return SQLITE_READONLY;
}

static int
velem_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
velem_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
velem_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
velem_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
velem_rename (sqlite3_vtab * pVTab, const char *zNew)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    if (zNew)
	zNew = zNew;		/* unused arg warning suppression */
    return SQLITE_ERROR;
}

static int
spliteVirtualElementaryInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_elem_module.iVersion = 1;
    my_elem_module.xCreate = &velem_create;
    my_elem_module.xConnect = &velem_connect;
    my_elem_module.xBestIndex = &velem_best_index;
    my_elem_module.xDisconnect = &velem_disconnect;
    my_elem_module.xDestroy = &velem_destroy;
    my_elem_module.xOpen = &velem_open;
    my_elem_module.xClose = &velem_close;
    my_elem_module.xFilter = &velem_filter;
    my_elem_module.xNext = &velem_next;
    my_elem_module.xEof = &velem_eof;
    my_elem_module.xColumn = &velem_column;
    my_elem_module.xRowid = &velem_rowid;
    my_elem_module.xUpdate = &velem_update;
    my_elem_module.xBegin = &velem_begin;
    my_elem_module.xSync = &velem_sync;
    my_elem_module.xCommit = &velem_commit;
    my_elem_module.xRollback = &velem_rollback;
    my_elem_module.xFindFunction = NULL;
    my_elem_module.xRename = &velem_rename;
    sqlite3_create_module_v2 (db, "VirtualElementary", &my_elem_module, NULL,
			      0);
    return rc;
}

SPATIALITE_PRIVATE int
virtual_elementary_extension_init (void *xdb)
{
    sqlite3 *db = (sqlite3 *) xdb;
    return spliteVirtualElementaryInit (db);
}
