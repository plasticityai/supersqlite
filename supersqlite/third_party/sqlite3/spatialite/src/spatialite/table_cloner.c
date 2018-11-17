/*

 table_cloner.c -- Cloning a Table

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

#include <spatialite.h>
#include <spatialite_private.h>
#include <spatialite/gaiaaux.h>

#ifdef _WIN32
#define strcasecmp	_stricmp
#define strncasecmp	_strnicmp
#endif /* not WIN32 */

struct aux_geometry
{
/* a Geometry column object */
    int type;
    int dims;
    int srid;
    int spatial_index;
    int cast2multi;
    int already_existing;
};

struct aux_trigger
{
/* a Trigger object */
    char *name;
    char *sql;
    int already_existing;
    struct aux_trigger *next;
};

struct aux_fk_columns
{
/* a Foreign Key Columns object */
    char *from;
    char *to;
    struct aux_fk_columns *next;
};

struct aux_foreign_key
{
/* a Foreign Key object */
    int id;
    char *name;
    char *references;
    char *on_update;
    char *on_delete;
    char *match;
    struct aux_fk_columns *first;
    struct aux_fk_columns *last;
    struct aux_foreign_key *next;
};

struct aux_index_column
{
/* an Index Column object */
    char *name;
    struct aux_index_column *next;
};

struct aux_index
{
/* a Table Index object */
    char *name;
    int unique;
    struct aux_index_column *first;
    struct aux_index_column *last;
    struct aux_index *next;
};

struct aux_column
{
/* a Table Column object */
    char *name;
    char *type;
    int notnull;
    char *deflt;
    int pk;
    int fk;
    int idx;
    struct aux_geometry *geometry;
    int ignore;
    int already_existing;
    int mismatching;
    struct aux_column *next;
};

struct aux_pk_column
{
/* Primary Key Columns */
    struct aux_column *column;
    struct aux_pk_column *next;
};

struct aux_cloner
{
/* the main Cloner object */
    sqlite3 *sqlite;
    char *db_prefix;
    char *in_table;
    char *out_table;
    struct aux_column *first_col;
    struct aux_column *last_col;
    struct aux_pk_column *first_pk;
    struct aux_pk_column *last_pk;
    struct aux_index *first_idx;
    struct aux_index *last_idx;
    struct aux_foreign_key *first_fk;
    struct aux_foreign_key *last_fk;
    struct aux_trigger *first_trigger;
    struct aux_trigger *last_trigger;
    struct aux_pk_column **sorted_pks;
    int pk_count;
    int autoincrement;
    int resequence;
    int with_fks;
    int with_triggers;
    int append;
    int already_existing;
};

