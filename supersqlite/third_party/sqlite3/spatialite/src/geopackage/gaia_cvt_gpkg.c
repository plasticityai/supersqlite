/*

 gaia_cvt_gpkg.c -- implements GPKG DB conversion [both directions]

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

Contributor(s): Brad Hards <bradh@frogmouth.net>

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

#include "config.h"

#ifdef ENABLE_GEOPACKAGE	/* enabling GeoPackage extensions */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <spatialite/sqlite.h>
#include <spatialite/debug.h>
#include <spatialite.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/geopackage.h>

struct pk_item
{
/* a struct wrapping a Primary Key item */
    int position;
    char *column;
    struct pk_item *next;
};

struct primary_key
{
/* a struct wrapping a Primary Key */
    struct pk_item *first;
    struct pk_item *last;
    int num_items;
    struct pk_item **array;
};

static struct primary_key *
alloc_pk ()
{
/* creating an empty Primary Key */
    struct primary_key *pk = malloc (sizeof (struct primary_key));
    pk->first = NULL;
    pk->last = NULL;
    pk->num_items = 0;
    pk->array = NULL;
    return pk;
}

static void
destroy_pk (struct primary_key *pk)
{
/* destroying a Primary Key */
    struct pk_item *pi;
    struct pk_item *pin;
    if (pk == NULL)
	return;
    pi = pk->first;
    while (pi != NULL)
      {
	  pin = pi->next;
	  if (pi->column != NULL)
	      free (pi->column);
	  free (pi);
	  pi = pin;
      }
    if (pk->array != NULL)
	free (pk->array);
    free (pk);
}

static void
add_to_pk (struct primary_key *pk, int position, const char *column)
{
/* appending an item to a Primary Key */
    int len;
    struct pk_item *pi;
    if (pk == NULL)
	return;
    pi = malloc (sizeof (struct pk_item));
    pi->position = position;
    len = strlen (column);
    pi->column = malloc (len + 1);
    strcpy (pi->column, column);
    pi->next = NULL;
    if (pk->first == NULL)
	pk->first = pi;
    if (pk->last != NULL)
	pk->last->next = pi;
    pk->last = pi;
    pk->num_items += 1;
}

static void
sort_pk (struct primary_key *pk)
{
/* sorting the PK items by increasing position */
    struct pk_item *pi;
    struct pk_item *pi2;
    int i;
    int ok = 1;
    if (pk == NULL)
	return;
    if (pk->array != NULL)
	free (pk->array);
    pk->array = NULL;
    if (pk->num_items <= 0)
	return;
    pk->array = malloc (sizeof (struct pk_item *) * pk->num_items);
    pi = pk->first;
    i = 0;
    while (pi != NULL)
      {
	  /* pre-loading the array */
	  *(pk->array + i++) = pi;
	  pi = pi->next;
      }
    while (ok)
      {
	  /* bubble sort */
	  ok = 0;
	  for (i = 1; i < pk->num_items; i++)
	    {
		pi = *(pk->array + (i - 1));
		pi2 = *(pk->array + i);
		if (pi->position > pi2->position)
		  {
		      /* swapping */
		      *(pk->array + (i - 1)) = pi2;
		      *(pk->array + i) = pi;
		      ok = 1;
		  }
	    }
      }
}

