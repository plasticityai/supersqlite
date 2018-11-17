/*

 virtualxpath.c -- SQLite3 extension [VIRTUAL XPath handler]

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
#include <math.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#ifdef ENABLE_LIBXML2		/* LIBXML2 enabled: supporting XML documents */

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#define LONG64_MAX	_I64_MAX
#define LONG64_MIN	_I64_MIN
#else
#define LONG64_MAX	9223372036854775807LL
#define LONG64_MIN	(-LONG64_MAX + 1)
#endif

#include <spatialite/sqlite.h>

#include <spatialite/spatialite.h>
#include <spatialite/debug.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/gaiageo.h>

#ifdef _WIN32
#define strcasecmp	_stricmp
#define strncasecmp	_strnicmp
#endif /* not WIN32 */

static struct sqlite3_module my_xpath_module;

/******************************************************************************
/
/ VirtualTable structs
/
******************************************************************************/

typedef struct VirtualXPathStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    const void *p_cache;	/* pointer to the internal cache */
    char *table;		/* the real-table name */
    char *column;		/* the real-column name */
} VirtualXPath;
typedef VirtualXPath *VirtualXPathPtr;

typedef struct VirtualXPathCursorStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualXPathPtr pVtab;	/* Virtual table of this cursor */
    int eof;			/* the EOF marker */
    sqlite3_stmt *stmt;
    char *xpathExpr;
    xmlDocPtr xmlDoc;
    xmlXPathContextPtr xpathContext;
    xmlXPathObjectPtr xpathObj;
    int xpathIdx;
    sqlite3_int64 current_row;
    int keyOp1;
    sqlite3_int64 keyVal1;
    int keyOp2;
    sqlite3_int64 keyVal2;
} VirtualXPathCursor;
typedef VirtualXPathCursor *VirtualXPathCursorPtr;

static void
vxpath_free_ns (struct vxpath_ns *ns)
{
/* memory cleanup - destroying a Namespace item */
    if (!ns)
	return;
    if (ns->Prefix)
	free (ns->Prefix);
    if (ns->Href)
	free (ns->Href);
    free (ns);
}

SPATIALITE_PRIVATE void
vxpath_free_namespaces (struct vxpath_namespaces *ns_list)
{
/* memory cleanup - destroying the Namespaces list */
    struct vxpath_ns *ns;
    struct vxpath_ns *nns;
    if (!ns_list)
	return;

    ns = ns_list->First;
    while (ns)
      {
	  nns = ns->Next;
	  vxpath_free_ns (ns);
	  ns = nns;
      }
    free (ns_list);
}

static void
vxpath_add_ns (struct vxpath_namespaces *ns_list, const char *prefix,
	       const char *href)
{
/* inserting a further Namespace into the list */
    int len;
    struct vxpath_ns *ns = ns_list->First;
    while (ns)
      {
	  /* checking if it's already defined */
	  if (ns->Prefix == NULL || prefix == NULL)
	    {
		if (ns->Prefix == NULL && prefix == NULL
		    && strcmp (ns->Href, href) == 0)
		  {
		      /* ok, already defined (default Namespace) */
		      return;
		  }
	    }
	  else
	    {
		if (strcmp (ns->Prefix, prefix) == 0
		    && strcmp (ns->Href, href) == 0)
		  {
		      /* ok, already defined */
		      return;
		  }
	    }
	  ns = ns->Next;
      }

/* inserting a new Namespace */
    ns = malloc (sizeof (struct vxpath_ns));
    if (prefix == NULL)
	ns->Prefix = NULL;
    else
      {
	  len = strlen (prefix);
	  ns->Prefix = malloc (len + 1);
	  strcpy (ns->Prefix, prefix);
      }
    len = strlen (href);
    ns->Href = malloc (len + 1);
    strcpy (ns->Href, href);
    ns->Next = NULL;
    if (ns_list->First == NULL)
	ns_list->First = ns;
    if (ns_list->Last != NULL)
	ns_list->Last->Next = ns;
    ns_list->Last = ns;
}