static int
create_column (sqlite3 * sqlite, const char *table, struct aux_column *column)
{
/* creating a further ordinary Column */
    char *sql;
    char *err_msg = NULL;
    int ret;
    char *xtable;
    char *xcolumn;

    xtable = gaiaDoubleQuotedSql (table);
    xcolumn = gaiaDoubleQuotedSql (column->name);
    if (column->notnull)
      {
	  if (column->deflt != NULL)
	      sql = sqlite3_mprintf ("ALTER TABLE main.\"%s\" "
				     "ADD COLUMN \"%s\" %s NOT NULL DEFAULT %s",
				     xtable, xcolumn, column->type,
				     column->deflt);
	  else
	      sql = sqlite3_mprintf ("ALTER TABLE main.\"%s\" "
				     "ADD COLUMN \"%s\" %s NOT NULL", xtable,
				     xcolumn, column->type);
      }
    else
      {
	  if (column->deflt != NULL)
	      sql = sqlite3_mprintf ("ALTER TABLE main.\"%s\" "
				     "ADD COLUMN \"%s\" %s DEFAULT %s", xtable,
				     xcolumn, column->type, column->deflt);
	  else
	      sql = sqlite3_mprintf ("ALTER TABLE main.\"%s\" "
				     "ADD COLUMN \"%s\" %s", xtable, xcolumn,
				     column->type);
      }
    free (xtable);
    free (xcolumn);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("ALTER TABLE ADD COLUMN error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }
    return 1;
}

static int
create_geometry (sqlite3 * sqlite, const char *table, struct aux_column *column)
{
/* creating a further Geometry Column */
    char *sql;
    char *err_msg = NULL;
    int ret;
    char *xtable;
    char *xcolumn;
    const char *type = "GEOMETRY";
    const char *dims = "XY";
    int gtype = column->geometry->type;

    if (column->geometry->cast2multi)
      {
	  /* promoting to the corresponding MultiType */
	  switch (column->geometry->type)
	    {
	    case 1:
		gtype = 4;
		break;
	    case 1001:
		gtype = 1004;
		break;
	    case 2001:
		gtype = 2004;
		break;
	    case 3001:
		gtype = 3004;
		break;
	    case 2:
		gtype = 5;
		break;
	    case 1002:
		gtype = 1005;
		break;
	    case 2002:
		gtype = 2005;
		break;
	    case 3002:
		gtype = 3005;
		break;
	    case 3:
		gtype = 6;
		break;
	    case 1003:
		gtype = 1006;
		break;
	    case 2003:
		gtype = 2006;
		break;
	    case 3003:
		gtype = 3006;
		break;
	    };
      }

    switch (gtype)
      {
      case 1:
	  type = "POINT";
	  dims = "XY";
	  break;
      case 1001:
	  type = "POINT";
	  dims = "XYZ";
	  break;
      case 2001:
	  type = "POINT";
	  dims = "XYM";
	  break;
      case 3001:
	  type = "POINT";
	  dims = "XYZM";
	  break;
      case 2:
	  type = "LINESTRING";
	  dims = "XY";
	  break;
      case 1002:
	  type = "LINESTRING";
	  dims = "XYZ";
	  break;
      case 2002:
	  type = "LINESTRING";
	  dims = "XYM";
	  break;
      case 3002:
	  type = "LINESTRING";
	  dims = "XYZM";
	  break;
      case 3:
	  type = "POLYGON";
	  dims = "XY";
	  break;
      case 1003:
	  type = "POLYGON";
	  dims = "XYZ";
	  break;
      case 2003:
	  type = "POLYGON";
	  dims = "XYM";
	  break;
      case 3003:
	  type = "POLYGON";
	  dims = "XYZM";
	  break;
      case 4:
	  type = "MULTIPOINT";
	  dims = "XY";
	  break;
      case 1004:
	  type = "MULTIPOINT";
	  dims = "XYZ";
	  break;
      case 2004:
	  type = "MULTIPOINT";
	  dims = "XYM";
	  break;
      case 3004:
	  type = "MULTIPOINT";
	  dims = "XYZM";
	  break;
      case 5:
	  type = "MULTILINESTRING";
	  dims = "XY";
	  break;
      case 1005:
	  type = "MULTILINESTRING";
	  dims = "XYZ";
	  break;
      case 2005:
	  type = "MULTILINESTRING";
	  dims = "XYM";
	  break;
      case 3005:
	  type = "MULTILINESTRING";
	  dims = "XYZM";
	  break;
      case 6:
	  type = "MULTIPOLYGON";
	  dims = "XY";
	  break;
      case 1006:
	  type = "MULTIPOLYGON";
	  dims = "XYZ";
	  break;
      case 2006:
	  type = "MULTIPOLYGON";
	  dims = "XYM";
	  break;
      case 3006:
	  type = "MULTIPOLYGON";
	  dims = "XYZM";
	  break;
      case 7:
	  type = "GEOMETRYCOLLECTION";
	  dims = "XY";
	  break;
      case 1007:
	  type = "GEOMETRYCOLLECTION";
	  dims = "XYZ";
	  break;
      case 2007:
	  type = "GEOMETRYCOLLECTION";
	  dims = "XYM";
	  break;
      case 3007:
	  type = "GEOMETRYCOLLECTION";
	  dims = "XYZM";
	  break;
      case 0:
	  type = "GEOMETRY";
	  dims = "XY";
	  break;
      case 1000:
	  type = "GEOMETRY";
	  dims = "XYZ";
	  break;
      case 2000:
	  type = "GEOMETRY";
	  dims = "XYM";
	  break;
      case 3000:
	  type = "GEOMETRY";
	  dims = "XYZM";
	  break;
      };

    xtable = gaiaDoubleQuotedSql (table);
    xcolumn = gaiaDoubleQuotedSql (column->name);
    if (column->notnull)
      {
	  sql = sqlite3_mprintf ("SELECT AddGeometryColumn(Lower(%Q), "
				 "Lower(%Q), %d, %Q, %Q, 1)", xtable, xcolumn,
				 column->geometry->srid, type, dims);
      }
    else
      {
	  sql = sqlite3_mprintf ("SELECT AddGeometryColumn(Lower(%Q), "
				 "Lower(%Q), %d, %Q, %Q)", xtable, xcolumn,
				 column->geometry->srid, type, dims);
      }
    free (xtable);
    free (xcolumn);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("ADD GEOMETRY COLUMN error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

    if (column->geometry->spatial_index)
      {
	  /* creating the corresponding Spatial Index */
	  xtable = gaiaDoubleQuotedSql (table);
	  xcolumn = gaiaDoubleQuotedSql (column->name);
	  sql = sqlite3_mprintf ("SELECT CreateSpatialIndex("
				 "Lower(%Q), Lower(%Q))", xtable, xcolumn);
	  free (xtable);
	  free (xcolumn);
	  ret = sqlite3_exec (sqlite, sql, NULL, NULL, &err_msg);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("CREATE SPATIAL INDEX error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return 0;
	    }
      }
    return 1;
}

static int
upgrade_output_table (struct aux_cloner *cloner)
{
/* creating any new Column required by APPEND */
    struct aux_column *column = cloner->first_col;
    while (column != NULL)
      {
	  if (column->ignore)
	    {
		/* skipping columns marked to be ignored */
		column = column->next;
		continue;
	    }
	  if (column->already_existing == 0)
	    {
		if (column->geometry != NULL)
		  {
		      /* creating a Geometry */
		      if (!create_geometry
			  (cloner->sqlite, cloner->out_table, column))
			{
			    spatialite_e
				("CloneTable: unable to ADD Geometry COLUMN \"%s\" on Table \"%s\"\n",
				 column->name, cloner->out_table);
			    return 0;
			}
		  }
		else
		  {
		      /* creating an ordinary Column */
		      if (!create_column
			  (cloner->sqlite, cloner->out_table, column))
			{
			    spatialite_e
				("CloneTable: unable to ADD COLUMN \"%s\" on Table \"%s\"\n",
				 column->name, cloner->out_table);
			    return 0;
			}
		  }
	    }
	  column = column->next;
      }
    return 1;
}

static void
sort_pk_columns (struct aux_cloner *cloner)
{
/* sorting the PK columns (if required) */
    struct aux_pk_column *ppk;
    int cnt;
    int ok;
    if (cloner->pk_count <= 1)
	return;
    cloner->sorted_pks =
	malloc (sizeof (struct aux_pk_column *) * cloner->pk_count);
    cnt = 0;
    ppk = cloner->first_pk;
    while (ppk != NULL)
      {
	  /* copying pointers to PK cols */
	  *(cloner->sorted_pks + cnt++) = ppk;
	  ppk = ppk->next;
      }
    ok = 1;
    while (ok)
      {
	  /* bubble sorting */
	  ok = 0;
	  for (cnt = 1; cnt < cloner->pk_count; cnt++)
	    {
		struct aux_pk_column *ppk1 = *(cloner->sorted_pks + cnt - 1);
		ppk = *(cloner->sorted_pks + cnt);
		if (ppk1->column->pk > ppk->column->pk)
		  {
		      /* swapping */
		      *(cloner->sorted_pks + cnt - 1) = ppk;
		      *(cloner->sorted_pks + cnt) = ppk1;
		      ok = 1;
		  }
	    }
      }
}

static void
adjust_ignore (struct aux_cloner *cloner)
{
/* adjusting Ignore columns */
    struct aux_column *column = cloner->first_col;
    while (column != NULL)
      {
	  if (column->ignore)
	    {
		if (column->pk)
		    column->ignore = 0;
		if (column->fk && cloner->with_fks)
		    column->ignore = 0;
		if (column->idx)
		    column->ignore = 0;
	    }
	  column = column->next;
      }
}

static const char *
get_pk_column (struct aux_cloner *cloner, int index)
{
/* returning a PK column name (by sorted index) */
    struct aux_pk_column *ppk;
    if (cloner->sorted_pks == NULL)
	return NULL;
    if (index < 0 || index >= cloner->pk_count)
	return NULL;
    ppk = *(cloner->sorted_pks + index);
    return ppk->column->name;
}

static void
mark_existing_trigger (struct aux_cloner *cloner, const char *name)
{
/* marking an existing Trigger */
    struct aux_trigger *trigger = cloner->first_trigger;
    while (trigger != NULL)
      {
	  if (strcasecmp (trigger->name, name) == 0)
	    {
		trigger->already_existing = 1;
		return;
	    }
	  trigger = trigger->next;
      }
}

static void
check_existing_triggers (struct aux_cloner *cloner)
{
/* exploring the output table - already existing Triggers */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    const char *name;

    sql = sqlite3_mprintf ("SELECT name FROM main.sqlite_master "
			   "WHERE type = 'trigger' AND Lower(tbl_name) = Lower(%Q)",
			   cloner->in_table);
    ret =
	sqlite3_get_table (cloner->sqlite, sql, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 0];
		mark_existing_trigger (cloner, name);
	    }
      }
    sqlite3_free_table (results);
}