static int
create_gpkg_destination (sqlite3 * handle, const char *create_sql,
			 const char *table_name, const char *column_name,
			 const char *geometry_type, int has_z, int has_m,
			 int srid, int spatial_index)
{
/* attempting to create a GPKG destination table */
    int ret;
    char *sql_err = NULL;
    char *sql;

/* creating the table */
    ret = sqlite3_exec (handle, create_sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE \"%s\" error: %s\n", table_name, sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

/* adding the geometry column */
    sql =
	sqlite3_mprintf
	("SELECT gpkgAddGeometryColumn(Lower(%Q), Lower(%Q), %Q, %d, %d, %d)",
	 table_name, column_name, geometry_type, has_z, has_m, srid);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("gpkgAddGeometryColumn \"%s\" error: %s\n", table_name,
			sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

/* adding the geometry triggers */
    sql =
	sqlite3_mprintf
	("SELECT gpkgAddGeometryTriggers(Lower(%Q), Lower(%Q))",
	 table_name, column_name);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("gpkgAddGeometryTriggers \"%s\" error: %s\n",
			table_name, sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

    if (spatial_index)
      {
	  /* adding Spatial Index support */
	  sql =
	      sqlite3_mprintf
	      ("SELECT gpkgAddSpatialIndex(Lower(%Q), Lower(%Q))",
	       table_name, column_name);
	  ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("gpkgAddSpatialIndex \"%s\" error: %s\n",
			      table_name, sql_err);
		sqlite3_free (sql_err);
		return 0;
	    }
      }
    return 1;
}

static int
create_spatialite_destination (sqlite3 * handle, const char *create_sql,
			       const char *table_name, const char *column_name,
			       const char *geometry_type, const char *dims,
			       int srid)
{
/* attempting to create a Spatialite destination table */
    int ret;
    char *sql_err = NULL;
    char *sql;
    int i;
    char **results;
    int rows;
    int columns;
    int ok = 0;

/* creating the table */
    ret = sqlite3_exec (handle, create_sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE \"%s\" error: %s\n", table_name, sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

/* adding the geometry column */
    sql =
	sqlite3_mprintf
	("SELECT AddGeometryColumn(Lower(%Q), Lower(%Q), %d, %Q, %Q)",
	 table_name, column_name, srid, geometry_type, dims);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	      ok = atoi (results[(i * columns) + 0]);
      }
    sqlite3_free_table (results);
    if (!ok)
      {
	  spatialite_e ("AddGeometryColumn \"%s\": error\n", table_name);
	  return 0;
      }
    return 1;
}

static char *
prepare_create_table (sqlite3 * handle, const char *table_name,
		      const char *column_name)
{
/* building a CREATE TABLE statement */
    int ret;
    char *sql;
    int i;
    char **results;
    int rows;
    int columns;
    char *create_sql;
    char *prev_sql;
    char *xtable;
    int first = 1;
    struct primary_key *pk = alloc_pk ();

    xtable = gaiaDoubleQuotedSql (table_name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xtable);
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    create_sql = sqlite3_mprintf ("CREATE TABLE \"%s\" (", xtable);
    free (xtable);
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		const char *name;
		const char *type;
		int not_null;
		int pk_idx;
		char *xname;
		char *xtype;
		name = results[(i * columns) + 1];
		if (strcasecmp (name, column_name) == 0)
		  {
		      /* skipping the geometry column */
		      continue;
		  }
		type = results[(i * columns) + 2];
		not_null = atoi (results[(i * columns) + 3]);
		pk_idx = atoi (results[(i * columns) + 5]);
		if (pk_idx > 0)
		    add_to_pk (pk, pk_idx, name);
		xname = gaiaDoubleQuotedSql (name);
		xtype = gaiaDoubleQuotedSql (type);
		prev_sql = create_sql;
		if (first)
		  {
		      if (not_null)
			  create_sql =
			      sqlite3_mprintf ("%s\n\t\"%s\" \"%s\" NOT NULL",
					       prev_sql, xname, xtype);
		      else
			  create_sql =
			      sqlite3_mprintf ("%s\n\t\"%s\" \"%s\"", prev_sql,
					       xname, xtype);
		      first = 0;
		  }
		else
		  {
		      if (not_null)
			  create_sql =
			      sqlite3_mprintf ("%s,\n\t\"%s\" \"%s\" NOT NULL",
					       prev_sql, xname, xtype);
		      else
			  create_sql =
			      sqlite3_mprintf ("%s,\n\t\"%s\" \"%s\"", prev_sql,
					       xname, xtype);
		  }
		free (xname);
		free (xtype);
		sqlite3_free (prev_sql);
	    }
      }
    sqlite3_free_table (results);

    if (pk->num_items > 0)
      {
	  /* setting up the Primary Key (if any) */
	  char *pk_name;
	  char *xpk_name;
	  int idx;
	  sort_pk (pk);
	  prev_sql = create_sql;
	  pk_name = sqlite3_mprintf ("pk_%s", table_name);
	  xpk_name = gaiaDoubleQuotedSql (pk_name);
	  sqlite3_free (pk_name);
	  create_sql =
	      sqlite3_mprintf ("%s,\n\tCONSTRAINT \"%s\" PRIMARY KEY (",
			       prev_sql, xpk_name);
	  free (xpk_name);
	  sqlite3_free (prev_sql);
	  for (idx = 0; idx < pk->num_items; idx++)
	    {
		char *xcolumn;
		struct pk_item *pi = *(pk->array + idx);
		prev_sql = create_sql;
		xcolumn = gaiaDoubleQuotedSql (pi->column);
		if (idx == 0)
		    create_sql =
			sqlite3_mprintf ("%s\"%s\"", prev_sql, xcolumn);
		else
		    create_sql =
			sqlite3_mprintf ("%s, \"%s\"", prev_sql, xcolumn);
		free (xcolumn);
		sqlite3_free (prev_sql);
	    }
	  prev_sql = create_sql;
	  create_sql = sqlite3_mprintf ("%s)", prev_sql);
	  sqlite3_free (prev_sql);
      }
    destroy_pk (pk);

    prev_sql = create_sql;
    create_sql = sqlite3_mprintf ("%s)", prev_sql);
    sqlite3_free (prev_sql);
    return create_sql;
}