static void
vxpath_feed_ns (struct vxpath_namespaces *ns_list, xmlNodePtr start)
{
/* recursively searching for Namespaces */
    xmlNodePtr node = start;
    while (node)
      {
	  if (node->ns != NULL)
	    {
		/* a Namespace is defined */
		vxpath_add_ns (ns_list, (const char *) (node->ns->prefix),
			       (const char *) (node->ns->href));
	    }
	  if (node->properties != NULL)
	    {
		/* exploring the Attribute list */
		struct _xmlAttr *attr = node->properties;
		while (attr)
		  {
		      if (attr->type == XML_ATTRIBUTE_NODE)
			{
			    if (attr->ns != NULL)
			      {
				  /* a Namespace is defined */
				  vxpath_add_ns (ns_list,
						 (const char *) (attr->ns->
								 prefix),
						 (const char *) (attr->ns->
								 href));
			      }
			}
		      attr = attr->next;
		  }
	    }
	  vxpath_feed_ns (ns_list, node->children);
	  node = node->next;
      }
}

SPATIALITE_PRIVATE struct vxpath_namespaces *
vxpath_get_namespaces (void *p_xml_doc)
{
/* creating and populating the Namespaces list */
    xmlDocPtr xml_doc = (xmlDocPtr) p_xml_doc;
    xmlNodePtr root = xmlDocGetRootElement (xml_doc);
    struct vxpath_namespaces *ns_list;
    ns_list = malloc (sizeof (struct vxpath_namespaces));
    ns_list->First = NULL;
    ns_list->Last = NULL;
    vxpath_feed_ns (ns_list, root);
    return ns_list;
}

static int
is_valid_cache (struct splite_internal_cache *cache)
{
/* testing if the passed cache is a valid one */
    if (cache == NULL)
	return 0;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return 0;
    return 1;
}

static void
vxpathError (void *ctx, const char *msg, ...)
{
/* appending to the current XPath Error buffer */
    struct splite_internal_cache *cache = (struct splite_internal_cache *) ctx;
    gaiaOutBufferPtr buf;
    char out[65536];
    va_list args;

    if (ctx != NULL)
	ctx = NULL;		/* suppressing stupid compiler warnings (unused args) */
    if (!is_valid_cache (cache))
	return;
    buf = (gaiaOutBufferPtr) (cache->xmlXPathErrors);

    va_start (args, msg);
    vsnprintf (out, 65536, msg, args);
    gaiaAppendToOutBuffer (buf, out);
    va_end (args);
}

static void
vxpathResetXmlErrors (struct splite_internal_cache *cache)
{
/* resetting the XPath Error buffer */
    gaiaOutBufferPtr buf;
    if (!is_valid_cache (cache))
	return;
    buf = (gaiaOutBufferPtr) (cache->xmlXPathErrors);
    gaiaOutBufferReset (buf);
}

GAIAGEO_DECLARE int
gaiaIsValidXPathExpression (const void *p_cache, const char *xpath_expr)
{
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    xmlXPathCompExprPtr result;
    xmlGenericErrorFunc xpathError;
    if (!is_valid_cache (cache))
	return 0;
    xpathError = (xmlGenericErrorFunc) vxpathError;

    vxpathResetXmlErrors (cache);
    xmlSetGenericErrorFunc (cache, xpathError);

/* testing an XPath expression */
    result = xmlXPathCompile ((const xmlChar *) xpath_expr);
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
    if (result)
      {
	  xmlXPathFreeCompExpr (result);
	  return 1;
      }
    return 0;
}

