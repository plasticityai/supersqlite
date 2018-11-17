/*

 virtualgpkg.c -- SQLite3 extension [VIRTUAL TABLE accessing GPKG tables]

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
 
Portions created by the Initial Developer are Copyright (C) 2015
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
#include <spatialite/geopackage.h>

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

#ifdef ENABLE_GEOPACKAGE	/* only if GeoPackage support is enabled */

struct sqlite3_module my_gpkg_module;

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

typedef struct VirtualGPKGStruct
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
    char *GeoColumn;		/* name of the Geometry column */
    int Srid;			/* SRID of the Geometry column */
    int GeoType;		/* Type of the Geometry column */
} VirtualGPKG;
typedef VirtualGPKG *VirtualGPKGPtr;

typedef struct VirtualGPKGCursorStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualGPKGPtr pVtab;	/* Virtual table of this cursor */
    sqlite3_stmt *stmt;
    sqlite3_int64 current_row;	/* the current row ID */
    int eof;			/* the EOF marker */
} VirtualGPKGCursor;
typedef VirtualGPKGCursor *VirtualGPKGCursorPtr;

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
vgpkg_read_row (VirtualGPKGCursorPtr cursor)
{
/* trying to read a row from GPKG real-table */
    sqlite3_stmt *stmt;
    int ret;
    int ic;
    const char *text;
    const unsigned char *blob;
    int size;
    sqlite3_int64 pk;
    stmt = cursor->stmt;
    sqlite3_bind_int64 (stmt, 1, cursor->current_row);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_ROW)
      {
	  pk = sqlite3_column_int64 (stmt, 0);
	  for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
	    {
		/* fetching column values */
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
vgpkg_insert_row (VirtualGPKGPtr p_vt, sqlite3_int64 * rowid, int argc,
		  sqlite3_value ** argv)
{
/* trying to insert a row into GPKG real-table */
    sqlite3_stmt *stmt;
    int ret;
    int i;
    int ic;
    char prefix[16];
    const char *text;
    const unsigned char *blob;
    gaiaOutBuffer sql_statement;
    int size;
    char *sql;
    char buf[256];
    char *xname;
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
	  if (strcasecmp (*(p_vt->Column + ic), p_vt->GeoColumn) == 0)
	    {
		/* this is the geometry column */
		sql = sqlite3_mprintf ("%sAsGPB(?)", prefix);
		gaiaAppendToOutBuffer (&sql_statement, sql);
		sqlite3_free (sql);
	    }
	  else
	    {
		sprintf (buf, "%s?", prefix);
		gaiaAppendToOutBuffer (&sql_statement, buf);
	    }
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
    sqlite3_finalize (stmt);
    *rowid = sqlite3_last_insert_rowid (p_vt->db);
    return SQLITE_OK;
}

static int
vgpkg_update_row (VirtualGPKGPtr p_vt, sqlite3_int64 rowid, int argc,
		  sqlite3_value ** argv)
{
/* trying to update a row in GPKG real-table */
    sqlite3_stmt *stmt;
    int ret;
    int i;
    int ic;
    char prefix[16];
    const char *text;
    const unsigned char *blob;
    gaiaOutBuffer sql_statement;
    int size;
    char *sql;
    char buf[256];
    char *xname;
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
	  if (strcasecmp (*(p_vt->Column + ic), p_vt->GeoColumn) == 0)
	    {
		/* this is the geometry column */
		sql = sqlite3_mprintf ("%s%s = AsGPB(?)", prefix, xname);
	    }
	  else
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
vgpkg_delete_row (VirtualGPKGPtr p_vt, sqlite3_int64 rowid)
{
/* trying to delete a row from GPKG real-table */
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

static void
free_table (VirtualGPKGPtr p_vt)
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
	sqlite3_free (p_vt->GeoColumn);
    sqlite3_free (p_vt);
}

static int
vgpkg_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some GPKG table */
    char *vtable = NULL;
    char *table = NULL;
    int ret;
    int i;
    int len;
    int n_rows;
    int n_columns;
    const char *col_name;
    const char *col_type;
    int not_null;
    const char *type;
    int has_z;
    int has_m;
    char **results;
    char *sql;
    char prefix[16];
    char *xname;
    gaiaOutBuffer sql_statement;
    VirtualGPKGPtr p_vt = NULL;
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
	      ("[VirtualGPKG module] CREATE VIRTUAL: illegal arg list {table_name}\n");
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
	  p_vt = (VirtualGPKGPtr) sqlite3_malloc (sizeof (VirtualGPKG));
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
	  p_vt->GeoColumn = NULL;
	  p_vt->Srid = -1;
	  p_vt->GeoType = GAIA_UNKNOWN;
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
    sql =
	sqlite3_mprintf
	("SELECT column_name, geometry_type_name, srs_id, z, m\n"
	 "FROM gpkg_geometry_columns WHERE Upper(table_name) = Upper(%Q)",
	 table);
    ret = sqlite3_get_table (db, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto illegal;
    if (n_rows >= 1)
      {
	  col_name = results[n_columns + 0];
	  type = results[n_columns + 1];
	  p_vt->Srid = atoi (results[n_columns + 2]);
	  has_z = atoi (results[n_columns + 3]);
	  has_m = atoi (results[n_columns + 4]);
	  len = strlen (col_name);
	  p_vt->GeoColumn = sqlite3_malloc (len + 1);
	  strcpy (p_vt->GeoColumn, col_name);
	  if (strcasecmp (type, "POINT") == 0)
	    {
		if (has_z && has_m)
		    p_vt->GeoType = GAIA_POINTZM;
		else if (has_z)
		    p_vt->GeoType = GAIA_POINTZ;
		else if (has_m)
		    p_vt->GeoType = GAIA_POINTM;
		else
		    p_vt->GeoType = GAIA_POINT;
	    }
	  if (strcasecmp (type, "LINESTRING") == 0)
	    {
		if (has_z && has_m)
		    p_vt->GeoType = GAIA_LINESTRINGZM;
		else if (has_z)
		    p_vt->GeoType = GAIA_LINESTRINGZ;
		else if (has_m)
		    p_vt->GeoType = GAIA_LINESTRINGM;
		else
		    p_vt->GeoType = GAIA_LINESTRING;
	    }
	  if (strcasecmp (type, "POLYGON") == 0)
	    {
		if (has_z && has_m)
		    p_vt->GeoType = GAIA_POLYGONZM;
		else if (has_z)
		    p_vt->GeoType = GAIA_POLYGONZ;
		else if (has_m)
		    p_vt->GeoType = GAIA_POLYGONM;
		else
		    p_vt->GeoType = GAIA_POLYGON;
	    }
	  if (strcasecmp (type, "MULTIPOINT") == 0)
	    {
		if (has_z && has_m)
		    p_vt->GeoType = GAIA_MULTIPOINTZM;
		else if (has_z)
		    p_vt->GeoType = GAIA_MULTIPOINTZ;
		else if (has_m)
		    p_vt->GeoType = GAIA_MULTIPOINTM;
		else
		    p_vt->GeoType = GAIA_MULTIPOINT;
	    }
	  if (strcasecmp (type, "MULTILINESTRING") == 0)
	    {
		if (has_z && has_m)
		    p_vt->GeoType = GAIA_MULTILINESTRINGZM;
		else if (has_z)
		    p_vt->GeoType = GAIA_MULTILINESTRINGZ;
		else if (has_m)
		    p_vt->GeoType = GAIA_MULTILINESTRINGM;
		else
		    p_vt->GeoType = GAIA_MULTILINESTRING;
	    }
	  if (strcasecmp (type, "MULTIPOLYGON") == 0)
	    {
		if (has_z && has_m)
		    p_vt->GeoType = GAIA_MULTIPOLYGONZM;
		else if (has_z)
		    p_vt->GeoType = GAIA_MULTIPOLYGONZ;
		else if (has_m)
		    p_vt->GeoType = GAIA_MULTIPOLYGONM;
		else
		    p_vt->GeoType = GAIA_MULTIPOLYGON;
	    }
	  if (strcasecmp (type, "GEOMCOLLECTION") == 0)
	    {
		if (has_z && has_m)
		    p_vt->GeoType = GAIA_GEOMETRYCOLLECTIONZM;
		else if (has_z)
		    p_vt->GeoType = GAIA_GEOMETRYCOLLECTIONZ;
		else if (has_m)
		    p_vt->GeoType = GAIA_GEOMETRYCOLLECTIONM;
		else
		    p_vt->GeoType = GAIA_GEOMETRYCOLLECTION;
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
		    ("[VirtualGPKG module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
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
	("[VirtualGPKG module] '%s' isn't a valid GPKG Geometry table\n",
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
vgpkg_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	       sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vgpkg_create() */
    return vgpkg_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vgpkg_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
{
/* best index selection */
    if (pVTab || pIndex)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vgpkg_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualGPKGPtr p_vt = (VirtualGPKGPtr) pVTab;
    free_table (p_vt);
    return SQLITE_OK;
}

static int
vgpkg_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vgpkg_disconnect() */
    return vgpkg_disconnect (pVTab);
}

static int
vgpkg_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    sqlite3_stmt *stmt;
    gaiaOutBuffer sql_statement;
    int ret;
    char *sql;
    int ic;
    char *xname;
    VirtualGPKGCursorPtr cursor =
	(VirtualGPKGCursorPtr) sqlite3_malloc (sizeof (VirtualGPKGCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = (VirtualGPKGPtr) pVTab;
    gaiaOutBufferInitialize (&sql_statement);
    gaiaAppendToOutBuffer (&sql_statement, "SELECT ROWID");
    for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
      {
	  value_set_null (*(cursor->pVtab->Value + ic));
	  xname = gaiaDoubleQuotedSql (*(cursor->pVtab->Column + ic));
	  if (strcasecmp
	      (*(cursor->pVtab->Column + ic), cursor->pVtab->GeoColumn) == 0)
	    {
		/* this is the geometry column */
		sql = sqlite3_mprintf (",GeomFromGPB(\"%s\")", xname);
	    }
	  else
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
    vgpkg_read_row (cursor);
    return SQLITE_OK;
}

static int
vgpkg_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    int ic;
    VirtualGPKGCursorPtr cursor = (VirtualGPKGCursorPtr) pCursor;
    for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
	value_set_null (*(cursor->pVtab->Value + ic));
    if (cursor->stmt)
	sqlite3_finalize (cursor->stmt);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vgpkg_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	      int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    if (pCursor || idxNum || idxStr || argc || argv)
	pCursor = pCursor;	/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vgpkg_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching next row from cursor */
    VirtualGPKGCursorPtr cursor = (VirtualGPKGCursorPtr) pCursor;
    (cursor->current_row)++;
    vgpkg_read_row (cursor);
    return SQLITE_OK;
}

static int
vgpkg_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualGPKGCursorPtr cursor = (VirtualGPKGCursorPtr) pCursor;
    return cursor->eof;
}

static int
vgpkg_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	      int column)
{
/* fetching value for the Nth column */
    VirtualGPKGCursorPtr cursor = (VirtualGPKGCursorPtr) pCursor;
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
vgpkg_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualGPKGCursorPtr cursor = (VirtualGPKGCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vgpkg_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	      sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    sqlite3_int64 rowid = 0;
    int ret;
    VirtualGPKGPtr p_vt = (VirtualGPKGPtr) pVTab;
    if (argc == 1)
      {
	  /* performing a DELETE */
	  if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
	    {
		rowid = sqlite3_value_int64 (argv[0]);
		ret = vgpkg_delete_row (p_vt, rowid);
	    }
	  else
	      ret = SQLITE_MISMATCH;
      }
    else
      {
	  if (sqlite3_value_type (argv[0]) == SQLITE_NULL)
	    {
		/* performing an INSERT */
		ret = vgpkg_insert_row (p_vt, &rowid, argc, argv);
		if (ret == SQLITE_OK)
		    *pRowid = rowid;
	    }
	  else
	    {
		/* performing an UPDATE */
		rowid = sqlite3_value_int64 (argv[0]);
		ret = vgpkg_update_row (p_vt, rowid, argc, argv);
	    }
      }
    return ret;
}

static int
vgpkg_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vgpkg_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vgpkg_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vgpkg_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vgpkg_rename (sqlite3_vtab * pVTab, const char *zNew)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    if (zNew)
	zNew = zNew;		/* unused arg warning suppression */
    return SQLITE_ERROR;
}

static int
spliteVirtualGPKGInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_gpkg_module.iVersion = 1;
    my_gpkg_module.xCreate = &vgpkg_create;
    my_gpkg_module.xConnect = &vgpkg_connect;
    my_gpkg_module.xBestIndex = &vgpkg_best_index;
    my_gpkg_module.xDisconnect = &vgpkg_disconnect;
    my_gpkg_module.xDestroy = &vgpkg_destroy;
    my_gpkg_module.xOpen = &vgpkg_open;
    my_gpkg_module.xClose = &vgpkg_close;
    my_gpkg_module.xFilter = &vgpkg_filter;
    my_gpkg_module.xNext = &vgpkg_next;
    my_gpkg_module.xEof = &vgpkg_eof;
    my_gpkg_module.xColumn = &vgpkg_column;
    my_gpkg_module.xRowid = &vgpkg_rowid;
    my_gpkg_module.xUpdate = &vgpkg_update;
    my_gpkg_module.xBegin = &vgpkg_begin;
    my_gpkg_module.xSync = &vgpkg_sync;
    my_gpkg_module.xCommit = &vgpkg_commit;
    my_gpkg_module.xRollback = &vgpkg_rollback;
    my_gpkg_module.xFindFunction = NULL;
    my_gpkg_module.xRename = &vgpkg_rename;
    sqlite3_create_module_v2 (db, "VirtualGPKG", &my_gpkg_module, NULL, 0);
    return rc;
}

SPATIALITE_PRIVATE int
virtualgpkg_extension_init (void *xdb)
{
    sqlite3 *db = (sqlite3 *) xdb;
    return spliteVirtualGPKGInit (db);
}

#endif /* end GEOPACKAGE conditional */