static int
create_Spatialite2GPKG_statements (sqlite3 * handle_in, sqlite3 * handle_out,
				   const char *table_name,
				   const char *geometry_column,
				   sqlite3_stmt ** stmt_in,
				   sqlite3_stmt ** stmt_out)
{
/* attempting to create the IN and OUT prepared stmts */
    int ret;
    char *sql;
    int i;
    char **results;
    int rows;
    int columns;
    char *in_sql;
    char *in2_sql;
    char *out_sql;
    char *out2_sql;
    char *prev_sql;
    char *prev2_sql;
    char *xtable;
    int first_in = 1;
    int first_out = 1;
    char *sql_err = NULL;
    sqlite3_stmt *xstmt_in;
    sqlite3_stmt *xstmt_out;

/* starting a transaction */
    ret = sqlite3_exec (handle_out, "BEGIN", NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("BEGIN TRANSACTION error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

/* building the IN and OUT sql expressions */
    xtable = gaiaDoubleQuotedSql (table_name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xtable);
    ret = sqlite3_get_table (handle_in, sql, &results, &rows, &columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    in_sql = sqlite3_mprintf ("SELECT");
    in2_sql = sqlite3_mprintf ("FROM \"%s\"", xtable);
    out_sql = sqlite3_mprintf ("INSERT INTO \"%s\" (", xtable);
    out2_sql = sqlite3_mprintf (") VALUES (");
    free (xtable);
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		const char *name;
		char *xname;
		name = results[(i * columns) + 1];
		xname = gaiaDoubleQuotedSql (name);
		if (strcasecmp (name, geometry_column) == 0)
		  {
		      /* the geometry column */
		      prev_sql = in_sql;
		      if (first_in)
			{
			    in_sql =
				sqlite3_mprintf ("%s AsGPB(\"%s\")", prev_sql,
						 xname);
			    first_in = 0;
			}
		      else
			{
			    in_sql =
				sqlite3_mprintf ("%s, AsGPB(\"%s\")", prev_sql,
						 xname);
			}
		      sqlite3_free (prev_sql);
		      prev_sql = out_sql;
		      prev2_sql = out2_sql;
		      if (first_out)
			{
			    out_sql =
				sqlite3_mprintf ("%s\"%s\"", prev_sql, xname);
			    out2_sql = sqlite3_mprintf ("%s?", prev2_sql);
			    first_out = 0;
			}
		      else
			{
			    out_sql =
				sqlite3_mprintf ("%s, \"%s\"", prev_sql, xname);
			    out2_sql = sqlite3_mprintf ("%s, ?", prev2_sql);
			}
		      sqlite3_free (prev_sql);
		      sqlite3_free (prev2_sql);
		      free (xname);
		      continue;
		  }
		prev_sql = in_sql;
		if (first_in)
		  {
		      in_sql = sqlite3_mprintf ("%s \"%s\"", prev_sql, xname);
		      first_in = 0;
		  }
		else
		  {
		      in_sql = sqlite3_mprintf ("%s, \"%s\"", prev_sql, xname);
		  }
		sqlite3_free (prev_sql);
		prev_sql = out_sql;
		prev2_sql = out2_sql;
		if (first_out)
		  {
		      out_sql = sqlite3_mprintf ("%s\"%s\"", prev_sql, xname);
		      out2_sql = sqlite3_mprintf ("%s?", prev2_sql);
		      first_out = 0;
		  }
		else
		  {
		      out_sql = sqlite3_mprintf ("%s, \"%s\"", prev_sql, xname);
		      out2_sql = sqlite3_mprintf ("%s, ?", prev2_sql);
		  }
		sqlite3_free (prev_sql);
		sqlite3_free (prev2_sql);
		free (xname);
	    }
      }
    sqlite3_free_table (results);
    prev_sql = in_sql;
    in_sql = sqlite3_mprintf ("%s %s", prev_sql, in2_sql);
    sqlite3_free (prev_sql);
    sqlite3_free (in2_sql);
    prev_sql = out_sql;
    out_sql = sqlite3_mprintf ("%s%s)", prev_sql, out2_sql);
    sqlite3_free (prev_sql);
    sqlite3_free (out2_sql);

/* preparing the IN stmt */
    ret =
	sqlite3_prepare_v2 (handle_in, in_sql, strlen (in_sql), &xstmt_in,
			    NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "SELECT FROM \"%s\" error: %s\n", table_name,
		   sqlite3_errmsg (handle_in));
	  goto stop;
      }

/* preparing the OUT stmt */
    ret =
	sqlite3_prepare_v2 (handle_out, out_sql, strlen (out_sql), &xstmt_out,
			    NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" error: %s\n", table_name,
		   sqlite3_errmsg (handle_out));
	  goto stop;
      }

    sqlite3_free (in_sql);
    sqlite3_free (out_sql);
    *stmt_in = xstmt_in;
    *stmt_out = xstmt_out;
    return 1;

  stop:
    sqlite3_free (in_sql);
    sqlite3_free (out_sql);
    if (xstmt_in != NULL)
	sqlite3_finalize (xstmt_in);
    if (xstmt_out != NULL)
	sqlite3_finalize (xstmt_out);
    return 0;
}