static int
create_output_table (struct aux_cloner *cloner)
{
/* creating the output Table */
    char *err_msg = NULL;
    int ret;
    struct aux_column *column;
    struct aux_foreign_key *fk;
    struct aux_fk_columns *fk_col;
    struct aux_index *index;
    struct aux_index_column *idx_column;
    char *sql;
    char *prev_sql;
    char *xtable;
    char *xcolumn;
    char *constraint;
    char *xconstraint;
    int first = 1;
    int i;
    int fk_no;

    sort_pk_columns (cloner);
    adjust_ignore (cloner);
    xtable = gaiaDoubleQuotedSql (cloner->out_table);
    sql = sqlite3_mprintf ("CREATE TABLE main.\"%s\"", xtable);
    free (xtable);
    prev_sql = sql;

    column = cloner->first_col;
    while (column != NULL)
      {
	  if (column->ignore)
	    {
		/* IGNORE requested */
		column = column->next;
		continue;
	    }
	  if (column->geometry != NULL)
	    {
		/* skipping any Geometry column */
		column = column->next;
		continue;
	    }
	  if (first)
	    {
		sql = sqlite3_mprintf ("%s (\n", prev_sql);
		first = 0;
	    }
	  else
	      sql = sqlite3_mprintf ("%s,\n", prev_sql);
	  sqlite3_free (prev_sql);
	  prev_sql = sql;
	  xcolumn = gaiaDoubleQuotedSql (column->name);
	  if (cloner->pk_count == 1 && column->pk)
	    {
		/* immediately declaring a single column Primary Key */
		if (cloner->autoincrement)
		  {
		      if (column->notnull)
			{
			    if (column->deflt != NULL)
				sql =
				    sqlite3_mprintf
				    ("%s\t\"%s\" %s NOT NULL PRIMARY KEY AUTOINCREMENT DEFAULT %s",
				     prev_sql, xcolumn, column->type,
				     column->deflt);
			    else
				sql =
				    sqlite3_mprintf
				    ("%s\t\"%s\" %s NOT NULL PRIMARY KEY AUTOINCREMENT",
				     prev_sql, xcolumn, column->type);
			}
		      else
			{
			    if (column->deflt != NULL)
				sql =
				    sqlite3_mprintf
				    ("%s\t\"%s\" %s PRIMARY KEY AUTOINCREMENT DEFAULT %s",
				     prev_sql, xcolumn, column->type,
				     column->deflt);
			    else
				sql =
				    sqlite3_mprintf
				    ("%s\t\"%s\" %s PRIMARY KEY AUTOINCREMENT",
				     prev_sql, xcolumn, column->type);
			}
		  }
		else
		  {
		      if (column->notnull)
			{
			    if (column->deflt != NULL)
				sql =
				    sqlite3_mprintf
				    ("%s\t\"%s\" %s NOT NULL PRIMARY KEY DEFAULT %s",
				     prev_sql, xcolumn, column->type,
				     column->deflt);
			    else
				sql =
				    sqlite3_mprintf
				    ("%s\t\"%s\" %s NOT NULL PRIMARY KEY",
				     prev_sql, xcolumn, column->type);
			}
		      else
			{
			    if (column->deflt != NULL)
				sql =
				    sqlite3_mprintf
				    ("%s\t\"%s\" %s PRIMARY KEY DEFAULT %s",
				     prev_sql, xcolumn, column->type,
				     column->deflt);
			    else
				sql =
				    sqlite3_mprintf
				    ("%s\t\"%s\" %s PRIMARY KEY", prev_sql,
				     xcolumn, column->type);
			}
		  }
		free (xcolumn);
		sqlite3_free (prev_sql);
		prev_sql = sql;
		column = column->next;
		continue;
	    }
	  if (column->notnull)
	    {
		if (column->deflt != NULL)
		    sql = sqlite3_mprintf ("%s\t\"%s\" %s NOT NULL DEFAULT %s",
					   prev_sql, xcolumn, column->type,
					   column->deflt);
		else
		    sql = sqlite3_mprintf ("%s\t\"%s\" %s NOT NULL",
					   prev_sql, xcolumn, column->type);
	    }
	  else
	    {
		if (column->deflt != NULL)
		    sql = sqlite3_mprintf ("%s\t\"%s\" %s DEFAULT %s",
					   prev_sql, xcolumn, column->type,
					   column->deflt);
		else
		    sql = sqlite3_mprintf ("%s\t\"%s\" %s",
					   prev_sql, xcolumn, column->type);
	    }
	  free (xcolumn);
	  sqlite3_free (prev_sql);
	  prev_sql = sql;
	  column = column->next;
      }

    if (cloner->pk_count > 1)
      {
	  /* declaring a PRIMARY KEY CONSTRAINT */
	  sql = sqlite3_mprintf ("%s,\n", prev_sql);
	  sqlite3_free (prev_sql);
	  prev_sql = sql;
	  constraint = sqlite3_mprintf ("pk_%s", cloner->out_table);
	  xconstraint = gaiaDoubleQuotedSql (constraint);
	  sqlite3_free (constraint);
	  sql =
	      sqlite3_mprintf ("%s\tCONSTRAINT \"%s\" PRIMARY KEY (", prev_sql,
			       xconstraint);
	  free (xconstraint);
	  sqlite3_free (prev_sql);
	  prev_sql = sql;
	  for (i = 0; i < cloner->pk_count; i++)
	    {
		xconstraint = gaiaDoubleQuotedSql (get_pk_column (cloner, i));
		if (i == 0)
		    sql = sqlite3_mprintf ("%s%s", prev_sql, xconstraint);
		else
		    sql = sqlite3_mprintf ("%s, %s", prev_sql, xconstraint);
		free (xconstraint);
		sqlite3_free (prev_sql);
		prev_sql = sql;
	    }
	  sql = sqlite3_mprintf ("%s)", prev_sql);
	  sqlite3_free (prev_sql);
	  prev_sql = sql;
      }

    if (cloner->with_fks)
      {
	  /* cloning all Foreign Key definitions */
	  fk = cloner->first_fk;
	  fk_no = 1;
	  while (fk != NULL)
	    {
		/* declaring all FOREIGN KEY CONSTRAINTs */
		sql = sqlite3_mprintf ("%s,\n", prev_sql);
		sqlite3_free (prev_sql);
		prev_sql = sql;
		constraint =
		    sqlite3_mprintf ("fk_%s_%d", cloner->out_table, fk_no++);
		xconstraint = gaiaDoubleQuotedSql (constraint);
		sqlite3_free (constraint);
		sql =
		    sqlite3_mprintf ("%s\tCONSTRAINT \"%s\" FOREIGN KEY (",
				     prev_sql, xconstraint);
		free (xconstraint);
		sqlite3_free (prev_sql);
		prev_sql = sql;
		fk_col = fk->first;
		while (fk_col != NULL)
		  {
		      xconstraint = gaiaDoubleQuotedSql (fk_col->from);
		      if (fk_col == fk->first)
			  sql = sqlite3_mprintf ("%s%s", prev_sql, xconstraint);
		      else
			  sql =
			      sqlite3_mprintf ("%s, %s", prev_sql, xconstraint);
		      free (xconstraint);
		      sqlite3_free (prev_sql);
		      prev_sql = sql;
		      fk_col = fk_col->next;
		  }
		xtable = gaiaDoubleQuotedSql (fk->references);
		sql =
		    sqlite3_mprintf ("%s) REFERENCES \"%s\" (", prev_sql,
				     xtable);
		free (xtable);
		sqlite3_free (prev_sql);
		prev_sql = sql;
		fk_col = fk->first;
		while (fk_col != NULL)
		  {
		      xconstraint = gaiaDoubleQuotedSql (fk_col->to);
		      if (fk_col == fk->first)
			  sql = sqlite3_mprintf ("%s%s", prev_sql, xconstraint);
		      else
			  sql =
			      sqlite3_mprintf ("%s, %s", prev_sql, xconstraint);
		      free (xconstraint);
		      sqlite3_free (prev_sql);
		      prev_sql = sql;
		      fk_col = fk_col->next;
		  }
		sql = sqlite3_mprintf ("%s)", prev_sql);
		sqlite3_free (prev_sql);
		prev_sql = sql;
		fk = fk->next;
	    }
      }
    sql = sqlite3_mprintf ("%s\n)", prev_sql);
    sqlite3_free (prev_sql);

    ret = sqlite3_exec (cloner->sqlite, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("CREATE TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return 0;
      }

    column = cloner->first_col;
    while (column != NULL)
      {
	  if (column->geometry != NULL && !(column->ignore))
	    {
		/* adding a Geometry Column */
		if (!create_geometry
		    (cloner->sqlite, cloner->out_table, column))
		    return 0;
	    }
	  column = column->next;
      }

    index = cloner->first_idx;
    fk_no = 1;
    while (index != NULL)
      {
	  /* creating an Index */
	  constraint =
	      sqlite3_mprintf ("idx_%s_%d", cloner->out_table, fk_no++);
	  xconstraint = gaiaDoubleQuotedSql (constraint);
	  xtable = gaiaDoubleQuotedSql (cloner->out_table);
	  if (index->unique)
	      sql =
		  sqlite3_mprintf ("CREATE UNIQUE INDEX main.\"%s\" ON \"%s\"",
				   xconstraint, xtable);
	  else
	      sql =
		  sqlite3_mprintf ("CREATE INDEX main.\"%s\" ON \"%s\"",
				   xconstraint, xtable);
	  free (xconstraint);
	  free (xtable);
	  prev_sql = sql;

	  idx_column = index->first;
	  first = 1;
	  while (idx_column != NULL)
	    {
		xcolumn = gaiaDoubleQuotedSql (idx_column->name);
		if (first)
		  {
		      sql = sqlite3_mprintf ("%s (\"%s\"", prev_sql, xcolumn);
		      first = 0;
		  }
		else
		    sql = sqlite3_mprintf ("%s, \"%s\"", prev_sql, xcolumn);
		free (xcolumn);
		sqlite3_free (prev_sql);
		prev_sql = sql;
		idx_column = idx_column->next;
	    }
	  sql = sqlite3_mprintf ("%s)\n", prev_sql);
	  sqlite3_free (prev_sql);

	  ret = sqlite3_exec (cloner->sqlite, sql, NULL, NULL, &err_msg);
	  sqlite3_free (sql);
	  if (ret != SQLITE_OK)
	    {
		spatialite_e ("CREATE INDEX error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return 0;
	    }
	  index = index->next;
      }

    if (cloner->with_triggers)
      {
	  struct aux_trigger *trigger;
	  check_existing_triggers (cloner);
	  trigger = cloner->first_trigger;
	  while (trigger != NULL)
	    {
		if (trigger->already_existing)
		  {
		      /* skipping already defined triggers */
		      trigger = trigger->next;
		      continue;
		  }
		/* adding a trigger */
		ret =
		    sqlite3_exec (cloner->sqlite, trigger->sql, NULL, NULL,
				  &err_msg);
		if (ret != SQLITE_OK)
		  {
		      spatialite_e ("CREATE TRIGGER error: %s\n", err_msg);
		      sqlite3_free (err_msg);
		      return 0;
		  }
		trigger = trigger->next;
	    }
      }
    return 1;
}

static int
copy_rows (struct aux_cloner *cloner)
{
/* copying all rows from the origin into the destination Table */
    sqlite3_stmt *stmt_in = NULL;
    sqlite3_stmt *stmt_out = NULL;
    int ret;
    struct aux_column *column;
    char *sql;
    char *prev_sql;
    char *xcolumn;
    char *xtable;
    char *xdb_prefix;
    int first = 1;

/* composing the SELECT statement */
    sql = sqlite3_mprintf ("SELECT ");
    prev_sql = sql;
    column = cloner->first_col;
    while (column != NULL)
      {
	  if (column->ignore)
	    {
		/* skipping columns to be IGNORED */
		column = column->next;
		continue;
	    }
	  xcolumn = gaiaDoubleQuotedSql (column->name);
	  if (first)
	    {
		sql = sqlite3_mprintf ("%s\"%s\"", prev_sql, xcolumn);
		first = 0;
	    }
	  else
	      sql = sqlite3_mprintf ("%s, \"%s\"", prev_sql, xcolumn);
	  free (xcolumn);
	  sqlite3_free (prev_sql);
	  prev_sql = sql;
	  column = column->next;
      }
    xdb_prefix = gaiaDoubleQuotedSql (cloner->db_prefix);
    xtable = gaiaDoubleQuotedSql (cloner->in_table);
    sql =
	sqlite3_mprintf ("%s FROM \"%s\".\"%s\"", prev_sql, xdb_prefix, xtable);
    free (xdb_prefix);
    free (xtable);
/* compiling the SELECT FROM statement */
    ret =
	sqlite3_prepare_v2 (cloner->sqlite, sql, strlen (sql), &stmt_in, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("SELECT FROM: \"%s\"\n",
			sqlite3_errmsg (cloner->sqlite));
	  goto error;
      }

/* composing the INSERT INTO statement */
    xtable = gaiaDoubleQuotedSql (cloner->out_table);
    sql = sqlite3_mprintf ("INSERT INTO \"%s\" (", xtable);
    free (xtable);
    prev_sql = sql;
    first = 1;
    column = cloner->first_col;
    while (column != NULL)
      {
	  if (column->ignore)
	    {
		/* skipping columns to be IGNORED */
		column = column->next;
		continue;
	    }
	  xcolumn = gaiaDoubleQuotedSql (column->name);
	  if (first)
	    {
		sql = sqlite3_mprintf ("%s\"%s\"", prev_sql, xcolumn);
		first = 0;
	    }
	  else
	      sql = sqlite3_mprintf ("%s, \"%s\"", prev_sql, xcolumn);
	  free (xcolumn);
	  sqlite3_free (prev_sql);
	  prev_sql = sql;
	  column = column->next;
      }
    sql = sqlite3_mprintf ("%s) VALUES (", prev_sql);
    sqlite3_free (prev_sql);
    prev_sql = sql;
    first = 1;
    column = cloner->first_col;
    while (column != NULL)
      {
	  if (column->ignore)
	    {
		/* skipping columns to be IGNORED */
		column = column->next;
		continue;
	    }
	  if (column->geometry != NULL)
	    {
		/* Geometry column */
		if (column->geometry->cast2multi)
		  {
		      /* casting to MultiType */
		      const char *expr = "CastToMulti(?)";
		      if (first)
			{
			    sql = sqlite3_mprintf ("%s%s", prev_sql, expr);
			    first = 0;
			}
		      else
			  sql = sqlite3_mprintf ("%s, %s", prev_sql, expr);
		      sqlite3_free (prev_sql);
		      prev_sql = sql;
		      column = column->next;
		      continue;
		  }
	    }
	  if (first)
	    {
		sql = sqlite3_mprintf ("%s?", prev_sql);
		first = 0;
	    }
	  else
	      sql = sqlite3_mprintf ("%s, ?", prev_sql);
	  sqlite3_free (prev_sql);
	  prev_sql = sql;
	  column = column->next;
      }
    sql = sqlite3_mprintf ("%s)", prev_sql);
    sqlite3_free (prev_sql);
/* compiling the INSERT INTO statement */
    ret =
	sqlite3_prepare_v2 (cloner->sqlite, sql, strlen (sql), &stmt_out, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  spatialite_e ("INSERT INTO: \"%s\"\n",
			sqlite3_errmsg (cloner->sqlite));
	  goto error;
      }

    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt_in);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* copying values between input and output tables */
		int pos = 0;
		sqlite3_reset (stmt_out);
		sqlite3_clear_bindings (stmt_out);
		column = cloner->first_col;
		while (column != NULL)
		  {
		      if (column->ignore)
			{
			    /* skipping columns to be IGNORED */
			    column = column->next;
			    continue;
			}
		      if (cloner->resequence && cloner->pk_count == 1
			  && cloner->autoincrement && column->pk)
			{
			    /* resequencing an AUTOINCREMENT PK */
			    sqlite3_bind_null (stmt_out, pos + 1);
			    pos++;
			    column = column->next;
			    continue;
			}
		      if (sqlite3_column_type (stmt_in, pos) == SQLITE_INTEGER)
			  sqlite3_bind_int64 (stmt_out, pos + 1,
					      sqlite3_column_int64 (stmt_in,
								    pos));
		      else if (sqlite3_column_type (stmt_in, pos) ==
			       SQLITE_FLOAT)
			  sqlite3_bind_double (stmt_out, pos + 1,
					       sqlite3_column_double (stmt_in,
								      pos));
		      else if (sqlite3_column_type (stmt_in, pos) ==
			       SQLITE_TEXT)
			  sqlite3_bind_text (stmt_out, pos + 1,
					     (const char *)
					     sqlite3_column_text (stmt_in, pos),
					     sqlite3_column_bytes (stmt_in,
								   pos),
					     SQLITE_STATIC);
		      else if (sqlite3_column_type (stmt_in, pos) ==
			       SQLITE_BLOB)
			  sqlite3_bind_blob (stmt_out, pos + 1,
					     sqlite3_column_blob (stmt_in, pos),
					     sqlite3_column_bytes (stmt_in,
								   pos),
					     SQLITE_STATIC);
		      else
			  sqlite3_bind_null (stmt_out, pos + 1);
		      pos++;
		      column = column->next;
		  }
		/* inserting into the output table */
		ret = sqlite3_step (stmt_out);
		if (ret == SQLITE_DONE || ret == SQLITE_ROW)
		    ;
		else
		  {
		      spatialite_e ("OUTPUT step error: <%s>\n",
				    sqlite3_errmsg (cloner->sqlite));
		      goto error;
		  }
	    }
	  else
	    {
		spatialite_e ("INPUT step error: <%s>\n",
			      sqlite3_errmsg (cloner->sqlite));
		goto error;
	    }
      }
    sqlite3_finalize (stmt_in);
    sqlite3_finalize (stmt_out);
    return 1;

  error:
    if (stmt_in != NULL)
	sqlite3_finalize (stmt_in);
    if (stmt_out != NULL)
	sqlite3_finalize (stmt_out);
    return 0;
}

