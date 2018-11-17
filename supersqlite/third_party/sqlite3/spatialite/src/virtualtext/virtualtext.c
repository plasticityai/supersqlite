/*

 virtualtext.c -- SQLite3 extension [VIRTUAL TABLE accessing CSV/TXT]

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

#if defined(_WIN32) && !defined(__MINGW32__)
/* MSVC strictly requires this include [off_t] */
#include <sys/types.h>
#endif

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

#if OMIT_ICONV == 0		/* if ICONV is disabled no TXT support is available */

struct sqlite3_module virtualtext_module;

typedef struct VirtualTextStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USED INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    gaiaTextReaderPtr reader;	/* the TextReader object */
} VirtualText;
typedef VirtualText *VirtualTextPtr;

typedef struct VirtualTextConstraintStruct
{
/* a constraint to be verified for xFilter */
    int iColumn;		/* Column on left-hand side of constraint */
    int op;			/* Constraint operator */
    char valueType;		/* value Type ('I'=int,'D'=double,'T'=text) */
    sqlite3_int64 intValue;	/* Int64 comparison value */
    double dblValue;		/* Double comparison value */
    char *txtValue;		/* Text comparison value */
    struct VirtualTextConstraintStruct *next;
} VirtualTextConstraint;
typedef VirtualTextConstraint *VirtualTextConstraintPtr;

typedef struct VirtualTextCursortStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualTextPtr pVtab;	/* Virtual table of this cursor */
    long current_row;		/* the current row ID */
    int eof;			/* the EOF marker */
    VirtualTextConstraintPtr firstConstraint;
    VirtualTextConstraintPtr lastConstraint;
} VirtualTextCursor;
typedef VirtualTextCursor *VirtualTextCursorPtr;

static void
text_clean_integer (char *value)
{
/* cleaning an integer value */
    char last;
    char buffer[35536];
    int len = strlen (value);
    last = value[len - 1];
    if (last == '-' || last == '+')
      {
	  /* trailing sign; transforming into a leading sign */
	  *buffer = last;
	  strcpy (buffer + 1, value);
	  buffer[len - 1] = '\0';
	  strcpy (value, buffer);
      }
}

static void
text_clean_double (char *value)
{
/* cleaning an integer value */
    char *p;
    char last;
    char buffer[35536];
    int len = strlen (value);
    last = value[len - 1];
    if (last == '-' || last == '+')
      {
	  /* trailing sign; transforming into a leading sign */
	  *buffer = last;
	  strcpy (buffer + 1, value);
	  buffer[len - 1] = '\0';
	  strcpy (value, buffer);
      }
    p = value;
    while (*p != '\0')
      {
	  /* transforming COMMAs into POINTs */
	  if (*p == ',')
	      *p = '.';
	  p++;
      }
}

static int
vtxt_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some TEXT file */
    char path[2048];
    char encoding[128];
    const char *vtable;
    const char *pEncoding = NULL;
    int len;
    gaiaTextReaderPtr text = NULL;
    const char *pPath = NULL;
    char field_separator = '\t';
    char text_separator = '"';
    char decimal_separator = '.';
    char first_line_titles = 1;
    int i;
    char sql[65535];
    int seed;
    int dup;
    int idup;
    char dummyName[4096];
    char **col_name = NULL;
    VirtualTextPtr p_vt;
    if (pAux)
	pAux = pAux;		/* unused arg warning suppression */
/* checking for TEXTfile PATH */
    if (argc >= 5 && argc <= 9)
      {
	  vtable = argv[1];
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
	  if (argc >= 6)
	    {
		if (*(argv[5]) == '0' || *(argv[5]) == 'n' || *(argv[5]) == 'N')
		    first_line_titles = 0;
	    }
	  if (argc >= 7)
	    {
		if (strcasecmp (argv[6], "COMMA") == 0)
		    decimal_separator = ',';
		if (strcasecmp (argv[6], "POINT") == 0)
		    decimal_separator = '.';
	    }
	  if (argc >= 8)
	    {
		if (strcasecmp (argv[7], "SINGLEQUOTE") == 0)
		    text_separator = '\'';
		if (strcasecmp (argv[7], "DOUBLEQUOTE") == 0)
		    text_separator = '"';
		if (strcasecmp (argv[7], "NONE") == 0)
		    text_separator = '\0';
	    }
	  if (argc == 9)
	    {
		if (strlen (argv[8]) == 3)
		  {
		      if (strcasecmp (argv[8], "TAB") == 0)
			  field_separator = '\t';
		      if (*(argv[8] + 0) == '\'' && *(argv[8] + 2) == '\'')
			  field_separator = *(argv[8] + 1);
		  }
	    }
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualText module] CREATE VIRTUAL: illegal arg list\n"
	       "\t\t{ text_path, encoding [, first_row_as_titles [, [decimal_separator [, text_separator, [field_separator] ] ] ] }\n");
	  return SQLITE_ERROR;
      }
    p_vt = (VirtualTextPtr) sqlite3_malloc (sizeof (VirtualText));
    if (!p_vt)
	return SQLITE_NOMEM;
    p_vt->pModule = &virtualtext_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
    p_vt->db = db;
    text = gaiaTextReaderAlloc (path, field_separator,
				text_separator, decimal_separator,
				first_line_titles, encoding);
    if (text)
      {
	  if (gaiaTextReaderParse (text) == 0)
	    {
		gaiaTextReaderDestroy (text);
		text = NULL;
	    }
      }
    if (!text)
      {
	  /* something is going the wrong way; creating a stupid default table */
	  spatialite_e ("VirtualText: invalid data source\n");
	  sprintf (sql, "CREATE TABLE %s (ROWNO INTEGER)", vtable);
	  if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
	    {
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualText module] cannot build a table from TEXT file\n");
		return SQLITE_ERROR;
	    }
	  p_vt->reader = NULL;
	  *ppVTab = (sqlite3_vtab *) p_vt;
	  return SQLITE_OK;
      }
    p_vt->reader = text;