static int
create_GPKG2Spatialite_statements (sqlite3 * handle_in, sqlite3 * handle_out,
				   const char *table_name,
				   const char *geometry_column,
				   sqlite3_stmt ** stmt_in,
				   sqlite3_stmt ** stmt_out)
{
/* attempting to create the IN and OUT prepared stmts */
    int ret;
    char *sql;
    int i;
    char **results;
    int rows;
    int columns;
    char *in_sql;
    char *in2_sql;
    char *out_sql;
    char *out2_sql;
    char *prev_sql;
    char *prev2_sql;
    char *xtable;
    int first_in = 1;
    int first_out = 1;
    char *sql_err = NULL;
    sqlite3_stmt *xstmt_in;
    sqlite3_stmt *xstmt_out;

/* starting a transaction */
    ret = sqlite3_exec (handle_out, "BEGIN", NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("BEGIN TRANSACTION error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }

/* building the IN and OUT sql expressions */
    xtable = gaiaDoubleQuotedSql (table_name);
    sql = sqlite3_mprintf ("PRAGMA table_info(\"%s\")", xtable);
    ret = sqlite3_get_table (handle_in, sql, &results, &rows, &columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    in_sql = sqlite3_mprintf ("SELECT");
    in2_sql = sqlite3_mprintf ("FROM \"%s\"", xtable);
    out_sql = sqlite3_mprintf ("INSERT INTO \"%s\" (", xtable);
    out2_sql = sqlite3_mprintf (") VALUES (");
    free (xtable);
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		const char *name;
		char *xname;
		name = results[(i * columns) + 1];
		xname = gaiaDoubleQuotedSql (name);
		if (strcasecmp (name, geometry_column) == 0)
		  {
		      /* the geometry column */
		      prev_sql = in_sql;
		      if (first_in)
			{
			    in_sql =
				sqlite3_mprintf ("%s GeomFromGPB(\"%s\")",
						 prev_sql, xname);
			    first_in = 0;
			}
		      else
			{
			    in_sql =
				sqlite3_mprintf ("%s, GeomFromGPB(\"%s\")",
						 prev_sql, xname);
			}
		      sqlite3_free (prev_sql);
		      prev_sql = out_sql;
		      prev2_sql = out2_sql;
		      if (first_out)
			{
			    out_sql =
				sqlite3_mprintf ("%s\"%s\"", prev_sql, xname);
			    out2_sql = sqlite3_mprintf ("%s?", prev2_sql);
			    first_out = 0;
			}
		      else
			{
			    out_sql =
				sqlite3_mprintf ("%s, \"%s\"", prev_sql, xname);
			    out2_sql = sqlite3_mprintf ("%s, ?", prev2_sql);
			}
		      sqlite3_free (prev_sql);
		      sqlite3_free (prev2_sql);
		      free (xname);
		      continue;
		  }
		prev_sql = in_sql;
		if (first_in)
		  {
		      in_sql = sqlite3_mprintf ("%s \"%s\"", prev_sql, xname);
		      first_in = 0;
		  }
		else
		  {
		      in_sql = sqlite3_mprintf ("%s, \"%s\"", prev_sql, xname);
		  }
		sqlite3_free (prev_sql);
		prev_sql = out_sql;
		prev2_sql = out2_sql;
		if (first_out)
		  {
		      out_sql = sqlite3_mprintf ("%s\"%s\"", prev_sql, xname);
		      out2_sql = sqlite3_mprintf ("%s?", prev2_sql);
		      first_out = 0;
		  }
		else
		  {
		      out_sql = sqlite3_mprintf ("%s, \"%s\"", prev_sql, xname);
		      out2_sql = sqlite3_mprintf ("%s, ?", prev2_sql);
		  }
		sqlite3_free (prev_sql);
		sqlite3_free (prev2_sql);
		free (xname);
	    }
      }
    sqlite3_free_table (results);
    prev_sql = in_sql;
    in_sql = sqlite3_mprintf ("%s %s", prev_sql, in2_sql);
    sqlite3_free (prev_sql);
    sqlite3_free (in2_sql);
    prev_sql = out_sql;
    out_sql = sqlite3_mprintf ("%s%s)", prev_sql, out2_sql);
    sqlite3_free (prev_sql);
    sqlite3_free (out2_sql);

/* preparing the IN stmt */
    ret =
	sqlite3_prepare_v2 (handle_in, in_sql, strlen (in_sql), &xstmt_in,
			    NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "SELECT FROM \"%s\" error: %s\n", table_name,
		   sqlite3_errmsg (handle_in));
	  goto stop;
      }

/* preparing the OUT stmt */
    ret =
	sqlite3_prepare_v2 (handle_out, out_sql, strlen (out_sql), &xstmt_out,
			    NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT INTO \"%s\" error: %s\n", table_name,
		   sqlite3_errmsg (handle_out));
	  goto stop;
      }

    sqlite3_free (in_sql);
    sqlite3_free (out_sql);
    *stmt_in = xstmt_in;
    *stmt_out = xstmt_out;
    return 1;

  stop:
    sqlite3_free (in_sql);
    sqlite3_free (out_sql);
    if (xstmt_in != NULL)
	sqlite3_finalize (xstmt_in);
    if (xstmt_out != NULL)
	sqlite3_finalize (xstmt_out);
    return 0;
}

