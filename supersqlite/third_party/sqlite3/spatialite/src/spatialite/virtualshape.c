/*

 virtualshape.c -- SQLite3 extension [VIRTUAL TABLE accessing Shapefile]

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
#include <spatialite_private.h>

#ifdef _WIN32
#define strcasecmp	_stricmp
#endif /* not WIN32 */

#ifndef OMIT_ICONV		/* if ICONV is disabled no SHP support is available */

static struct sqlite3_module my_shape_module;

typedef struct VirtualShapeStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    gaiaShapefilePtr Shp;	/* the Shapefile struct */
    int Srid;			/* the Shapefile SRID */
    int text_dates;
} VirtualShape;
typedef VirtualShape *VirtualShapePtr;

typedef struct VirtualShapeConstraintStruct
{
/* a constraint to be verified for xFilter */
    int iColumn;		/* Column on left-hand side of constraint */
    int op;			/* Constraint operator */
    char valueType;		/* value Type ('I'=int,'D'=double,'T'=text) */
    sqlite3_int64 intValue;	/* Int64 comparison value */
    double dblValue;		/* Double comparison value */
    char *txtValue;		/* Text comparison value */
    struct VirtualShapeConstraintStruct *next;
} VirtualShapeConstraint;
typedef VirtualShapeConstraint *VirtualShapeConstraintPtr;


typedef struct VirtualShapeCursorStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualShapePtr pVtab;	/* Virtual table of this cursor */
    long current_row;		/* the current row ID */
    int blobSize;
    unsigned char *blobGeometry;
    int eof;			/* the EOF marker */
    VirtualShapeConstraintPtr firstConstraint;
    VirtualShapeConstraintPtr lastConstraint;
} VirtualShapeCursor;
typedef VirtualShapeCursor *VirtualShapeCursorPtr;

static int
vshp_has_metadata (sqlite3 * db, int *geotype)
{
/* testing the layout of virts_geometry_columns table */
    char **results;
    int ret;
    int rows;
    int columns;
    int i;
    int ok_virt_name = 0;
    int ok_virt_geometry = 0;
    int ok_srid = 0;
    int ok_geometry_type = 0;
    int ok_type = 0;
    int ok_coord_dimension = 0;

    ret =
	sqlite3_get_table (db, "PRAGMA table_info(virts_geometry_columns)",
			   &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    for (i = 1; i <= rows; i++)
      {
	  if (strcasecmp ("virt_name", results[(i * columns) + 1]) == 0)
	      ok_virt_name = 1;
	  if (strcasecmp ("virt_geometry", results[(i * columns) + 1]) == 0)
	      ok_virt_geometry = 1;
	  if (strcasecmp ("srid", results[(i * columns) + 1]) == 0)
	      ok_srid = 1;
	  if (strcasecmp ("geometry_type", results[(i * columns) + 1]) == 0)
	      ok_geometry_type = 1;
	  if (strcasecmp ("type", results[(i * columns) + 1]) == 0)
	      ok_type = 1;
	  if (strcasecmp ("coord_dimension", results[(i * columns) + 1]) == 0)
	      ok_coord_dimension = 1;
      }
    sqlite3_free_table (results);

    if (ok_virt_name && ok_virt_geometry && ok_srid && ok_geometry_type
	&& ok_coord_dimension)
      {
	  *geotype = 1;
	  return 1;
      }
    if (ok_virt_name && ok_virt_geometry && ok_srid && ok_type)
      {
	  *geotype = 0;
	  return 1;
      }
    return 0;
}

static int
vshp_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some shapefile */
    char *sql;
    VirtualShapePtr p_vt;
    char path[2048];
    char encoding[128];
    const char *pEncoding = NULL;
    int len;
    const char *pPath = NULL;
    int srid;
    gaiaDbfFieldPtr pFld;
    int cnt;
    int col_cnt;
    int seed;
    int dup;
    int idup;
    int text_dates = 0;
    char *xname;
    char **col_name = NULL;
    int geotype;
    gaiaOutBuffer sql_statement;
    if (pAux)
	pAux = pAux;		/* unused arg warning suppression */
/* checking for shapefile PATH */
    if (argc == 6 || argc == 7)
      {
	  pPath = argv[3];
	  len = strlen (pPath);
	  if ((*(pPath + 0) == '\'' || *(pPath + 0) == '"')
	      && (*(pPath + len - 1) == '\'' || *(pPath + len - 1) == '"'))
	    {
		/* the path is enclosed between quotes - we need to dequote it */
		strcpy (path, pPath + 1);
		len = strlen (path);
		*(path + len - 1) = '\0';
	    }
	  else
	      strcpy (path, pPath);
	  pEncoding = argv[4];
	  len = strlen (pEncoding);
	  if ((*(pEncoding + 0) == '\'' || *(pEncoding + 0) == '"')
	      && (*(pEncoding + len - 1) == '\''
		  || *(pEncoding + len - 1) == '"'))
	    {
		/* the charset-name is enclosed between quotes - we need to dequote it */
		strcpy (encoding, pEncoding + 1);
		len = strlen (encoding);
		*(encoding + len - 1) = '\0';
	    }
	  else
	      strcpy (encoding, pEncoding);
	  srid = atoi (argv[5]);
	  if (srid < 0)
	      srid = -1;
	  if (argc == 7)
	      text_dates = atoi (argv[6]);
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualShape module] CREATE VIRTUAL: illegal arg list {shp_path, encoding, srid}");
	  return SQLITE_ERROR;
      }
    p_vt = (VirtualShapePtr) sqlite3_malloc (sizeof (VirtualShape));
    if (!p_vt)
	return SQLITE_NOMEM;
    p_vt->pModule = &my_shape_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
    p_vt->db = db;
    p_vt->Shp = gaiaAllocShapefile ();
    p_vt->Srid = srid;
    p_vt->text_dates = text_dates;