SPATIALITE_PRIVATE int
vxpath_eval_expr (const void *p_cache, void *x_xml_doc, const char *xpath_expr,
		  void *x_xpathCtx, void *x_xpathObj)
{
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
/* evaluating an XPath expression */
    xmlDocPtr xml_doc = (xmlDocPtr) x_xml_doc;
    xmlXPathContextPtr *p_xpathCtx = (xmlXPathContextPtr *) x_xpathCtx;
    xmlXPathObjectPtr *p_xpathObj = (xmlXPathObjectPtr *) x_xpathObj;
    xmlXPathObjectPtr xpathObj;
    xmlXPathContextPtr xpathCtx;
    xmlGenericErrorFunc xpathError = (xmlGenericErrorFunc) vxpathError;

/* attempting to identify all required Namespaces */
    struct vxpath_ns *ns;
    struct vxpath_namespaces *ns_list = vxpath_get_namespaces (xml_doc);

    if (is_valid_cache (cache))
      {
	  vxpathResetXmlErrors (cache);
	  xmlSetGenericErrorFunc (cache, xpathError);
      }

/* creating an XPath context */
    xpathCtx = xmlXPathNewContext (xml_doc);
    if (xpathCtx == NULL)
      {
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return 0;
      }

/* registering all Namespaces */
    if (xpathCtx != NULL && ns_list != NULL)
      {
	  ns = ns_list->First;
	  while (ns)
	    {
		if (ns->Prefix == NULL)
		  {
		      /* the default Namespace always is "dflt:xx" */
		      xmlXPathRegisterNs (xpathCtx, (xmlChar *) "dflt",
					  (xmlChar *) ns->Href);
		  }
		else
		  {
		      /* a fully qualified Namespace */
		      xmlXPathRegisterNs (xpathCtx, (xmlChar *) ns->Prefix,
					  (xmlChar *) ns->Href);
		  }
		ns = ns->Next;
	    }
      }
    vxpath_free_namespaces (ns_list);

/* evaluating the XPath expression */
    xpathObj = xmlXPathEvalExpression ((const xmlChar *) xpath_expr, xpathCtx);
    if (xpathObj != NULL)
      {
	  xmlNodeSetPtr nodes = xpathObj->nodesetval;
	  int num_nodes = (nodes) ? nodes->nodeNr : 0;
	  if (num_nodes >= 1)
	    {
		/* OK: match found */
		*p_xpathCtx = xpathCtx;
		*p_xpathObj = xpathObj;
		xmlSetGenericErrorFunc ((void *) stderr, NULL);
		return 1;
	    }
	  /* invalid: empty nodeset */
	  xmlXPathFreeObject (xpathObj);
      }
    xmlXPathFreeContext (xpathCtx);

    xmlSetGenericErrorFunc ((void *) stderr, NULL);
    return 0;
}

