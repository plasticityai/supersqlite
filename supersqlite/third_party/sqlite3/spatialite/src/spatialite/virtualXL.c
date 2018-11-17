/*

 virtualXLc -- SQLite3 extension [VIRTUAL TABLE accessing .XLS]

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

#include <spatialite/spatialite.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/gaiageo.h>

#ifndef OMIT_FREEXL
#include <freexl.h>
#endif

#ifdef _WIN32
#define strcasecmp	_stricmp
#endif /* not WIN32 */

#ifndef OMIT_ICONV		/* if ICONV is disabled no XL support is available */
#ifndef OMIT_FREEXL		/* FreeXL is disabled */

static struct sqlite3_module my_XL_module;

typedef struct VirtualXLStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    const void *XL_handle;	/* the XL handle */
    unsigned int rows;		/* Worksheet #rows */
    unsigned short columns;	/* Worksheet #columns */
    char firstLineTitles;	/* 'Y' or 'N' */
} VirtualXL;
typedef VirtualXL *VirtualXLPtr;

typedef struct VirtualXLConstraintStruct
{
/* a constraint to be verified for xFilter */
    int iColumn;		/* Column on left-hand side of constraint */
    int op;			/* Constraint operator */
    char valueType;		/* value Type ('I'=int,'D'=double,'T'=text) */
    sqlite3_int64 intValue;	/* Int64 comparison value */
    double dblValue;		/* Double comparison value */
    char *txtValue;		/* Text comparison value */
    struct VirtualXLConstraintStruct *next;
} VirtualXLConstraint;
typedef VirtualXLConstraint *VirtualXLConstraintPtr;

typedef struct VirtualXLCursorStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualXLPtr pVtab;		/* Virtual table of this cursor */
    unsigned int current_row;	/* the current row ID */
    int eof;			/* the EOF marker */
    VirtualXLConstraintPtr firstConstraint;
    VirtualXLConstraintPtr lastConstraint;
} VirtualXLCursor;
typedef VirtualXLCursor *VirtualXLCursorPtr;

static int
vXL_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	    sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some XLS file */
    char *sql;
    VirtualXLPtr p_vt;
    char path[2048];
    char firstLineTitles = 'N';
    unsigned int worksheet = 0;
    unsigned int max_worksheet;
    unsigned int info;
    unsigned int rows;
    unsigned short columns;
    unsigned short col;
    int len;
    int ret;
    const void *handle;
    const char *pPath = NULL;
    char *xname;
    gaiaOutBuffer sql_statement;
    if (pAux)
	pAux = pAux;		/* unused arg warning suppression */
/* checking for XLS PATH */
    if (argc == 4 || argc == 5 || argc == 6)
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
	  if (argc == 5 || argc == 6)
	      worksheet = atoi (argv[4]);
	  if (argc == 6)
	    {
		if (atoi (argv[5]) == 1)
		    firstLineTitles = 'Y';
	    }
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualXL module] CREATE VIRTUAL: illegal arg list {xls_path [, worksheet_index [, first_line_titles(1/0)]]}");
	  return SQLITE_ERROR;
      }
/* allocating the main XL module */
    p_vt = (VirtualXLPtr) sqlite3_malloc (sizeof (VirtualXL));
    if (!p_vt)
	return SQLITE_NOMEM;
    p_vt->pModule = &my_XL_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
    p_vt->db = db;
    p_vt->XL_handle = NULL;
    p_vt->rows = 0;
    p_vt->columns = 0;
    p_vt->firstLineTitles = firstLineTitles;
/* opening the .XLS file [Workbook] */
    ret = freexl_open (path, &handle);
    if (ret != FREEXL_OK)
      {
	  /* free memory */
	  freexl_close (handle);
	  /* something is going the wrong way; creating a stupid default table */
	  xname = gaiaDoubleQuotedSql (argv[2]);
	  sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (PKUID INTEGER)", xname);
	  free (xname);
	  if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
	    {
		sqlite3_free (sql);
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualXL module] cannot build a table from XL\n");
		return SQLITE_ERROR;
	    }
	  sqlite3_free (sql);
	  *ppVTab = (sqlite3_vtab *) p_vt;
	  return SQLITE_OK;
      }
/* checking if Password Protected [obfuscated] */
    freexl_get_info (handle, FREEXL_BIFF_PASSWORD, &info);
    if (info != FREEXL_BIFF_PLAIN)
      {
	  /* free memory */
	  freexl_close (handle);
	  /* Obfuscated: creating a stupid default table */
	  xname = gaiaDoubleQuotedSql (argv[2]);
	  sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (PKUID INTEGER)", xname);
	  free (xname);
	  if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
	    {
		sqlite3_free (sql);
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualXL module] Password protected [obfuscated] .xls\n");
		return SQLITE_ERROR;
	    }
	  sqlite3_free (sql);
	  *ppVTab = (sqlite3_vtab *) p_vt;
	  return SQLITE_OK;
      }