/* preparing the COLUMNs for this VIRTUAL TABLE */
    sprintf (sql, "CREATE TABLE %s (ROWNO INTEGER", vtable);
    col_name = malloc (sizeof (char *) * text->max_fields);
    seed = 0;
    for (i = 0; i < text->max_fields; i++)
      {
	  strcat (sql, ", ");
	  sprintf (dummyName, "\"%s\"", text->columns[i].name);
	  dup = 0;
	  for (idup = 0; idup < i; idup++)
	    {
		if (strcasecmp (dummyName, *(col_name + idup)) == 0)
		    dup = 1;
	    }
	  if (strcasecmp (dummyName, "ROWNO") == 0)
	      dup = 1;
	  if (dup)
	      sprintf (dummyName, "DUPCOL_%d", seed++);
	  len = strlen (dummyName);
	  *(col_name + i) = malloc (len + 1);
	  strcpy (*(col_name + i), dummyName);
	  strcat (sql, dummyName);
	  if (text->columns[i].type == VRTTXT_INTEGER)
	      strcat (sql, " INTEGER");
	  else if (text->columns[i].type == VRTTXT_DOUBLE)
	      strcat (sql, " DOUBLE");
	  else
	      strcat (sql, " TEXT");
      }
    strcat (sql, ")");
    if (col_name)
      {
	  /* releasing memory allocation for column names */
	  for (i = 0; i < text->max_fields; i++)
	      free (*(col_name + i));
	  free (col_name);
      }
    if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualText module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
	       sql);
	  return SQLITE_ERROR;
      }
    *ppVTab = (sqlite3_vtab *) p_vt;
    return SQLITE_OK;
}

static int
vtxt_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vshp_create() */
    return vtxt_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vtxt_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
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
vtxt_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualTextPtr p_vt = (VirtualTextPtr) pVTab;
    if (p_vt->reader)
	gaiaTextReaderDestroy (p_vt->reader);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
vtxt_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vtxt_disconnect() */
    return vtxt_disconnect (pVTab);
}

static int
vtxt_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    gaiaTextReaderPtr text;
    VirtualTextCursorPtr cursor =
	(VirtualTextCursorPtr) sqlite3_malloc (sizeof (VirtualTextCursor));
    if (cursor == NULL)
	return SQLITE_NOMEM;
    cursor->pVtab = (VirtualTextPtr) pVTab;
    cursor->current_row = 0;
    cursor->eof = 0;
    cursor->firstConstraint = NULL;
    cursor->lastConstraint = NULL;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    text = cursor->pVtab->reader;
    if (!text)
	cursor->eof = 1;
    else
      {
	  if (!gaiaTextReaderGetRow (text, cursor->current_row))
	      cursor->eof = 1;
      }
    return SQLITE_OK;
}


static void
vtxt_free_constraints (VirtualTextCursorPtr cursor)
{
/* memory cleanup - cursor constraints */
    VirtualTextConstraintPtr pC;
    VirtualTextConstraintPtr pCn;
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
vtxt_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    vtxt_free_constraints (cursor);
    sqlite3_free (cursor);
    return SQLITE_OK;
}