static void
vxpath_read_row (VirtualXPathCursorPtr cursor)
{
/* trying to read a row from the real-table */
    sqlite3_stmt *stmt;
    int ret;
    sqlite3_int64 pk;
    int eof;
    if (cursor->stmt == NULL || cursor->xpathExpr == NULL)
	return;

    if (cursor->xpathObj)
	xmlXPathFreeObject (cursor->xpathObj);
    if (cursor->xpathContext)
	xmlXPathFreeContext (cursor->xpathContext);
    if (cursor->xmlDoc)
	xmlFreeDoc (cursor->xmlDoc);
    cursor->xmlDoc = NULL;
    cursor->xpathContext = NULL;
    cursor->xpathObj = NULL;

    stmt = cursor->stmt;
    sqlite3_bind_int64 (stmt, 1, cursor->current_row);
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_ROW)
	    {
		pk = sqlite3_column_int64 (stmt, 0);
		/* filtering the PK value */
		eof = 0;
		switch (cursor->keyOp1)
		  {
		  case SQLITE_INDEX_CONSTRAINT_EQ:
		      if (pk > cursor->keyVal1)
			  eof = 1;
		      break;
		  case SQLITE_INDEX_CONSTRAINT_LT:
		      if (pk >= cursor->keyVal1)
			  eof = 1;
		      break;
		  case SQLITE_INDEX_CONSTRAINT_LE:
		      if (pk > cursor->keyVal1)
			  eof = 1;
		      break;
		  };
		switch (cursor->keyOp2)
		  {
		  case SQLITE_INDEX_CONSTRAINT_EQ:
		      if (pk > cursor->keyVal2)
			  eof = 1;
		      break;
		  case SQLITE_INDEX_CONSTRAINT_LT:
		      if (pk >= cursor->keyVal2)
			  eof = 1;
		      break;
		  case SQLITE_INDEX_CONSTRAINT_LE:
		      if (pk > cursor->keyVal2)
			  eof = 1;
		      break;
		  };
		if (eof)
		  {
		      cursor->eof = 1;
		      return;
		  }

		if (sqlite3_column_type (stmt, 1) == SQLITE_BLOB)
		  {
		      xmlDocPtr xml_doc;
		      int xml_len;
		      unsigned char *xml;
		      const unsigned char *blob = sqlite3_column_blob (stmt, 1);
		      int size = sqlite3_column_bytes (stmt, 1);
		      gaiaXmlFromBlob (blob, size, -1, &xml, &xml_len);
		      if (!xml)
			  continue;
		      xml_doc =
			  xmlReadMemory ((const char *) xml, xml_len,
					 "noname.xml", NULL, 0);
		      if (xml_doc != NULL)
			{
			    xmlXPathContextPtr xpathCtx;
			    xmlXPathObjectPtr xpathObj;
			    if (vxpath_eval_expr
				(cursor->pVtab->p_cache, xml_doc,
				 cursor->xpathExpr, &xpathCtx, &xpathObj))
			      {
				  free (xml);
				  if (cursor->xpathObj)
				      xmlXPathFreeObject (cursor->xpathObj);
				  if (cursor->xpathContext)
				      xmlXPathFreeContext
					  (cursor->xpathContext);
				  if (cursor->xmlDoc)
				      xmlFreeDoc (cursor->xmlDoc);
				  cursor->xmlDoc = xml_doc;
				  cursor->xpathContext = xpathCtx;
				  cursor->xpathObj = xpathObj;
				  cursor->xpathIdx = 0;
				  break;
			      }
			    free (xml);
			    xmlFreeDoc (xml_doc);
			}
		  }
	    }
	  else
	    {
		/* an error occurred */
		cursor->eof = 1;
		return;
	    }
      }
    cursor->eof = 0;
    cursor->current_row = pk;
}

static void
vxpath_check (sqlite3 * db, const char *table, const char *column, int *okTable,
	      int *okCol)
{
/* checking if both Table and Column exist */
    char **results;
    char *sql;
    char *xname;
    int ret;
    int i;
    int n_rows;
    int n_columns;
    xname = gaiaDoubleQuotedSql (table);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xname);
    free (xname);
    ret = sqlite3_get_table (db, sql, &results, &n_rows, &n_columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return;
    if (n_rows >= 1)
      {
	  *okTable = 1;
	  for (i = 1; i <= n_rows; i++)
	    {
		const char *col_name = results[(i * n_columns) + 1];
		if (strcasecmp (col_name, column) == 0)
		    *okCol = 1;
	    }
      }
    sqlite3_free_table (results);
}

static int
vxpath_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	       sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table for XPath */
    VirtualXPathPtr p_vt;
    char *vtable = NULL;
    char *table = NULL;
    char *column = NULL;
    char *xname;
    char *sql;
    int okTable = 0;
    int okCol = 0;
    if (argc == 5)
      {
	  vtable = gaiaDequotedSql ((char *) argv[2]);
	  table = gaiaDequotedSql ((char *) argv[3]);
	  column = gaiaDequotedSql ((char *) argv[4]);
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualXPath module] CREATE VIRTUAL: illegal arg list {void}\n");
	  return SQLITE_ERROR;
      }
    vxpath_check (db, table, column, &okTable, &okCol);
    if (!okTable || !okCol)
	goto illegal;
    xname = gaiaDoubleQuotedSql (vtable);
    sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (pkid INTEGER, sub INTEGER, "
			   "parent TEXT, node TEXT, attribute TEXT, "
			   "value TEXT, xpath_expr TEXT)", xname);
    free (xname);
    if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
      {
	  sqlite3_free (sql);
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualXPath module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
	       sql);
	  goto error;
      }
    sqlite3_free (sql);
    p_vt = (VirtualXPathPtr) sqlite3_malloc (sizeof (VirtualXPath));
    if (!p_vt)
	return SQLITE_NOMEM;
    p_vt->db = db;
    p_vt->p_cache = pAux;
    if (p_vt->p_cache == NULL)
	spatialite_e ("VirtualXPath WARNING - no XML cache is available !!!\n");
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
    p_vt->table = table;
    p_vt->column = column;
    *ppVTab = (sqlite3_vtab *) p_vt;
    free (vtable);
    return SQLITE_OK;
  illegal:
