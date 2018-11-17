/*

 check_gpkg_base_core_contents_data_table_def - Test case for GeoPackage Extensions

 Author: Brad Hards <bradh@frogmouth.net>

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

The Original Code is GeoPackage extensions

The Initial Developer of the Original Code is Brad Hards
 
Portions created by the Initial Developer are Copyright (C) 2013
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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define __USE_GNU
#include <string.h>

#include "sqlite3.h"
#include "spatialite.h"

#include "test_helpers.h"

struct tableDefinitionElement
{
    const char *column_name;
    const char *column_type;
    bool mayBeNull;
    const char *defaultValue;
    const char *keySignature;
    bool wasFound;
};

struct tableDefinitionElement tableValues[] = {
    {"table_name", "TEXT", false, NULL, "PRIMARY KEY", false},
    {"data_type", "TEXT", false, NULL, NULL, false},
    {"identifier", "TEXT", true, NULL, NULL, false},
    {"description", "TEXT", true, "''", NULL, false},
    {"min_x", "DOUBLE", true, NULL, NULL, false},
    {"min_y", "DOUBLE", true, NULL, NULL, false},
    {"max_x", "DOUBLE", true, NULL, NULL, false},
    {"max_y", "DOUBLE", true, NULL, NULL, false},
    {"srs_id", "INTEGER", true, NULL, NULL, false},
    {NULL, NULL, false, NULL, NULL, false}
};

int
main (int argc UNUSED, char *argv[]UNUSED)
{
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
    char *sql_statement;
    sqlite3_stmt *stmt;
    void *cache = spatialite_alloc_connection ();
    int i, j;
    char *sql;
    char *str;
    char *token;
    char *saveptr;
    bool valid_last_change = false;
    bool valid_fk_constraint = false;

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    /* For debugging / testing if required */
    /*
       ret = sqlite3_open_v2 ("check_gpkg_base_core_spatial_ref_sys_data_table_def.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
     */
    spatialite_init_ex (db_handle, cache, 0);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  db_handle = NULL;
	  return -1;
      }

    ret =
	sqlite3_exec (db_handle, "SELECT gpkgCreateBaseTables()", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected gpkgCreateBaseTables() result: %i, (%s)\n", ret,
		   err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

    /*
       / Test Case ID    /base/core/contents/data/table_def
       / Test Purpose Verify that the gpkg_contents table exists and has the correct definition.
       / Test Method
       /    SELECT sql FROM sqlite_master WHERE type = table AND tbl_name = gpkg_contents
       /    Fail if returns an empty result set.
       /    Pass if the column names and column definitions in the returned CREATE TABLE statement,
       /       including data type, nullability, default values and primary, foreign and unique key
       /       constraints match all of those in the contents of C.2 Table 18. Column order, check
       /       constraint and trigger definitions, and other column definitions in the returned sql
       /       are irrelevant.
       /    Fail Otherwise
       / Reference  Clause 1.1.3.1.1 Req 13:
       / Test Type Basic
     */
    sql_statement =
	"SELECT sql FROM sqlite_master WHERE type = 'table' AND tbl_name = 'gpkg_contents'";
    ret =
	sqlite3_prepare_v2 (db_handle, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "failed to prepare SQL SELECT statement: %i (%s)\n",
		   ret, sqlite3_errmsg (db_handle));
	  return -10;
      }
    ret = sqlite3_step (stmt);
    if (ret != SQLITE_ROW)
      {
	  fprintf (stderr, "unexpected return value for first step: %i (%s)\n",
		   ret, sqlite3_errmsg (db_handle));
	  return -11;
      }
    if (sqlite3_column_type (stmt, 0) != SQLITE_TEXT)
      {
	  fprintf (stderr, "bad type for column 0: %i\n",
		   sqlite3_column_type (stmt, 0));
	  return -12;
      }
    sql =
	strdup (strstr ((const char *) sqlite3_column_text (stmt, 0), "(") +
		strlen ("("));
    if (!sql)
      {
	  fprintf (stderr, "unrecognised format for SQL\n");
	  return -13;
      }

    for (j = 1, str = sql;; j++, str = NULL)
      {
	  token = strtok_r (str, ",", &saveptr);
	  if (token == NULL)
	    {
		break;
	    }
	  for (i = 0; tableValues[i].column_name != NULL; ++i)
	    {
		while (isspace (*token))
		  {
		      token++;
		  }
		if (strncasecmp (token, "CONSTRAINT", strlen ("CONSTRAINT")) ==
		    0)
		  {
		      if (strcasestr
			  (token,
			   "FOREIGN KEY (srs_id) REFERENCES gpkg_spatial_ref_sys(srs_id)"))
			{
			    valid_fk_constraint = true;
			}
		      continue;
		  }
		if (strcasestr (token, tableValues[i].column_name))
		  {
		      if (!strcasestr (token, tableValues[i].column_type))
			{
			    fprintf (stderr,
				     "missing COLUMN TYPE %s for %s: %s\n",
				     tableValues[i].column_type,
				     tableValues[i].column_name, token);
			    return -20;
			}
		      if (tableValues[i].mayBeNull
			  && strcasestr (token, "NOT NULL"))
			{
			    fprintf (stderr,
				     "Unexpected NOT NULL constraint for %s: %s\n",
				     tableValues[i].column_name, token);
			    return -21;
			}
		      if ((!tableValues[i].mayBeNull)
			  && (!strcasestr (token, "NOT NULL")))
			{
			    fprintf (stderr,
				     "Missing NOT NULL constraint for %s: %s\n",
				     tableValues[i].column_name, token);
			    return -22;
			}
		      if ((tableValues[i].keySignature != NULL)
			  && (!strcasestr (token, tableValues[i].keySignature)))
			{
			    fprintf (stderr,
				     "Missing %s constraint for %s: %s\n",
				     tableValues[i].keySignature,
				     tableValues[i].column_name, token);
			    return -23;
			}
		      if ((tableValues[i].keySignature == NULL)
			  && (strcasestr (token, "KEY")))
			{
			    fprintf (stderr,
				     "Unexpected key constraint for %s: %s\n",
				     tableValues[i].column_name, token);
			    return -24;
			}
		      if ((tableValues[i].defaultValue == NULL)
			  && (strcasestr (token, "DEFAULT")))
			{
			    fprintf (stderr,
				     "Unexpected default value for %s: %s\n",
				     tableValues[i].column_name, token);
			    return -25;
			}
		      if ((tableValues[i].defaultValue != NULL)
			  && (!strcasestr (token, tableValues[i].defaultValue)))
			{
			    fprintf (stderr,
				     "Missing default value for %s: %s\n",
				     tableValues[i].column_name, token);
			    return -26;
			}
		      tableValues[i].wasFound = true;
		  }
	    }
	  if (strcasestr (token, "last_change "))
	    {
		/* last_change default has an embedded ',' so we need to grab the next token too. */
		char *nexttoken = strtok_r (str, ",", &saveptr);
		char *fulltoken =
		    malloc (strlen (token) + strlen (",") + strlen (nexttoken) +
			    1);
		strcpy (fulltoken, token);
		strcat (fulltoken, ",");
		strcat (fulltoken, nexttoken);
		if (!strcasestr (fulltoken, "DATETIME"))
		  {
		      fprintf (stderr,
			       "missing COLUMN TYPE DATETIME for last_change: %s\n",
			       fulltoken);
		      free (fulltoken);
		      return -40;
		  }
		if (!strcasestr (fulltoken, "NOT NULL"))
		  {
		      fprintf (stderr,
			       "Missing NOT NULL constraint for last_change: %s\n",
			       fulltoken);
		      free (fulltoken);
		      return -41;
		  }
		if (strcasestr (fulltoken, "KEY"))
		  {
		      fprintf (stderr,
			       "Unexpected KEY constraint for last_change: %s\n",
			       token);
		      free (fulltoken);
		      return -42;
		  }
		if (!strcasestr
		    (fulltoken,
		     "DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ',CURRENT_TIMESTAMP))"))
		  {
		      fprintf (stderr,
			       "Missing default value for last_change: %s\n",
			       fulltoken);
		      free (fulltoken);
		      return -43;
		  }
		free (fulltoken);
		valid_last_change = true;
	    }
      }
    free (sql);

    for (i = 0; tableValues[i].column_name != NULL; ++i)
      {
	  if (tableValues[i].wasFound == false)
	    {
		fprintf (stderr, "did not find expected %s column\n",
			 tableValues[i].column_name);
		return -50;
	    }
      }
    if (!valid_last_change)
      {
	  fprintf (stderr,
		   "did not find expected last_change column, or required properties were not found\n");
	  return -51;
      }
    if (!valid_fk_constraint)
      {
	  fprintf (stderr, "did not find expected FK constraint\n");
	  return -52;
      }


    ret = sqlite3_step (stmt);
    if (ret != SQLITE_DONE)
      {
	  fprintf (stderr, "unexpected return value for second step: %i\n",
		   ret);
	  return -199;
      }
    ret = sqlite3_finalize (stmt);

    sqlite3_free (err_msg);

    ret = sqlite3_close (db_handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (db_handle));
	  return -200;
      }

    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