static int
vtxt_parse_constraint (const char *str, int index, int *iColumn, int *op)
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
vtxt_eval_constraints (VirtualTextCursorPtr cursor)
{
/* evaluating Filter constraints */
    int nCol;
    int i;
    char buf[4096];
    int type;
    const char *value = NULL;
    sqlite3_int64 int_value;
    double dbl_value;
    char *txt_value = NULL;
    int is_int = 0;
    int is_dbl = 0;
    int is_txt = 0;
    gaiaTextReaderPtr text = cursor->pVtab->reader;
    VirtualTextConstraintPtr pC;
    if (text->current_line_ready == 0)
	return SQLITE_ERROR;
    pC = cursor->firstConstraint;
    while (pC)
      {
	  int ok = 0;
	  if (pC->iColumn == 0)
	    {
		/* the ROWNO column */
		int_value = cursor->current_row;
		is_int = 1;
		goto eval;
	    }
	  nCol = 1;
	  for (i = 0; i < text->max_fields; i++)
	    {
		is_int = 0;
		is_dbl = 0;
		is_txt = 0;
		if (nCol == pC->iColumn)
		  {

		      if (!gaiaTextReaderFetchField (text, i, &type, &value))
			  ;
		      else
			{
			    if (type == VRTTXT_INTEGER)
			      {
				  strcpy (buf, value);
				  text_clean_integer (buf);
#if defined(_WIN32) || defined(__MINGW32__)
/* CAVEAT - M$ runtime has non-standard functions for 64 bits */
				  int_value = _atoi64 (buf);
#else
				  int_value = atoll (buf);
#endif
				  is_int = 1;
			      }
			    else if (type == VRTTXT_DOUBLE)
			      {
				  strcpy (buf, value);
				  text_clean_double (buf);
				  dbl_value = atof (buf);
				  is_dbl = 1;
			      }
			    else if (type == VRTTXT_TEXT)
			      {
				  txt_value = (char *) value;
				  is_txt = 1;
			      }
			}
		      goto eval;
		  }
		nCol++;
	    }
	  return 0;
	eval:
	  ok = 0;
	  if (pC->valueType == 'I')
	    {
		if (is_int)
		  {
		      switch (pC->op)
			{
			case SQLITE_INDEX_CONSTRAINT_EQ:
			    if (int_value == pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GT:
			    if (int_value > pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LE:
			    if (int_value <= pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LT:
			    if (int_value < pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GE:
			    if (int_value >= pC->intValue)
				ok = 1;
			    break;
			};
		  }
		if (is_dbl)
		  {
		      switch (pC->op)
			{
			case SQLITE_INDEX_CONSTRAINT_EQ:
			    if (dbl_value == pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GT:
			    if (dbl_value > pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LE:
			    if (dbl_value <= pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LT:
			    if (dbl_value < pC->intValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GE:
			    if (dbl_value >= pC->intValue)
				ok = 1;
			    break;
			};
		  }
	    }
	  if (pC->valueType == 'D')
	    {
		if (is_int)
		  {
		      switch (pC->op)
			{
			case SQLITE_INDEX_CONSTRAINT_EQ:
			    if (int_value == pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GT:
			    if (int_value > pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LE:
			    if (int_value <= pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LT:
			    if (int_value < pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GE:
			    if (int_value >= pC->dblValue)
				ok = 1;
			    break;
			};
		  }
		if (is_dbl)
		  {
		      switch (pC->op)
			{
			case SQLITE_INDEX_CONSTRAINT_EQ:
			    if (dbl_value == pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GT:
			    if (dbl_value > pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LE:
			    if (dbl_value <= pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_LT:
			    if (dbl_value < pC->dblValue)
				ok = 1;
			    break;
			case SQLITE_INDEX_CONSTRAINT_GE:
			    if (dbl_value >= pC->dblValue)
				ok = 1;
			    break;
			};
		  }
	    }
	  if (pC->valueType == 'T')
	    {
		if (is_txt)
		  {
		      int ret = strcmp (txt_value, pC->txtValue);
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
	    }
	  if (txt_value)
	    {
		free (txt_value);
		txt_value = NULL;
	    }
	  if (!ok)
	      return 0;
	  pC = pC->next;
      }
    if (txt_value)
	free (txt_value);
    return 1;
}

static int
vtxt_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    int i;
    int iColumn;
    int op;
    int len;
    VirtualTextConstraintPtr pC;
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    gaiaTextReaderPtr text = cursor->pVtab->reader;
    if (idxNum)
	idxNum = idxNum;	/* unused arg warning suppression */

/* resetting any previously set filter constraint */
    vtxt_free_constraints (cursor);

    for (i = 0; i < argc; i++)
      {
	  if (!vtxt_parse_constraint (idxStr, i, &iColumn, &op))
	      continue;
	  pC = sqlite3_malloc (sizeof (VirtualTextConstraint));
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
	  if (!gaiaTextReaderGetRow (text, cursor->current_row))
	    {
		cursor->eof = 1;
		break;
	    }
	  if (vtxt_eval_constraints (cursor))
	      break;
	  cursor->current_row++;
      }
    return SQLITE_OK;
}

static int
vtxt_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching next row from cursor */
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    gaiaTextReaderPtr text = cursor->pVtab->reader;
    if (!text)
	cursor->eof = 1;
    else
      {
	  while (1)
	    {
		cursor->current_row++;
		if (!gaiaTextReaderGetRow (text, cursor->current_row))
		  {
		      cursor->eof = 1;
		      break;
		  }
		if (vtxt_eval_constraints (cursor))
		    break;
	    }
      }
    return SQLITE_OK;
}

static int
vtxt_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    return cursor->eof;
}

static int
vtxt_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    int nCol = 1;
    int i;
    char buf[4096];
    int type;
    const char *value;
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    gaiaTextReaderPtr text = cursor->pVtab->reader;
    if (column == 0)
      {
	  /* the ROWNO column */
	  sqlite3_result_int (pContext, cursor->current_row);
	  return SQLITE_OK;
      }
    if (text->current_line_ready == 0)
	return SQLITE_ERROR;
    for (i = 0; i < text->max_fields; i++)
      {
	  if (nCol == column)
	    {
		if (!gaiaTextReaderFetchField (text, i, &type, &value))
		    sqlite3_result_null (pContext);
		else
		  {
		      if (type == VRTTXT_INTEGER)
			{
			    strcpy (buf, value);
			    text_clean_integer (buf);
#if defined(_WIN32) || defined(__MINGW32__)
/* CAVEAT - M$ runtime has non-standard functions for 64 bits */
			    sqlite3_result_int64 (pContext, _atoi64 (buf));
#else
			    sqlite3_result_int64 (pContext, atoll (buf));
#endif
			}
		      else if (type == VRTTXT_DOUBLE)
			{
			    strcpy (buf, value);
			    text_clean_double (buf);
			    sqlite3_result_double (pContext, atof (buf));
			}
		      else if (type == VRTTXT_TEXT)
			  sqlite3_result_text (pContext, value, strlen (value),
					       free);
		      else
			  sqlite3_result_null (pContext);
		  }
	    }
	  nCol++;
      }
    return SQLITE_OK;
}

static int
vtxt_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vtxt_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    if (pVTab || argc || argv || pRowid)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_READONLY;
}

static int
vtxt_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vtxt_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vtxt_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vtxt_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
sqlite3VirtualTextInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    virtualtext_module.iVersion = 1;
    virtualtext_module.xCreate = &vtxt_create;
    virtualtext_module.xConnect = &vtxt_connect;
    virtualtext_module.xBestIndex = &vtxt_best_index;
    virtualtext_module.xDisconnect = &vtxt_disconnect;
    virtualtext_module.xDestroy = &vtxt_destroy;
    virtualtext_module.xOpen = &vtxt_open;
    virtualtext_module.xClose = &vtxt_close;
    virtualtext_module.xFilter = &vtxt_filter;
    virtualtext_module.xNext = &vtxt_next;
    virtualtext_module.xEof = &vtxt_eof;
    virtualtext_module.xColumn = &vtxt_column;
    virtualtext_module.xRowid = &vtxt_rowid;
    virtualtext_module.xUpdate = &vtxt_update;
    virtualtext_module.xBegin = &vtxt_begin;
    virtualtext_module.xSync = &vtxt_sync;
    virtualtext_module.xCommit = &vtxt_commit;
    virtualtext_module.xRollback = &vtxt_rollback;
    virtualtext_module.xFindFunction = NULL;
    sqlite3_create_module_v2 (db, "VirtualText", &virtualtext_module, NULL, 0);
    return rc;
}

SPATIALITE_PRIVATE int
virtualtext_extension_init (void *xdb)
{
    sqlite3 *db = (sqlite3 *) xdb;
    return sqlite3VirtualTextInit (db);
}

/*
**
** TextReader implementation
**
*/

static struct vrttxt_row_block *
vrttxt_block_alloc ()
{
/* allocating a rows Block */
    struct vrttxt_row_block *p = malloc (sizeof (struct vrttxt_row_block));
    if (!p)
	return NULL;
    p->num_rows = 0;
    p->min_line_no = -1;
    p->max_line_no = -1;
    p->next = NULL;
    return p;
}

static void
vrttxt_block_destroy (struct vrttxt_row_block *p)
{
/* destroying a rows Block */
    if (p)
	free (p);
}

GAIAGEO_DECLARE void
gaiaTextReaderDestroy (gaiaTextReaderPtr reader)
{
/* destroying the main TXT-Reader */
    int col;
    struct vrttxt_row_block *blk;
    struct vrttxt_row_block *blkN;
    if (reader)
      {
	  blk = reader->first;
	  while (blk)
	    {
		/* destroying the row offset Blocks */
		blkN = blk->next;
		vrttxt_block_destroy (blk);
		blk = blkN;
	    }
	  /* freeing the input buffers */
	  if (reader->line_buffer)
	      free (reader->line_buffer);
	  if (reader->field_buffer)
	      free (reader->field_buffer);
	  /* freeing the row offsets array */
	  if (reader->rows)
	      free (reader->rows);
	  /* closing the input file */
	  fclose (reader->text_file);
	  for (col = 0; col < VRTTXT_FIELDS_MAX; col++)
	    {
		/* destroying column headers */
		if (reader->columns[col].name != NULL)
		    free (reader->columns[col].name);
	    }
	  gaiaFreeUTF8Converter (reader->toUtf8);
	  free (reader);
      }
}

GAIAGEO_DECLARE gaiaTextReaderPtr
gaiaTextReaderAlloc (const char *path, char field_separator,
		     char text_separator, char decimal_separator,
		     int first_line_titles, const char *encoding)
{
/* allocating the main TXT-Reader */
    int col;
    gaiaTextReaderPtr reader;
    FILE *in = fopen (path, "rb");	/* opening the input file */
    if (in == NULL)
	return NULL;

/* allocating and initializing the struct */
    reader = malloc (sizeof (gaiaTextReader));
    if (!reader)
      {
	  fclose (in);
	  return NULL;
      }
    reader->text_file = in;
    reader->field_separator = field_separator;
    reader->text_separator = text_separator;
    reader->decimal_separator = decimal_separator;
    reader->first_line_titles = first_line_titles;
    reader->toUtf8 = gaiaCreateUTF8Converter (encoding);
    if (reader->toUtf8 == (void *) 0)
      {
	  fclose (in);
	  return NULL;
      }
    reader->error = 0;
    reader->first = NULL;
    reader->last = NULL;
    reader->rows = NULL;
    reader->num_rows = 0;
    reader->line_no = 0;
    reader->max_fields = 0;
    reader->max_current_field = 0;
    reader->current_line_ready = 0;
    reader->current_buf_sz = 1024;
    reader->line_buffer = malloc (1024);
    reader->field_buffer = malloc (1024);
    if (reader->line_buffer == NULL || reader->field_buffer == NULL)
      {
	  /* insufficient memory: no input buffers */
	  gaiaTextReaderDestroy (reader);
	  return NULL;
      }
    for (col = 0; col < VRTTXT_FIELDS_MAX; col++)
      {
	  /* initializing column headers */
	  reader->columns[col].name = NULL;
	  reader->columns[col].type = VRTTXT_NULL;
      }
    return reader;
}

static void
vrttxt_line_init (struct vrttxt_line *line, off_t offset)
{
/* initializing a LINE struct */
    line->offset = offset;
    line->len = 0;
    line->num_fields = 0;
    line->error = 0;
}

static void
vrttxt_line_end (struct vrttxt_line *line, off_t offset)
{
/* completing a Line struct (EndOfLine encountered) */
    line->len = offset - line->offset;
}

static void
vrttxt_add_field (struct vrttxt_line *line, off_t offset)
{
/* adding a Field offset to the current Line */
    if (line->num_fields >= VRTTXT_FIELDS_MAX)
      {
	  line->error = 1;
	  return;
      }
    line->field_offsets[line->num_fields] = offset - line->offset;
    line->num_fields++;
}

static int
vrttxt_is_integer (const char *value)
{
/* checking if this value can be an INTEGER */
    int invalids = 0;
    int digits = 0;
    int signs = 0;
    char last = '\0';
    const char *p = value;
    while (*p != '\0')
      {
	  last = *p;
	  if (*p >= '0' && *p <= '9')
	      digits++;
	  else if (*p == '+' || *p == '-')
	      signs++;
	  else
	      invalids++;
	  p++;
      }
    if (invalids)
	return 0;		/* invalid chars were found */
    if (signs > 1)
	return 0;		/* more than a single sign */
    if (signs)
      {
	  if (*value == '+' || *value == '-' || last == '+' || last == '-')
	      ;
	  else
	      return 0;		/* sign is not the first/last string char */
      }
    return 1;			/* ok, can be a valid INTEGER value */
}

static int
vrttxt_is_double (const char *value, char decimal_separator)
{
/* checking if this value can be a DOUBLE */
    int invalids = 0;
    int digits = 0;
    int signs = 0;
    int points = 0;
    char last = '\0';
    const char *p = value;
    while (*p != '\0')
      {
	  last = *p;
	  if (*p >= '0' && *p <= '9')
	      digits++;
	  else if (*p == '+' || *p == '-')
	      signs++;
	  else
	    {
		if (decimal_separator == ',')
		  {
		      if (*p == ',')
			  points++;
		      else
			  invalids++;
		  }
		else
		  {
		      if (*p == '.')
			  points++;
		      else
			  invalids++;
		  }
	    }
	  p++;
      }
    if (invalids)
	return 0;		/* invalid chars were found */
    if (points > 1)
	return 0;		/* more than a single decimal separator */
    if (signs > 1)
	return 0;		/* more than a single sign */
    if (signs)
      {
	  if (*value == '+' || *value == '-' || last == '+' || last == '-')
	      ;
	  else
	      return 0;		/* sign is not the first/last string char */
      }
    return 1;			/* ok, can be a valid DOUBLE value */
}

static int
vrttxt_check_type (const char *value, char decimal_separator)
{
/* checking the Field type */
    if (*value == '\0')
	return VRTTXT_NULL;
    if (vrttxt_is_integer (value))
	return VRTTXT_INTEGER;
    if (vrttxt_is_double (value, decimal_separator))
	return VRTTXT_DOUBLE;
    return VRTTXT_TEXT;
}

static int
vrttxt_set_column_title (gaiaTextReaderPtr txt, int col_no, const char *name)
{
/* setting a Column header name */
    int err;
    int ind;
    char *utf8text;
    char *str = (char *) name;
    int len = strlen (str);
    if (len <= 0)
	return 0;
    if (str[0] == txt->text_separator && str[len - 1] == txt->text_separator)
      {
	  /* cleaning the enclosing quotes */
	  str[len - 1] = '\0';
	  str = (char *) (name + 1);
	  len -= 2;
	  if (len <= 0)
	      return 0;
      }
    utf8text = gaiaConvertToUTF8 (txt->toUtf8, str, len, &err);
    if (err)
      {
	  if (utf8text)
	      free (utf8text);
	  return 0;
      }
    else
	str = utf8text;
    len = strlen (str);
    for (ind = 0; ind < len; ind++)
      {
	  /* masking spaces and so on within the column name */
	  switch (str[ind])
	    {
	    case ' ':
	    case '\t':
	    case '-':
	    case '+':
	    case '*':
	    case '/':
	    case '(':
	    case ')':
	    case '[':
	    case ']':
	    case '{':
	    case '}':
		str[ind] = '_';
		break;
	    }
      }
    if (txt->columns[col_no].name)
	free (txt->columns[col_no].name);
    txt->columns[col_no].name = malloc (len + 1);
    if (txt->columns[col_no].name == NULL)
	return 0;
    strcpy (txt->columns[col_no].name, utf8text);
    free (utf8text);
    return 1;
}

static void
vrttxt_add_line (gaiaTextReaderPtr txt, struct vrttxt_line *line)
{
/* appending a Line offset to the main TXT-Reader */
    struct vrttxt_row_block *p_block;
    struct vrttxt_row *p_row;
    int ind;
    int off;
    int len;
    int value_type;
    int column_type;
    int first_line = 0;
    if (txt->line_no == 0)
	first_line = 1;
    if (line->error)
      {
	  txt->error = 1;
	  txt->line_no++;
	  return;
      }
    if (line->num_fields == 0)
      {
	  txt->line_no++;
	  return;
      }
    p_block = txt->last;
    if (p_block == NULL)
      {
	  /* the offset Blocks list is empty: allocating the first Block */
	  p_block = vrttxt_block_alloc ();
	  if (!p_block)
	    {
		txt->error = 1;
		txt->line_no++;
		return;
	    }
	  if (txt->first == NULL)
	      txt->first = p_block;
	  if (txt->last != NULL)
	      txt->last->next = p_block;
	  txt->last = p_block;
      }
    else if (p_block->num_rows >= VRTTXT_BLOCK_MAX)
      {
	  /* the current offset Block is full: expanding the list */
	  p_block = vrttxt_block_alloc ();
	  if (!p_block)
	    {
		txt->error = 1;
		txt->line_no++;
		return;
	    }
	  if (txt->first == NULL)
	      txt->first = p_block;
	  if (txt->last != NULL)
	      txt->last->next = p_block;
	  txt->last = p_block;
      }
/* inserting the Row offset into the offset Block */
    p_row = p_block->rows + p_block->num_rows;
    p_block->num_rows++;
    p_row->line_no = txt->line_no;
    if (p_block->min_line_no < 0)
	p_block->min_line_no = p_row->line_no;
    if (p_block->max_line_no < p_row->line_no)
	p_block->max_line_no = p_row->line_no;
    txt->line_no++;
    p_row->offset = line->offset;
    p_row->len = line->len;
    p_row->num_fields = line->num_fields;
    if (line->num_fields > txt->max_fields)
	txt->max_fields = line->num_fields;
    off = 0;
    for (ind = 0; ind < p_row->num_fields; ind++)
      {
	  /* setting the corresponding Column (aka Field) header */
	  len = line->field_offsets[ind] - off;
	  if (len == 0)
	      *(txt->field_buffer) = '\0';
	  else
	    {
		/* retrieving the current Field Value */
		memcpy (txt->field_buffer, txt->line_buffer + off, len);
		*(txt->field_buffer + len) = '\0';
	    }
	  if (txt->first_line_titles && first_line)
	    {
		/* first line: the current value is the Column Name */
		if (!vrttxt_set_column_title (txt, ind, txt->field_buffer))
		    txt->error = 1;
	    }
	  else
	    {
		/* plain Field Value */
		value_type =
		    vrttxt_check_type (txt->field_buffer,
				       txt->decimal_separator);
		column_type = txt->columns[ind].type;
		switch (value_type)
		  {
		      /* checking the Column type */
		  case VRTTXT_INTEGER:
		      if (column_type == VRTTXT_NULL)
			  txt->columns[ind].type = VRTTXT_INTEGER;
		      break;
		  case VRTTXT_DOUBLE:
		      if (column_type == VRTTXT_NULL
			  || column_type == VRTTXT_INTEGER)
			  txt->columns[ind].type = VRTTXT_DOUBLE;
		      break;
		  case VRTTXT_TEXT:
		      txt->columns[ind].type = VRTTXT_TEXT;
		      break;
		  default:
		      break;
		  };
	    }
	  off = line->field_offsets[ind] + 1;
      }
}

static void
vrttxt_line_push (gaiaTextReaderPtr txt, char c)
{
/* inserting a single char into the dynamically growing buffer */
    if (txt->error)
	return;
    if ((txt->current_buf_off + 1) >= txt->current_buf_sz)
      {
	  /* expanding the input buffer */
	  int new_sz;
	  char *new_buf;
	  /*
	     / allocation strategy:
	     / - the input buffer has an initial size of 1024 bytes
	     /   (good for short lines)
	     / - the second step allocates 4196 bytes
	     / - the third step allocates 65536 bytes
	     /   (good for medium sized lines)
	     / - after this the buffer allocation will be increased
	     /   be 1MB at each step (good for huge sized lines)
	   */
	  if (txt->current_buf_sz < 4196)
	      new_sz = 4196;
	  else if (txt->current_buf_sz < 65536)
	      new_sz = 65536;
	  else
	      new_sz = txt->current_buf_sz + (1024 * 1024);
	  new_buf = malloc (new_sz);
	  if (!new_buf)
	    {
		txt->error = 1;
		return;
	    }
	  txt->current_buf_sz = new_sz;
	  memcpy (new_buf, txt->line_buffer, txt->current_buf_off);
	  free (txt->line_buffer);
	  txt->line_buffer = new_buf;
	  free (txt->field_buffer);
	  txt->field_buffer = malloc (new_sz);
	  if (txt->field_buffer == NULL)
	    {
		txt->error = 1;
		return;
	    }
      }
    *(txt->line_buffer + txt->current_buf_off) = c;
    txt->current_buf_off++;
/* ensuring that input buffer will be null terminated anyway */
    *(txt->line_buffer + txt->current_buf_off) = '\0';
}

static void
vrttxt_build_line_array (gaiaTextReaderPtr txt)
{
/* creating the final Line offsets array */
    struct vrttxt_row_block *p_block;
    int i;
    int cnt = 0;
    int first_line = 1;
    if (txt->rows)
	free (txt->rows);
    txt->rows = NULL;
    txt->num_rows = 0;
    p_block = txt->first;
    while (p_block)
      {
	  /* counting how many lines are there */
	  if (p_block == txt->first && txt->first_line_titles)
	      txt->num_rows += p_block->num_rows - 1;
	  else
	      txt->num_rows += p_block->num_rows;
	  p_block = p_block->next;
      }
    txt->rows = malloc (sizeof (struct vrttxt_row *) * txt->num_rows);
    if (txt->rows == NULL)
      {
	  /* insufficient memory */
	  txt->error = 1;
	  return;
      }
    p_block = txt->first;
    while (p_block)
      {
	  for (i = 0; i < p_block->num_rows; i++)
	    {
		/* setting Line references into the array */
		if (first_line && txt->first_line_titles)
		  {
		      first_line = 0;
		      continue;	/* skipping the first line (column names) */
		  }
		*(txt->rows + cnt++) = p_block->rows + i;
	    }
	  p_block = p_block->next;
      }
}

GAIAGEO_DECLARE int
gaiaTextReaderParse (gaiaTextReaderPtr txt)
{
/* 
/ preliminary parsing
/ - reading the input file until EOF
/ - then feeding the Row offsets structs
/   to be used for any subsequent access
*/
    char name[64];
    int ind;
    int i2;
    int c;
    int masked = 0;
    int token_start = 1;
    int row_offset = 0;
    off_t offset = 0;
    struct vrttxt_line line;
    vrttxt_line_init (&line, 0);
    txt->current_buf_off = 0;

    while ((c = getc (txt->text_file)) != EOF)
      {
	  if (c == txt->text_separator)
	    {
		if (masked)
		    masked = 0;
		else
		  {
		      if (token_start)
			  masked = 1;
		  }
		vrttxt_line_push (txt, c);
		if (txt->error)
		    return 0;
		row_offset++;
		offset++;
		continue;
	    }
	  token_start = 0;
	  if (c == '\r')
	    {
		if (masked)
		  {
		      vrttxt_line_push (txt, c);
		      if (txt->error)
			  return 0;
		      row_offset++;
		  }
		offset++;
		continue;
	    }
	  if (c == '\n')
	    {
		if (masked)
		  {
		      vrttxt_line_push (txt, c);
		      if (txt->error)
			  return 0;
		      row_offset++;
		      offset++;
		      continue;
		  }
		vrttxt_add_field (&line, offset);
		vrttxt_line_end (&line, offset);
		vrttxt_add_line (txt, &line);
		if (txt->error)
		    return 0;
		vrttxt_line_init (&line, offset + 1);
		txt->current_buf_off = 0;
		token_start = 1;
		row_offset = 0;
		offset++;
		continue;
	    }
	  if (c == txt->field_separator)
	    {
		if (masked)
		  {
		      vrttxt_line_push (txt, c);
		      if (txt->error)
			  return 0;
		      row_offset++;
		      offset++;
		      continue;
		  }
		vrttxt_line_push (txt, c);
		if (txt->error)
		    return 0;
		row_offset++;
		vrttxt_add_field (&line, offset);
		token_start = 1;
		offset++;
		continue;
	    }
	  vrttxt_line_push (txt, c);
	  if (txt->error)
	      return 0;
	  row_offset++;
	  offset++;
      }
    if (txt->error)
	return 0;
    if (txt->first_line_titles)
      {
	  /* checking for duplicate column names */
	  for (ind = 0; ind < txt->max_fields; ind++)
	    {
		for (i2 = 0; i2 < ind; i2++)
		  {
		      if (strcasecmp
			  (txt->columns[i2].name, txt->columns[ind].name) == 0)
			{
			    sprintf (name, "COL%03d", ind + 1);
			    if (!vrttxt_set_column_title (txt, ind, name))
			      {
				  txt->error = 1;
				  return 0;
			      }
			}
		  }
	    }
      }
    else
      {
	  /* setting convenience column names */
	  for (ind = 0; ind < txt->max_fields; ind++)
	    {
		sprintf (name, "COL%03d", ind + 1);
		if (!vrttxt_set_column_title (txt, ind, name))
		  {
		      txt->error = 1;
		      return 0;
		  }
	    }
      }
    if (txt->error)
	return 0;
    vrttxt_build_line_array (txt);
    if (txt->error)
	return 0;
    return 1;
}

GAIAGEO_DECLARE int
gaiaTextReaderGetRow (gaiaTextReaderPtr txt, int line_no)
{
/* reading a Line (identified by relative number */
    int i;
    char c;
    int masked = 0;
    int token_start = 1;
    int fld = 0;
    int offset = 0;
    struct vrttxt_row *p_row;
    txt->current_line_ready = 0;
    txt->max_current_field = 0;
    if (line_no < 0 || line_no >= txt->num_rows || txt->rows == NULL)
	return 0;
    p_row = *(txt->rows + line_no);
    if (fseek (txt->text_file, p_row->offset, SEEK_SET) != 0)
	return 0;
    if (fread (txt->line_buffer, 1, p_row->len, txt->text_file) !=
	(unsigned int) (p_row->len))
	return 0;
    txt->field_offsets[0] = 0;
    for (i = 0; i < p_row->len; i++)
      {
	  /* parsing Fields */
	  c = *(txt->line_buffer + i);
	  if (c == txt->text_separator)
	    {
		if (masked)
		    masked = 0;
		else
		  {
		      if (token_start)
			  masked = 1;
		  }
		offset++;
		continue;
	    }
	  token_start = 0;
	  if (c == '\r')
	    {
		offset++;
		continue;
	    }
	  if (c == txt->field_separator)
	    {
		if (masked)
		  {
		      offset++;
		      continue;
		  }
		txt->field_offsets[fld + 1] = offset + 1;
		txt->field_lens[fld] = -1;
		txt->field_lens[fld] = offset - txt->field_offsets[fld];
		fld++;
		txt->max_current_field = fld;
		token_start = 1;
		offset++;
		continue;
	    }
	  offset++;
      }
    if (offset > 0)
      {
	  txt->field_lens[fld] = offset - txt->field_offsets[fld];
	  fld++;
	  txt->max_current_field = fld;
      }
    txt->current_line_ready = 1;
    return 1;
}

GAIAGEO_DECLARE int
gaiaTextReaderFetchField (gaiaTextReaderPtr txt, int field_idx, int *type,
			  const char **value)
{
/* fetching a field value */
    char *utf8text = NULL;
    int err;
    int len;
    char *str;
    if (txt->current_line_ready == 0)
      {
	  *type = VRTTXT_NULL;
	  *value = NULL;
	  return 0;
      }
    if (field_idx < 0 || field_idx >= txt->max_fields)
      {
	  *type = VRTTXT_NULL;
	  *value = NULL;
	  return 0;
      }
    if (field_idx < 0 || field_idx >= txt->max_current_field)
      {
	  *type = VRTTXT_NULL;
	  *value = NULL;
	  return 0;
      }
    *type = txt->columns[field_idx].type;
    if (txt->field_lens[field_idx] == 0)
	*(txt->field_buffer) = '\0';
    memcpy (txt->field_buffer, txt->line_buffer + txt->field_offsets[field_idx],
	    txt->field_lens[field_idx]);
    *(txt->field_buffer + txt->field_lens[field_idx]) = '\0';
    *value = txt->field_buffer;
/* sandro 2012-02-01: fixing CR handling for last column [windows] */
    if (*(txt->field_buffer) == '\r' && txt->field_lens[field_idx] == 1
	&& (field_idx + 1) == txt->max_fields)
	*(txt->field_buffer) = '\0';
    if (*(txt->field_buffer) == '\0')
	*type = VRTTXT_NULL;
    else if (*type == VRTTXT_TEXT)
      {
	  /* converting to UTF-8 */
	  str = (char *) *value;
	  len = strlen (str);
	  if (str[len - 1] == '\r')
	    {
		/* skipping trailing CR, if any */
		str[len - 1] = '\0';
		len--;
	    }
	  if (str[0] == txt->text_separator
	      && str[len - 1] == txt->text_separator)
	    {
		/* cleaning the enclosing quotes */
		str[len - 1] = '\0';
		str = (char *) (*value + 1);
		len -= 2;
		if (len <= 0)
		  {
		      *type = VRTTXT_NULL;
		      *value = NULL;
		      return 1;
		  }
	    }
	  utf8text = gaiaConvertToUTF8 (txt->toUtf8, str, len, &err);
	  if (err)
	    {
		/* memory cleanup: Kashif Rasul 14 Jan 2010 */
		if (utf8text)
		    free (utf8text);
		*type = VRTTXT_NULL;
		*value = NULL;
		return 0;
	    }
	  *value = utf8text;
      }
    return 1;
}

#endif /* ICONV enabled/disabled */