/* trying to open files etc in order to ensure we actually have a genuine shapefile */
    gaiaOpenShpRead (p_vt->Shp, path, encoding, "UTF-8");
    if (!(p_vt->Shp->Valid))
      {
	  /* something is going the wrong way; creating a stupid default table */
	  xname = gaiaDoubleQuotedSql ((const char *) argv[2]);
	  sql =
	      sqlite3_mprintf
	      ("CREATE TABLE \"%s\" (PKUID INTEGER, Geometry BLOB)", xname);
	  free (xname);
	  if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
	    {
		sqlite3_free (sql);
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualShape module] cannot build a table from Shapefile\n");
		return SQLITE_ERROR;
	    }
	  sqlite3_free (sql);
	  *ppVTab = (sqlite3_vtab *) p_vt;
	  return SQLITE_OK;
      }
    if (p_vt->Shp->Shape == 3 || p_vt->Shp->Shape == 13 ||
	p_vt->Shp->Shape == 23 || p_vt->Shp->Shape == 5 ||
	p_vt->Shp->Shape == 15 || p_vt->Shp->Shape == 25)
      {
	  /* fixing anyway the Geometry type for LINESTRING/MULTILINESTRING or POLYGON/MULTIPOLYGON */
	  gaiaShpAnalyze (p_vt->Shp);
      }
