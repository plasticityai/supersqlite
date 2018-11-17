/*

 virtualdbf.c -- SQLite3 extension [VIRTUAL TABLE accessing DBF]

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

#ifdef _WIN32
#define strcasecmp	_stricmp
#endif /* not WIN32 */

#ifndef OMIT_ICONV		/* if ICONV is disabled no DBF support is available */

static struct sqlite3_module my_dbf_module;

typedef struct VirtualDbfStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    gaiaDbfPtr dbf;		/* the DBF struct */
    int text_dates;
} VirtualDbf;
typedef VirtualDbf *VirtualDbfPtr;

typedef struct VirtualDbfConstraintStruct
{
/* a constraint to be verified for xFilter */
    int iColumn;		/* Column on left-hand side of constraint */
    int op;			/* Constraint operator */
    char valueType;		/* value Type ('I'=int,'D'=double,'T'=text) */
    sqlite3_int64 intValue;	/* Int64 comparison value */
    double dblValue;		/* Double comparison value */
    char *txtValue;		/* Text comparison value */
    struct VirtualDbfConstraintStruct *next;
} VirtualDbfConstraint;
typedef VirtualDbfConstraint *VirtualDbfConstraintPtr;

typedef struct VirtualDbfCursorStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualDbfPtr pVtab;	/* Virtual table of this cursor */
    long current_row;		/* the current row ID */
    int eof;			/* the EOF marker */
    VirtualDbfConstraintPtr firstConstraint;
    VirtualDbfConstraintPtr lastConstraint;
} VirtualDbfCursor;

typedef VirtualDbfCursor *VirtualDbfCursorPtr;

static int
vdbf_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some DBF */
    char *sql;
    VirtualDbfPtr p_vt;
    char path[2048];
    char encoding[128];
    const char *pEncoding = NULL;
    int len;
    const char *pPath = NULL;
    gaiaDbfFieldPtr pFld;
    int cnt;
    int col_cnt;
    int seed;
    int dup;
    int idup;
    int text_dates = 0;
    char *xname;
    char **col_name = NULL;
    gaiaOutBuffer sql_statement;
    if (pAux)
	pAux = pAux;		/* unused arg warning suppression */
/* checking for DBF PATH */
    if (argc == 5 || argc == 6)
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
	  if (argc == 6)
	      text_dates = atoi (argv[5]);
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualDbf module] CREATE VIRTUAL: illegal arg list {dbf_path, encoding}");
	  return SQLITE_ERROR;
      }
    p_vt = (VirtualDbfPtr) sqlite3_malloc (sizeof (VirtualDbf));
    if (!p_vt)
	return SQLITE_NOMEM;
    p_vt->pModule = &my_dbf_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
    p_vt->db = db;
    p_vt->dbf = gaiaAllocDbf ();
    p_vt->text_dates = text_dates;
/* trying to open file */
    gaiaOpenDbfRead (p_vt->dbf, path, encoding, "UTF-8");
    if (!(p_vt->dbf->Valid))
      {
	  /* something is going the wrong way; creating a stupid default table */
	  xname = gaiaDoubleQuotedSql ((const char *) argv[2]);
	  sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (PKUID INTEGER)", xname);
	  free (xname);
	  if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
	    {
		sqlite3_free (sql);
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualDbf module] cannot build a table from DBF\n");
		return SQLITE_ERROR;
	    }
	  sqlite3_free (sql);
	  *ppVTab = (sqlite3_vtab *) p_vt;
	  return SQLITE_OK;
      }