/* something is going the wrong way */
    if (!okTable == 0)
	*pzErr =
	    sqlite3_mprintf
	    ("[VirtualXPath module] table \"%s\" doesn't exists\n", table);
    else if (!okCol)
	*pzErr =
	    sqlite3_mprintf
	    ("[VirtualXPath module] table \"%s\" exists, but has no \"%s\" column\n",
	     table, column);
  error:
    return SQLITE_ERROR;
}

static int
vxpath_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
		sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table - simply aliases vxpath_create() */
    return vxpath_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vxpath_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIdxInfo)
{
/* best index selection */
    int i;
    int errors = 0;
    int err = 1;
    int xpath = 0;
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    for (i = 0; i < pIdxInfo->nConstraint; i++)
      {
	  /* verifying the constraints */
	  struct sqlite3_index_constraint *p = &(pIdxInfo->aConstraint[i]);
	  if (p->usable)
	    {
		if (p->iColumn == 0);
		else if (p->iColumn == 6 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		    xpath++;
		else
		    errors++;
	    }
      }
    if (xpath == 1 && errors == 0)
      {
	  /* this one is a valid XPath query */
	  pIdxInfo->idxNum = 1;
	  pIdxInfo->estimatedCost = 1.0;
	  pIdxInfo->idxStr = sqlite3_malloc (pIdxInfo->nConstraint * 2);
	  pIdxInfo->needToFreeIdxStr = 1;
	  for (i = 0; i < pIdxInfo->nConstraint; i++)
	    {
		struct sqlite3_index_constraint *p =
		    &(pIdxInfo->aConstraint[i]);
		if (p->usable)
		  {
		      char *pStr = pIdxInfo->idxStr + (i * 2);
		      if (p->iColumn == 6)
			  *pStr = 0;
		      else
			  *pStr = 1;
		      pStr++;
		      *pStr = p->op;
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
vxpath_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualXPathPtr p_vt = (VirtualXPathPtr) pVTab;
    free (p_vt->column);
    free (p_vt->table);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
vxpath_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vxpath_disconnect() */
    return vxpath_disconnect (pVTab);
}

static int
vxpath_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    sqlite3_stmt *stmt;
    int ret;
    char *sql;
    char *xname;
    char *xcolumn;
    VirtualXPathCursorPtr cursor =
	(VirtualXPathCursorPtr) sqlite3_malloc (sizeof (VirtualXPathCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = (VirtualXPathPtr) pVTab;
    cursor->xmlDoc = NULL;
    cursor->xpathContext = NULL;
    cursor->xpathObj = NULL;
    cursor->xpathExpr = NULL;
    cursor->stmt = NULL;
    cursor->keyOp1 = 0;
    cursor->keyVal1 = 0;
    cursor->keyOp2 = 0;
    cursor->keyVal2 = 0;
    xcolumn = gaiaDoubleQuotedSql (cursor->pVtab->column);
    xname = gaiaDoubleQuotedSql (cursor->pVtab->table);
    sql = sqlite3_mprintf ("SELECT ROWID, \"%s\" FROM \"%s\""
			   " WHERE ROWID >= ?", xcolumn, xname);
    free (xname);
    free (xcolumn);
    ret =
	sqlite3_prepare_v2 (cursor->pVtab->db, sql, strlen (sql), &stmt, NULL);
    sqlite3_free (sql);
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
    return SQLITE_OK;
}

static int
vxpath_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualXPathCursorPtr cursor = (VirtualXPathCursorPtr) pCursor;
    if (cursor->stmt)
	sqlite3_finalize (cursor->stmt);
    if (cursor->xpathObj)
	xmlXPathFreeObject (cursor->xpathObj);
    if (cursor->xpathContext)
	xmlXPathFreeContext (cursor->xpathContext);
    if (cursor->xmlDoc)
	xmlFreeDoc (cursor->xmlDoc);
    if (cursor->xpathExpr)
	free (cursor->xpathExpr);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vxpath_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	       int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    int ok = 0;
    VirtualXPathCursorPtr cursor = (VirtualXPathCursorPtr) pCursor;
    if (idxStr)
	idxStr = idxStr;	/* unused arg warning suppression */
    cursor->eof = 1;
    if (idxNum == 1)
      {
	  int i;
	  cursor->keyOp1 = 0;
	  cursor->keyOp2 = 0;
	  for (i = 0; i < argc; i++)
	    {
		const char *pStr = idxStr + (i * 2);
		if (*pStr == 0)
		  {
		      /* retrieving the XPath expression param */
		      if (sqlite3_value_type (argv[i]) == SQLITE_TEXT)
			{
			    char *exp = (char *) sqlite3_value_text (argv[i]);
			    if (exp != NULL)
			      {
				  int len = strlen (exp);
				  cursor->xpathExpr = malloc (len + 1);
				  strcpy (cursor->xpathExpr, exp);
			      }
			    ok = 1;
			}
		  }
		else
		  {
		      /* evaluating a ROWID contraint */
		      pStr++;
		      if (cursor->keyOp1 == 0)
			{
			    cursor->keyOp1 = *pStr;
			    cursor->keyVal1 = sqlite3_value_int64 (argv[i]);
			}
		      else
			{
			    cursor->keyOp2 = *pStr;
			    cursor->keyVal2 = sqlite3_value_int64 (argv[i]);
			}
		  }
	    }
      }
    if (!ok)
      {
	  cursor->eof = 1;
	  return SQLITE_OK;
      }
/* attempting to set a reasonable ROWID */
    switch (cursor->keyOp1)
      {
      case SQLITE_INDEX_CONSTRAINT_EQ:
	  cursor->current_row = cursor->keyVal1;
	  break;
      case SQLITE_INDEX_CONSTRAINT_GT:
	  cursor->current_row = cursor->keyVal1 + 1;
	  break;
      case SQLITE_INDEX_CONSTRAINT_GE:
	  cursor->current_row = cursor->keyVal1;
	  break;
      };
    switch (cursor->keyOp2)
      {
      case SQLITE_INDEX_CONSTRAINT_EQ:
	  cursor->current_row = cursor->keyVal2;
	  break;
      case SQLITE_INDEX_CONSTRAINT_GT:
	  cursor->current_row = cursor->keyVal2 + 1;
	  break;
      case SQLITE_INDEX_CONSTRAINT_GE:
	  cursor->current_row = cursor->keyVal2;
	  break;
      };
    vxpath_read_row (cursor);
    return SQLITE_OK;
}

static int
vxpath_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching a next row from cursor */
    VirtualXPathCursorPtr cursor = (VirtualXPathCursorPtr) pCursor;
    if (cursor->xpathObj != NULL)
      {
	  /* we already have a current XPath node-set */
	  xmlNodeSetPtr nodes = cursor->xpathObj->nodesetval;
	  int num_nodes = (nodes) ? nodes->nodeNr : 0;
	  if ((cursor->xpathIdx + 1) < num_nodes)
	    {
		/* ok, consuming the current XPath node from the node-set */
		cursor->xpathIdx += 1;
		return SQLITE_OK;
	    }
      }
    (cursor->current_row)++;
    vxpath_read_row (cursor);
    return SQLITE_OK;
}

static int
vxpath_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualXPathCursorPtr cursor = (VirtualXPathCursorPtr) pCursor;
    return cursor->eof;
}

static int
vxpath_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	       int column)
{
/* fetching value for the Nth column */
    VirtualXPathCursorPtr cursor = (VirtualXPathCursorPtr) pCursor;
    xmlNodeSetPtr nodeset = cursor->xpathObj->nodesetval;
    xmlNodePtr node = nodeset->nodeTab[cursor->xpathIdx];
    xmlNodePtr parent = node->parent;
    char *xParent = NULL;
    char *xNode = NULL;
    char *xAttribute = NULL;
    char *xValue = NULL;
    char *prefix;

    if (node->type == XML_ELEMENT_NODE)
      {
	  if (parent != NULL)
	    {
		if (parent->ns != NULL && parent->name != NULL)
		  {
		      prefix = (char *) (parent->ns->prefix);
		      if (prefix == NULL)
			  prefix = "dflt";
		      xParent = sqlite3_mprintf ("%s:%s", prefix, parent->name);
		  }
		else if (parent->name != NULL)
		    xParent = sqlite3_mprintf ("%s", parent->name);
	    }
	  if (node->ns != NULL && node->name != NULL)
	    {
		prefix = (char *) (node->ns->prefix);
		if (prefix == NULL)
		    prefix = "dflt";
		xNode = sqlite3_mprintf ("%s:%s", prefix, node->name);
	    }
	  else if (node->name != NULL)
	      xNode = sqlite3_mprintf ("%s", node->name);
      }
    else if (node->type == XML_ATTRIBUTE_NODE)
      {
	  if (parent != NULL)
	    {
		xmlNodePtr granpa = parent->parent;
		if (granpa != NULL)
		  {
		      if (granpa->ns != NULL && granpa->name != NULL)
			{
			    prefix = (char *) (granpa->ns->prefix);
			    if (prefix == NULL)
				prefix = "dflt";
			    xParent =
				sqlite3_mprintf ("%s:%s", prefix, granpa->name);
			}
		      else if (granpa->name != NULL)
			  xParent = sqlite3_mprintf ("%s", granpa->name);
		  }
		if (parent->ns != NULL && parent->name != NULL)
		  {
		      prefix = (char *) (parent->ns->prefix);
		      if (prefix == NULL)
			  prefix = "dflt";
		      xNode = sqlite3_mprintf ("%s:%s", prefix, parent->name);
		  }
		else if (parent->name != NULL)
		    xNode = sqlite3_mprintf ("%s", parent->name);
	    }
	  if (node->ns != NULL && node->name != NULL)
	    {
		prefix = (char *) (node->ns->prefix);
		if (prefix == NULL)
		    prefix = "dflt";
		xAttribute = sqlite3_mprintf ("%s:%s", prefix, node->name);
	    }
	  else if (node->name != NULL)
	      xAttribute = sqlite3_mprintf ("%s", node->name);
	  if (node->children != NULL)
	    {
		if (node->children->content != NULL)
		    xValue = sqlite3_mprintf ("%s", node->children->content);
	    }
      }
    else if (node->type == XML_TEXT_NODE)
      {
	  if (parent != NULL)
	    {
		xmlNodePtr granpa = parent->parent;
		if (granpa != NULL)
		  {
		      if (granpa->ns != NULL && granpa->name != NULL)
			{
			    prefix = (char *) (granpa->ns->prefix);
			    if (prefix == NULL)
				prefix = "dflt";
			    xParent =
				sqlite3_mprintf ("%s:%s", prefix, granpa->name);
			}
		      else if (granpa->name != NULL)
			  xParent = sqlite3_mprintf ("%s", granpa->name);
		  }
		if (parent->ns != NULL && parent->name != NULL)
		  {
		      prefix = (char *) (parent->ns->prefix);
		      if (prefix == NULL)
			  prefix = "dflt";
		      xNode = sqlite3_mprintf ("%s:%s", prefix, parent->name);
		  }
		else if (parent->name != NULL)
		    xNode = sqlite3_mprintf ("%s", parent->name);
	    }
	  if (node->content != NULL)
	      xValue = sqlite3_mprintf ("%s", node->content);
      }

    if (column == 0)
	sqlite3_result_int64 (pContext, cursor->current_row);
    else if (column == 1)
	sqlite3_result_int (pContext, cursor->xpathIdx);
    else if (column == 2)
      {
	  if (!xParent)
	      sqlite3_result_null (pContext);
	  else
	      sqlite3_result_text (pContext, xParent,
				   strlen (xParent), SQLITE_TRANSIENT);
      }
    else if (column == 3)
      {
	  if (!xNode)
	      sqlite3_result_null (pContext);
	  else
	      sqlite3_result_text (pContext, xNode,
				   strlen (xNode), SQLITE_TRANSIENT);
      }
    else if (column == 4)
      {
	  if (!xAttribute)
	      sqlite3_result_null (pContext);
	  else
	      sqlite3_result_text (pContext, xAttribute,
				   strlen (xAttribute), SQLITE_TRANSIENT);
      }
    else if (column == 5)
      {
	  if (!xValue)
	      sqlite3_result_null (pContext);
	  else
	      sqlite3_result_text (pContext, xValue,
				   strlen (xValue), SQLITE_TRANSIENT);
      }
    else if (column == 6)
	sqlite3_result_text (pContext, cursor->xpathExpr,
			     strlen (cursor->xpathExpr), SQLITE_STATIC);
    else
	sqlite3_result_null (pContext);
    if (xParent)
	sqlite3_free (xParent);
    if (xNode)
	sqlite3_free (xNode);
    if (xAttribute)
	sqlite3_free (xAttribute);
    if (xValue)
	sqlite3_free (xValue);
    return SQLITE_OK;
}

static int
vxpath_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualXPathCursorPtr cursor = (VirtualXPathCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vxpath_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	       sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    if (pRowid || argc || argv || pVTab)
	pRowid = pRowid;	/* unused arg warning suppression */
/* read only datasource */
    return SQLITE_READONLY;
}

static int
vxpath_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vxpath_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vxpath_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vxpath_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    return SQLITE_OK;
}