/* preparing the COLUMNs for this VIRTUAL TABLE */
    gaiaOutBufferInitialize (&sql_statement);
    xname = gaiaDoubleQuotedSql (argv[2]);
    sql =
	sqlite3_mprintf ("CREATE TABLE \"%s\" (PKUID INTEGER, Geometry BLOB",
			 xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
/* checking for duplicate / illegal column names and antialising them */
    col_cnt = 0;
    pFld = p_vt->Shp->Dbf->First;
    while (pFld)
      {
	  /* counting DBF fields */
	  col_cnt++;
	  pFld = pFld->Next;
      }
    col_name = malloc (sizeof (char *) * col_cnt);
    cnt = 0;
    seed = 0;
    pFld = p_vt->Shp->Dbf->First;
    while (pFld)
      {
	  xname = gaiaDoubleQuotedSql (pFld->Name);
	  dup = 0;
	  for (idup = 0; idup < cnt; idup++)
	    {
		if (strcasecmp (xname, *(col_name + idup)) == 0)
		    dup = 1;
	    }
	  if (strcasecmp (xname, "\"PKUID\"") == 0)
	      dup = 1;
	  if (strcasecmp (xname, "\"Geometry\"") == 0)
	      dup = 1;
	  if (dup)
	    {
		free (xname);
		sql = sqlite3_mprintf ("COL_%d", seed++);
		xname = gaiaDoubleQuotedSql (sql);
		sqlite3_free (sql);
	    }
	  if (pFld->Type == 'N')
	    {
		if (pFld->Decimals > 0 || pFld->Length > 18)
		    sql = sqlite3_mprintf (", \"%s\" DOUBLE", xname);
		else
		    sql = sqlite3_mprintf (", \"%s\" INTEGER", xname);
	    }
	  else if (pFld->Type == 'F')
	      sql = sqlite3_mprintf (", \"%s\" DOUBLE", xname);
	  else if (pFld->Type == 'D')
	    {
		if (text_dates)
		    sql =
			sqlite3_mprintf (", \"%s\" VARCHAR(%d)", xname,
					 pFld->Length);
		else
		    sql = sqlite3_mprintf (", \"%s\" DOUBLE", xname);
	    }
	  else
	      sql =
		  sqlite3_mprintf (", \"%s\" VARCHAR(%d)", xname, pFld->Length);
	  gaiaAppendToOutBuffer (&sql_statement, sql);
	  sqlite3_free (sql);
	  *(col_name + cnt) = xname;
	  cnt++;
	  pFld = pFld->Next;
      }
    gaiaAppendToOutBuffer (&sql_statement, ")");
    if (col_name)
      {
	  /* releasing memory allocation for column names */
	  for (cnt = 0; cnt < col_cnt; cnt++)
	      free (*(col_name + cnt));
	  free (col_name);
      }
    if (sql_statement.Error == 0 && sql_statement.Buffer != NULL)
      {
	  if (sqlite3_declare_vtab (db, sql_statement.Buffer) != SQLITE_OK)
	    {
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualShape module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
		     sql_statement.Buffer);
		gaiaOutBufferReset (&sql_statement);
		return SQLITE_ERROR;
	    }
      }
    gaiaOutBufferReset (&sql_statement);
    *ppVTab = (sqlite3_vtab *) p_vt;

    if (vshp_has_metadata (db, &geotype))
      {
	  /* registering the Virtual Geometry */
	  if (geotype)
	    {
		int xtype = 0;
		int xdims = 0;
		switch (p_vt->Shp->EffectiveType)
		  {
		  case GAIA_POINT:
		      switch (p_vt->Shp->EffectiveDims)
			{
			case GAIA_XY_Z_M:
			    xtype = 3001;
			    xdims = 4;
			    break;
			case GAIA_XY_M:
			    xtype = 2001;
			    xdims = 3;
			    break;
			case GAIA_XY_Z:
			    xtype = 1001;
			    xdims = 3;
			    break;
			default:
			    xtype = 1;
			    xdims = 2;
			    break;
			};
		      break;
		  case GAIA_LINESTRING:
		      switch (p_vt->Shp->EffectiveDims)
			{
			case GAIA_XY_Z_M:
			    xtype = 3002;
			    xdims = 4;
			    break;
			case GAIA_XY_M:
			    xtype = 2002;
			    xdims = 3;
			    break;
			case GAIA_XY_Z:
			    xtype = 1002;
			    xdims = 3;
			    break;
			default:
			    xtype = 2;
			    xdims = 2;
			    break;
			};
		      break;
		  case GAIA_POLYGON:
		      switch (p_vt->Shp->EffectiveDims)
			{
			case GAIA_XY_Z_M:
			    xtype = 3003;
			    xdims = 4;
			    break;
			case GAIA_XY_M:
			    xtype = 2003;
			    xdims = 3;
			    break;
			case GAIA_XY_Z:
			    xtype = 1003;
			    xdims = 3;
			    break;
			default:
			    xtype = 3;
			    xdims = 2;
			    break;
			};
		      break;
		  case GAIA_MULTIPOINT:
		      switch (p_vt->Shp->EffectiveDims)
			{
			case GAIA_XY_Z_M:
			    xtype = 3004;
			    xdims = 4;
			    break;
			case GAIA_XY_M:
			    xtype = 2004;
			    xdims = 3;
			    break;
			case GAIA_XY_Z:
			    xtype = 1004;
			    xdims = 3;
			    break;
			default:
			    xtype = 4;
			    xdims = 2;
			    break;
			};
		      break;
		  case GAIA_MULTILINESTRING:
		      switch (p_vt->Shp->EffectiveDims)
			{
			case GAIA_XY_Z_M:
			    xtype = 3005;
			    xdims = 4;
			    break;
			case GAIA_XY_M:
			    xtype = 2005;
			    xdims = 3;
			    break;
			case GAIA_XY_Z:
			    xtype = 1005;
			    xdims = 3;
			    break;
			default:
			    xtype = 5;
			    xdims = 2;
			    break;
			};
		      break;
		  case GAIA_MULTIPOLYGON:
		      switch (p_vt->Shp->EffectiveDims)
			{
			case GAIA_XY_Z_M:
			    xtype = 3006;
			    xdims = 4;
			    break;
			case GAIA_XY_M:
			    xtype = 2006;
			    xdims = 3;
			    break;
			case GAIA_XY_Z:
			    xtype = 1006;
			    xdims = 3;
			    break;
			default:
			    xtype = 6;
			    xdims = 2;
			    break;
			};
		      break;
		  };
		sql =
		    sqlite3_mprintf
		    ("INSERT OR IGNORE INTO virts_geometry_columns "
		     "(virt_name, virt_geometry, geometry_type, coord_dimension, srid) "
		     "VALUES (Lower(%Q), 'geometry', %d, %d, %d)", argv[2],
		     xtype, xdims, p_vt->Srid);
	    }
	  else
	    {
		const char *xgtype = "GEOMETRY";
		switch (p_vt->Shp->EffectiveType)
		  {
		  case GAIA_POINT:
		      xgtype = "POINT";
		      break;
		  case GAIA_LINESTRING:
		      xgtype = "LINESTRING";
		      break;
		  case GAIA_POLYGON:
		      xgtype = "POLYGON";
		      break;
		  case GAIA_MULTIPOINT:
		      xgtype = "MULTIPOINT";
		      break;
		  case GAIA_MULTILINESTRING:
		      xgtype = "MULTILINESTRING";
		      break;
		  case GAIA_MULTIPOLYGON:
		      xgtype = "MULTIPOLYGON";
		      break;
		  };
		sql =
		    sqlite3_mprintf
		    ("INSERT OR IGNORE INTO virts_geometry_columns "
		     "(virt_name, virt_geometry, type, srid) "
		     "VALUES (Lower(%Q), 'geometry', %Q, %d)", argv[2], xgtype,
		     p_vt->Srid);
	    }
	  sqlite3_exec (db, sql, NULL, NULL, NULL);
	  sqlite3_free (sql);
      }
    if (checkSpatialMetaData (db) == 3)
      {
	  /* current metadata style >= v.4.0.0 */

	  /* inserting a row into VIRTS_GEOMETRY_COLUMNS_AUTH */
	  sql = sqlite3_mprintf ("INSERT OR IGNORE INTO "
				 "virts_geometry_columns_auth (virt_name, virt_geometry, hidden) "
				 "VALUES (Lower(%Q), 'geometry', 0)", argv[2]);
	  sqlite3_exec (db, sql, NULL, NULL, NULL);
	  sqlite3_free (sql);

	  /* inserting a row into GEOMETRY_COLUMNS_STATISTICS */
	  sql = sqlite3_mprintf ("INSERT OR IGNORE INTO "
				 "virts_geometry_columns_statistics (virt_name, virt_geometry) "
				 "VALUES (Lower(%Q), 'geometry')", argv[2]);
	  sqlite3_exec (db, sql, NULL, NULL, NULL);
	  sqlite3_free (sql);
      }

    return SQLITE_OK;
}