/* querying how many Worksheets are there */
    freexl_get_info (handle, FREEXL_BIFF_SHEET_COUNT, &max_worksheet);
    if (worksheet >= max_worksheet)
      {
	  /* free memory */
	  freexl_close (handle);
	  /* no such Worksheet: creating a stupid default table */
	  xname = gaiaDoubleQuotedSql (argv[2]);
	  sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (PKUID INTEGER)", xname);
	  free (xname);
	  if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
	    {
		sqlite3_free (sql);
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualXL module] no such Worksheet [index=%u]\n",
		     worksheet);
		return SQLITE_ERROR;
	    }
	  sqlite3_free (sql);
	  *ppVTab = (sqlite3_vtab *) p_vt;
	  return SQLITE_OK;
      }
/* selecting the currently active XL Worksheet */
    freexl_select_active_worksheet (handle, worksheet);
    freexl_worksheet_dimensions (handle, &rows, &columns);
    p_vt->XL_handle = handle;
    p_vt->rows = rows;
    p_vt->columns = columns;
/* preparing the COLUMNs for this VIRTUAL TABLE */
    gaiaOutBufferInitialize (&sql_statement);
    xname = gaiaDoubleQuotedSql (argv[2]);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (row_no INTEGER", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
    if (firstLineTitles == 'Y')
      {
	  /* fetching column names */
	  for (col = 0; col < columns; col++)
	    {
		FreeXL_CellValue cell;
		int ret = freexl_get_cell_value (handle, 0, col, &cell);
		if (ret != FREEXL_OK)
		    sql = sqlite3_mprintf ("col_%d", col);
		else
		  {
		      if (cell.type == FREEXL_CELL_INT)
			  sql = sqlite3_mprintf ("%d", cell.value.int_value);
		      else if (cell.type == FREEXL_CELL_DOUBLE)
			  sql =
			      sqlite3_mprintf ("%1.2f",
					       cell.value.double_value);
		      else if (cell.type == FREEXL_CELL_TEXT
			       || cell.type == FREEXL_CELL_SST_TEXT
			       || cell.type == FREEXL_CELL_DATE
			       || cell.type == FREEXL_CELL_DATETIME
			       || cell.type == FREEXL_CELL_TIME)
			{
			    int len = strlen (cell.value.text_value);
			    if (len < 256)
				sql =
				    sqlite3_mprintf ("%s",
						     cell.value.text_value);
			    else
				sql = sqlite3_mprintf ("col_%d", col);
			}
		      else
			  sql = sqlite3_mprintf ("col_%d", col);
		  }
		xname = gaiaDoubleQuotedSql (sql);
		sqlite3_free (sql);
		sql = sqlite3_mprintf (", \"%s\"", xname);
		free (xname);
		gaiaAppendToOutBuffer (&sql_statement, sql);
		sqlite3_free (sql);
	    }
      }
    else
      {
	  /* setting default column names */
	  for (col = 0; col < columns; col++)
	    {
		sql = sqlite3_mprintf ("col_%d", col);
		xname = gaiaDoubleQuotedSql (sql);
		sqlite3_free (sql);
		sql = sqlite3_mprintf (", \"%s\"", xname);
		free (xname);
		gaiaAppendToOutBuffer (&sql_statement, sql);
		sqlite3_free (sql);
	    }
      }
    gaiaAppendToOutBuffer (&sql_statement, ")");
    if (sql_statement.Error == 0 && sql_statement.Buffer != NULL)
      {
	  if (sqlite3_declare_vtab (db, sql_statement.Buffer) != SQLITE_OK)
	    {
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualXL module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
		     sql_statement.Buffer);
		gaiaOutBufferReset (&sql_statement);
		return SQLITE_ERROR;
	    }
      }
    gaiaOutBufferReset (&sql_statement);
    *ppVTab = (sqlite3_vtab *) p_vt;
    return SQLITE_OK;
}

static int
vXL_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to a .xls file - simply aliases vXL_create() */
    return vXL_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vXL_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
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
vXL_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualXLPtr p_vt = (VirtualXLPtr) pVTab;
    if (p_vt->XL_handle)
	freexl_close (p_vt->XL_handle);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
vXL_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vXL_disconnect() */
    return vXL_disconnect (pVTab);
}

static void
vXL_read_row (VirtualXLCursorPtr cursor)
{
/* trying to read a "row" from XL */
    cursor->current_row++;
    if (cursor->current_row > cursor->pVtab->rows)
      {
	  cursor->eof = 1;
	  return;
      }
}