static void
add_geometry (struct aux_cloner *cloner, const char *name, int type, int dims,
	      int srid, int spatial_index)
{
/* adding a Geometry Column definition */
    struct aux_column *pc = cloner->first_col;
    while (pc != NULL)
      {
	  if (strcasecmp (pc->name, name) == 0)
	    {
		struct aux_geometry *geom =
		    malloc (sizeof (struct aux_geometry));
		geom->type = type;
		geom->dims = dims;
		geom->srid = srid;
		geom->spatial_index = spatial_index;
		geom->cast2multi = 0;
		geom->already_existing = 0;
		if (pc->geometry != NULL)
		    free (pc->geometry);
		pc->geometry = geom;
		return;
	    }
	  pc = pc->next;
      }
}

static void
mark_existing_geometry (struct aux_cloner *cloner, const char *name, int type,
			int dims, int srid)
{
/* marking an existing Geometry Column definition */
    struct aux_column *pc = cloner->first_col;
    while (pc != NULL)
      {
	  if (strcasecmp (pc->name, name) == 0)
	    {
		if (pc->geometry == NULL)
		  {
		      /* gosh, it's not a Geometry */
		      pc->mismatching = 1;
		      return;
		  }
		if (pc->geometry->type == type && pc->geometry->dims == dims
		    && pc->geometry->srid == srid)
		  {
		      /* matching arguments: confirmed */
		      pc->geometry->already_existing = 1;
		      return;
		  }
		/* different arguments: invalid */
		pc->mismatching = 1;
		return;
	    }
	  pc = pc->next;
      }
}