static int
do_copy_table (sqlite3 * handle_in, sqlite3 * handle_out,
	       sqlite3_stmt * stmt_in, sqlite3_stmt * stmt_out,
	       const char *table_name)
{
/* copying all rows from IN and OUT tables */
    int ret;
    char *sql_err = NULL;

    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt_in);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		int c;
		int cols = sqlite3_column_count (stmt_in);
		sqlite3_reset (stmt_out);
		sqlite3_clear_bindings (stmt_out);
		for (c = 0; c < cols; c++)
		  {
		      /* binding column values */
		      switch (sqlite3_column_type (stmt_in, c))
			{
			case SQLITE_INTEGER:
			    sqlite3_bind_int64 (stmt_out, c + 1,
						sqlite3_column_int64 (stmt_in,
								      c));
			    break;
			case SQLITE_FLOAT:
			    sqlite3_bind_double (stmt_out, c + 1,
						 sqlite3_column_double (stmt_in,
									c));
			    break;
			case SQLITE_TEXT:
			    sqlite3_bind_text (stmt_out, c + 1,
					       (const char *)
					       sqlite3_column_text (stmt_in, c),
					       sqlite3_column_bytes (stmt_in,
								     c),
					       SQLITE_STATIC);
			    break;
			case SQLITE_BLOB:
			    sqlite3_bind_blob (stmt_out, c + 1,
					       sqlite3_column_blob (stmt_in, c),
					       sqlite3_column_bytes (stmt_in,
								     c),
					       SQLITE_STATIC);
			    break;
			default:
			    sqlite3_bind_null (stmt_out, c + 1);
			    break;
			};
		  }
		ret = sqlite3_step (stmt_out);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      /* an unexpected error occurred */
		      spatialite_e ("Error while inserting into \"%s\": %s\n",
				    table_name, sqlite3_errmsg (handle_out));
		      goto stop;
		  }
	    }
	  else
	    {
		/* an unexpected error occurred */
		spatialite_e ("Error while querying from \"%s\": %s\n",
			      table_name, sqlite3_errmsg (handle_in));
		goto stop;
	    }
      }