/* preparing the COLUMNs for this VIRTUAL TABLE */
    gaiaOutBufferInitialize (&sql_statement);
    xname = gaiaDoubleQuotedSql (argv[2]);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (PKUID INTEGER", xname);
    free (xname);
    gaiaAppendToOutBuffer (&sql_statement, sql);
    sqlite3_free (sql);
/* checking for duplicate / illegal column names and antialising them */
    col_cnt = 0;
    pFld = p_vt->dbf->Dbf->First;
    while (pFld)
      {
	  /* counting DBF fields */
	  col_cnt++;
	  pFld = pFld->Next;
      }
    col_name = malloc (sizeof (char *) * col_cnt);
    cnt = 0;
    seed = 0;
    pFld = p_vt->dbf->Dbf->First;
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
		    ("[VirtualDbf module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
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
vdbf_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some DBF - simply aliases vdbf_create() */
    return vdbf_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vdbf_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
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
vdbf_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualDbfPtr p_vt = (VirtualDbfPtr) pVTab;
    if (p_vt->dbf)
	gaiaFreeDbf (p_vt->dbf);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
vdbf_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vdbf_disconnect() */
    return vdbf_disconnect (pVTab);
}

static void
vdbf_read_row (VirtualDbfCursorPtr cursor, int *deleted_row)
{
/* trying to read a "row" from DBF */
    int ret;
    int deleted;
    if (!(cursor->pVtab->dbf->Valid))
      {
	  cursor->eof = 1;
	  return;
      }
    ret =
	gaiaReadDbfEntity_ex (cursor->pVtab->dbf, cursor->current_row, &deleted,
			      cursor->pVtab->text_dates);
    if (!ret)
      {
	  if (!(cursor->pVtab->dbf->LastError))	/* normal DBF EOF */
	    {
		cursor->eof = 1;
		return;
	    }
	  /* an error occurred */
	  spatialite_e ("%s\n", cursor->pVtab->dbf->LastError);
	  cursor->eof = 1;
	  return;
      }
    cursor->current_row++;
    *deleted_row = deleted;
}

static int
vdbf_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    int deleted;
    VirtualDbfCursorPtr cursor =
	(VirtualDbfCursorPtr) sqlite3_malloc (sizeof (VirtualDbfCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->firstConstraint = NULL;
    cursor->lastConstraint = NULL;
    cursor->pVtab = (VirtualDbfPtr) pVTab;
    cursor->current_row = 0;
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    while (1)
      {
	  vdbf_read_row (cursor, &deleted);
	  if (cursor->eof)
	      break;
	  if (!deleted)
	      break;
      }
    return SQLITE_OK;
}

static void
vdbf_free_constraints (VirtualDbfCursorPtr cursor)
{
/* memory cleanup - cursor constraints */
    VirtualDbfConstraintPtr pC;
    VirtualDbfConstraintPtr pCn;
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
vdbf_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualDbfCursorPtr cursor = (VirtualDbfCursorPtr) pCursor;
    vdbf_free_constraints (cursor);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vdbf_parse_constraint (const char *str, int index, int *iColumn, int *op)
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
vdbf_eval_constraints (VirtualDbfCursorPtr cursor)
{
/* evaluating Filter constraints */
    int nCol;
    gaiaDbfFieldPtr pFld;
    VirtualDbfConstraintPtr pC = cursor->firstConstraint;
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
	  nCol = 1;
	  pFld = cursor->pVtab->dbf->Dbf->First;
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
vdbf_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    int i;
    int iColumn;
    int op;
    int len;
    int deleted;
    VirtualDbfConstraintPtr pC;
    VirtualDbfCursorPtr cursor = (VirtualDbfCursorPtr) pCursor;
    if (idxNum)
	idxNum = idxNum;	/* unused arg warning suppression */

/* resetting any previously set filter constraint */
    vdbf_free_constraints (cursor);

    for (i = 0; i < argc; i++)
      {
	  if (!vdbf_parse_constraint (idxStr, i, &iColumn, &op))
	      continue;
	  pC = sqlite3_malloc (sizeof (VirtualDbfConstraint));
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
    cursor->eof = 0;
    while (1)
      {
	  vdbf_read_row (cursor, &deleted);
	  if (cursor->eof)
	      break;
	  if (deleted)
	      continue;
	  if (vdbf_eval_constraints (cursor))
	      break;
      }
    return SQLITE_OK;
}

static int
vdbf_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching a next row from cursor */
    int deleted;
    VirtualDbfCursorPtr cursor = (VirtualDbfCursorPtr) pCursor;
    while (1)
      {
	  vdbf_read_row (cursor, &deleted);
	  if (cursor->eof)
	      break;
	  if (deleted)
	      continue;
	  if (vdbf_eval_constraints (cursor))
	      break;
      }
    return SQLITE_OK;
}

static int
vdbf_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualDbfCursorPtr cursor = (VirtualDbfCursorPtr) pCursor;
    return cursor->eof;
}

static int
vdbf_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    int nCol = 1;
    gaiaDbfFieldPtr pFld;
    VirtualDbfCursorPtr cursor = (VirtualDbfCursorPtr) pCursor;
    if (column == 0)
      {
	  /* the PRIMARY KEY column */
	  sqlite3_result_int (pContext, cursor->current_row);
	  return SQLITE_OK;
      }
    pFld = cursor->pVtab->dbf->Dbf->First;
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
vdbf_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualDbfCursorPtr cursor = (VirtualDbfCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vdbf_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    if (pVTab || argc || argv || pRowid)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_READONLY;
}

static int
vdbf_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vdbf_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vdbf_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vdbf_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vdbf_rename (sqlite3_vtab * pVTab, const char *zNew)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    if (zNew)
	zNew = zNew;		/* unused arg warning suppression */
    return SQLITE_ERROR;
}

static int
spliteVirtualDbfInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_dbf_module.iVersion = 1;
    my_dbf_module.xCreate = &vdbf_create;
    my_dbf_module.xConnect = &vdbf_connect;
    my_dbf_module.xBestIndex = &vdbf_best_index;
    my_dbf_module.xDisconnect = &vdbf_disconnect;
    my_dbf_module.xDestroy = &vdbf_destroy;
    my_dbf_module.xOpen = &vdbf_open;
    my_dbf_module.xClose = &vdbf_close;
    my_dbf_module.xFilter = &vdbf_filter;
    my_dbf_module.xNext = &vdbf_next;
    my_dbf_module.xEof = &vdbf_eof;
    my_dbf_module.xColumn = &vdbf_column;
    my_dbf_module.xRowid = &vdbf_rowid;
    my_dbf_module.xUpdate = &vdbf_update;
    my_dbf_module.xBegin = &vdbf_begin;
    my_dbf_module.xSync = &vdbf_sync;
    my_dbf_module.xCommit = &vdbf_commit;
    my_dbf_module.xRollback = &vdbf_rollback;
    my_dbf_module.xFindFunction = NULL;
    my_dbf_module.xRename = &vdbf_rename;
    sqlite3_create_module_v2 (db, "VirtualDbf", &my_dbf_module, NULL, 0);
    return rc;
}

SPATIALITE_PRIVATE int
virtualdbf_extension_init (void *xdb)
{
    sqlite3 *db = (sqlite3 *) xdb;
    return spliteVirtualDbfInit (db);
}

#endif /* ICONV enabled/disabled */