static void
add_trigger (struct aux_cloner *cloner, const char *name, const char *sql)
{
/* adding a Trigger definition */
    int len;
    struct aux_trigger *trigger = malloc (sizeof (struct aux_trigger));
    len = strlen (name);
    trigger->name = malloc (len + 1);
    strcpy (trigger->name, name);
    len = strlen (sql);
    trigger->sql = malloc (len + 1);
    strcpy (trigger->sql, sql);
    trigger->already_existing = 0;
    trigger->next = NULL;
/* updating the linked list */
    if (cloner->first_trigger == NULL)
	cloner->first_trigger = trigger;
    if (cloner->last_trigger != NULL)
	cloner->last_trigger->next = trigger;
    cloner->last_trigger = trigger;
}

static void
add_fk_columns (struct aux_foreign_key *fk, struct aux_column *first_col,
		const char *from, const char *to)
{
/* adding Columns correspondencies into a Foreign Key definition */
    int len;
    struct aux_column *column;
    struct aux_fk_columns *col = malloc (sizeof (struct aux_fk_columns));
    len = strlen (from);
    col->from = malloc (len + 1);
    strcpy (col->from, from);
    len = strlen (to);
    col->to = malloc (len + 1);
    strcpy (col->to, to);
    col->next = NULL;
/* updating the linked list */
    if (fk->first == NULL)
	fk->first = col;
    if (fk->last != NULL)
	fk->last->next = col;
    fk->last = col;
/* marking the column as a Foreign Key */
    column = first_col;
    while (column != NULL)
      {
	  if (strcasecmp (column->name, from) == 0)
	    {
		column->fk = 1;
		break;
	    }
	  column = column->next;
      }
}

static void
add_foreign_key (struct aux_cloner *cloner, int id, const char *references,
		 const char *from, const char *to, const char *on_update,
		 const char *on_delete, const char *match)
{
/* adding a Foreign Key definition */
    int len;
    struct aux_foreign_key *fk;
    if (cloner->last_fk != NULL)
      {
	  if (cloner->last_fk->id == id)
	    {
		/* continuing with the latest FK */
		add_fk_columns (cloner->last_fk, cloner->first_col, from, to);
		return;
	    }
      }
    fk = malloc (sizeof (struct aux_foreign_key));
    fk->id = id;
    fk->name = NULL;
    len = strlen (references);
    fk->references = malloc (len + 1);
    strcpy (fk->references, references);
    fk->on_update = NULL;
    fk->on_delete = NULL;
    fk->match = NULL;
    if (on_update != NULL)
      {
	  len = strlen (on_update);
	  fk->on_update = malloc (len + 1);
	  strcpy (fk->on_update, on_update);
      }
    if (on_delete != NULL)
      {
	  len = strlen (on_delete);
	  fk->on_delete = malloc (len + 1);
	  strcpy (fk->on_delete, on_delete);
      }
    if (match != NULL)
      {
	  len = strlen (match);
	  fk->match = malloc (len + 1);
	  strcpy (fk->match, match);
      }
    fk->first = NULL;
    fk->last = NULL;
    fk->next = NULL;
    add_fk_columns (fk, cloner->first_col, from, to);
/* updating the linked list */
    if (cloner->first_fk == NULL)
	cloner->first_fk = fk;
    if (cloner->last_fk != NULL)
	cloner->last_fk->next = fk;
    cloner->last_fk = fk;
}

