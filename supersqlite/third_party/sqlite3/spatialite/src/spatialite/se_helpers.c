/*

 se_helpers.c -- SLD/SE helper functions 

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

/*
 
CREDITS:

this module has been partly funded by:
Regione Toscana - Settore Sistema Informativo Territoriale ed Ambientale
(implementing XML support - ISO Metadata and SLD/SE Styles) 

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
#include <spatialite/debug.h>
#include <spatialite/gaiaaux.h>

#include <spatialite.h>
#include <spatialite_private.h>

#ifdef _WIN32
#define strcasecmp	_stricmp
#endif /* not WIN32 */


#ifdef ENABLE_LIBXML2		/* including LIBXML2 */

static int
check_external_graphic (sqlite3 * sqlite, const char *xlink_href)
{
/* checks if an ExternalGraphic Resource already exists */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int exists = 0;
    sql = "SELECT xlink_href FROM SE_external_graphics WHERE xlink_href = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("checkExternalGraphic: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, xlink_href, strlen (xlink_href), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      exists = 1;
      }
    sqlite3_finalize (stmt);
    return exists;
}

SPATIALITE_PRIVATE int
register_external_graphic (void *p_sqlite, const char *xlink_href,
			   const unsigned char *p_blob, int n_bytes,
			   const char *title, const char *abstract,
			   const char *file_name)
{
/* auxiliary function: inserts or updates an ExternalGraphic Resource */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int exists = 0;
    int extras = 0;
    int retval = 0;

/* checking if already exists */
    if (xlink_href == NULL)
	return 0;
    exists = check_external_graphic (sqlite, xlink_href);

    if (title != NULL && abstract != NULL && file_name != NULL)
	extras = 1;
    if (exists)
      {
	  /* update */
	  if (extras)
	    {
		/* full infos */
		sql = "UPDATE SE_external_graphics "
		    "SET resource = ?, title = ?, abstract = ?, file_name = ? "
		    "WHERE xlink_href = ?";
	    }
	  else
	    {
		/* limited basic infos */
		sql = "UPDATE SE_external_graphics "
		    "SET resource = ? WHERE xlink_href = ?";
	    }
      }
    else
      {
	  /* insert */
	  if (extras)
	    {
		/* full infos */
		sql = "INSERT INTO SE_external_graphics "
		    "(xlink_href, resource, title, abstract, file_name) "
		    "VALUES (?, ?, ?, ?, ?)";
	    }
	  else
	    {
		/* limited basic infos */
		sql = "INSERT INTO SE_external_graphics "
		    "(xlink_href, resource) VALUES (?, ?)";
	    }
      }
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("registerExternalGraphic: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    if (exists)
      {
	  /* update */
	  if (extras)
	    {
		/* full infos */
		sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
		sqlite3_bind_text (stmt, 2, title, strlen (title),
				   SQLITE_STATIC);
		sqlite3_bind_text (stmt, 3, abstract, strlen (abstract),
				   SQLITE_STATIC);
		sqlite3_bind_text (stmt, 4, file_name, strlen (file_name),
				   SQLITE_STATIC);
		sqlite3_bind_text (stmt, 5, xlink_href, strlen (xlink_href),
				   SQLITE_STATIC);
	    }
	  else
	    {
		/* limited basic infos */
		sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
		sqlite3_bind_text (stmt, 2, xlink_href, strlen (xlink_href),
				   SQLITE_STATIC);
	    }
      }
    else
      {
	  /* insert */
	  if (extras)
	    {
		/* full infos */
		sqlite3_bind_text (stmt, 1, xlink_href, strlen (xlink_href),
				   SQLITE_STATIC);
		sqlite3_bind_blob (stmt, 2, p_blob, n_bytes, SQLITE_STATIC);
		sqlite3_bind_text (stmt, 3, title, strlen (title),
				   SQLITE_STATIC);
		sqlite3_bind_text (stmt, 4, abstract, strlen (abstract),
				   SQLITE_STATIC);
		sqlite3_bind_text (stmt, 5, file_name, strlen (file_name),
				   SQLITE_STATIC);
	    }
	  else
	    {
		/* limited basic infos */
		sqlite3_bind_text (stmt, 1, xlink_href, strlen (xlink_href),
				   SQLITE_STATIC);
		sqlite3_bind_blob (stmt, 2, p_blob, n_bytes, SQLITE_STATIC);
	    }
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("registerExternalGraphic() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
unregister_external_graphic (void *p_sqlite, const char *xlink_href)
{
/* auxiliary function: deletes an ExternalGraphic Resource */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int exists = 0;
    int retval = 0;

/* checking if already exists */
    if (xlink_href == NULL)
	return 0;
    exists = check_external_graphic (sqlite, xlink_href);
    if (!exists)
	return 0;

    sql = "DELETE FROM SE_external_graphics WHERE xlink_href = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterExternalGraphic: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, xlink_href, strlen (xlink_href), SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterExternalGraphic() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

static int
vector_style_causes_duplicate_name (sqlite3 * sqlite, sqlite3_int64 id,
				    const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: checks for an eventual duplicate name */
    int count = 0;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    sql = "SELECT Count(*) FROM SE_vector_styles "
	"WHERE Lower(style_name) = Lower(XB_GetName(?)) AND style_id <> ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("VectorStyle duplicate Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count = sqlite3_column_int (stmt, 0);
      }
    sqlite3_finalize (stmt);
    if (count != 0)
	return 1;
    return 0;
}

SPATIALITE_PRIVATE int
register_vector_style (void *p_sqlite, const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: inserts a Vector Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (p_blob != NULL && n_bytes > 0)
      {
	  /* attempting to insert the Vector Style */
	  if (vector_style_causes_duplicate_name (sqlite, -1, p_blob, n_bytes))
	      return 0;
	  sql = "INSERT INTO SE_vector_styles "
	      "(style_id, style) VALUES (NULL, ?)";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("registerVectorStyle: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		return 0;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("registerVectorStyle() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  sqlite3_finalize (stmt);
	  return 1;
      }
    else
	return 0;
}

static int
check_vector_style_by_id (sqlite3 * sqlite, int style_id)
{
/* checks if a Vector Style do actually exists - by ID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT style_id FROM SE_vector_styles " "WHERE style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Style by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int (stmt, 1, style_id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 1)
	return 1;
    return 0;
  stop:
    return 0;
}

static int
check_vector_style_by_name (sqlite3 * sqlite, const char *style_name,
			    sqlite3_int64 * id)
{
/* checks if a Vector Style do actually exists - by name */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    sqlite3_int64 xid;

    sql = "SELECT style_id FROM SE_vector_styles "
	"WHERE Lower(style_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Style by Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, style_name, strlen (style_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1)
      {
	  *id = xid;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
check_vector_style_refs_by_id (sqlite3 * sqlite, int style_id, int *has_refs)
{
/* checks if a Vector Style do actually exists - by ID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    int ref_count = 0;

    sql = "SELECT s.style_id, l.style_id FROM SE_vector_styles AS s "
	"LEFT JOIN SE_vector_styled_layers AS l ON (l.style_id = s.style_id) "
	"WHERE s.style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Style Refs by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int (stmt, 1, style_id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		count++;
		if (sqlite3_column_type (stmt, 1) == SQLITE_INTEGER)
		    ref_count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count >= 1)
      {
	  if (ref_count > 0)
	      *has_refs = 1;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
check_vector_style_refs_by_name (sqlite3 * sqlite, const char *style_name,
				 sqlite3_int64 * id, int *has_refs)
{
/* checks if a Vector Style do actually exists - by name */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    int ref_count = 0;
    sqlite3_int64 xid;

    sql = "SELECT style_id FROM SE_vector_styles "
	"WHERE Lower(style_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Style Refs by Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, style_name, strlen (style_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count != 1)
	return 0;
    *id = xid;
    sql = "SELECT s.style_id, l.style_id FROM SE_vector_styles AS s "
	"LEFT JOIN SE_vector_styled_layers AS l ON (l.style_id = s.style_id) "
	"WHERE s.style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Style Refs by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int64 (stmt, 1, *id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 1) == SQLITE_INTEGER)
		    ref_count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (ref_count > 0)
	*has_refs = 1;
    return 1;
    return 0;
  stop:
    return 0;
}

static int
do_insert_vector_style_layer (sqlite3 * sqlite, const char *coverage_name,
			      sqlite3_int64 id)
{
/* auxiliary function: really inserting a Vector Styled Layer */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "INSERT INTO SE_vector_styled_layers "
	"(coverage_name, style_id) VALUES (?, ?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("registerVectorStyledLayer: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("registerVectorStyledLayer() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

static int
do_delete_vector_style_refs (sqlite3 * sqlite, sqlite3_int64 id)
{
/* auxiliary function: deleting all Vector Style references */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "DELETE FROM SE_vector_styled_layers WHERE style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterVectorStyle: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int64 (stmt, 1, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterVectorStyle() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

static int
do_delete_vector_style (sqlite3 * sqlite, sqlite3_int64 id)
{
/* auxiliary function: really deleting a Vector Style */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "DELETE FROM SE_vector_styles WHERE style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterVectorStyle: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int64 (stmt, 1, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterVectorStyle() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
unregister_vector_style (void *p_sqlite, int style_id,
			 const char *style_name, int remove_all)
{
/* auxiliary function: deletes a Vector Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;
    int has_refs = 0;

    if (style_id >= 0)
      {
	  /* checking if the Vector Style do actually exists */
	  if (check_vector_style_refs_by_id (sqlite, style_id, &has_refs))
	      id = style_id;
	  else
	      return 0;
	  if (has_refs)
	    {
		if (!remove_all)
		    return 0;
		/* deleting all references */
		if (!do_delete_vector_style_refs (sqlite, id))
		    return 0;
	    }
	  /* deleting the Vector Style */
	  return do_delete_vector_style (sqlite, id);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Vector Style do actually exists */
	  if (!check_vector_style_refs_by_name
	      (sqlite, style_name, &id, &has_refs))
	      return 0;
	  if (has_refs)
	    {
		if (!remove_all)
		    return 0;
		/* deleting all references */
		if (!do_delete_vector_style_refs (sqlite, id))
		    return 0;
	    }
	  /* deleting the Vector Style */
	  return do_delete_vector_style (sqlite, id);
      }
    else
	return 0;
}

static int
do_reload_vector_style (sqlite3 * sqlite, sqlite3_int64 id,
			const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: reloads a Vector Style definition */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (p_blob != NULL && n_bytes > 0)
      {
	  /* attempting to update the Vector Style */
	  sql = "UPDATE SE_vector_styles SET style = ? " "WHERE style_id = ?";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("reloadVectorStyle: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		return 0;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
	  sqlite3_bind_int64 (stmt, 2, id);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("reloadVectorStyle() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  sqlite3_finalize (stmt);
	  return 1;
      }
    else
	return 0;
}

SPATIALITE_PRIVATE int
reload_vector_style (void *p_sqlite, int style_id,
		     const char *style_name,
		     const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: reloads a Vector Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;

    if (style_id >= 0)
      {
	  /* checking if the Vector Style do actually exists */
	  if (check_vector_style_by_id (sqlite, style_id))
	      id = style_id;
	  else
	      return 0;
	  /* reloading the Vector Style */
	  if (vector_style_causes_duplicate_name (sqlite, id, p_blob, n_bytes))
	      return 0;
	  return do_reload_vector_style (sqlite, id, p_blob, n_bytes);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Vector Style do actually exists */
	  if (!check_vector_style_by_name (sqlite, style_name, &id))
	      return 0;
	  /* reloading the Vector Style */
	  if (vector_style_causes_duplicate_name (sqlite, id, p_blob, n_bytes))
	      return 0;
	  return do_reload_vector_style (sqlite, id, p_blob, n_bytes);
      }
    else
	return 0;
}

SPATIALITE_PRIVATE int
register_vector_styled_layer_ex (void *p_sqlite, const char *coverage_name,
				 int style_id, const char *style_name)
{
/* auxiliary function: inserts a Vector Styled Layer definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;

    if (coverage_name == NULL)
	return 0;

    if (style_id >= 0)
      {
	  /* checking if the Vector Style do actually exists */
	  if (check_vector_style_by_id (sqlite, style_id))
	      id = style_id;
	  else
	      return 0;
	  /* inserting the Vector Styled Layer */
	  return do_insert_vector_style_layer (sqlite, coverage_name, id);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Vector Style do actually exists */
	  if (!check_vector_style_by_name (sqlite, style_name, &id))
	      return 0;
	  /* inserting the Vector Styled Layer */
	  return do_insert_vector_style_layer (sqlite, coverage_name, id);
      }
    else
	return 0;
}

SPATIALITE_PRIVATE int
register_vector_styled_layer (void *p_sqlite, const char *f_table_name,
			      const char *f_geometry_column, int style_id,
			      const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: inserts a Vector Styled Layer definition - DEPRECATED */
    if (p_blob != NULL && n_bytes <= 0 && f_geometry_column != NULL)
      {
	  /* silencing compiler complaints */
	  p_blob = NULL;
	  n_bytes = 0;
	  f_geometry_column = NULL;
      }
    return register_vector_styled_layer_ex (p_sqlite, f_table_name, style_id,
					    NULL);
}

static int
check_vector_styled_layer_by_id (sqlite3 * sqlite, const char *coverage_name,
				 int style_id)
{
/* checks if a Vector Styled Layer do actually exists - by ID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT style_id FROM SE_vector_styled_layers "
	"WHERE Lower(coverage_name) = Lower(?) AND style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Styled Layer by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, style_id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 1)
	return 1;
    return 0;
  stop:
    return 0;
}

static int
check_vector_styled_layer_by_name (sqlite3 * sqlite, const char *coverage_name,
				   const char *style_name, sqlite3_int64 * id)
{
/* checks if a Vector Styled Layer do actually exists - by name */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    sqlite3_int64 xid;

    sql = "SELECT l.style_id FROM SE_vector_styled_layers AS l "
	"JOIN SE_vector_styles AS s ON (l.style_id = s.style_id) "
	"WHERE Lower(l.coverage_name) = Lower(?) "
	"AND Lower(s.style_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Styled Layer by Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, style_name, strlen (style_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1)
      {
	  *id = xid;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
do_delete_vector_style_layer (sqlite3 * sqlite, const char *coverage_name,
			      sqlite3_int64 id)
{
/* auxiliary function: really deleting a Vector Styled Layer */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "DELETE FROM SE_vector_styled_layers "
	"WHERE Lower(coverage_name) = Lower(?) AND style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterVectorStyledLayer: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterVectorStyledLayer() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
unregister_vector_styled_layer (void *p_sqlite, const char *coverage_name,
				int style_id, const char *style_name)
{
/* auxiliary function: removes a Vector Styled Layer definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;

    if (coverage_name == NULL)
	return 0;

    if (style_id >= 0)
      {
	  /* checking if the Vector Styled Layer do actually exists */
	  if (check_vector_styled_layer_by_id (sqlite, coverage_name, style_id))
	      id = style_id;
	  else
	      return 0;
	  /* removing the Vector Styled Layer */
	  return do_delete_vector_style_layer (sqlite, coverage_name, id);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Vector Styled Layer do actually exists */
	  if (!check_vector_styled_layer_by_name
	      (sqlite, coverage_name, style_name, &id))
	      return 0;
	  /* removing the Vector Styled Layer */
	  return do_delete_vector_style_layer (sqlite, coverage_name, id);
      }
    else
	return 0;
}

static int
raster_style_causes_duplicate_name (sqlite3 * sqlite, sqlite3_int64 id,
				    const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: checks for an eventual duplicate name */
    int count = 0;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    sql = "SELECT Count(*) FROM SE_raster_styles "
	"WHERE Lower(style_name) = Lower(XB_GetName(?)) AND style_id <> ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("RasterStyle duplicate Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count = sqlite3_column_int (stmt, 0);
      }
    sqlite3_finalize (stmt);
    if (count != 0)
	return 1;
    return 0;
}

SPATIALITE_PRIVATE int
register_raster_style (void *p_sqlite, const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: inserts a Raster Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (p_blob != NULL && n_bytes > 0)
      {
	  /* attempting to insert the Raster Style */
	  if (raster_style_causes_duplicate_name (sqlite, -1, p_blob, n_bytes))
	      return 0;
	  sql = "INSERT INTO SE_raster_styles "
	      "(style_id, style) VALUES (NULL, ?)";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("registerRasterStyle: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		return 0;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("registerRasterStyle() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  sqlite3_finalize (stmt);
	  return 1;
      }
    else
	return 0;
}

static int
do_delete_raster_style_refs (sqlite3 * sqlite, sqlite3_int64 id)
{
/* auxiliary function: deleting all Raster Style references */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "DELETE FROM SE_raster_styled_layers WHERE style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterRasterStyle: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int64 (stmt, 1, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterRasterStyle() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

static int
check_raster_style_refs_by_id (sqlite3 * sqlite, int style_id, int *has_refs)
{
/* checks if a Raster Style do actually exists - by ID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    int ref_count = 0;

    sql = "SELECT s.style_id, l.style_id FROM SE_raster_styles AS s "
	"LEFT JOIN SE_raster_styled_layers AS l ON (l.style_id = s.style_id) "
	"WHERE s.style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Style Refs by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int (stmt, 1, style_id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		count++;
		if (sqlite3_column_type (stmt, 1) == SQLITE_INTEGER)
		    ref_count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count >= 1)
      {
	  if (ref_count > 0)
	      *has_refs = 1;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
check_raster_style_refs_by_name (sqlite3 * sqlite, const char *style_name,
				 sqlite3_int64 * id, int *has_refs)
{
/* checks if a Raster Style do actually exists - by name */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    int ref_count = 0;
    sqlite3_int64 xid;

    sql = "SELECT style_id FROM SE_raster_styles "
	"WHERE Lower(style_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Style Refs by Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, style_name, strlen (style_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count != 1)
	return 0;
    *id = xid;
    sql = "SELECT s.style_id, l.style_id FROM SE_raster_styles AS s "
	"LEFT JOIN SE_raster_styled_layers AS l ON (l.style_id = s.style_id) "
	"WHERE s.style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Style Refs by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int64 (stmt, 1, *id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 1) == SQLITE_INTEGER)
		    ref_count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (ref_count > 0)
	*has_refs = 1;
    return 1;
    return 0;
  stop:
    return 0;
}

static int
do_delete_raster_style (sqlite3 * sqlite, sqlite3_int64 id)
{
/* auxiliary function: really deleting a Raster Style */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "DELETE FROM SE_raster_styles WHERE style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterRasterStyle: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int64 (stmt, 1, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterRasterStyle() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
unregister_raster_style (void *p_sqlite, int style_id,
			 const char *style_name, int remove_all)
{
/* auxiliary function: deletes a Raster Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;
    int has_refs = 0;

    if (style_id >= 0)
      {
	  /* checking if the Raster Style do actually exists */
	  if (check_raster_style_refs_by_id (sqlite, style_id, &has_refs))
	      id = style_id;
	  else
	      return 0;
	  if (has_refs)
	    {
		if (!remove_all)
		    return 0;
		/* deleting all references */
		if (!do_delete_raster_style_refs (sqlite, id))
		    return 0;
	    }
	  /* deleting the Raster Style */
	  return do_delete_raster_style (sqlite, id);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Raster Style do actually exists */
	  if (!check_raster_style_refs_by_name
	      (sqlite, style_name, &id, &has_refs))
	      return 0;
	  if (has_refs)
	    {
		if (!remove_all)
		    return 0;
		/* deleting all references */
		if (!do_delete_raster_style_refs (sqlite, id))
		    return 0;
	    }
	  /* deleting the Raster Style */
	  return do_delete_raster_style (sqlite, id);
      }
    else
	return 0;
}

static int
check_raster_style_by_id (sqlite3 * sqlite, int style_id)
{
/* checks if a Raster Style do actually exists - by ID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT style_id FROM SE_raster_styles " "WHERE style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Style by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int (stmt, 1, style_id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 1)
	return 1;
    return 0;
  stop:
    return 0;
}

static int
check_raster_style_by_name (sqlite3 * sqlite, const char *style_name,
			    sqlite3_int64 * id)
{
/* checks if a Raster Style do actually exists - by name */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    sqlite3_int64 xid;

    sql = "SELECT style_id FROM SE_raster_styles "
	"WHERE Lower(style_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Style by Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, style_name, strlen (style_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1)
      {
	  *id = xid;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
do_reload_raster_style (sqlite3 * sqlite, sqlite3_int64 id,
			const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: reloads a Raster Style definition */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (p_blob != NULL && n_bytes > 0)
      {
	  /* attempting to update the Raster Style */
	  sql = "UPDATE SE_raster_styles SET style = ? " "WHERE style_id = ?";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("reloadRasterStyle: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		return 0;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
	  sqlite3_bind_int64 (stmt, 2, id);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("reloadRasterStyle() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  sqlite3_finalize (stmt);
	  return 1;
      }
    else
	return 0;
}

SPATIALITE_PRIVATE int
reload_raster_style (void *p_sqlite, int style_id,
		     const char *style_name,
		     const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: reloads a Raster Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;

    if (style_id >= 0)
      {
	  /* checking if the Raster Style do actually exists */
	  if (check_raster_style_by_id (sqlite, style_id))
	      id = style_id;
	  else
	      return 0;
	  /* reloading the Raster Style */
	  if (raster_style_causes_duplicate_name (sqlite, id, p_blob, n_bytes))
	      return 0;
	  return do_reload_raster_style (sqlite, id, p_blob, n_bytes);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Raster Style do actually exists */
	  if (!check_raster_style_by_name (sqlite, style_name, &id))
	      return 0;
	  /* reloading the Raster Style */
	  if (raster_style_causes_duplicate_name (sqlite, id, p_blob, n_bytes))
	      return 0;
	  return do_reload_raster_style (sqlite, id, p_blob, n_bytes);
      }
    else
	return 0;
}

static int
do_insert_raster_style_layer (sqlite3 * sqlite, const char *coverage_name,
			      sqlite3_int64 id)
{
/* auxiliary function: really inserting a Raster Styled Layer */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "INSERT INTO SE_raster_styled_layers "
	"(coverage_name, style_id) VALUES (?, ?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("registerRasterStyledLayer: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("registerRasterStyledLayer() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
register_raster_styled_layer_ex (void *p_sqlite, const char *coverage_name,
				 int style_id, const char *style_name)
{
/* auxiliary function: inserts a Raster Styled Layer definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;

    if (coverage_name == NULL)
	return 0;

    if (style_id >= 0)
      {
	  /* checking if the Raster Style do actually exists */
	  if (check_raster_style_by_id (sqlite, style_id))
	      id = style_id;
	  else
	      return 0;
	  /* inserting the Raster Styled Layer */
	  return do_insert_raster_style_layer (sqlite, coverage_name, id);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Raster Style do actually exists */
	  if (!check_raster_style_by_name (sqlite, style_name, &id))
	      return 0;
	  /* inserting the Raster Styled Layer */
	  return do_insert_raster_style_layer (sqlite, coverage_name, id);
      }
    else
	return 0;
}

SPATIALITE_PRIVATE int
register_raster_styled_layer (void *p_sqlite, const char *coverage_name,
			      int style_id, const unsigned char *p_blob,
			      int n_bytes)
{
/* auxiliary function: inserts a Raster Styled Layer definition - DEPRECATED */
    if (p_blob != NULL && n_bytes <= 0)
      {
	  /* silencing compiler complaints */
	  p_blob = NULL;
	  n_bytes = 0;
      }
    return register_raster_styled_layer_ex (p_sqlite, coverage_name, style_id,
					    NULL);
}

static int
check_raster_styled_layer_by_id (sqlite3 * sqlite, const char *coverage_name,
				 int style_id)
{
/* checks if a Raster Styled Layer do actually exists - by ID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT style_id FROM SE_raster_styled_layers "
	"WHERE Lower(coverage_name) = Lower(?) AND style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Styled Layer by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, style_id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 1)
	return 1;
    return 0;
  stop:
    return 0;
}

static int
check_raster_styled_layer_by_name (sqlite3 * sqlite, const char *coverage_name,
				   const char *style_name, sqlite3_int64 * id)
{
/* checks if a Raster Styled Layer do actually exists - by name */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    sqlite3_int64 xid;

    sql = "SELECT l.style_id FROM SE_raster_styled_layers AS l "
	"JOIN SE_raster_styles AS s ON (l.style_id = s.style_id) "
	"WHERE Lower(l.coverage_name) = Lower(?) AND "
	"Lower(s.style_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Styled Layer by Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, style_name, strlen (style_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1)
      {
	  *id = xid;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
do_delete_raster_style_layer (sqlite3 * sqlite, const char *coverage_name,
			      sqlite3_int64 id)
{
/* auxiliary function: really deleting a Raster Styled Layer */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "DELETE FROM SE_raster_styled_layers "
	"WHERE Lower(coverage_name) = Lower(?) AND " "style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterRasterStyledLayer: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterRasterStyledLayer() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
unregister_raster_styled_layer (void *p_sqlite, const char *coverage_name,
				int style_id, const char *style_name)
{
/* auxiliary function: removes a Raster Styled Layer definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;

    if (coverage_name == NULL)
	return 0;

    if (style_id >= 0)
      {
	  /* checking if the Raster Styled Layer do actually exists */
	  if (check_raster_styled_layer_by_id (sqlite, coverage_name, style_id))
	      id = style_id;
	  else
	      return 0;
	  /* removing the Raster Styled Layer */
	  return do_delete_raster_style_layer (sqlite, coverage_name, id);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Raster Styled Layer do actually exists */
	  if (!check_raster_styled_layer_by_name
	      (sqlite, coverage_name, style_name, &id))
	      return 0;
	  /* removing the Raster Styled Layer */
	  return do_delete_raster_style_layer (sqlite, coverage_name, id);
      }
    else
	return 0;
}

static int
check_styled_group (sqlite3 * sqlite, const char *group_name)
{
/* checking if the Group already exists */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int exists = 0;

    sql = "SELECT group_name FROM SE_styled_groups "
	"WHERE group_name = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("checkStyledGroup: \"%s\"\n", sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      exists = 1;
      }
    sqlite3_finalize (stmt);
    return exists;
}

static int
do_insert_styled_group (sqlite3 * sqlite, const char *group_name,
			const char *title, const char *abstract)
{
/* inserting a Styled Group */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;

    if (title != NULL && abstract != NULL)
	sql =
	    "INSERT INTO SE_styled_groups (group_name, title, abstract) VALUES (?, ?, ?)";
    else
	sql = "INSERT INTO SE_styled_groups (group_name) VALUES (?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("insertStyledGroup: \"%s\"\n", sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    if (title != NULL && abstract != NULL)
      {
	  sqlite3_bind_text (stmt, 2, title, strlen (title), SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 3, abstract, strlen (abstract),
			     SQLITE_STATIC);
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("insertStyledGroup() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
}

static int
get_next_paint_order (sqlite3 * sqlite, const char *group_name)
{
/* retrieving the next available Paint Order for a Styled Group */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int paint_order = 0;

    sql = "SELECT Max(paint_order) FROM SE_styled_group_refs "
	"WHERE group_name = Lower(?) ";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("nextPaintOrder: \"%s\"\n", sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_INTEGER)
		    paint_order = sqlite3_column_int (stmt, 0) + 1;
	    }
      }
    sqlite3_finalize (stmt);
    return paint_order;
}

static int
get_next_paint_order_by_item (sqlite3 * sqlite, int item_id)
{
/* retrieving the next available Paint Order for a Styled Group - BY ITEM ID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int paint_order = 0;

    sql = "SELECT Max(r.paint_order) FROM SE_styled_group_refs AS x "
	"JOIN SE_styled_groups AS g ON (x.group_name = g.group_name) "
	"JOIN SE_styled_group_refs AS r ON (r.group_name = g.group_name) "
	"WHERE x.id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("nextPaintOrderByItem: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int (stmt, 1, item_id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_INTEGER)
		    paint_order = sqlite3_column_int (stmt, 0) + 1;
	    }
      }
    sqlite3_finalize (stmt);
    return paint_order;
}

SPATIALITE_PRIVATE int
register_styled_group_ex (void *p_sqlite, const char *group_name,
			  const char *vector_coverage_name,
			  const char *raster_coverage_name)
{
/* auxiliary function: inserts a Styled Group Item */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int exists_group = 0;
    int retval = 0;
    int paint_order;

    if (vector_coverage_name == NULL && raster_coverage_name == NULL)
	return 0;
    if (vector_coverage_name != NULL && raster_coverage_name != NULL)
	return 0;

    /* checking if the Raster Styled Layer do actually exists */
    exists_group = check_styled_group (sqlite, group_name);

    if (!exists_group)
      {
	  /* insert group */
	  retval = do_insert_styled_group (sqlite, group_name, NULL, NULL);
	  if (retval == 0)
	      goto stop;
	  retval = 0;
      }

    /* assigning the next paint_order value */
    paint_order = get_next_paint_order (sqlite, group_name);

    /* insert */
    if (vector_coverage_name != NULL)
      {
	  /* vector styled layer */
	  sql = "INSERT INTO SE_styled_group_refs "
	      "(id, group_name, vector_coverage_name, paint_order) "
	      "VALUES (NULL, ?, ?, ?)";
      }
    else
      {
	  /* raster styled layer */
	  sql = "INSERT INTO SE_styled_group_refs "
	      "(id, group_name, raster_coverage_name, paint_order) "
	      "VALUES (NULL, ?, ?, ?)";
      }
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("registerStyledGroupsRefs: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    /* insert */
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    if (vector_coverage_name != NULL)
      {
	  /* vector styled layer */
	  sqlite3_bind_text (stmt, 2, vector_coverage_name,
			     strlen (vector_coverage_name), SQLITE_STATIC);
	  sqlite3_bind_int (stmt, 3, paint_order);
      }
    else
      {
	  /* raster styled layer */
	  sqlite3_bind_text (stmt, 2, raster_coverage_name,
			     strlen (raster_coverage_name), SQLITE_STATIC);
	  sqlite3_bind_int (stmt, 3, paint_order);
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("registerStyledGroupsRefs() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
register_styled_group (void *p_sqlite, const char *group_name,
		       const char *f_table_name,
		       const char *f_geometry_column,
		       const char *coverage_name, int paint_order)
{
/* auxiliary function: inserts a Styled Group Item - DEPRECATED */
    if (paint_order < 0 || f_geometry_column != NULL)
      {
	  f_geometry_column = NULL;
	  paint_order = -1;	/* silencing compiler complaints */
      }
    return register_styled_group_ex (p_sqlite, group_name, f_table_name,
				     coverage_name);
}

SPATIALITE_PRIVATE int
styled_group_set_infos (void *p_sqlite, const char *group_name,
			const char *title, const char *abstract)
{
/* auxiliary function: inserts or updates the Styled Group descriptive infos */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int exists = 0;
    int retval = 0;

    if (group_name == NULL)
	return 0;

    /* checking if the Raster Styled Layer do actually exists */
    exists = check_styled_group (sqlite, group_name);

    if (!exists)
      {
	  /* insert group */
	  retval = do_insert_styled_group (sqlite, group_name, title, abstract);
      }
    else
      {
	  /* update group */
	  sql =
	      "UPDATE SE_styled_groups SET title = ?, abstract = ? WHERE group_name = ?";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("styledGroupSetInfos: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		goto stop;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  if (title == NULL)
	      sqlite3_bind_null (stmt, 1);
	  else
	      sqlite3_bind_text (stmt, 1, title, strlen (title), SQLITE_STATIC);
	  if (abstract == NULL)
	      sqlite3_bind_null (stmt, 2);
	  else
	      sqlite3_bind_text (stmt, 2, abstract, strlen (abstract),
				 SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 3, group_name, strlen (group_name),
			     SQLITE_STATIC);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      retval = 1;
	  else
	      spatialite_e ("styledGroupSetInfos() error: \"%s\"\n",
			    sqlite3_errmsg (sqlite));
	  sqlite3_finalize (stmt);
      }
    return retval;
  stop:
    return 0;
}

static int
do_delete_styled_group (sqlite3 * sqlite, const char *group_name)
{
/* completely removing a Styled Group */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;

/* deleting Group Styles */
    sql =
	"DELETE FROM SE_styled_group_styles WHERE Lower(group_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("deleteStyledGroup: \"%s\"\n", sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("deleteStyledGroup() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    if (!retval)
	return 0;

/* deleting Group Items */
    retval = 0;
    sql = "DELETE FROM SE_styled_group_refs WHERE Lower(group_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("deleteStyledGroup: \"%s\"\n", sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("deleteStyledGroup() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    if (!retval)
	return 0;

/* deleting the Styled Group itself */
    retval = 0;
    sql = "DELETE FROM SE_styled_groups WHERE Lower(group_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("deleteStyledGroup: \"%s\"\n", sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("deleteStyledGroup() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
}

SPATIALITE_PRIVATE int
unregister_styled_group (void *p_sqlite, const char *group_name)
{
/* auxiliary function: completely removes a Styled Group definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;

    if (group_name == NULL)
	return 0;

    /* checking if the Raster Styled Layer do actually exists */
    if (!check_styled_group (sqlite, group_name))
	return 0;
    /* removing the Styled Group */
    return do_delete_styled_group (sqlite, group_name);
}

static int
check_styled_group_layer_by_id (sqlite3 * sqlite, int id)
{
/* checks if a Group Layer Item exists */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int exists = 0;

    sql = "SELECT id FROM SE_styled_group_refs " "WHERE id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("checkStyledGroupItem: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int (stmt, 1, id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      exists = 1;
      }
    sqlite3_finalize (stmt);
    return exists;
}

static int
check_styled_group_raster (sqlite3 * sqlite, const char *group_name,
			   const char *coverage_name, sqlite3_int64 * id)
{
/* checks if a Styled Group Layer (Raster) do actually exists */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    sqlite3_int64 xid;

    sql = "SELECT id FROM SE_styled_group_refs WHERE "
	"Lower(group_name) = Lower(?) AND Lower(raster_coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("checkStyledGroupRasterItem: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1)
      {
	  *id = xid;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
check_styled_group_vector (sqlite3 * sqlite, const char *group_name,
			   const char *coverage_name, sqlite3_int64 * id)
{
/* checks if a Styled Group Layer (Vector) do actually exists */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    sqlite3_int64 xid;

    sql = "SELECT id FROM SE_styled_group_refs WHERE "
	"Lower(group_name) = Lower(?) AND Lower(vector_coverage_name) = Lower(?) ";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("checkStyledGroupVectorItem: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1)
      {
	  *id = xid;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
do_update_styled_group_layer_paint_order (sqlite3 * sqlite, sqlite3_int64 id,
					  int paint_order)
{
/* auxiliary function: really updating a Group Styled Layer Paint Order */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "UPDATE SE_styled_group_refs SET paint_order = ? " "WHERE id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updatePaintOrder: \"%s\"\n", sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int (stmt, 1, paint_order);
    sqlite3_bind_int64 (stmt, 2, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("updatePaintOrder error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
set_styled_group_layer_paint_order (void *p_sqlite, int item_id,
				    const char *group_name,
				    const char *vector_coverage_name,
				    const char *raster_coverage_name,
				    int paint_order)
{
/* auxiliary function: set the Paint Order for a Layer within a Styled Group */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;
    int pos = paint_order;

    if (vector_coverage_name != NULL && raster_coverage_name != NULL)
	return 0;

    if (item_id >= 0)
      {
	  /* checking if the Layer Item do actually exists */
	  if (check_styled_group_layer_by_id (sqlite, item_id))
	      id = item_id;
	  else
	      return 0;
	  if (pos < 0)
	      pos = get_next_paint_order_by_item (sqlite, item_id);
	  /* updating the Styled Group Layer Paint Order */
	  return do_update_styled_group_layer_paint_order (sqlite, id, pos);
      }
    else if (group_name != NULL && raster_coverage_name != NULL)
      {
	  /* checking if a Raster Layer Item do actually exists */
	  if (!check_styled_group_raster
	      (sqlite, group_name, raster_coverage_name, &id))
	      return 0;
	  if (pos < 0)
	      pos = get_next_paint_order (sqlite, group_name);
	  /* updating the Styled Group Layer Paint Order */
	  return do_update_styled_group_layer_paint_order (sqlite, id, pos);
      }
    else if (group_name != NULL && vector_coverage_name != NULL)
      {
	  /* checking if a Vector Layer Item do actually exists */
	  if (!check_styled_group_vector
	      (sqlite, group_name, vector_coverage_name, &id))
	      return 0;
	  if (pos < 0)
	      pos = get_next_paint_order (sqlite, group_name);
	  /* updating the Styled Group Layer Paint Order */
	  return do_update_styled_group_layer_paint_order (sqlite, id, pos);
      }
    else
	return 0;
}

static int
do_delete_styled_group_layer (sqlite3 * sqlite, sqlite3_int64 id)
{
/* completely removing a Styled Group */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;

    sql = "DELETE FROM SE_styled_group_refs WHERE id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("deleteStyledGroupLayer: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int64 (stmt, 1, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("deleteStyledGroupLayer() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
}

SPATIALITE_PRIVATE int
unregister_styled_group_layer (void *p_sqlite, int item_id,
			       const char *group_name,
			       const char *vector_coverage_name,
			       const char *raster_coverage_name)
{
/* auxiliary function: removing a Layer form within a Styled Group */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;

    if (item_id >= 0)
      {
	  /* checking if the Layer Item do actually exists */
	  if (check_styled_group_layer_by_id (sqlite, item_id))
	      id = item_id;
	  else
	      return 0;
	  /* removing the Styled Group Layer */
	  return do_delete_styled_group_layer (sqlite, id);
      }
    else if (group_name != NULL && raster_coverage_name != NULL)
      {
	  /* checking if a Raster Layer Item do actually exists */
	  if (!check_styled_group_raster
	      (sqlite, group_name, raster_coverage_name, &id))
	      return 0;
	  /* removing the Styled Group Layer */
	  return do_delete_styled_group_layer (sqlite, id);
      }
    else if (group_name != NULL && vector_coverage_name != NULL)
      {
	  /* checking if a Vector Layer Item do actually exists */
	  if (!check_styled_group_vector
	      (sqlite, group_name, vector_coverage_name, &id))
	      return 0;
	  /* removing the Styled Group Layer */
	  return do_delete_styled_group_layer (sqlite, id);
      }
    else
	return 0;
}

static int
group_style_causes_duplicate_name (sqlite3 * sqlite, sqlite3_int64 id,
				   const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: checks for an eventual duplicate name */
    int count = 0;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    sql = "SELECT Count(*) FROM SE_group_styles "
	"WHERE Lower(style_name) = Lower(XB_GetName(?)) AND style_id <> ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("GroupStyle duplicate Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count = sqlite3_column_int (stmt, 0);
      }
    sqlite3_finalize (stmt);
    if (count != 0)
	return 1;
    return 0;
}

SPATIALITE_PRIVATE int
register_group_style_ex (void *p_sqlite, const unsigned char *p_blob,
			 int n_bytes)
{
/* auxiliary function: inserts a Group Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (p_blob != NULL && n_bytes > 0)
      {
	  /* attempting to insert the Group Style */
	  if (group_style_causes_duplicate_name (sqlite, -1, p_blob, n_bytes))
	      return 0;
	  sql = "INSERT INTO SE_group_styles "
	      "(style_id, style) VALUES (NULL, ?)";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("registerGroupStyle: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		return 0;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("registerGroupStyle() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  sqlite3_finalize (stmt);
	  return 1;
      }
    else
	return 0;
}

SPATIALITE_PRIVATE int
register_group_style (void *p_sqlite, const char *group_name, int style_id,
		      const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: inserts a Group Style - DEPRECATED */
    if (group_name == NULL || style_id < 0)
      {
	  /* silencing compiler complaints */
	  group_name = NULL;
	  style_id = -1;
      }
    return register_group_style_ex (p_sqlite, p_blob, n_bytes);
}

static int
do_delete_group_style_refs (sqlite3 * sqlite, sqlite3_int64 id)
{
/* auxiliary function: deleting all Group Style references */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "DELETE FROM SE_styled_group_styles WHERE style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterGroupStyle: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int64 (stmt, 1, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterGroupStyle() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

static int
check_group_style_refs_by_id (sqlite3 * sqlite, int style_id, int *has_refs)
{
/* checks if a Group Style do actually exists - by ID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    int ref_count = 0;

    sql = "SELECT s.style_id, l.style_id FROM SE_group_styles AS s "
	"LEFT JOIN SE_styled_group_styles AS l ON (l.style_id = s.style_id) "
	"WHERE s.style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Group Style Refs by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int (stmt, 1, style_id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		count++;
		if (sqlite3_column_type (stmt, 1) == SQLITE_INTEGER)
		    ref_count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1)
      {
	  if (ref_count > 0)
	      *has_refs = 1;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
check_group_style_refs_by_name (sqlite3 * sqlite, const char *style_name,
				sqlite3_int64 * id, int *has_refs)
{
/* checks if a Group Style do actually exists - by name */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    int ref_count = 0;
    sqlite3_int64 xid;

    sql = "SELECT style_id FROM SE_group_styles "
	"WHERE Lower(style_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Group Style Refs by Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, style_name, strlen (style_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count != 1)
	return 0;
    *id = xid;
    sql = "SELECT s.style_id, l.style_id FROM SE_group_styles AS s "
	"LEFT JOIN SE_styled_group_styles AS l ON (l.style_id = s.style_id) "
	"WHERE s.style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Group Style Refs by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int64 (stmt, 1, *id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 1) == SQLITE_INTEGER)
		    ref_count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (ref_count > 0)
	*has_refs = 1;
    return 1;
    return 0;
  stop:
    return 0;
}

static int
do_delete_group_style (sqlite3 * sqlite, sqlite3_int64 id)
{
/* auxiliary function: really deleting a Group Style */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "DELETE FROM SE_group_styles WHERE style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterGroupStyle: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int64 (stmt, 1, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterGroupStyle() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
unregister_group_style (void *p_sqlite, int style_id,
			const char *style_name, int remove_all)
{
/* auxiliary function: deletes a Group Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;
    int has_refs = 0;

    if (style_id >= 0)
      {
	  /* checking if the Group Style do actually exists */
	  if (check_group_style_refs_by_id (sqlite, style_id, &has_refs))
	      id = style_id;
	  else
	      return 0;
	  if (has_refs)
	    {
		if (!remove_all)
		    return 0;
		/* deleting all references */
		if (!do_delete_group_style_refs (sqlite, id))
		    return 0;
	    }
	  /* deleting the Group Style */
	  return do_delete_group_style (sqlite, id);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Group Style do actually exists */
	  if (!check_group_style_refs_by_name
	      (sqlite, style_name, &id, &has_refs))
	      return 0;
	  if (has_refs)
	    {
		if (!remove_all)
		    return 0;
		/* deleting all references */
		if (!do_delete_group_style_refs (sqlite, id))
		    return 0;
	    }
	  /* deleting the Group Style */
	  return do_delete_group_style (sqlite, id);
      }
    else
	return 0;
}

static int
check_group_style_by_id (sqlite3 * sqlite, int style_id)
{
/* checks if a Group Style do actually exists - by ID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT style_id FROM SE_group_styles " "WHERE style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Group Style by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_int (stmt, 1, style_id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 1)
	return 1;
    return 0;
  stop:
    return 0;
}

static int
check_group_style_by_name (sqlite3 * sqlite, const char *style_name,
			   sqlite3_int64 * id)
{
/* checks if a Group Style do actually exists - by name */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    sqlite3_int64 xid;

    sql = "SELECT style_id FROM SE_group_styles "
	"WHERE Lower(style_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Group Style by Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, style_name, strlen (style_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1)
      {
	  *id = xid;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
do_reload_group_style (sqlite3 * sqlite, sqlite3_int64 id,
		       const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: reloads a Group Style definition */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (p_blob != NULL && n_bytes > 0)
      {
	  /* attempting to update the Group Style */
	  sql = "UPDATE SE_group_styles SET style = ? " "WHERE style_id = ?";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("reloadGroupStyle: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		return 0;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_blob (stmt, 1, p_blob, n_bytes, SQLITE_STATIC);
	  sqlite3_bind_int64 (stmt, 2, id);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("reloadGroupStyle() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  sqlite3_finalize (stmt);
	  return 1;
      }
    else
	return 0;
}

SPATIALITE_PRIVATE int
reload_group_style (void *p_sqlite, int style_id,
		    const char *style_name,
		    const unsigned char *p_blob, int n_bytes)
{
/* auxiliary function: reloads a Group Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;

    if (style_id >= 0)
      {
	  /* checking if the Group Style do actually exists */
	  if (check_group_style_by_id (sqlite, style_id))
	      id = style_id;
	  else
	      return 0;
	  /* reloading the Group Style */
	  if (group_style_causes_duplicate_name (sqlite, id, p_blob, n_bytes))
	      return 0;
	  return do_reload_group_style (sqlite, id, p_blob, n_bytes);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Group Style do actually exists */
	  if (!check_group_style_by_name (sqlite, style_name, &id))
	      return 0;
	  /* reloading the Group Style */
	  if (group_style_causes_duplicate_name (sqlite, id, p_blob, n_bytes))
	      return 0;
	  return do_reload_group_style (sqlite, id, p_blob, n_bytes);
      }
    else
	return 0;
}

static int
do_insert_styled_group_style (sqlite3 * sqlite, const char *group_name,
			      sqlite3_int64 id)
{
/* auxiliary function: really inserting a Styled Group Style */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "INSERT INTO SE_styled_group_styles "
	"(group_name, style_id) VALUES (?, ?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("registerStyledGroupStyle: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("registerGroupStyledLayer() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
register_styled_group_style (void *p_sqlite, const char *group_name,
			     int style_id, const char *style_name)
{
/* auxiliary function: inserts a Styled Group Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;

    if (group_name == NULL)
	return 0;

    if (style_id >= 0)
      {
	  /* checking if the Group Style do actually exists */
	  if (check_group_style_by_id (sqlite, style_id))
	      id = style_id;
	  else
	      return 0;
	  /* inserting the Styled Group Style */
	  return do_insert_styled_group_style (sqlite, group_name, id);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Group Style do actually exists */
	  if (!check_group_style_by_name (sqlite, style_name, &id))
	      return 0;
	  /* inserting the Styled Group Style */
	  return do_insert_styled_group_style (sqlite, group_name, id);
      }
    else
	return 0;
}

static int
check_styled_group_style_by_id (sqlite3 * sqlite, const char *group_name,
				int style_id)
{
/* checks if a Styled Group Style do actually exists - by ID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT style_id FROM SE_styled_group_styles "
	"WHERE Lower(group_name) = Lower(?) AND style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Styled Group Style by ID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, style_id);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 1)
	return 1;
    return 0;
  stop:
    return 0;
}

static int
check_styled_group_style_by_name (sqlite3 * sqlite, const char *group_name,
				  const char *style_name, sqlite3_int64 * id)
{
/* checks if a Styled Group Style do actually exists - by name */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    sqlite3_int64 xid;

    sql = "SELECT l.style_id FROM SE_styled_group_styles AS l "
	"JOIN SE_group_styles AS s ON (l.style_id = s.style_id) "
	"WHERE Lower(l.group_name) = Lower(?) AND "
	"Lower(s.style_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Styled Group Style by Name: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, style_name, strlen (style_name), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		xid = sqlite3_column_int64 (stmt, 0);
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1)
      {
	  *id = xid;
	  return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
do_delete_styled_group_style (sqlite3 * sqlite, const char *group_name,
			      sqlite3_int64 id)
{
/* auxiliary function: really deleting a Styled Group Style */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "DELETE FROM SE_styled_group_styles "
	"WHERE Lower(group_name) = Lower(?) AND " "style_id = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterStyledGroupStyle: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, group_name, strlen (group_name), SQLITE_STATIC);
    sqlite3_bind_int64 (stmt, 2, id);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterStyledGroupStyle() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
unregister_styled_group_style (void *p_sqlite, const char *group_name,
			       int style_id, const char *style_name)
{
/* auxiliary function: removes a Styled Group Style definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 id;

    if (group_name == NULL)
	return 0;

    if (style_id >= 0)
      {
	  /* checking if the Styled Group Style do actually exists */
	  if (check_styled_group_style_by_id (sqlite, group_name, style_id))
	      id = style_id;
	  else
	      return 0;
	  /* removing the Styled Group Style */
	  return do_delete_styled_group_style (sqlite, group_name, id);
      }
    else if (style_name != NULL)
      {
	  /* checking if the Styled Group Style do actually exists */
	  if (!check_styled_group_style_by_name
	      (sqlite, group_name, style_name, &id))
	      return 0;
	  /* removing the Styled Group Style */
	  return do_delete_styled_group_style (sqlite, group_name, id);
      }
    else
	return 0;
}

SPATIALITE_PRIVATE int
register_vector_coverage (void *p_sqlite, const char *coverage_name,
			  const char *f_table_name,
			  const char *f_geometry_column, const char *title,
			  const char *abstract)
{
/* auxiliary function: inserts a Vector Coverage definition */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (coverage_name != NULL && f_table_name != NULL
	&& f_geometry_column != NULL && title != NULL && abstract != NULL)
      {
	  /* attempting to insert the Vector Coverage */
	  sql = "INSERT INTO vector_coverages "
	      "(coverage_name, f_table_name, f_geometry_column, title, abstract) "
	      "VALUES (Lower(?), Lower(?), Lower(?), ?, ?)";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("registerVectorCoverage: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		return 0;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
			     SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 2, f_table_name, strlen (f_table_name),
			     SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 3, f_geometry_column,
			     strlen (f_geometry_column), SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 4, title, strlen (title), SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 5, abstract, strlen (abstract),
			     SQLITE_STATIC);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("registerVectorCoverage() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  sqlite3_finalize (stmt);
	  return 1;
      }
    else if (coverage_name != NULL && f_table_name != NULL
	     && f_geometry_column != NULL)
      {
	  /* attempting to insert the Vector Coverage */
	  sql = "INSERT INTO vector_coverages "
	      "(coverage_name, f_table_name, f_geometry_column) "
	      "VALUES (Lower(?), Lower(?), Lower(?))";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("registerVectorCoverage: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		return 0;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
			     SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 2, f_table_name, strlen (f_table_name),
			     SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 3, f_geometry_column,
			     strlen (f_geometry_column), SQLITE_STATIC);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("registerVectorCoverage() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  sqlite3_finalize (stmt);
	  return 1;
      }
    else
	return 0;
}

static int
check_vector_coverage (sqlite3 * sqlite, const char *coverage_name)
{
/* checks if a Vector Coverage do actually exists */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT coverage_name FROM vector_coverages "
	"WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Coverage: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 1)
	return 1;
    return 0;
  stop:
    return 0;
}

static void
do_delete_vector_coverage_srid (sqlite3 * sqlite, const char *coverage_name,
				int srid)
{
/* auxiliary function: deleting a Vector Coverage alternative SRID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (srid < 0)
	sql = "DELETE FROM vector_coverages_srid "
	    "WHERE Lower(coverage_name) = Lower(?)";
    else
	sql = "DELETE FROM vector_coverages_srid "
	    "WHERE Lower(coverage_name) = Lower(?) AND srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterVectorCoverageSrid: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    if (srid >= 0)
	sqlite3_bind_int (stmt, 2, srid);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
	spatialite_e ("unregisterVectorCoverageSrid() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
}

static void
do_delete_vector_coverage_keyword (sqlite3 * sqlite, const char *coverage_name,
				   const char *keyword)
{
/* auxiliary function: deleting an Vector Coverage Keyword */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (keyword == NULL)
	sql = "DELETE FROM vector_coverages_keyword "
	    "WHERE Lower(coverage_name) = Lower(?)";
    else
	sql = "DELETE FROM vector_coverages_keyword "
	    "WHERE Lower(coverage_name) = Lower(?) AND Lower(keyword) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterVectorCoverageKeyword: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    if (keyword != NULL)
	sqlite3_bind_text (stmt, 2, keyword, strlen (keyword), SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
	spatialite_e ("unregisterVectorCoverageKeyword() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
}

static void
do_delete_vector_coverage_styled_layers (sqlite3 * sqlite,
					 const char *coverage_name)
{
/* auxiliary function: deleting all Vector Coverage Styled references */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    sql = "DELETE FROM SE_vector_styled_layers WHERE coverage_name = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterVectorCoverageStyles: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
	spatialite_e ("unregisterVectorCoverageStyles() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
}

static void
do_delete_vector_coverage_styled_groups (sqlite3 * sqlite,
					 const char *coverage_name)
{
/* auxiliary function: deleting all Vector Coverage Styled Group references */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    sql = "DELETE FROM SE_styled_group_refs WHERE vector_coverage_name = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterVectorCoverageGroups: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
	spatialite_e ("unregisterVectorCoverageGroups() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
}

static int
do_delete_vector_coverage (sqlite3 * sqlite, const char *coverage_name)
{
/* auxiliary function: deleting a Vector Coverage */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int retval = 0;
    sql = "DELETE FROM vector_coverages "
	"WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterVectorCoverage: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("unregisterVectorCoverage() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
unregister_vector_coverage (void *p_sqlite, const char *coverage_name)
{
/* auxiliary function: deletes a Vector Coverage definition (and any related) */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;

    if (coverage_name == NULL)
	return 0;

    /* checking if the Vector Coverage do actually exists */
    if (!check_vector_coverage (sqlite, coverage_name))
	return 0;
    /* deleting all alternative SRIDs */
    do_delete_vector_coverage_srid (sqlite, coverage_name, -1);
    /* deleting all Keywords */
    do_delete_vector_coverage_keyword (sqlite, coverage_name, NULL);
    /* deleting all Styled Layers */
    do_delete_vector_coverage_styled_layers (sqlite, coverage_name);
    /* deleting all Styled Group references */
    do_delete_vector_coverage_styled_groups (sqlite, coverage_name);
    /* deleting the Vector Coverage itself */
    return do_delete_vector_coverage (sqlite, coverage_name);
}

SPATIALITE_PRIVATE int
set_vector_coverage_infos (void *p_sqlite, const char *coverage_name,
			   const char *title, const char *abstract)
{
/* auxiliary function: updates the descriptive infos supporting a Vector Coverage */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (coverage_name != NULL && title != NULL && abstract != NULL)
      {
	  /* attempting to update the Vector Coverage */
	  sql = "UPDATE vector_coverages SET title = ?, abstract = ? "
	      "WHERE Lower(coverage_name) = Lower(?)";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("registerVectorCoverage: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		return 0;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_text (stmt, 1, title, strlen (title), SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 2, abstract, strlen (abstract),
			     SQLITE_STATIC);
	  sqlite3_bind_text (stmt, 3, coverage_name, strlen (coverage_name),
			     SQLITE_STATIC);
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	      ;
	  else
	    {
		spatialite_e ("setVectorCoverageInfos() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		sqlite3_finalize (stmt);
		return 0;
	    }
	  sqlite3_finalize (stmt);
	  return 1;
      }
    else
	return 0;
}

static int
check_vector_coverage_srid2 (sqlite3 * sqlite, const char *coverage_name,
			     int srid)
{
/* checks if a Vector Coverage SRID do actually exists */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT srid FROM vector_coverages_srid "
	"WHERE Lower(coverage_name) = Lower(?) AND srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Coverage SRID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int (stmt, 2, srid);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 1)
	return 1;
    return 0;
  stop:
    return 0;
}

static int
check_vector_coverage_srid1 (sqlite3 * sqlite, const char *coverage_name,
			     int srid)
{
/* checks if a Vector Coverage do actually exists and check its SRID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    int same_srid = 0;

    sql = "SELECT c.srid FROM vector_coverages AS v "
	"JOIN geometry_columns AS c ON (Lower(v.f_table_name) = Lower(c.f_table_name) "
	"AND Lower(v.f_geometry_column) = Lower(c.f_geometry_column)) "
	"WHERE Lower(v.coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Coverage SRID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		int natural_srid = sqlite3_column_int (stmt, 0);
		if (srid == natural_srid)
		    same_srid++;
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1 && same_srid == 0)
      {
	  if (check_vector_coverage_srid2 (sqlite, coverage_name, srid))
	      return 0;
	  else
	      return 1;
      }
    return 0;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
register_vector_coverage_srid (void *p_sqlite, const char *coverage_name,
			       int srid)
{
/* auxiliary function: inserting a Vector Coverage alternative SRID */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (coverage_name == NULL)
	return 0;
    if (srid <= 0)
	return 0;

    /* checking if the Vector Coverage do actually exists */
    if (!check_vector_coverage_srid1 (sqlite, coverage_name, srid))
	return 0;

    /* attempting to insert the Vector Coverage alternative SRID */
    sql = "INSERT INTO vector_coverages_srid "
	"(coverage_name, srid) VALUES (Lower(?), ?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("registerVectorCoverageSrid: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int (stmt, 2, srid);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("registerVectorCoverageSrid() error: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  sqlite3_finalize (stmt);
	  return 0;
      }
    sqlite3_finalize (stmt);
    return 1;
}

SPATIALITE_PRIVATE int
unregister_vector_coverage_srid (void *p_sqlite, const char *coverage_name,
				 int srid)
{
/* auxiliary function: deletes a Vector Coverage alternative SRID */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;

    if (coverage_name == NULL)
	return 0;

    /* checking if the Vector Coverage SRID do actually exists */
    if (!check_vector_coverage_srid2 (sqlite, coverage_name, srid))
	return 0;
    /* deleting the alternative SRID */
    do_delete_vector_coverage_srid (sqlite, coverage_name, srid);
    return 1;
}

static int
check_vector_coverage_keyword0 (sqlite3 * sqlite, const char *coverage_name)
{
/* checks if a Vector Coverage do actually exists */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql =
	"SELECT coverage_name FROM vector_coverages WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Coverage Keyword: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 0)
	return 0;
    return 1;
  stop:
    return 0;
}

static int
check_vector_coverage_keyword1 (sqlite3 * sqlite, const char *coverage_name,
				const char *keyword)
{
/* checks if a Vector Coverage do actually exists and check the Keyword */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int same_kw = 0;

    sql =
	"SELECT keyword FROM vector_coverages_keyword WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Coverage Keyword: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		const char *kw = (const char *) sqlite3_column_text (stmt, 0);
		if (strcasecmp (kw, keyword) == 0)
		    same_kw++;
	    }
      }
    sqlite3_finalize (stmt);
    if (same_kw == 0)
      {
	  if (!check_vector_coverage_keyword0 (sqlite, coverage_name))
	      return 0;
	  else
	      return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
check_vector_coverage_keyword2 (sqlite3 * sqlite, const char *coverage_name,
				const char *keyword)
{
/* checks if a Vector Coverage do actually exists and check the Keyword */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT keyword FROM vector_coverages_keyword "
	"WHERE Lower(coverage_name) = Lower(?) AND Lower(keyword) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Vector Coverage Keyword: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, keyword, strlen (keyword), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 0)
	return 0;
    return 1;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
register_vector_coverage_keyword (void *p_sqlite, const char *coverage_name,
				  const char *keyword)
{
/* auxiliary function: inserting a Vector Coverage Keyword */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (coverage_name == NULL)
	return 0;
    if (keyword == NULL)
	return 0;

    /* checking if the Vector Coverage do actually exists */
    if (!check_vector_coverage_keyword1 (sqlite, coverage_name, keyword))
	return 0;

    /* attempting to insert the Vector Coverage Keyword */
    sql = "INSERT INTO vector_coverages_keyword "
	"(coverage_name, keyword) VALUES (Lower(?), ?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("registerVectorCoverageKeyword: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, keyword, strlen (keyword), SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("registerVectorCoverageKeyword() error: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  sqlite3_finalize (stmt);
	  return 0;
      }
    sqlite3_finalize (stmt);
    return 1;
}

SPATIALITE_PRIVATE int
unregister_vector_coverage_keyword (void *p_sqlite, const char *coverage_name,
				    const char *keyword)
{
/* auxiliary function: deletes a Vector Coverage Keyword */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;

    if (coverage_name == NULL)
	return 0;
    if (keyword == NULL)
	return 0;

    /* checking if the Vector Coverage Keyword do actually exists */
    if (!check_vector_coverage_keyword2 (sqlite, coverage_name, keyword))
	return 0;
    /* deleting the Keyword */
    do_delete_vector_coverage_keyword (sqlite, coverage_name, keyword);
    return 1;
}

static int
do_null_vector_coverage_extents (sqlite3 * sqlite, sqlite3_stmt * stmt_upd_cvg,
				 sqlite3_stmt * stmt_null_srid,
				 const char *coverage_name)
{
/* setting the main Coverage Extent to NULL */
    int ret;
    sqlite3_reset (stmt_upd_cvg);
    sqlite3_clear_bindings (stmt_upd_cvg);
    sqlite3_bind_null (stmt_upd_cvg, 1);
    sqlite3_bind_null (stmt_upd_cvg, 2);
    sqlite3_bind_null (stmt_upd_cvg, 3);
    sqlite3_bind_null (stmt_upd_cvg, 4);
    sqlite3_bind_null (stmt_upd_cvg, 5);
    sqlite3_bind_null (stmt_upd_cvg, 6);
    sqlite3_bind_null (stmt_upd_cvg, 7);
    sqlite3_bind_null (stmt_upd_cvg, 8);
    sqlite3_bind_text (stmt_upd_cvg, 9, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    ret = sqlite3_step (stmt_upd_cvg);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("updateVectorCoverageExtent error: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
/* setting all alternativ Coverage Extent to NULL */
    sqlite3_reset (stmt_null_srid);
    sqlite3_clear_bindings (stmt_null_srid);
    sqlite3_bind_text (stmt_null_srid, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    ret = sqlite3_step (stmt_null_srid);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("updateVectorCoverageExtent error: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    return 1;
}

static int
do_update_vector_coverage_extents (sqlite3 * sqlite, const void *cache,
				   sqlite3_stmt * stmt_upd_cvg,
				   sqlite3_stmt * stmt_srid,
				   sqlite3_stmt * stmt_upd_srid,
				   const char *coverage_name, int natural_srid,
				   double minx, double miny, double maxx,
				   double maxy)
{
/* updating the Coverage Extents */
    int ret;
    int geographic = 0;
    double geo_minx = minx;
    double geo_miny = miny;
    double geo_maxx = maxx;
    double geo_maxy = maxy;
    char *proj_from = NULL;
    char *proj_to = NULL;
    gaiaGeomCollPtr in;
    gaiaGeomCollPtr out;
    gaiaPointPtr pt;

    getProjParams (sqlite, natural_srid, &proj_from);
    if (proj_from == NULL)
	goto error;

    ret = srid_is_geographic (sqlite, natural_srid, &geographic);
    if (!ret)
	return 0;
    if (!geographic)
      {
	  /* computing the geographic extent */
	  getProjParams (sqlite, 4326, &proj_to);
	  if (proj_to == NULL)
	      goto error;
	  in = gaiaAllocGeomColl ();
	  in->Srid = natural_srid;
	  gaiaAddPointToGeomColl (in, minx, miny);
	  if (cache != NULL)
	      out = gaiaTransform_r (cache, in, proj_from, proj_to);
	  else
	      out = gaiaTransform (in, proj_from, proj_to);
	  if (out == NULL)
	    {
		gaiaFreeGeomColl (in);
		goto error;
	    }
	  pt = out->FirstPoint;
	  if (pt == NULL)
	    {
		gaiaFreeGeomColl (in);
		gaiaFreeGeomColl (out);
		goto error;
	    }
	  geo_minx = pt->X;
	  geo_miny = pt->Y;
	  gaiaFreeGeomColl (in);
	  gaiaFreeGeomColl (out);
	  in = gaiaAllocGeomColl ();
	  in->Srid = natural_srid;
	  gaiaAddPointToGeomColl (in, maxx, maxy);
	  if (cache != NULL)
	      out = gaiaTransform_r (cache, in, proj_from, proj_to);
	  else
	      out = gaiaTransform (in, proj_from, proj_to);
	  if (out == NULL)
	    {
		gaiaFreeGeomColl (in);
		goto error;
	    }
	  pt = out->FirstPoint;
	  if (pt == NULL)
	    {
		gaiaFreeGeomColl (in);
		gaiaFreeGeomColl (out);
		goto error;
	    }
	  geo_maxx = pt->X;
	  geo_maxy = pt->Y;
	  gaiaFreeGeomColl (in);
	  gaiaFreeGeomColl (out);
	  free (proj_to);
	  proj_to = NULL;
      }

/* setting the main Coverage Extent */
    sqlite3_reset (stmt_upd_cvg);
    sqlite3_clear_bindings (stmt_upd_cvg);
    sqlite3_bind_double (stmt_upd_cvg, 1, geo_minx);
    sqlite3_bind_double (stmt_upd_cvg, 2, geo_miny);
    sqlite3_bind_double (stmt_upd_cvg, 3, geo_maxx);
    sqlite3_bind_double (stmt_upd_cvg, 4, geo_maxy);
    sqlite3_bind_double (stmt_upd_cvg, 5, minx);
    sqlite3_bind_double (stmt_upd_cvg, 6, miny);
    sqlite3_bind_double (stmt_upd_cvg, 7, maxx);
    sqlite3_bind_double (stmt_upd_cvg, 8, maxy);
    sqlite3_bind_text (stmt_upd_cvg, 9, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    ret = sqlite3_step (stmt_upd_cvg);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("updateVectorCoverageExtent error: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

/* updating any alternative SRID supporting this Vector Coverage */
    sqlite3_reset (stmt_srid);
    sqlite3_clear_bindings (stmt_srid);
    sqlite3_bind_text (stmt_srid, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt_srid);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a single alternative SRID Extent */
		double alt_minx;
		double alt_miny;
		double alt_maxx;
		double alt_maxy;
		int srid = sqlite3_column_int (stmt_srid, 0);
		getProjParams (sqlite, srid, &proj_to);
		if (proj_to == NULL)
		    goto error;
		in = gaiaAllocGeomColl ();
		in->Srid = natural_srid;
		gaiaAddPointToGeomColl (in, minx, miny);
		if (cache != NULL)
		    out = gaiaTransform_r (cache, in, proj_from, proj_to);
		else
		    out = gaiaTransform (in, proj_from, proj_to);
		if (out == NULL)
		  {
		      gaiaFreeGeomColl (in);
		      goto error;
		  }
		pt = out->FirstPoint;
		if (pt == NULL)
		  {
		      gaiaFreeGeomColl (in);
		      gaiaFreeGeomColl (out);
		      goto error;
		  }
		alt_minx = pt->X;
		alt_miny = pt->Y;
		gaiaFreeGeomColl (in);
		gaiaFreeGeomColl (out);
		in = gaiaAllocGeomColl ();
		in->Srid = natural_srid;
		gaiaAddPointToGeomColl (in, maxx, maxy);
		if (cache != NULL)
		    out = gaiaTransform_r (cache, in, proj_from, proj_to);
		else
		    out = gaiaTransform (in, proj_from, proj_to);
		if (out == NULL)
		  {
		      gaiaFreeGeomColl (in);
		      goto error;
		  }
		pt = out->FirstPoint;
		if (pt == NULL)
		  {
		      gaiaFreeGeomColl (in);
		      gaiaFreeGeomColl (out);
		      goto error;
		  }
		alt_maxx = pt->X;
		alt_maxy = pt->Y;
		gaiaFreeGeomColl (in);
		gaiaFreeGeomColl (out);
		free (proj_to);
		proj_to = NULL;

/* setting the alternative Srid Extent */
		sqlite3_reset (stmt_upd_srid);
		sqlite3_clear_bindings (stmt_upd_srid);
		sqlite3_bind_double (stmt_upd_srid, 1, alt_minx);
		sqlite3_bind_double (stmt_upd_srid, 2, alt_miny);
		sqlite3_bind_double (stmt_upd_srid, 3, alt_maxx);
		sqlite3_bind_double (stmt_upd_srid, 4, alt_maxy);
		sqlite3_bind_text (stmt_upd_srid, 5, coverage_name,
				   strlen (coverage_name), SQLITE_STATIC);
		sqlite3_bind_int (stmt_upd_srid, 6, srid);
		ret = sqlite3_step (stmt_upd_srid);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      spatialite_e
			  ("updateVectorCoverageExtent error: \"%s\"\n",
			   sqlite3_errmsg (sqlite));
		      goto error;
		  }
	    }
	  else
	    {
		spatialite_e ("updateVectorCoverageExtent() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		goto error;
	    }
      }

    free (proj_from);
    return 1;

  error:
    if (proj_from)
	free (proj_from);
    if (proj_to)
	free (proj_to);
    return 0;
}

SPATIALITE_PRIVATE int
update_vector_coverage_extent (void *p_sqlite, const void *cache,
			       const char *coverage_name, int transaction)
{
/* updates one (or all) Vector Coverage Extents */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    char *sql;
    sqlite3_stmt *stmt = NULL;
    sqlite3_stmt *stmt_ext = NULL;
    sqlite3_stmt *stmt_upd_cvg = NULL;
    sqlite3_stmt *stmt_upd_srid = NULL;
    sqlite3_stmt *stmt_null_srid = NULL;
    sqlite3_stmt *stmt_srid = NULL;

/* preparing the ancillary SQL statements */
    sql = "SELECT srid FROM vector_coverages_srid "
	"WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_srid, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateVectorCoverageExtent: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

    sql = "UPDATE vector_coverages SET geo_minx = ?, geo_miny = ?, "
	"geo_maxx = ?, geo_maxy = ?, extent_minx = ?, extent_miny = ?, "
	"extent_maxx = ?, extent_maxy = ? "
	"WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_upd_cvg, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateVectorCoverageExtent: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

    sql = "UPDATE vector_coverages_srid SET extent_minx = NULL, "
	"extent_miny = NULL, extent_maxx = NULL, extent_maxy = NULL "
	"WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_null_srid, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateVectorCoverageExtent: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

    sql = "UPDATE vector_coverages_srid SET extent_minx = ?, "
	"extent_miny = ?, extent_maxx = ?, extent_maxy = ? "
	"WHERE Lower(coverage_name) = Lower(?) AND srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_upd_srid, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateVectorCoverageExtent: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

/* preparing the main SQL statement */
    if (coverage_name == NULL)
      {
	  sql = "SELECT v.coverage_name, v.f_table_name, v.f_geometry_column, "
	      "c.srid FROM vector_coverages AS v "
	      "JOIN geometry_columns AS c ON (Lower(v.f_table_name) = "
	      "Lower(c.f_table_name) AND Lower(v.f_geometry_column) = "
	      "Lower(c.f_geometry_column))";
      }
    else
      {

	  sql = "SELECT v.coverage_name, v.f_table_name, v.f_geometry_column, "
	      "c.srid FROM vector_coverages AS v "
	      "JOIN geometry_columns AS c ON (Lower(v.f_table_name) = "
	      "Lower(c.f_table_name) AND Lower(v.f_geometry_column) = "
	      "Lower(c.f_geometry_column)) "
	      "WHERE Lower(v.coverage_name) = Lower(?)";
      }
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateVectorCoverageExtent: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

    if (transaction)
      {
	  /* starting a Transaction */
	  ret = sqlite3_exec (sqlite, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	      goto error;
      }

    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    if (coverage_name != NULL)
	sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
			   SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a single Vector Coverage */
		char *table;
		char *geom;
		const char *cvg = (const char *) sqlite3_column_text (stmt, 0);
		const char *xtable =
		    (const char *) sqlite3_column_text (stmt, 1);
		const char *xgeom =
		    (const char *) sqlite3_column_text (stmt, 2);
		int natural_srid = sqlite3_column_int (stmt, 3);
		table = gaiaDoubleQuotedSql (xtable);
		geom = gaiaDoubleQuotedSql (xgeom);
		sql =
		    sqlite3_mprintf
		    ("SELECT Min(MbrMinX(\"%s\")), Min(MbrMinY(\"%s\")), "
		     "Max(MbrMaxX(\"%s\")), Max(MbrMaxY(\"%s\")) "
		     "FROM \"%s\"", geom, geom, geom, geom, table);
		free (table);
		free (geom);
		ret =
		    sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_ext,
					NULL);
		sqlite3_free (sql);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("updateVectorCoverageExtent: \"%s\"\n",
				    sqlite3_errmsg (sqlite));
		      goto error;
		  }
		while (1)
		  {
		      /* scrolling the result set rows */
		      ret = sqlite3_step (stmt_ext);
		      if (ret == SQLITE_DONE)
			  break;	/* end of result set */
		      if (ret == SQLITE_ROW)
			{
			    int null_minx = 1;
			    int null_miny = 1;
			    int null_maxx = 1;
			    int null_maxy = 1;
			    double minx;
			    double miny;
			    double maxx;
			    double maxy;
			    if (sqlite3_column_type (stmt_ext, 0) ==
				SQLITE_FLOAT)
			      {
				  minx = sqlite3_column_double (stmt_ext, 0);
				  null_minx = 0;
			      }
			    if (sqlite3_column_type (stmt_ext, 1) ==
				SQLITE_FLOAT)
			      {
				  miny = sqlite3_column_double (stmt_ext, 1);
				  null_miny = 0;
			      }
			    if (sqlite3_column_type (stmt_ext, 2) ==
				SQLITE_FLOAT)
			      {
				  maxx = sqlite3_column_double (stmt_ext, 2);
				  null_maxx = 0;
			      }
			    if (sqlite3_column_type (stmt_ext, 3) ==
				SQLITE_FLOAT)
			      {
				  maxy = sqlite3_column_double (stmt_ext, 3);
				  null_maxy = 0;
			      }
			    if (null_minx || null_miny || null_maxx
				|| null_maxy)
				ret =
				    do_null_vector_coverage_extents (sqlite,
								     stmt_upd_cvg,
								     stmt_null_srid,
								     cvg);
			    else
				ret =
				    do_update_vector_coverage_extents (sqlite,
								       cache,
								       stmt_upd_cvg,
								       stmt_srid,
								       stmt_upd_srid,
								       cvg,
								       natural_srid,
								       minx,
								       miny,
								       maxx,
								       maxy);
			    if (!ret)
				goto error;
			}
		      else
			{
			    spatialite_e
				("updateVectorCoverageExtent() error: \"%s\"\n",
				 sqlite3_errmsg (sqlite));
			    goto error;
			}
		  }
		sqlite3_finalize (stmt_ext);
		stmt_ext = NULL;
	    }
	  else
	    {
		spatialite_e ("updateVectorCoverageExtent() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		goto error;
	    }
      }

    if (transaction)
      {
	  /* committing the still pending Transaction */
	  ret = sqlite3_exec (sqlite, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	      goto error;
      }

    sqlite3_finalize (stmt);
    sqlite3_finalize (stmt_upd_cvg);
    sqlite3_finalize (stmt_upd_srid);
    sqlite3_finalize (stmt_null_srid);
    sqlite3_finalize (stmt_srid);
    return 1;

  error:
    if (stmt != NULL)
	sqlite3_finalize (stmt);
    if (stmt_ext != NULL)
	sqlite3_finalize (stmt_ext);
    if (stmt_upd_cvg != NULL)
	sqlite3_finalize (stmt_upd_cvg);
    if (stmt_upd_srid != NULL)
	sqlite3_finalize (stmt_upd_srid);
    if (stmt_null_srid != NULL)
	sqlite3_finalize (stmt_null_srid);
    if (stmt_srid != NULL)
	sqlite3_finalize (stmt_srid);
    return 0;
}

static int
check_raster_coverage_srid2 (sqlite3 * sqlite, const char *coverage_name,
			     int srid)
{
/* checks if a Raster Coverage SRID do actually exists */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT srid FROM raster_coverages_srid "
	"WHERE Lower(coverage_name) = Lower(?) AND srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Coverage SRID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int (stmt, 2, srid);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 1)
	return 1;
    return 0;
  stop:
    return 0;
}

static int
check_raster_coverage_srid1 (sqlite3 * sqlite, const char *coverage_name,
			     int srid)
{
/* checks if a Raster Coverage do actually exists and check its SRID */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;
    int same_srid = 0;

    sql = "SELECT srid FROM raster_coverages "
	"WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Coverage SRID: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		int natural_srid = sqlite3_column_int (stmt, 0);
		if (srid == natural_srid)
		    same_srid++;
		count++;
	    }
      }
    sqlite3_finalize (stmt);
    if (count == 1 && same_srid == 0)
      {
	  if (check_raster_coverage_srid2 (sqlite, coverage_name, srid))
	      return 0;
	  else
	      return 1;
      }
    return 0;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
register_raster_coverage_srid (void *p_sqlite, const char *coverage_name,
			       int srid)
{
/* auxiliary function: inserting a Raster Coverage alternative SRID */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (coverage_name == NULL)
	return 0;
    if (srid <= 0)
	return 0;

    /* checking if the Raster Coverage do actually exists */
    if (!check_raster_coverage_srid1 (sqlite, coverage_name, srid))
	return 0;

    /* attempting to insert the Raster Coverage alternative SRID */
    sql = "INSERT INTO raster_coverages_srid "
	"(coverage_name, srid) VALUES (Lower(?), ?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("registerRasterCoverageSrid: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int (stmt, 2, srid);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("registerRasterCoverageSrid() error: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  sqlite3_finalize (stmt);
	  return 0;
      }
    sqlite3_finalize (stmt);
    return 1;
}

static void
do_delete_raster_coverage_srid (sqlite3 * sqlite, const char *coverage_name,
				int srid)
{
/* auxiliary function: deleting all Raster Coverage alternative SRIDs */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    sql = "DELETE FROM raster_coverages_srid "
	"WHERE Lower(coverage_name) = Lower(?) AND srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterRasterCoverageSrid: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_int (stmt, 2, srid);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
	spatialite_e ("unregisterRasterCoverageSrid() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
}

SPATIALITE_PRIVATE int
unregister_raster_coverage_srid (void *p_sqlite, const char *coverage_name,
				 int srid)
{
/* auxiliary function: deletes a Raster Coverage alternative SRID */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;

    if (coverage_name == NULL)
	return 0;

    /* checking if the Raster Coverage SRID do actually exists */
    if (!check_raster_coverage_srid2 (sqlite, coverage_name, srid))
	return 0;
    /* deleting the alternative SRID */
    do_delete_raster_coverage_srid (sqlite, coverage_name, srid);
    return 1;
}

static int
check_raster_coverage_keyword0 (sqlite3 * sqlite, const char *coverage_name)
{
/* checks if a Raster Coverage do actually exists */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql =
	"SELECT coverage_name FROM raster_coverages WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Coverage Keyword: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 0)
	return 0;
    return 1;
  stop:
    return 0;
}

static int
check_raster_coverage_keyword1 (sqlite3 * sqlite, const char *coverage_name,
				const char *keyword)
{
/* checks if a Raster Coverage do actually exists and check the Keyword */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int same_kw = 0;

    sql =
	"SELECT keyword FROM raster_coverages_keyword WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Coverage Keyword: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		const char *kw = (const char *) sqlite3_column_text (stmt, 0);
		if (strcasecmp (kw, keyword) == 0)
		    same_kw++;
	    }
      }
    sqlite3_finalize (stmt);
    if (same_kw == 0)
      {
	  if (!check_raster_coverage_keyword0 (sqlite, coverage_name))
	      return 0;
	  else
	      return 1;
      }
    return 0;
  stop:
    return 0;
}

static int
check_raster_coverage_keyword2 (sqlite3 * sqlite, const char *coverage_name,
				const char *keyword)
{
/* checks if a Raster Coverage do actually exists and check the Keyword */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int count = 0;

    sql = "SELECT keyword FROM raster_coverages_keyword "
	"WHERE Lower(coverage_name) = Lower(?) AND Lower(keyword) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("check Raster Coverage Keyword: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, keyword, strlen (keyword), SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	      count++;
      }
    sqlite3_finalize (stmt);
    if (count == 0)
	return 0;
    return 1;
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
register_raster_coverage_keyword (void *p_sqlite, const char *coverage_name,
				  const char *keyword)
{
/* auxiliary function: inserting a Raster Coverage Keyword */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    if (coverage_name == NULL)
	return 0;
    if (keyword == NULL)
	return 0;

    /* checking if the Raster Coverage do actually exists */
    if (!check_raster_coverage_keyword1 (sqlite, coverage_name, keyword))
	return 0;

    /* attempting to insert the Raster Coverage Keyword */
    sql = "INSERT INTO raster_coverages_keyword "
	"(coverage_name, keyword) VALUES (Lower(?), ?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("registerRasterCoverageKeyword: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, keyword, strlen (keyword), SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("registerRasterCoverageKeyword() error: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  sqlite3_finalize (stmt);
	  return 0;
      }
    sqlite3_finalize (stmt);
    return 1;
}

static void
do_delete_raster_coverage_keyword (sqlite3 * sqlite, const char *coverage_name,
				   const char *keyword)
{
/* auxiliary function: deleting all Raster Coverage Keyword */
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;

    sql = "DELETE FROM raster_coverages_keyword "
	"WHERE Lower(coverage_name) = Lower(?) AND Lower(keyword) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("unregisterRasterCoverageKeyword: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, keyword, strlen (keyword), SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
	spatialite_e ("unregisterRasterCoverageKeyword() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
}

SPATIALITE_PRIVATE int
unregister_raster_coverage_keyword (void *p_sqlite, const char *coverage_name,
				    const char *keyword)
{
/* auxiliary function: deletes a Raster Coverage Keyword */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;

    if (coverage_name == NULL)
	return 0;
    if (keyword == NULL)
	return 0;

    /* checking if the Raster Coverage Keyword do actually exists */
    if (!check_raster_coverage_keyword2 (sqlite, coverage_name, keyword))
	return 0;
    /* deleting the Keyword */
    do_delete_raster_coverage_keyword (sqlite, coverage_name, keyword);
    return 1;
}

static int
do_null_raster_coverage_extents (sqlite3 * sqlite, sqlite3_stmt * stmt_upd_cvg,
				 sqlite3_stmt * stmt_null_srid,
				 const char *coverage_name)
{
/* setting the main Coverage Extent to NULL */
    int ret;
    sqlite3_reset (stmt_upd_cvg);
    sqlite3_clear_bindings (stmt_upd_cvg);
    sqlite3_bind_null (stmt_upd_cvg, 1);
    sqlite3_bind_null (stmt_upd_cvg, 2);
    sqlite3_bind_null (stmt_upd_cvg, 3);
    sqlite3_bind_null (stmt_upd_cvg, 4);
    sqlite3_bind_null (stmt_upd_cvg, 5);
    sqlite3_bind_null (stmt_upd_cvg, 6);
    sqlite3_bind_null (stmt_upd_cvg, 7);
    sqlite3_bind_null (stmt_upd_cvg, 8);
    sqlite3_bind_text (stmt_upd_cvg, 9, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    ret = sqlite3_step (stmt_upd_cvg);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("updateRasterCoverageExtent error: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
/* setting all alternativ Coverage Extent to NULL */
    sqlite3_reset (stmt_null_srid);
    sqlite3_clear_bindings (stmt_null_srid);
    sqlite3_bind_text (stmt_null_srid, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    ret = sqlite3_step (stmt_null_srid);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("updateRasterCoverageExtent error: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  return 0;
      }
    return 1;
}

static int
do_update_raster_coverage_extents (sqlite3 * sqlite, const void *cache,
				   sqlite3_stmt * stmt_upd_cvg,
				   sqlite3_stmt * stmt_srid,
				   sqlite3_stmt * stmt_upd_srid,
				   const char *coverage_name, int natural_srid,
				   double minx, double miny, double maxx,
				   double maxy)
{
/* updating the Coverage Extents */
    int ret;
    int geographic = 0;
    double geo_minx = minx;
    double geo_miny = miny;
    double geo_maxx = maxx;
    double geo_maxy = maxy;
    char *proj_from = NULL;
    char *proj_to = NULL;
    gaiaGeomCollPtr in;
    gaiaGeomCollPtr out;
    gaiaPointPtr pt;

    getProjParams (sqlite, natural_srid, &proj_from);
    if (proj_from == NULL)
	goto error;

    ret = srid_is_geographic (sqlite, natural_srid, &geographic);
    if (!ret)
	return 0;
    if (!geographic)
      {
	  /* computing the geographic extent */
	  getProjParams (sqlite, 4326, &proj_to);
	  if (proj_to == NULL)
	      goto error;
	  in = gaiaAllocGeomColl ();
	  in->Srid = natural_srid;
	  gaiaAddPointToGeomColl (in, minx, miny);
	  if (cache != NULL)
	      out = gaiaTransform_r (cache, in, proj_from, proj_to);
	  else
	      out = gaiaTransform (in, proj_from, proj_to);
	  if (out == NULL)
	    {
		gaiaFreeGeomColl (in);
		goto error;
	    }
	  pt = out->FirstPoint;
	  if (pt == NULL)
	    {
		gaiaFreeGeomColl (in);
		gaiaFreeGeomColl (out);
		goto error;
	    }
	  geo_minx = pt->X;
	  geo_miny = pt->Y;
	  gaiaFreeGeomColl (in);
	  gaiaFreeGeomColl (out);
	  in = gaiaAllocGeomColl ();
	  in->Srid = natural_srid;
	  gaiaAddPointToGeomColl (in, maxx, maxy);
	  if (cache != NULL)
	      out = gaiaTransform_r (cache, in, proj_from, proj_to);
	  else
	      out = gaiaTransform (in, proj_from, proj_to);
	  if (out == NULL)
	    {
		gaiaFreeGeomColl (in);
		goto error;
	    }
	  pt = out->FirstPoint;
	  if (pt == NULL)
	    {
		gaiaFreeGeomColl (in);
		gaiaFreeGeomColl (out);
		goto error;
	    }
	  geo_maxx = pt->X;
	  geo_maxy = pt->Y;
	  gaiaFreeGeomColl (in);
	  gaiaFreeGeomColl (out);
	  free (proj_to);
	  proj_to = NULL;
      }

/* setting the main Coverage Extent */
    sqlite3_reset (stmt_upd_cvg);
    sqlite3_clear_bindings (stmt_upd_cvg);
    sqlite3_bind_double (stmt_upd_cvg, 1, geo_minx);
    sqlite3_bind_double (stmt_upd_cvg, 2, geo_miny);
    sqlite3_bind_double (stmt_upd_cvg, 3, geo_maxx);
    sqlite3_bind_double (stmt_upd_cvg, 4, geo_maxy);
    sqlite3_bind_double (stmt_upd_cvg, 5, minx);
    sqlite3_bind_double (stmt_upd_cvg, 6, miny);
    sqlite3_bind_double (stmt_upd_cvg, 7, maxx);
    sqlite3_bind_double (stmt_upd_cvg, 8, maxy);
    sqlite3_bind_text (stmt_upd_cvg, 9, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    ret = sqlite3_step (stmt_upd_cvg);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  spatialite_e ("updateRasterCoverageExtent error: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

/* updating any alternative SRID supporting this Raster Coverage */
    sqlite3_reset (stmt_srid);
    sqlite3_clear_bindings (stmt_srid);
    sqlite3_bind_text (stmt_srid, 1, coverage_name, strlen (coverage_name),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt_srid);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a single alternative SRID Extent */
		double alt_minx;
		double alt_miny;
		double alt_maxx;
		double alt_maxy;
		int srid = sqlite3_column_int (stmt_srid, 0);
		getProjParams (sqlite, srid, &proj_to);
		if (proj_to == NULL)
		    goto error;
		in = gaiaAllocGeomColl ();
		in->Srid = natural_srid;
		gaiaAddPointToGeomColl (in, minx, miny);
		if (cache != NULL)
		    out = gaiaTransform_r (cache, in, proj_from, proj_to);
		else
		    out = gaiaTransform (in, proj_from, proj_to);
		if (out == NULL)
		  {
		      gaiaFreeGeomColl (in);
		      goto error;
		  }
		pt = out->FirstPoint;
		if (pt == NULL)
		  {
		      gaiaFreeGeomColl (in);
		      gaiaFreeGeomColl (out);
		      goto error;
		  }
		alt_minx = pt->X;
		alt_miny = pt->Y;
		gaiaFreeGeomColl (in);
		gaiaFreeGeomColl (out);
		in = gaiaAllocGeomColl ();
		in->Srid = natural_srid;
		gaiaAddPointToGeomColl (in, maxx, maxy);
		if (cache != NULL)
		    out = gaiaTransform_r (cache, in, proj_from, proj_to);
		else
		    out = gaiaTransform (in, proj_from, proj_to);
		if (out == NULL)
		  {
		      gaiaFreeGeomColl (in);
		      goto error;
		  }
		pt = out->FirstPoint;
		if (pt == NULL)
		  {
		      gaiaFreeGeomColl (in);
		      gaiaFreeGeomColl (out);
		      goto error;
		  }
		alt_maxx = pt->X;
		alt_maxy = pt->Y;
		gaiaFreeGeomColl (in);
		gaiaFreeGeomColl (out);
		free (proj_to);
		proj_to = NULL;

/* setting the alternative Srid Extent */
		sqlite3_reset (stmt_upd_srid);
		sqlite3_clear_bindings (stmt_upd_srid);
		sqlite3_bind_double (stmt_upd_srid, 1, alt_minx);
		sqlite3_bind_double (stmt_upd_srid, 2, alt_miny);
		sqlite3_bind_double (stmt_upd_srid, 3, alt_maxx);
		sqlite3_bind_double (stmt_upd_srid, 4, alt_maxy);
		sqlite3_bind_text (stmt_upd_srid, 5, coverage_name,
				   strlen (coverage_name), SQLITE_STATIC);
		sqlite3_bind_int (stmt_upd_srid, 6, srid);
		ret = sqlite3_step (stmt_upd_srid);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      spatialite_e
			  ("updateRasterCoverageExtent error: \"%s\"\n",
			   sqlite3_errmsg (sqlite));
		      goto error;
		  }
	    }
	  else
	    {
		spatialite_e ("updateRasterCoverageExtent() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		goto error;
	    }
      }

    free (proj_from);
    return 1;

  error:
    if (proj_from)
	free (proj_from);
    if (proj_to)
	free (proj_to);
    return 0;
}

SPATIALITE_PRIVATE int
update_raster_coverage_extent (void *p_sqlite, const void *cache,
			       const char *coverage_name, int transaction)
{
/* updates one (or all) Raster Coverage Extents */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    int ret;
    char *sql;
    sqlite3_stmt *stmt = NULL;
    sqlite3_stmt *stmt_ext = NULL;
    sqlite3_stmt *stmt_upd_cvg = NULL;
    sqlite3_stmt *stmt_upd_srid = NULL;
    sqlite3_stmt *stmt_null_srid = NULL;
    sqlite3_stmt *stmt_srid = NULL;

/* preparing the ancillary SQL statements */
    sql = "SELECT srid FROM raster_coverages_srid "
	"WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_srid, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateRasterCoverageExtent: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

    sql = "UPDATE raster_coverages SET geo_minx = ?, geo_miny = ?, "
	"geo_maxx = ?, geo_maxy = ?, extent_minx = ?, extent_miny = ?, "
	"extent_maxx = ?, extent_maxy = ? "
	"WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_upd_cvg, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateRasterCoverageExtent: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

    sql = "UPDATE raster_coverages_srid SET extent_minx = NULL, "
	"extent_miny = NULL, extent_maxx = NULL, extent_maxy = NULL "
	"WHERE Lower(coverage_name) = Lower(?)";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_null_srid, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateRasterCoverageExtent: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

    sql = "UPDATE raster_coverages_srid SET extent_minx = ?, "
	"extent_miny = ?, extent_maxx = ?, extent_maxy = ? "
	"WHERE Lower(coverage_name) = Lower(?) AND srid = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_upd_srid, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateRasterCoverageExtent: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

/* preparing the main SQL statement */
    if (coverage_name == NULL)
      {
	  sql = "SELECT coverage_name, srid FROM raster_coverages";
      }
    else
      {

	  sql = "SELECT coverage_name, srid FROM raster_coverages "
	      "WHERE Lower(coverage_name) = Lower(?)";
      }
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("updateRasterCoverageExtent: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto error;
      }

    if (transaction)
      {
	  /* starting a Transaction */
	  ret = sqlite3_exec (sqlite, "BEGIN", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	      goto error;
      }

    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    if (coverage_name != NULL)
	sqlite3_bind_text (stmt, 1, coverage_name, strlen (coverage_name),
			   SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* processing a single Raster Coverage */
		char *xtile_table;
		char *tile_table;
		const char *cvg = (const char *) sqlite3_column_text (stmt, 0);
		int natural_srid = sqlite3_column_int (stmt, 1);
		xtile_table = sqlite3_mprintf ("%s_tiles", cvg);
		tile_table = gaiaDoubleQuotedSql (xtile_table);
		sqlite3_free (xtile_table);
		sql =
		    sqlite3_mprintf
		    ("SELECT Min(MbrMinX(geometry)), Min(MbrMinY(geometry)), "
		     "Max(MbrMaxX(geometry)), Max(MbrMaxY(geometry)) FROM \"%s\"",
		     tile_table);
		free (tile_table);
		ret =
		    sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt_ext,
					NULL);
		sqlite3_free (sql);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("updateRasterCoverageExtent: \"%s\"\n",
				    sqlite3_errmsg (sqlite));
		      goto error;
		  }
		while (1)
		  {
		      /* scrolling the result set rows */
		      ret = sqlite3_step (stmt_ext);
		      if (ret == SQLITE_DONE)
			  break;	/* end of result set */
		      if (ret == SQLITE_ROW)
			{
			    int null_minx = 1;
			    int null_miny = 1;
			    int null_maxx = 1;
			    int null_maxy = 1;
			    double minx;
			    double miny;
			    double maxx;
			    double maxy;
			    if (sqlite3_column_type (stmt_ext, 0) ==
				SQLITE_FLOAT)
			      {
				  minx = sqlite3_column_double (stmt_ext, 0);
				  null_minx = 0;
			      }
			    if (sqlite3_column_type (stmt_ext, 1) ==
				SQLITE_FLOAT)
			      {
				  miny = sqlite3_column_double (stmt_ext, 1);
				  null_miny = 0;
			      }
			    if (sqlite3_column_type (stmt_ext, 2) ==
				SQLITE_FLOAT)
			      {
				  maxx = sqlite3_column_double (stmt_ext, 2);
				  null_maxx = 0;
			      }
			    if (sqlite3_column_type (stmt_ext, 3) ==
				SQLITE_FLOAT)
			      {
				  maxy = sqlite3_column_double (stmt_ext, 3);
				  null_maxy = 0;
			      }
			    if (null_minx || null_miny || null_maxx
				|| null_maxy)
				ret =
				    do_null_raster_coverage_extents (sqlite,
								     stmt_upd_cvg,
								     stmt_null_srid,
								     cvg);
			    else
				ret =
				    do_update_raster_coverage_extents (sqlite,
								       cache,
								       stmt_upd_cvg,
								       stmt_srid,
								       stmt_upd_srid,
								       cvg,
								       natural_srid,
								       minx,
								       miny,
								       maxx,
								       maxy);
			    if (!ret)
				goto error;
			}
		      else
			{
			    spatialite_e
				("updateRasterCoverageExtent() error: \"%s\"\n",
				 sqlite3_errmsg (sqlite));
			    goto error;
			}
		  }
		sqlite3_finalize (stmt_ext);
		stmt_ext = NULL;
	    }
	  else
	    {
		spatialite_e ("updateRasterCoverageExtent() error: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		goto error;
	    }
      }

    if (transaction)
      {
	  /* committing the still pending Transaction */
	  ret = sqlite3_exec (sqlite, "COMMIT", NULL, NULL, NULL);
	  if (ret != SQLITE_OK)
	      goto error;
      }

    sqlite3_finalize (stmt);
    sqlite3_finalize (stmt_upd_cvg);
    sqlite3_finalize (stmt_upd_srid);
    sqlite3_finalize (stmt_null_srid);
    sqlite3_finalize (stmt_srid);
    return 1;

  error:
    if (stmt != NULL)
	sqlite3_finalize (stmt);
    if (stmt_ext != NULL)
	sqlite3_finalize (stmt_ext);
    if (stmt_upd_cvg != NULL)
	sqlite3_finalize (stmt_upd_cvg);
    if (stmt_upd_srid != NULL)
	sqlite3_finalize (stmt_upd_srid);
    if (stmt_null_srid != NULL)
	sqlite3_finalize (stmt_null_srid);
    if (stmt_srid != NULL)
	sqlite3_finalize (stmt_srid);
    return 0;
}

SPATIALITE_PRIVATE int
get_iso_metadata_id (void *p_sqlite, const char *fileIdentifier, void *p_id)
{
/* auxiliary function: return the ID of the row corresponding to "fileIdentifier" */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 *p64 = (sqlite3_int64 *) p_id;
    sqlite3_int64 id;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int ok = 0;

    sql = "SELECT id FROM ISO_metadata WHERE fileId = ?";
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("getIsoMetadataId: \"%s\"\n", sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_text (stmt, 1, fileIdentifier, strlen (fileIdentifier),
		       SQLITE_STATIC);
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		ok++;
		id = sqlite3_column_int64 (stmt, 0);
	    }
      }
    sqlite3_finalize (stmt);

    if (ok == 1)
      {
	  *p64 = id;
	  return 1;
      }
  stop:
    return 0;
}

SPATIALITE_PRIVATE int
register_iso_metadata (void *p_sqlite, const char *scope,
		       const unsigned char *p_blob, int n_bytes, void *p_id,
		       const char *fileIdentifier)
{
/* auxiliary function: inserts or updates an ISO Metadata */
    sqlite3 *sqlite = (sqlite3 *) p_sqlite;
    sqlite3_int64 *p64 = (sqlite3_int64 *) p_id;
    sqlite3_int64 id = *p64;
    int ret;
    const char *sql;
    sqlite3_stmt *stmt;
    int exists = 0;
    int retval = 0;

    if (id >= 0)
      {
	  /* checking if already exists - by ID */
	  sql = "SELECT id FROM ISO_metadata WHERE id = ?";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("registerIsoMetadata: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		goto stop;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_int64 (stmt, 1, id);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		    exists = 1;
	    }
	  sqlite3_finalize (stmt);
      }
    if (fileIdentifier != NULL)
      {
	  /* checking if already exists - by fileIdentifier */
	  sql = "SELECT id FROM ISO_metadata WHERE fileId = ?";
	  ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("registerIsoMetadata: \"%s\"\n",
			      sqlite3_errmsg (sqlite));
		goto stop;
	    }
	  sqlite3_reset (stmt);
	  sqlite3_clear_bindings (stmt);
	  sqlite3_bind_text (stmt, 1, fileIdentifier, strlen (fileIdentifier),
			     SQLITE_STATIC);
	  while (1)
	    {
		/* scrolling the result set rows */
		ret = sqlite3_step (stmt);
		if (ret == SQLITE_DONE)
		    break;	/* end of result set */
		if (ret == SQLITE_ROW)
		  {
		      exists = 1;
		      id = sqlite3_column_int64 (stmt, 0);
		  }
	    }
	  sqlite3_finalize (stmt);
      }

    if (exists)
      {
	  /* update */
	  sql = "UPDATE ISO_metadata SET md_scope = ?, metadata = ? "
	      "WHERE id = ?";
      }
    else
      {
	  /* insert */
	  sql = "INSERT INTO ISO_metadata "
	      "(id, md_scope, metadata) VALUES (?, ?, ?)";
      }
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("registerIsoMetadata: \"%s\"\n",
			sqlite3_errmsg (sqlite));
	  goto stop;
      }
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    if (exists)
      {
	  /* update */
	  sqlite3_bind_text (stmt, 1, scope, strlen (scope), SQLITE_STATIC);
	  sqlite3_bind_blob (stmt, 2, p_blob, n_bytes, SQLITE_STATIC);
	  sqlite3_bind_int64 (stmt, 3, id);
      }
    else
      {
	  /* insert */
	  if (id < 0)
	      sqlite3_bind_null (stmt, 1);
	  else
	      sqlite3_bind_int64 (stmt, 1, id);
	  sqlite3_bind_text (stmt, 2, scope, strlen (scope), SQLITE_STATIC);
	  sqlite3_bind_blob (stmt, 3, p_blob, n_bytes, SQLITE_STATIC);
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	retval = 1;
    else
	spatialite_e ("registerIsoMetadata() error: \"%s\"\n",
		      sqlite3_errmsg (sqlite));
    sqlite3_finalize (stmt);
    return retval;
  stop:
    return 0;
}

#endif /* end including LIBXML2 */