static int
vshp_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vshp_create() */
    return vshp_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vshp_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
{
/* best index selection */
    int i;
    int iArg = 0;
    char str[2048];
    char buf[64];

    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */

    *str = '\0';
    for (i = 0; i < pIndex->nConstraint; i++)
      {
	  if (pIndex->aConstraint[i].usable)
	    {
		iArg++;
		pIndex->aConstraintUsage[i].argvIndex = iArg;
		pIndex->aConstraintUsage[i].omit = 1;
		sprintf (buf, "%d:%d,", pIndex->aConstraint[i].iColumn,
			 pIndex->aConstraint[i].op);
		strcat (str, buf);
	    }
      }
    if (*str != '\0')
      {
	  pIndex->idxStr = sqlite3_mprintf ("%s", str);
	  pIndex->needToFreeIdxStr = 1;
      }

    return SQLITE_OK;
}

static int
vshp_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualShapePtr p_vt = (VirtualShapePtr) pVTab;
    if (p_vt->Shp)
	gaiaFreeShapefile (p_vt->Shp);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
vshp_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vshp_disconnect() */
    return vshp_disconnect (pVTab);
}

static void
vshp_read_row (VirtualShapeCursorPtr cursor)
{
/* trying to read a "row" from shapefile */
    int ret;
    gaiaGeomCollPtr geom;
    if (!(cursor->pVtab->Shp->Valid))
      {
	  cursor->eof = 1;
	  return;
      }
    if (cursor->blobGeometry)
      {
	  free (cursor->blobGeometry);
	  cursor->blobGeometry = NULL;
      }
    ret =
	gaiaReadShpEntity_ex (cursor->pVtab->Shp, cursor->current_row,
			      cursor->pVtab->Srid, cursor->pVtab->text_dates);
    if (!ret)
      {
	  if (!(cursor->pVtab->Shp->LastError))	/* normal SHP EOF */
	    {
		cursor->eof = 1;
		return;
	    }
	  /* an error occurred */
	  spatialite_e ("%s\n", cursor->pVtab->Shp->LastError);
	  cursor->eof = 1;
	  return;
      }
    cursor->current_row++;
    geom = cursor->pVtab->Shp->Dbf->Geometry;
    if (geom)
      {
	  /* preparing the BLOB representing Geometry */
	  gaiaToSpatiaLiteBlobWkb (geom, &(cursor->blobGeometry),
				   &(cursor->blobSize));
      }
}