static void
add_index_column (struct aux_index *index, struct aux_column *first_col,
		  const char *name)
{
/* adding a Column into an Index definition */
    int len;
    struct aux_column *col;
    struct aux_index_column *column = malloc (sizeof (struct aux_index_column));
    len = strlen (name);
    column->name = malloc (len + 1);
    strcpy (column->name, name);
    column->next = NULL;
/* updating the linked list */
    if (index->first == NULL)
	index->first = column;
    if (index->last != NULL)
	index->last->next = column;
    index->last = column;
/* marking the column as a Foreign Key */
    col = first_col;
    while (col != NULL)
      {
	  if (strcasecmp (col->name, name) == 0)
	    {
		col->idx = 1;
		break;
	    }
	  col = col->next;
      }
}

static struct aux_index *
add_index (struct aux_cloner *cloner, const char *name, int unique)
{
/* adding an Index definition */
    int len;
    struct aux_index *index = malloc (sizeof (struct aux_index));
    len = strlen (name);
    index->name = malloc (len + 1);
    strcpy (index->name, name);
    index->unique = unique;
    index->first = NULL;
    index->last = NULL;
    index->next = NULL;
/* updating the linked list */
    if (cloner->first_idx == NULL)
	cloner->first_idx = index;
    if (cloner->last_idx != NULL)
	cloner->last_idx->next = index;
    cloner->last_idx = index;
    return index;
}

static void
add_column (struct aux_cloner *cloner, const char *name, const char *type,
	    int notnull, const char *deflt, int pk)
{
/* adding a Column definition */
    int len;
    struct aux_column *column = malloc (sizeof (struct aux_column));
    len = strlen (name);
    column->name = malloc (len + 1);
    strcpy (column->name, name);
    len = strlen (type);
    column->type = malloc (len + 1);
    strcpy (column->type, type);
    column->notnull = notnull;
    if (deflt == NULL)
	column->deflt = NULL;
    else
      {
	  len = strlen (deflt);
	  column->deflt = malloc (len + 1);
	  strcpy (column->deflt, deflt);
      }
    column->pk = pk;
    column->fk = 0;
    column->idx = 0;
    column->geometry = NULL;
    column->ignore = 0;
    column->already_existing = 0;
    column->mismatching = 0;
    column->next = NULL;
/* updating the linked list */
    if (cloner->first_col == NULL)
	cloner->first_col = column;
    if (cloner->last_col != NULL)
	cloner->last_col->next = column;
    cloner->last_col = column;
    if (pk)
      {
	  struct aux_pk_column *ppk = malloc (sizeof (struct aux_pk_column));
	  ppk->column = column;
	  ppk->next = NULL;
	  /* updating the linked list */
	  if (cloner->first_pk == NULL)
	      cloner->first_pk = ppk;
	  if (cloner->last_pk != NULL)
	      cloner->last_pk->next = ppk;
	  cloner->last_pk = ppk;
	  cloner->pk_count += 1;
      }
}

static void
mark_existing_column (struct aux_cloner *cloner, const char *name)
{
/* marking an existing Column */
    struct aux_column *column = cloner->first_col;
    while (column != NULL)
      {
	  if (strcasecmp (column->name, name) == 0)
	    {
		column->already_existing = 1;
		return;
	    }
	  column = column->next;
      }
}

static int
check_input_table_columns (struct aux_cloner *cloner)
{
/* exploring the input table - Columns */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    const char *name;
    const char *type;
    int notnull;
    const char *deflt;
    int pk;
    char *xprefix;
    char *xtable;

    xprefix = gaiaDoubleQuotedSql (cloner->db_prefix);
    xtable = gaiaDoubleQuotedSql (cloner->in_table);
    sql = sqlite3_mprintf ("PRAGMA \"%s\".table_info(\"%s\")", xprefix, xtable);
    free (xprefix);
    free (xtable);
    ret =
	sqlite3_get_table (cloner->sqlite, sql, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return 0;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		type = results[(i * columns) + 2];
		notnull = atoi (results[(i * columns) + 3]);
		deflt = results[(i * columns) + 4];
		pk = atoi (results[(i * columns) + 5]);
		add_column (cloner, name, type, notnull, deflt, pk);
	    }
      }
    sqlite3_free_table (results);

    if (cloner->first_col == NULL)
      {
	  spatialite_e
	      ("CloneTable: input table \"%s\".\"%s\" does not exist\n",
	       cloner->db_prefix, cloner->in_table);
	  goto error;
      }
    return 1;

  error:
    return 0;
}

static void
check_input_table_autoincrement (struct aux_cloner *cloner)
{
/* exploring the input table AUTOINCREMENT property */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    char *xprefix;

    if (cloner->pk_count != 1)
	return;

    xprefix = gaiaDoubleQuotedSql (cloner->db_prefix);
    sql = sqlite3_mprintf ("SELECT Count(*) FROM \"%s\".sqlite_sequence "
			   "WHERE Lower(name) = Lower(%Q)",
			   xprefix, cloner->in_table);
    free (xprefix);
    ret =
	sqlite3_get_table (cloner->sqlite, sql, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		if (atoi (results[(i * columns) + 0]) > 0)
		    cloner->autoincrement = 1;
	    }
      }
    sqlite3_free_table (results);
}

static void
check_output_table_columns (struct aux_cloner *cloner)
{
/* exploring the output table - Columns */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    const char *name;
    char *xtable;

    xtable = gaiaDoubleQuotedSql (cloner->out_table);
    sql = sqlite3_mprintf ("PRAGMA main.table_info(\"%s\")", xtable);
    free (xtable);
    ret =
	sqlite3_get_table (cloner->sqlite, sql, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		mark_existing_column (cloner, name);
	    }
      }
    sqlite3_free_table (results);
}

static void
expand_index (struct aux_cloner *cloner, struct aux_index *index)
{
/* expanding an Index definitions */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    const char *name;
    char *xprefix;
    char *xindex;

    xprefix = gaiaDoubleQuotedSql (cloner->db_prefix);
    xindex = gaiaDoubleQuotedSql (index->name);
    sql = sqlite3_mprintf ("PRAGMA \"%s\".index_info(\"%s\")", xprefix, xindex);
    free (xprefix);
    free (xindex);
    ret =
	sqlite3_get_table (cloner->sqlite, sql, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 2];
		add_index_column (index, cloner->first_col, name);
	    }
      }
    sqlite3_free_table (results);
}

static void
check_input_table_index_defs (struct aux_cloner *cloner)
{
/* exploring the input table - Index definitions */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    const char *name;
    int unique;
    char *xprefix;
    char *xtable;
    struct aux_index *idx;

    xprefix = gaiaDoubleQuotedSql (cloner->db_prefix);
    xtable = gaiaDoubleQuotedSql (cloner->in_table);
    sql = sqlite3_mprintf ("PRAGMA \"%s\".index_list(\"%s\")", xprefix, xtable);
    free (xprefix);
    free (xtable);
    ret =
	sqlite3_get_table (cloner->sqlite, sql, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strncasecmp (name, "sqlite_autoindex_", 17) == 0)
		  {
		      /* ignoring any AUTOINDEX defined by SQLite */
		      continue;
		  }
		unique = atoi (results[(i * columns) + 2]);
		idx = add_index (cloner, name, unique);
		expand_index (cloner, idx);
	    }
      }
    sqlite3_free_table (results);
}