static int
vxpath_rename (sqlite3_vtab * pVTab, const char *zNew)
{
/* BEGIN TRANSACTION */
    if (pVTab)
	pVTab = pVTab;		/* unused arg warning suppression */
    if (zNew)
	zNew = zNew;		/* unused arg warning suppression */
    return SQLITE_ERROR;
}

static int
spliteVirtualXPathInit (sqlite3 * db, void *p_cache)
{
    int rc = SQLITE_OK;
    my_xpath_module.iVersion = 1;
    my_xpath_module.xCreate = &vxpath_create;
    my_xpath_module.xConnect = &vxpath_connect;
    my_xpath_module.xBestIndex = &vxpath_best_index;
    my_xpath_module.xDisconnect = &vxpath_disconnect;
    my_xpath_module.xDestroy = &vxpath_destroy;
    my_xpath_module.xOpen = &vxpath_open;
    my_xpath_module.xClose = &vxpath_close;
    my_xpath_module.xFilter = &vxpath_filter;
    my_xpath_module.xNext = &vxpath_next;
    my_xpath_module.xEof = &vxpath_eof;
    my_xpath_module.xColumn = &vxpath_column;
    my_xpath_module.xRowid = &vxpath_rowid;
    my_xpath_module.xUpdate = &vxpath_update;
    my_xpath_module.xBegin = &vxpath_begin;
    my_xpath_module.xSync = &vxpath_sync;
    my_xpath_module.xCommit = &vxpath_commit;
    my_xpath_module.xRollback = &vxpath_rollback;
    my_xpath_module.xFindFunction = NULL;
    my_xpath_module.xRename = &vxpath_rename;
    sqlite3_create_module_v2 (db, "VirtualXPath", &my_xpath_module, p_cache, 0);
    return rc;
}

SPATIALITE_PRIVATE int
virtual_xpath_extension_init (void *xdb, const void *p_cache)
{
    sqlite3 *db = (sqlite3 *) xdb;
    return spliteVirtualXPathInit (db, (void *) p_cache);
}

#endif /* end LIBXML2: supporting XML documents */