/* committing the still pending transaction */
    ret = sqlite3_exec (handle_out, "COMMIT", NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("COMMIT TRANSACTION error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }
    return 1;

  stop:
/* invalidating the still pending transaction */
    ret = sqlite3_exec (handle_out, "ROLLBACK", NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("ROLLBACK TRANSACTION error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }
    return 0;
}

static int
do_insert_content (sqlite3 * handle, const char *table_name,
		   const char *geometry_column, int srid)
{
/* registering the GPKG table in GPKG_CONTENTS */
    char *sql;
    char *xgeom;
    char *xtable;
    int ret;
    char *sql_err = NULL;

    xtable = gaiaDoubleQuotedSql (table_name);
    xgeom = gaiaDoubleQuotedSql (geometry_column);
    sql = sqlite3_mprintf ("INSERT INTO gpkg_contents (table_name, data_type, "
			   "identifier, description, last_change, min_x, min_y, max_x, max_y, srs_id) "
			   "SELECT Lower(%Q), 'features', Lower(%Q), ' ', "
			   "strftime('%%Y-%%m-%%dT%%H:%%M:%%fZ', 'now'), Min(ST_MinX(\"%s\")), "
			   "Min(ST_MinY(\"%s\")), Max(ST_MaxX(\"%s\")), Max(ST_MaxY(\"%s\")), %d "
			   "FROM \"%s\"", table_name, table_name, xgeom, xgeom,
			   xgeom, xgeom, srid, xtable);
    free (xgeom);
    free (xtable);
    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("INSERT INTO gpkg_contents error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }
    return 1;
}

static int
copy_spatialite2GPKG (sqlite3 * handle_in, sqlite3 * handle_out, int legacy)
{
/* attempting to copy all Geometry Tables */
    int ret;
    const char *sql;
    int i;
    char **results;
    int rows;
    int columns;
    char *create_sql;
    sqlite3_stmt *stmt_in = NULL;
    sqlite3_stmt *stmt_out = NULL;

    if (legacy)
      {
	  sql = "SELECT f_table_name, f_geometry_column, type, "
	      "coord_dimension, srid, spatial_index_enabled "
	      "FROM geometry_columns";
      }
    else
      {
	  sql = "SELECT f_table_name, f_geometry_column, geometry_type, "
	      "srid, spatial_index_enabled FROM geometry_columns";
      }
    ret = sqlite3_get_table (handle_in, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		const char *table_name = NULL;
		const char *geometry_column = NULL;
		const char *geometry_type = NULL;
		int has_z = 0;
		int has_m = 0;
		int srid;
		int spatial_index;
		table_name = results[(i * columns) + 0];
		geometry_column = results[(i * columns) + 1];
		if (legacy)
		  {
		      const char *dims;
		      geometry_type = results[(i * columns) + 2];
		      if (strcasecmp (geometry_type, "GEOMETRYCOLLECTION") == 0)
			  geometry_type = "GEOMCOLLECTION";
		      dims = results[(i * columns) + 2];
		      if (strcasecmp (dims, "XYZ") == 0
			  || strcasecmp (dims, "XYZM") == 0)
			  has_z = 1;
		      if (strcasecmp (dims, "XYM") == 0
			  || strcasecmp (dims, "XYZM") == 0)
			  has_m = 1;
		      srid = atoi (results[(i * columns) + 4]);
		      spatial_index = atoi (results[(i * columns) + 5]);
		  }
		else
		  {
		      int gtype = atoi (results[(i * columns) + 2]);
		      switch (gtype)
			{
			case GAIA_POINT:
			case GAIA_POINTM:
			case GAIA_POINTZ:
			case GAIA_POINTZM:
			    geometry_type = "POINT";
			    break;
			case GAIA_LINESTRING:
			case GAIA_LINESTRINGM:
			case GAIA_LINESTRINGZ:
			case GAIA_LINESTRINGZM:
			    geometry_type = "LINESTRING";
			    break;
			case GAIA_POLYGON:
			case GAIA_POLYGONM:
			case GAIA_POLYGONZ:
			case GAIA_POLYGONZM:
			    geometry_type = "POLYGON";
			    break;
			case GAIA_MULTIPOINT:
			case GAIA_MULTIPOINTM:
			case GAIA_MULTIPOINTZ:
			case GAIA_MULTIPOINTZM:
			    geometry_type = "MULTIPOINT";
			    break;
			case GAIA_MULTILINESTRING:
			case GAIA_MULTILINESTRINGM:
			case GAIA_MULTILINESTRINGZ:
			case GAIA_MULTILINESTRINGZM:
			    geometry_type = "MULTILINESTRING";
			    break;
			case GAIA_MULTIPOLYGON:
			case GAIA_MULTIPOLYGONM:
			case GAIA_MULTIPOLYGONZ:
			case GAIA_MULTIPOLYGONZM:
			    geometry_type = "MULTIPOLYGON";
			    break;
			case GAIA_GEOMETRYCOLLECTION:
			case GAIA_GEOMETRYCOLLECTIONM:
			case GAIA_GEOMETRYCOLLECTIONZ:
			case GAIA_GEOMETRYCOLLECTIONZM:
			    geometry_type = "GEOMCOLLECTION";
			    break;
			case 0:
			case 1000:
			case 2000:
			case 3000:
			    geometry_type = "GEOMETRY";
			    break;
			};
		      switch (gtype)
			{
			case 2000:
			case GAIA_POINTM:
			case GAIA_LINESTRINGM:
			case GAIA_POLYGONM:
			case GAIA_MULTIPOINTM:
			case GAIA_MULTILINESTRINGM:
			case GAIA_MULTIPOLYGONM:
			case GAIA_GEOMETRYCOLLECTIONM:
			    has_m = 1;
			    break;
			case 1000:
			case GAIA_POINTZ:
			case GAIA_LINESTRINGZ:
			case GAIA_POLYGONZ:
			case GAIA_MULTIPOINTZ:
			case GAIA_MULTILINESTRINGZ:
			case GAIA_MULTIPOLYGONZ:
			case GAIA_GEOMETRYCOLLECTIONZ:
			    has_z = 1;
			    break;
			case 3000:
			case GAIA_POINTZM:
			case GAIA_LINESTRINGZM:
			case GAIA_POLYGONZM:
			case GAIA_MULTIPOINTZM:
			case GAIA_MULTILINESTRINGZM:
			case GAIA_MULTIPOLYGONZM:
			case GAIA_GEOMETRYCOLLECTIONZM:
			    has_z = 1;
			    has_m = 1;
			    break;
			};
		      srid = atoi (results[(i * columns) + 3]);
		      spatial_index = atoi (results[(i * columns) + 4]);
		  }
		create_sql =
		    prepare_create_table (handle_in, table_name,
					  geometry_column);
		if (create_sql == NULL)
		  {
		      /* error: unable to create CREATE TABLE sql statement */
		      sqlite3_free_table (results);
		      return 0;
		  }
		if (!create_gpkg_destination
		    (handle_out, create_sql, table_name, geometry_column,
		     geometry_type, has_z, has_m, srid, spatial_index))
		  {
		      /* error: unable to create the target destination */
		      sqlite3_free (create_sql);
		      sqlite3_free_table (results);
		      return 0;
		  }
		sqlite3_free (create_sql);
		if (!create_Spatialite2GPKG_statements
		    (handle_in, handle_out, table_name, geometry_column,
		     &stmt_in, &stmt_out))
		  {
		      /* error: unable to create the IN and OUT stmts */
		      sqlite3_free_table (results);
		      return 0;
		  }
		if (!do_copy_table
		    (handle_in, handle_out, stmt_in, stmt_out, table_name))
		  {
		      sqlite3_finalize (stmt_in);
		      sqlite3_finalize (stmt_out);
		      sqlite3_free_table (results);
		      return 0;
		  }
		sqlite3_finalize (stmt_in);
		sqlite3_finalize (stmt_out);
		if (!do_insert_content
		    (handle_out, table_name, geometry_column, srid))
		  {
		      sqlite3_free_table (results);
		      return 0;
		  }
	    }
      }
    sqlite3_free_table (results);
    return 1;
}

static int
copy_GPKG2Spatialite (sqlite3 * handle_in, sqlite3 * handle_out)
{
/* attempting to copy all Geometry Tables */
    int ret;
    const char *sql;
    int i;
    char **results;
    int rows;
    int columns;
    char *create_sql;
    sqlite3_stmt *stmt_in = NULL;
    sqlite3_stmt *stmt_out = NULL;

    sql = "SELECT table_name, column_name, geometry_type_name, "
	"srs_id, z, m FROM gpkg_geometry_columns";
    ret = sqlite3_get_table (handle_in, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		const char *table_name;
		const char *geometry_column;
		const char *geometry_type;
		int has_z = 0;
		int has_m = 0;
		int srid;
		const char *dims = "XY";
		table_name = results[(i * columns) + 0];
		geometry_column = results[(i * columns) + 1];
		geometry_type = results[(i * columns) + 2];
		if (strcasecmp (geometry_type, "GEOMCOLLECTION") == 0)
		    geometry_type = "GEOMETRYCOLLECTION";
		srid = atoi (results[(i * columns) + 3]);
		has_z = atoi (results[(i * columns) + 4]);
		has_m = atoi (results[(i * columns) + 5]);
		if (has_z && has_m)
		    dims = "XYZM";
		else if (has_z)
		    dims = "XYZ";
		else if (has_m)
		    dims = "XYM";
		create_sql =
		    prepare_create_table (handle_in, table_name,
					  geometry_column);
		if (create_sql == NULL)
		  {
		      /* error: unable to create CREATE TABLE sql statement */
		      sqlite3_free_table (results);
		      return 0;
		  }
		if (!create_spatialite_destination
		    (handle_out, create_sql, table_name, geometry_column,
		     geometry_type, dims, srid))
		  {
		      /* error: unable to create the target destination */
		      sqlite3_free (create_sql);
		      sqlite3_free_table (results);
		      return 0;
		  }
		sqlite3_free (create_sql);
		if (!create_GPKG2Spatialite_statements
		    (handle_in, handle_out, table_name, geometry_column,
		     &stmt_in, &stmt_out))
		  {
		      /* error: unable to create the IN and OUT stmts */
		      sqlite3_free_table (results);
		      return 0;
		  }
		if (!do_copy_table
		    (handle_in, handle_out, stmt_in, stmt_out, table_name))
		  {
		      sqlite3_finalize (stmt_in);
		      sqlite3_finalize (stmt_out);
		      sqlite3_free_table (results);
		      return 0;
		  }
		sqlite3_finalize (stmt_in);
		sqlite3_finalize (stmt_out);
	    }
      }
    sqlite3_free_table (results);
    return 1;
}

static int
check_valid_gpkg (sqlite3 * handle)
{
/* checking for a valid GPKG origin */
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    int result;
    const char *sql;

/* checking Metadata tables */
    sql = "SELECT CheckGeoPackageMetaData()";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    result = 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	      result = atoi (results[(i * columns) + 0]);
      }
    sqlite3_free_table (results);
    if (result != 1)
      {
	  /* not presenting acceptable Metadata tables */
	  return 0;
      }

/* testing that at least one Geometry is actually defined */
    sql = "SELECT Count(*) FROM gpkg_geometry_columns";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    result = 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	      result = atoi (results[(i * columns) + 0]);
      }
    sqlite3_free_table (results);
    if (result <= 0)
	return 0;
    return 1;
}