static void
check_input_table_foreign_keys (struct aux_cloner *cloner)
{
/* exploring the input table - Foreign Keys */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    int id;
    const char *references;
    const char *from;
    const char *to;
    const char *on_update;
    const char *on_delete;
    const char *match;
    char *xprefix;
    char *xtable;

    xprefix = gaiaDoubleQuotedSql (cloner->db_prefix);
    xtable = gaiaDoubleQuotedSql (cloner->in_table);
    sql =
	sqlite3_mprintf ("PRAGMA \"%s\".foreign_key_list(\"%s\")", xprefix,
			 xtable);
    free (xprefix);
    free (xtable);
    ret =
	sqlite3_get_table (cloner->sqlite, sql, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		id = atoi (results[(i * columns) + 0]);
		references = results[(i * columns) + 2];
		from = results[(i * columns) + 3];
		to = results[(i * columns) + 4];
		on_update = results[(i * columns) + 5];
		on_delete = results[(i * columns) + 6];
		match = results[(i * columns) + 7];
		if (strcasecmp (on_update, "NO ACTION") == 0)
		    on_update = NULL;
		if (strcasecmp (on_delete, "NO ACTION") == 0)
		    on_delete = NULL;
		if (strcasecmp (match, "NONE") == 0)
		    match = NULL;
		add_foreign_key (cloner, id, references, from, to, on_update,
				 on_delete, match);
	    }
      }
    sqlite3_free_table (results);
}

static void
check_input_table_triggers (struct aux_cloner *cloner)
{
/* exploring the input table - Triggers */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    const char *name;
    const char *sqlx;
    char *xprefix;

    xprefix = gaiaDoubleQuotedSql (cloner->db_prefix);
    sql = sqlite3_mprintf ("SELECT name, sql FROM \"%s\".sqlite_master "
			   "WHERE type = 'trigger' AND Lower(tbl_name) = Lower(%Q)",
			   xprefix, cloner->in_table);
    free (xprefix);
    ret =
	sqlite3_get_table (cloner->sqlite, sql, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 0];
		sqlx = results[(i * columns) + 1];
		add_trigger (cloner, name, sqlx);
	    }
      }
    sqlite3_free_table (results);
}

static void
check_input_table_geometries (struct aux_cloner *cloner)
{
/* exploring the input table - Geometries */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    const char *name;
    int type;
    int dims;
    int srid;
    int spatial_index;
    char *xprefix;

    xprefix = gaiaDoubleQuotedSql (cloner->db_prefix);
    sql = sqlite3_mprintf ("SELECT f_geometry_column, geometry_type, "
			   "coord_dimension, srid, spatial_index_enabled "
			   "FROM \"%s\".geometry_columns "
			   "WHERE Lower(f_table_name) = Lower(%Q)",
			   xprefix, cloner->in_table);
    free (xprefix);
    ret =
	sqlite3_get_table (cloner->sqlite, sql, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 0];
		type = atoi (results[(i * columns) + 1]);
		dims = atoi (results[(i * columns) + 2]);
		srid = atoi (results[(i * columns) + 3]);
		spatial_index = atoi (results[(i * columns) + 4]);
		add_geometry (cloner, name, type, dims, srid, spatial_index);
	    }
      }
    sqlite3_free_table (results);
}

static void
check_output_table_geometries (struct aux_cloner *cloner)
{
/* exploring the output table - Geometries */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    const char *name;
    int type;
    int dims;
    int srid;

    sql = sqlite3_mprintf ("SELECT f_geometry_column, geometry_type, "
			   "coord_dimension, srid, spatial_index_enabled "
			   "FROM main.geometry_columns "
			   "WHERE Lower(f_table_name) = Lower(%Q)",
			   cloner->out_table);
    ret =
	sqlite3_get_table (cloner->sqlite, sql, &results, &rows, &columns,
			   NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	return;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 0];
		type = atoi (results[(i * columns) + 1]);
		dims = atoi (results[(i * columns) + 2]);
		srid = atoi (results[(i * columns) + 3]);
		mark_existing_geometry (cloner, name, type, dims, srid);
	    }
      }
    sqlite3_free_table (results);
}

static void
free_trigger (struct aux_trigger *trigger)
{
/* memory cleanup - destroying a Trigger object */
    if (trigger == NULL)
	return;
    if (trigger->name != NULL)
	free (trigger->name);
    if (trigger->sql != NULL)
	free (trigger->sql);
    free (trigger);
}

static void
free_fk_columns (struct aux_fk_columns *col)
{
/* memory cleanup - destroying a Foreign Key Columns object */
    if (col == NULL)
	return;
    if (col->from != NULL)
	free (col->from);
    if (col->to != NULL)
	free (col->to);
    free (col);
}

static void
free_foreign_key (struct aux_foreign_key *fk)
{
/* memory cleanup - destroying a Foreign Key object */
    struct aux_fk_columns *pc;
    struct aux_fk_columns *pcn;
    if (fk == NULL)
	return;
    if (fk->name != NULL)
	free (fk->name);
    if (fk->references != NULL)
	free (fk->references);
    if (fk->on_update != NULL)
	free (fk->on_update);
    if (fk->on_delete != NULL)
	free (fk->on_delete);
    if (fk->match != NULL)
	free (fk->match);
    pc = fk->first;
    while (pc != NULL)
      {
	  pcn = pc->next;
	  free_fk_columns (pc);
	  pc = pcn;
      }
    free (fk);
}

static void
free_index_column (struct aux_index_column *column)
{
/* memory cleanup - destroying an Index Column object */
    if (column == NULL)
	return;
    if (column->name != NULL)
	free (column->name);
    free (column);
}

static void
free_index (struct aux_index *index)
{
/* memory cleanup - destroying an Index object */
    struct aux_index_column *pc;
    struct aux_index_column *pcn;
    if (index == NULL)
	return;
    if (index->name != NULL)
	free (index->name);
    pc = index->first;
    while (pc != NULL)
      {
	  pcn = pc->next;
	  free_index_column (pc);
	  pc = pcn;
      }
    free (index);
}

static void
free_column (struct aux_column *column)
{
/* memory cleanup - destroying a Column object */
    if (column == NULL)
	return;
    if (column->name != NULL)
	free (column->name);
    if (column->type != NULL)
	free (column->type);
    if (column->deflt != NULL)
	free (column->deflt);
    if (column->geometry != NULL)
	free (column->geometry);
    free (column);
}

static void
free_cloner (struct aux_cloner *cloner)
{
/* memory cleanup - destroying a Cloner object */
    struct aux_column *pc;
    struct aux_column *pcn;
    struct aux_pk_column *ppk;
    struct aux_pk_column *ppkn;
    struct aux_index *pi;
    struct aux_index *pin;
    struct aux_foreign_key *pfk;
    struct aux_foreign_key *pfkn;
    struct aux_trigger *ptrg;
    struct aux_trigger *ptrgn;
    if (cloner == NULL)
	return;
    if (cloner->db_prefix != NULL)
	free (cloner->db_prefix);
    if (cloner->in_table != NULL)
	free (cloner->in_table);
    if (cloner->out_table != NULL)
	free (cloner->out_table);
    pc = cloner->first_col;
    while (pc != NULL)
      {
	  pcn = pc->next;
	  free_column (pc);
	  pc = pcn;
      }
    ppk = cloner->first_pk;
    while (ppk != NULL)
      {
	  ppkn = ppk->next;
	  free (ppk);
	  ppk = ppkn;
      }
    pi = cloner->first_idx;
    while (pi != NULL)
      {
	  pin = pi->next;
	  free_index (pi);
	  pi = pin;
      }
    pfk = cloner->first_fk;
    while (pfk != NULL)
      {
	  pfkn = pfk->next;
	  free_foreign_key (pfk);
	  pfk = pfkn;
      }
    ptrg = cloner->first_trigger;
    while (ptrg != NULL)
      {
	  ptrgn = ptrg->next;
	  free_trigger (ptrg);
	  ptrg = ptrgn;
      }
    if (cloner->sorted_pks != NULL)
	free (cloner->sorted_pks);
    free (cloner);
}

