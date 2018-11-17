/*

 check_gpkg_opt_extension_mechanism_extensions_data_table_def - Test case for GeoPackage Extensions

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
    {"table_name", "TEXT", true, NULL, NULL, false},
    {"column_name", "TEXT", true, NULL, NULL, false},
    {"extension_name", "TEXT", false, NULL, NULL, false},
    {"definition", "TEXT", false, NULL, NULL, false},
    {"scope", "TEXT", false, NULL, NULL, false},
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
    bool valid_fk_constraint = false;

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    /* For debugging / testing if required */
    /*
       ret = sqlite3_open_v2 ("check_gpkg_opt_extension_mechanism_extensions_data_table_def.sqlite", &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
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
       / Test Case ID  /opt/extension_mechanism/extensions/data/table_def
       / Test Purpose  Verify that a gpkg_extensions table exists and has the correct definition.
       / Test Method:
       /   1)    SELECT sql FROM sqlite_master WHERE type = table AND tbl_name = gpkg_extensions
       /   2)    Fail if returns an empty result set.
       /   3)    Pass if the column names and column definitions in the returned Create TABLE statement
       /         in the sql column value, including data type, nullability, default values and primary,
       /         foreign and unique key constraints match all of those in the contents of Table 36.
       /         Column order, check constraint and trigger definitions, and other column definitions
       /         in the returned sql are irrelevant.
       /   4)    Fail otherwise.
       /   Reference  Clause 2.5.2.1.1 Req 78:
       /   Test Type  Basic
     */

    sql_statement =
	"SELECT sql FROM sqlite_master WHERE type = 'table' AND tbl_name = 'gpkg_extensions'";
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
	  while (isspace (*token))
	    {
		token++;
	    }
	  if (strncasecmp (token, "CONSTRAINT", strlen ("CONSTRAINT")) == 0)
	    {
		/* unique constraint has embedded ','s so we need to grab the next two tokens */
		char *nexttoken1 = strtok_r (str, ",", &saveptr);
		char *nexttoken2 = strtok_r (str, ",", &saveptr);
		char *fulltoken =
		    malloc (strlen (token) + strlen (",") +
			    strlen (nexttoken1) + strlen (",") +
			    strlen (nexttoken2) + 1);
		strcpy (fulltoken, token);
		strcat (fulltoken, ",");
		strcat (fulltoken, nexttoken1);
		strcat (fulltoken, ",");
		strcat (fulltoken, nexttoken2);
		if (strcasestr
		    (fulltoken,
		     "UNIQUE (table_name, column_name, extension_name)"))
		  {
		      valid_fk_constraint = true;
		  }
		continue;
	    }
	  for (i = 0; tableValues[i].column_name != NULL; ++i)
	    {

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