static int
check_valid_spatialite (sqlite3 * handle, int *legacy)
{
/* checking for a valid SpatiaLite origin */
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    int result;
    const char *sql;

/* checking Metadata tables */
    sql = "SELECT CheckSpatialMetaData()";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    result = 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	      result = atoi (results[(i * columns) + 0]);
      }
    sqlite3_free_table (results);
    if (result == 1 || result == 3)
	;
    else
      {
	  /* not presenting acceptable Metadata tables */
	  return 0;
      }
    if (result == 1)
	*legacy = 1;
    else
	*legacy = 0;

/* testing that at least one Geometry is actually defined */
    sql = "SELECT Count(*) FROM geometry_columns";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    result = 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	      result = atoi (results[(i * columns) + 0]);
      }
    sqlite3_free_table (results);
    if (result <= 0)
	return 0;
    return 1;
}

static int
initialize_gpkg (sqlite3 * handle)
{
/* attempting to initialize a GPKG destination */
    int ret;
    const char *sql;
    char *sql_err = NULL;

/* creating Metadata tables */
    sql = "SELECT gpkgCreateBaseTables()";

    ret = sqlite3_exec (handle, sql, NULL, NULL, &sql_err);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("gpkgCreateBaseTables error: %s\n", sql_err);
	  sqlite3_free (sql_err);
	  return 0;
      }
    return 1;
}