static int
vshp_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    VirtualShapeCursorPtr cursor =
	(VirtualShapeCursorPtr) sqlite3_malloc (sizeof (VirtualShapeCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->firstConstraint = NULL;
    cursor->lastConstraint = NULL;
    cursor->pVtab = (VirtualShapePtr) pVTab;
    cursor->current_row = 0;
    cursor->blobGeometry = NULL;
    cursor->blobSize = 0;
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    vshp_read_row (cursor);
    return SQLITE_OK;
}

static void
vshp_free_constraints (VirtualShapeCursorPtr cursor)
{
/* memory cleanup - cursor constraints */
    VirtualShapeConstraintPtr pC;
    VirtualShapeConstraintPtr pCn;
    pC = cursor->firstConstraint;
    while (pC)
      {
	  pCn = pC->next;
	  if (pC->txtValue)
	      sqlite3_free (pC->txtValue);
	  sqlite3_free (pC);
	  pC = pCn;
      }
    cursor->firstConstraint = NULL;
    cursor->lastConstraint = NULL;
}

static int
vshp_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    if (cursor->blobGeometry)
	free (cursor->blobGeometry);
    vshp_free_constraints (cursor);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vshp_parse_constraint (const char *str, int index, int *iColumn, int *op)
{
/* parsing a constraint string */
    char buf[64];
    const char *in = str;
    char *out = buf;
    int i = 0;
    int found = 0;

    *out = '\0';
    while (*in != '\0')
      {
	  if (*in == ',')
	    {
		if (index == i)
		  {
		      *out = '\0';
		      found = 1;
		      break;
		  }
		i++;
		in++;
		continue;
	    }
	  if (index == i)
	      *out++ = *in;
	  in++;
      }
    if (!found)
	return 0;
    in = buf;
    for (i = 0; i < (int) strlen (buf); i++)
      {
	  if (buf[i] == ':')
	    {
		buf[i] = '\0';
		*iColumn = atoi (buf);
		*op = atoi (buf + i + 1);
		return 1;
	    }
	  in++;
      }
    return 0;
}

static int
vshp_eval_constraints (VirtualShapeCursorPtr cursor)
{
/* evaluating Filter constraints */
    int nCol;
    gaiaDbfFieldPtr pFld;
    VirtualShapeConstraintPtr pC = cursor->firstConstraint;
    if (pC == NULL)
	return 1;
    while (pC)
      {
	  int ok = 0;
	  if (pC->iColumn == 0)
	    {
		/* the PRIMARY KEY column */
		if (pC->valueType == 'I')
		  {
		      switch (pC->op)
			{
			case SQLITE_INDEX_CONSTRAINT_EQ:
			    if (cursor->current_row == pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GT:
			    if (cursor->current_row > pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LE:
			    if (cursor->current_row <= pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LT:
			    if (cursor->current_row < pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GE:
			    if (cursor->current_row >= pC->intValue)
				ok = 1;
			    break;
			};
		  }
		goto done;
	    }
	  nCol = 2;
	  pFld = cursor->pVtab->Shp->Dbf->First;
	  while (pFld)
	    {
		if (nCol == pC->iColumn)
		  {
		      if ((pFld->Value))
			{
			    switch (pFld->Value->Type)
			      {
			      case GAIA_INT_VALUE:
				  if (pC->valueType == 'I')
				    {
					switch (pC->op)
					  {
					  case SQLITE_INDEX_CONSTRAINT_EQ:
					      if (pFld->Value->IntValue ==
						  pC->intValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_GT:
					      if (pFld->Value->IntValue >
						  pC->intValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_LE:
					      if (pFld->Value->IntValue <=
						  pC->intValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_LT:
					      if (pFld->Value->IntValue <
						  pC->intValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_GE:
					      if (pFld->Value->IntValue >=
						  pC->intValue)
						  ok = 1;
					      break;
					  };
				    }
				  break;
			      case GAIA_DOUBLE_VALUE:
				  if (pC->valueType == 'I')
				    {
					switch (pC->op)
					  {
					  case SQLITE_INDEX_CONSTRAINT_EQ:
					      if (pFld->Value->DblValue ==
						  pC->intValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_GT:
					      if (pFld->Value->DblValue >
						  pC->intValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_LE:
					      if (pFld->Value->DblValue <=
						  pC->intValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_LT:
					      if (pFld->Value->DblValue <
						  pC->intValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_GE:
					      if (pFld->Value->DblValue >=
						  pC->intValue)
						  ok = 1;
					      break;
					  };
				    }
				  if (pC->valueType == 'D')
				    {
					switch (pC->op)
					  {
					  case SQLITE_INDEX_CONSTRAINT_EQ:
					      if (pFld->Value->DblValue ==
						  pC->dblValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_GT:
					      if (pFld->Value->DblValue >
						  pC->dblValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_LE:
					      if (pFld->Value->DblValue <=
						  pC->dblValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_LT:
					      if (pFld->Value->DblValue <
						  pC->dblValue)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_GE:
					      if (pFld->Value->DblValue >=
						  pC->dblValue)
						  ok = 1;
					      break;
					  }
				    }
				  break;
			      case GAIA_TEXT_VALUE:
				  if (pC->valueType == 'T' && pC->txtValue)
				    {
					int ret;
					ret =
					    strcmp (pFld->Value->TxtValue,
						    pC->txtValue);
					switch (pC->op)
					  {
					  case SQLITE_INDEX_CONSTRAINT_EQ:
					      if (ret == 0)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_GT:
					      if (ret > 0)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_LE:
					      if (ret <= 0)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_LT:
					      if (ret < 0)
						  ok = 1;
					      break;
					  case SQLITE_INDEX_CONSTRAINT_GE:
					      if (ret >= 0)
						  ok = 1;
					      break;
					  };
				    }
				  break;
			      };
			}
		      goto done;
		  }
		nCol++;
		pFld = pFld->Next;
	    }
	done:
	  if (!ok)
	      return 0;
	  pC = pC->next;
      }
    return 1;
}

static int
vshp_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    int i;
    int iColumn;
    int op;
    int len;
    VirtualShapeConstraintPtr pC;
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    if (idxNum)
	idxNum = idxNum;	/* unused arg warning suppression */

/* resetting any previously set filter constraint */
    vshp_free_constraints (cursor);

    for (i = 0; i < argc; i++)
      {
	  if (!vshp_parse_constraint (idxStr, i, &iColumn, &op))
	      continue;
	  pC = sqlite3_malloc (sizeof (VirtualShapeConstraint));
	  if (!pC)
	      continue;
	  pC->iColumn = iColumn;
	  pC->op = op;
	  pC->valueType = '\0';
	  pC->txtValue = NULL;
	  pC->next = NULL;

	  if (sqlite3_value_type (argv[i]) == SQLITE_INTEGER)
	    {
		pC->valueType = 'I';
		pC->intValue = sqlite3_value_int64 (argv[i]);
	    }
	  if (sqlite3_value_type (argv[i]) == SQLITE_FLOAT)
	    {
		pC->valueType = 'D';
		pC->dblValue = sqlite3_value_double (argv[i]);
	    }
	  if (sqlite3_value_type (argv[i]) == SQLITE_TEXT)
	    {
		pC->valueType = 'T';
		len = sqlite3_value_bytes (argv[i]) + 1;
		pC->txtValue = (char *) sqlite3_malloc (len);
		if (pC->txtValue)
		    strcpy (pC->txtValue,
			    (char *) sqlite3_value_text (argv[i]));
	    }
	  if (cursor->firstConstraint == NULL)
	      cursor->firstConstraint = pC;
	  if (cursor->lastConstraint != NULL)
	      cursor->lastConstraint->next = pC;
	  cursor->lastConstraint = pC;
      }

    cursor->current_row = 0;
    if (cursor->blobGeometry)
	free (cursor->blobGeometry);
    cursor->blobGeometry = NULL;
    cursor->blobSize = 0;
    cursor->eof = 0;
    while (1)
      {
	  vshp_read_row (cursor);
	  if (cursor->eof)
	      break;
	  if (vshp_eval_constraints (cursor))
	      break;
      }
    return SQLITE_OK;
}

static int
vshp_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching a next row from cursor */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    while (1)
      {
	  vshp_read_row (cursor);
	  if (cursor->eof)
	      break;
	  if (vshp_eval_constraints (cursor))
	      break;
      }
    return SQLITE_OK;
}

static int
vshp_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    return cursor->eof;
}

static int
vshp_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    int nCol = 2;
    gaiaGeomCollPtr geom;
    gaiaDbfFieldPtr pFld;
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    if (column == 0)
      {
	  /* the PRIMARY KEY column */
	  sqlite3_result_int (pContext, cursor->current_row);
	  return SQLITE_OK;
      }
    if (column == 1)
      {
	  /* the GEOMETRY column */
	  geom = cursor->pVtab->Shp->Dbf->Geometry;
	  if (geom)
	      sqlite3_result_blob (pContext, cursor->blobGeometry,
				   cursor->blobSize, SQLITE_STATIC);
	  else
	      sqlite3_result_null (pContext);
	  return SQLITE_OK;
      }
    pFld = cursor->pVtab->Shp->Dbf->First;
    while (pFld)
      {
	  /* column values */
	  if (nCol == column)
	    {
		if (!(pFld->Value))
		    sqlite3_result_null (pContext);
		else
		  {
		      switch (pFld->Value->Type)
			{
			case GAIA_INT_VALUE:
			    sqlite3_result_int64 (pContext,
						  pFld->Value->IntValue);
			    break;
			case GAIA_DOUBLE_VALUE:
			    sqlite3_result_double (pContext,
						   pFld->Value->DblValue);
			    break;
			case GAIA_TEXT_VALUE:
			    sqlite3_result_text (pContext,
						 pFld->Value->TxtValue,
						 strlen (pFld->Value->TxtValue),
						 SQLITE_STATIC);
			    break;
			default:
			    sqlite3_result_null (pContext);
			    break;
			}
		  }
		break;
	    }
	  nCol++;
	  pFld = pFld->Next;
      }
    return SQLITE_OK;
}

static int
vshp_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vshp_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    if (pVTab || argc || argv || pRowid)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_READONLY;
}

static int
vshp_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vshp_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vshp_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vshp_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vshp_rename (sqlite3_vtab * pVTab, const char *zNew)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    if (zNew)
	zNew = zNew;		/* unused arg warning suppression */
    return SQLITE_ERROR;
}

static int
spliteVirtualShapeInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_shape_module.iVersion = 1;
    my_shape_module.xCreate = &vshp_create;
    my_shape_module.xConnect = &vshp_connect;
    my_shape_module.xBestIndex = &vshp_best_index;
    my_shape_module.xDisconnect = &vshp_disconnect;
    my_shape_module.xDestroy = &vshp_destroy;
    my_shape_module.xOpen = &vshp_open;
    my_shape_module.xClose = &vshp_close;
    my_shape_module.xFilter = &vshp_filter;
    my_shape_module.xNext = &vshp_next;
    my_shape_module.xEof = &vshp_eof;
    my_shape_module.xColumn = &vshp_column;
    my_shape_module.xRowid = &vshp_rowid;
    my_shape_module.xUpdate = &vshp_update;
    my_shape_module.xBegin = &vshp_begin;
    my_shape_module.xSync = &vshp_sync;
    my_shape_module.xCommit = &vshp_commit;
    my_shape_module.xRollback = &vshp_rollback;
    my_shape_module.xFindFunction = NULL;
    my_shape_module.xRename = &vshp_rename;
    sqlite3_create_module_v2 (db, "VirtualShape", &my_shape_module, NULL, 0);
    return rc;
}

SPATIALITE_PRIVATE int
virtualshape_extension_init (void *xdb)
{
    sqlite3 *db = (sqlite3 *) xdb;
    return spliteVirtualShapeInit (db);
}

#endif /* ICONV enabled/disabled */