static int
already_existing_table (sqlite3 * sqlite, const char *table)
{
/* testing if the target Table is already defined */
    char *sql;
    int ret;
    int i;
    char **results;
    int rows;
    int columns;
    int count = 0;

    sql = sqlite3_mprintf ("SELECT Count(*) FROM main.sqlite_master "
			   "WHERE type = 'table' AND Lower(name) = Lower(%Q)",
			   table);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
	goto stop;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	      count = atoi (results[(i * columns) + 0]);
      }
    sqlite3_free_table (results);
  stop:
    return count;
}

SPATIALITE_PRIVATE const void *
gaiaAuxClonerCreate (const void *sqlite, const char *db_prefix,
		     const char *in_table, const char *out_table)
{
/* creating a Cloner object */
    int len;
    struct aux_cloner *cloner;
    if (checkSpatialMetaData ((sqlite3 *) sqlite) < 3)
      {
	  spatialite_e ("CloneTable: obsolete DB-layout (< 4.0.0)\n");
	  return NULL;
      }

    cloner = malloc (sizeof (struct aux_cloner));
    if (cloner == NULL)
	return NULL;
    cloner->sqlite = (sqlite3 *) sqlite;
    cloner->db_prefix = NULL;
    cloner->in_table = NULL;
    cloner->out_table = NULL;
    len = strlen (db_prefix);
    cloner->db_prefix = malloc (len + 1);
    strcpy (cloner->db_prefix, db_prefix);
    len = strlen (in_table);
    cloner->in_table = malloc (len + 1);
    strcpy (cloner->in_table, in_table);
    len = strlen (out_table);
    cloner->out_table = malloc (len + 1);
    strcpy (cloner->out_table, out_table);
    cloner->first_col = NULL;
    cloner->last_col = NULL;
    cloner->first_pk = NULL;
    cloner->last_pk = NULL;
    cloner->first_idx = NULL;
    cloner->last_idx = NULL;
    cloner->first_fk = NULL;
    cloner->last_fk = NULL;
    cloner->first_trigger = NULL;
    cloner->last_trigger = NULL;
    cloner->pk_count = 0;
    cloner->sorted_pks = NULL;
    cloner->autoincrement = 0;
    cloner->resequence = 0;
    cloner->with_fks = 0;
    cloner->with_triggers = 0;
    cloner->append = 0;
    cloner->already_existing = 0;

/* exploring the input table - Columns */
    if (!check_input_table_columns (cloner))
	goto error;
/* exploring PRIMARY KEY AUTOINCREMENT */
    check_input_table_autoincrement (cloner);
/* exploring the input table - Index definitions */
    check_input_table_index_defs (cloner);
/* exploring the input table - Foreign Key definitions */
    check_input_table_foreign_keys (cloner);
/* exploring the input table - Trigger definitions */
    check_input_table_triggers (cloner);
/* exploring the input table - Geometry definitions */
    check_input_table_geometries (cloner);

    if (already_existing_table (cloner->sqlite, out_table))
	cloner->already_existing = 1;
    return cloner;
  error:
    free_cloner (cloner);
    return NULL;
}

SPATIALITE_PRIVATE void
gaiaAuxClonerDestroy (const void *handle)
{
/* destroying a Cloner object */
    struct aux_cloner *cloner = (struct aux_cloner *) handle;
    if (handle == NULL)
	return;
    free_cloner (cloner);
}

static void
ignore_column (struct aux_cloner *cloner, const char *column)
{
/* marking a Column to be ignored */
    struct aux_column *pc = cloner->first_col;
    while (pc != NULL)
      {
	  if (strcasecmp (pc->name, column) == 0)
	    {
		pc->ignore = 1;
		return;
	    }
	  pc = pc->next;
      }
}

static int
check_append (struct aux_cloner *cloner)
{
/* cheching for APPEND validity */
    int error = 0;
    struct aux_column *column = cloner->first_col;
    while (column != NULL)
      {
	  if (column->mismatching)
	      error = 1;
	  column = column->next;
      }
    if (error)
	return 0;
    return 1;
}

static void
cast2multi_column (struct aux_cloner *cloner, const char *column)
{
/* marking a Geometry Column to be casted to MultiType */
    struct aux_column *pc = cloner->first_col;
    while (pc != NULL)
      {
	  if (strcasecmp (pc->name, column) == 0 && pc->geometry != NULL)
	    {
		pc->geometry->cast2multi = 1;
		return;
	    }
	  pc = pc->next;
      }
}

SPATIALITE_PRIVATE void
gaiaAuxClonerAddOption (const void *handle, const char *option)
{
/* parsing an Option */
    struct aux_cloner *cloner = (struct aux_cloner *) handle;
    if (handle == NULL)
	return;
    if (strncasecmp (option, "::ignore::", 10) == 0)
	ignore_column (cloner, option + 10);
    if (strncasecmp (option, "::cast2multi::", 14) == 0)
	cast2multi_column (cloner, option + 14);
    if (strncasecmp (option, "::resequence::", 14) == 0)
	cloner->resequence = 1;
    if (strncasecmp (option, "::with-foreign-keys::", 21) == 0)
	cloner->with_fks = 1;
    if (strncasecmp (option, "::with-triggers::", 17) == 0)
	cloner->with_triggers = 1;
    if (strncasecmp (option, "::append::", 10) == 0)
      {
	  cloner->append = 1;
	  cloner->resequence = 1;
      }
    return;
}

SPATIALITE_PRIVATE int
gaiaAuxClonerCheckValidTarget (const void *handle)
{
/* checking the Target Table for validity */
    struct aux_cloner *cloner = (struct aux_cloner *) handle;
    if (handle == NULL)
	return 0;

    if (cloner->already_existing)
      {
	  if (cloner->append)
	    {
		/* exploring the output table - Columns */
		check_output_table_columns (cloner);
		/* exploring the output table - Geometries */
		check_output_table_geometries (cloner);
		/* checking for validity */
		if (!check_append (cloner))
		  {
		      spatialite_e
			  ("CloneTable: output table \"%s\" can't support APPEND\n",
			   cloner->out_table);
		      return 0;
		  }
	    }
	  else
	    {
		spatialite_e
		    ("CloneTable: output table \"%s\" already exists and APPEND is not enabled\n",
		     cloner->out_table);
		return 0;
	    }
      }
    return 1;
}

SPATIALITE_PRIVATE int
gaiaAuxClonerExecute (const void *handle)
{
/* executing the actual work */
    struct aux_cloner *cloner = (struct aux_cloner *) handle;
    if (handle == NULL)
	return 0;
    if (cloner->already_existing)
      {
	  /* creating any further column if required */
	  if (!upgrade_output_table (cloner))
	    {
		spatialite_e
		    ("CloneTable: unable to updgrade the output table \"%s\"\n",
		     cloner->out_table);
		return 0;
	    }
      }
    else
      {
	  /* creating the output table */
	  if (!create_output_table (cloner))
	    {
		spatialite_e
		    ("CloneTable: unable to create the output table \"%s\"\n",
		     cloner->out_table);
		return 0;
	    }
      }
    if (!copy_rows (cloner))
      {
	  spatialite_e ("CloneTable: unable to copy Table rows\n");
	  return 0;
      }
    return 1;
}