static int
initialize_spatialite (sqlite3 * handle)
{
/* attempting to initialize a SpatiaLite destination */
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    int result;
    const char *sql;

/* creating Metadata tables */
    sql = "SELECT InitSpatialMetaData(1)";
    ret = sqlite3_get_table (handle, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	return 0;
    result = 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	      result = atoi (results[(i * columns) + 0]);
      }
    sqlite3_free_table (results);
    return result;
}

SPATIALITE_DECLARE int
gaiaGPKG2Spatialite (sqlite3 * handle_in, const char *gpkg_in_path,
		     sqlite3 * handle_out, const char *splite_out_path)
{
/* attempting to create a SpatiaLite DB by converting a GPKG DB */
    if (handle_in == NULL)
	goto error;
    if (!check_valid_gpkg (handle_in))
      {
	  /* not a valid IN GPKG: quitting */
	  spatialite_e ("Invalid GPKG origin:\n\"%s\"\n", gpkg_in_path);
	  goto error;
      }
    if (handle_out == NULL)
	goto error;
    if (!initialize_spatialite (handle_out))
      {
	  /* unable to initialize a valid SpatiaLite DB: quitting */
	  spatialite_e
	      ("Unable to initialize SpatiaLite destination:\n\"%s\"\n",
	       splite_out_path);
	  goto error;
      }

/* copying/converting all Spatial tables */
    if (!copy_GPKG2Spatialite (handle_in, handle_out))
	goto error;
    return 1;

  error:
    spatialite_e ("Conversion aborted due to previous error(s)\n");
    return 0;
}

SPATIALITE_DECLARE int
gaiaSpatialite2GPKG (sqlite3 * handle_in, const char *splite_in_path,
		     sqlite3 * handle_out, const char *gpkg_out_path)
{
/* attempting to create a GPKG DB by converting a SpatiaLite DB */
    int legacy;

    if (handle_in == NULL)
	goto error;
    if (!check_valid_spatialite (handle_in, &legacy))
      {
	  /* not a valid IN GPKG: quitting */
	  spatialite_e ("Invalid SpatiaLite origin:\n\"%s\"\n", splite_in_path);
	  goto error;
      }
    if (handle_out == NULL)
	goto error;
    if (!initialize_gpkg (handle_out))
      {
	  /* unable to initialize a valid GPKG DB: quitting */
	  spatialite_e ("Unable to initialize GPKG destination:\n\"%s\"\n",
			gpkg_out_path);
	  goto error;
      }

/* copying/converting all Spatial tables */
    if (!copy_spatialite2GPKG (handle_in, handle_out, legacy))
	goto error;
    return 1;

  error:
    spatialite_e ("Conversion aborted due to previous error(s)\n");
    return 0;
}

#endif /* end enabling GeoPackage extensions */