static int
vXL_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    VirtualXLCursorPtr cursor =
	(VirtualXLCursorPtr) sqlite3_malloc (sizeof (VirtualXLCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->firstConstraint = NULL;
    cursor->lastConstraint = NULL;
    cursor->pVtab = (VirtualXLPtr) pVTab;
    if (cursor->pVtab->firstLineTitles == 'Y')
	cursor->current_row = 1;
    else
	cursor->current_row = 0;
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    vXL_read_row (cursor);
    return SQLITE_OK;
}

static void
vXL_free_constraints (VirtualXLCursorPtr cursor)
{
/* memory cleanup - cursor constraints */
    VirtualXLConstraintPtr pC;
    VirtualXLConstraintPtr pCn;
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
vXL_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualXLCursorPtr cursor = (VirtualXLCursorPtr) pCursor;
    vXL_free_constraints (cursor);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vXL_parse_constraint (const char *str, int index, int *iColumn, int *op)
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
vXL_eval_constraints (VirtualXLCursorPtr cursor)
{
/* evaluating Filter constraints */
    FreeXL_CellValue cell;
    VirtualXLConstraintPtr pC = cursor->firstConstraint;
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
		      int cur_row = cursor->current_row;
		      if (cursor->pVtab->firstLineTitles == 'Y')
			  cur_row--;
		      switch (pC->op)
			{
			case SQLITE_INDEX_CONSTRAINT_EQ:
			    if (cur_row == pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GT:
			    if (cur_row > pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LE:
			    if (cur_row <= pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LT:
			    if (cur_row < pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GE:
			    if (cur_row >= pC->intValue)
				ok = 1;
			    break;
			};
		  }
		goto done;
	    }
	  if (cursor->pVtab->XL_handle != NULL
	      && cursor->current_row <= cursor->pVtab->rows
	      && pC->iColumn <= cursor->pVtab->columns)
	      freexl_get_cell_value (cursor->pVtab->XL_handle,
				     cursor->current_row - 1, pC->iColumn - 1,
				     &cell);
	  else
	      cell.type = FREEXL_CELL_NULL;
	  if (cell.type == FREEXL_CELL_INT)
	    {
		if (pC->valueType == 'I')
		  {
		      switch (pC->op)
			{
			case SQLITE_INDEX_CONSTRAINT_EQ:
			    if (cell.value.int_value == pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GT:
			    if (cell.value.int_value > pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LE:
			    if (cell.value.int_value <= pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LT:
			    if (cell.value.int_value < pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GE:
			    if (cell.value.int_value >= pC->intValue)
				ok = 1;
			    break;
			};
		  }
		if (pC->valueType == 'D')
		  {
		      switch (pC->op)
			{
			case SQLITE_INDEX_CONSTRAINT_EQ:
			    if (cell.value.int_value == pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GT:
			    if (cell.value.int_value > pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LE:
			    if (cell.value.int_value <= pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LT:
			    if (cell.value.int_value < pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GE:
			    if (cell.value.int_value >= pC->dblValue)
				ok = 1;
			    break;
			};
		  }
	    }
	  if (cell.type == FREEXL_CELL_DOUBLE)
	    {
		if (pC->valueType == 'I')
		  {
		      switch (pC->op)
			{
			case SQLITE_INDEX_CONSTRAINT_EQ:
			    if (cell.value.double_value == pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GT:
			    if (cell.value.double_value > pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LE:
			    if (cell.value.double_value <= pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LT:
			    if (cell.value.double_value < pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GE:
			    if (cell.value.double_value >= pC->intValue)
				ok = 1;
			    break;
			};
		  }
		if (pC->valueType == 'D')
		  {
		      switch (pC->op)
			{
			case SQLITE_INDEX_CONSTRAINT_EQ:
			    if (cell.value.double_value == pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GT:
			    if (cell.value.double_value > pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LE:
			    if (cell.value.double_value <= pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LT:
			    if (cell.value.double_value < pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GE:
			    if (cell.value.double_value >= pC->dblValue)
				ok = 1;
			    break;
			};
		  }
	    }
	  if ((cell.type == FREEXL_CELL_TEXT
	       || cell.type == FREEXL_CELL_SST_TEXT
	       || cell.type == FREEXL_CELL_DATE
	       || cell.type == FREEXL_CELL_DATETIME
	       || cell.type == FREEXL_CELL_TIME) && pC->valueType == 'T')
	    {
		int ret = strcmp (cell.value.text_value, pC->txtValue);
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
	done:
	  if (!ok)
	      return 0;
	  pC = pC->next;
      }
    return 1;
}

static int
vXL_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	    int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    int i;
    int iColumn;
    int op;
    int len;
    VirtualXLConstraintPtr pC;
    VirtualXLCursorPtr cursor = (VirtualXLCursorPtr) pCursor;
    if (idxNum)
	idxNum = idxNum;	/* unused arg warning suppression */

/* resetting any previously set filter constraint */
    vXL_free_constraints (cursor);

    for (i = 0; i < argc; i++)
      {
	  if (!vXL_parse_constraint (idxStr, i, &iColumn, &op))
	      continue;
	  pC = sqlite3_malloc (sizeof (VirtualXLConstraint));
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

    if (cursor->pVtab->firstLineTitles == 'Y')
	cursor->current_row = 1;
    else
	cursor->current_row = 0;
    cursor->eof = 0;
    while (1)
      {
	  vXL_read_row (cursor);
	  if (cursor->eof)
	      break;
	  if (vXL_eval_constraints (cursor))
	      break;
      }
    return SQLITE_OK;
}

static int
vXL_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching a next row from cursor */
    VirtualXLCursorPtr cursor = (VirtualXLCursorPtr) pCursor;
    while (1)
      {
	  vXL_read_row (cursor);
	  if (cursor->eof)
	      break;
	  if (vXL_eval_constraints (cursor))
	      break;
      }
    return SQLITE_OK;
}

static int
vXL_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualXLCursorPtr cursor = (VirtualXLCursorPtr) pCursor;
    return cursor->eof;
}

static int
vXL_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	    int column)
{
/* fetching value for the Nth column */
    FreeXL_CellValue cell;
    VirtualXLCursorPtr cursor = (VirtualXLCursorPtr) pCursor;
    if (column == 0)
      {
	  /* the PRIMARY KEY column */
	  if (cursor->pVtab->firstLineTitles == 'Y')
	      sqlite3_result_int (pContext, cursor->current_row - 1);
	  else
	      sqlite3_result_int (pContext, cursor->current_row);
	  return SQLITE_OK;
      }
    if (cursor->pVtab->XL_handle != NULL
	&& cursor->current_row <= cursor->pVtab->rows
	&& column <= cursor->pVtab->columns)
	freexl_get_cell_value (cursor->pVtab->XL_handle,
			       cursor->current_row - 1, column - 1, &cell);
    else
	cell.type = FREEXL_CELL_NULL;
    switch (cell.type)
      {
      case FREEXL_CELL_INT:
	  sqlite3_result_int (pContext, cell.value.int_value);
	  break;
      case FREEXL_CELL_DOUBLE:
	  sqlite3_result_double (pContext, cell.value.double_value);
	  break;
      case FREEXL_CELL_TEXT:
      case FREEXL_CELL_SST_TEXT:
      case FREEXL_CELL_DATE:
      case FREEXL_CELL_DATETIME:
      case FREEXL_CELL_TIME:
	  sqlite3_result_text (pContext, cell.value.text_value,
			       strlen (cell.value.text_value), SQLITE_STATIC);
	  break;
      default:
	  sqlite3_result_null (pContext);
	  break;
      };
    return SQLITE_OK;
}

static int
vXL_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualXLCursorPtr cursor = (VirtualXLCursorPtr) pCursor;
    if (cursor->pVtab->firstLineTitles == 'Y')
	*pRowid = cursor->current_row - 1;
    else
	*pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vXL_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	    sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    if (pVTab || argc || argv || pRowid)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_READONLY;
}

static int
vXL_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vXL_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vXL_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vXL_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vXL_rename (sqlite3_vtab * pVTab, const char *zNew)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    if (zNew)
	zNew = zNew;		/* unused arg warning suppression */
    return SQLITE_ERROR;
}

static int
spliteVirtualXLInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_XL_module.iVersion = 1;
    my_XL_module.xCreate = &vXL_create;
    my_XL_module.xConnect = &vXL_connect;
    my_XL_module.xBestIndex = &vXL_best_index;
    my_XL_module.xDisconnect = &vXL_disconnect;
    my_XL_module.xDestroy = &vXL_destroy;
    my_XL_module.xOpen = &vXL_open;
    my_XL_module.xClose = &vXL_close;
    my_XL_module.xFilter = &vXL_filter;
    my_XL_module.xNext = &vXL_next;
    my_XL_module.xEof = &vXL_eof;
    my_XL_module.xColumn = &vXL_column;
    my_XL_module.xRowid = &vXL_rowid;
    my_XL_module.xUpdate = &vXL_update;
    my_XL_module.xBegin = &vXL_begin;
    my_XL_module.xSync = &vXL_sync;
    my_XL_module.xCommit = &vXL_commit;
    my_XL_module.xRollback = &vXL_rollback;
    my_XL_module.xFindFunction = NULL;
    my_XL_module.xRename = &vXL_rename;
    sqlite3_create_module_v2 (db, "VirtualXL", &my_XL_module, NULL, 0);
    return rc;
}

SPATIALITE_PRIVATE int
virtualXL_extension_init (void *xdb)
{
    sqlite3 *db = (sqlite3 *) xdb;
    return spliteVirtualXLInit (db);
}

#endif /* FreeXL enabled/disabled */
#endif /* ICONV enabled/disabled */
