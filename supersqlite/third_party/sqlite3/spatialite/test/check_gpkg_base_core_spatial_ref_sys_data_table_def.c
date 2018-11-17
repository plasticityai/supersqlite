/*

 check_gpkg_base_core_spatial_ref_sys_data_table_def - Test case for GeoPackage Extensions

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

#define __USE_GNU
#include <string.h>

#include "sqlite3.h"
#include "spatialite.h"

#include "test_helpers.h"

int
main (int argc UNUSED, char *argv[]UNUSED)
{
    sqlite3 *db_handle = NULL;
    int ret;
    char *err_msg = NULL;
    char *sql_statement;
    sqlite3_stmt *stmt;
    void *cache = spatialite_alloc_connection ();
    int j;
    char *sql;
    char *str;
    char *token;
    char *saveptr;
    bool valid_srs_name = false;
    bool valid_srs_id = false;
    bool valid_organization = false;
    bool valid_organization_coordsys_id = false;
    bool valid_definition = false;
    bool valid_description = false;


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
       / Test Case ID: /base/core/spatial_ref_sys/data/table_def
       / Test Purpose: Verify that the gpkg_spatial_ref_sys table exists and has the correct definition.
       / Test Method:
       / 1) SELECT sql FROM sqlite_master WHERE type = 'table' AND tbl_name = 'gpkg_spatial_ref_sys'
       / 2) Fail if returns an empty result set
       / 3) Pass if column names and column definitions in the returned CREATE TABLE statement in the sql column value, including data type, nullability, and primary key
       /     constraints match all of those in the contents of C.1 Table 18. Column order, check constraint and trigger definitions, and other column definitions in the returned sql are
       /     irrelevant.
       / 4) Fail otherwise.
       / Reference: Clause 1.1.2.1.1 Req 10:
       / Test Type: Basic
     */

    sql_statement =
	"SELECT sql FROM sqlite_master WHERE type = 'table' AND tbl_name = 'gpkg_spatial_ref_sys'";
    ret =
	sqlite3_prepare_v2 (db_handle, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "failed to prepare SQL SELECT statement: %i (%s)\n",
		   ret, sqlite3_errmsg (db_handle));
	  return -30;
      }
    ret = sqlite3_step (stmt);
    if (ret != SQLITE_ROW)
      {
	  fprintf (stderr, "unexpected return value for first step: %i (%s)\n",
		   ret, sqlite3_errmsg (db_handle));
	  return -31;
      }
    if (sqlite3_column_type (stmt, 0) != SQLITE_TEXT)
      {
	  fprintf (stderr, "bad type for column 0: %i\n",
		   sqlite3_column_type (stmt, 0));
	  return -32;
      }
    sql =
	strdup (strstr ((const char *) sqlite3_column_text (stmt, 0), "(") +
		strlen ("("));
    if (!sql)
      {
	  fprintf (stderr, "unrecognised format for SQL\n");
	  return -33;
      }
    /*
       / Column Name                Column Type   Column Description                                                            Null     Key
       / srs_name                   TEXT          Human readable name of this SRS                                               no
       / srs_id                     INTEGER       Unique identifier for each Spatial Reference System within a GeoPackage       no       PK
       / organization               TEXT          Case-insensitive name of the defining organization e.g. EPSG or epsg          no
       / organization_coordsys_id   INTEGER       Numeric ID of the Spatial Reference System assigned by the organization       no
       / definition                 TEXT          Well-known Text Representation of the Spatial Reference System                no
       / description                TEXT          Human readable description of this SRS                                        yes
     */

    for (j = 1, str = sql;; j++, str = NULL)
      {
	  token = strtok_r (str, ",", &saveptr);
	  if (token == NULL)
	    {
		break;
	    }
	  if (strstr (token, "srs_name "))
	    {
		if (!strstr (token, "TEXT"))
		  {
		      fprintf (stderr,
			       "missing COLUMN TYPE TEXT for srs_name: %s\n",
			       token);
		      return -34;
		  }
		if (!strstr (token, "NOT NULL"))
		  {
		      fprintf (stderr,
			       "Missing NOT NULL constraint for srs_name: %s\n",
			       token);
		      return -35;
		  }
		if (strstr (token, "KEY"))
		  {
		      fprintf (stderr,
			       "Unexpected KEY constraint for srs_name: %s\n",
			       token);
		      return -36;
		  }
		valid_srs_name = true;
	    }
	  if (strstr (token, "srs_id "))
	    {
		if (!strstr (token, "INTEGER"))
		  {
		      fprintf (stderr,
			       "missing COLUMN TYPE INTEGER for srs_id: %s\n",
			       token);
		      return -37;
		  }
		if (!strstr (token, "NOT NULL"))
		  {
		      fprintf (stderr,
			       "Missing NOT NULL constraint for srs_id: %s\n",
			       token);
		      return -38;
		  }
		if (!strstr (token, "PRIMARY KEY"))
		  {
		      fprintf (stderr,
			       "Missing PRIMARY KEY constraint for srs_id: %s\n",
			       token);
		      return -39;
		  }
		valid_srs_id = true;
	    }
	  if (strstr (token, "organization "))
	    {
		if (!strstr (token, "TEXT"))
		  {
		      fprintf (stderr,
			       "missing COLUMN TYPE TEXT for organization: %s\n",
			       token);
		      return -40;
		  }
		if (!strstr (token, "NOT NULL"))
		  {
		      fprintf (stderr,
			       "Missing NOT NULL constraint for organization: %s\n",
			       token);
		      return -41;
		  }
		if (strstr (token, "KEY"))
		  {
		      fprintf (stderr,
			       "Unexpected KEY constraint for organization: %s\n",
			       token);
		      return -42;
		  }
		valid_organization = true;
	    }
	  if (strstr (token, "organization_coordsys_id "))
	    {
		if (!strstr (token, "INTEGER"))
		  {
		      fprintf (stderr,
			       "missing COLUMN TYPE INTEGER for organization_coordsys_id: %s\n",
			       token);
		      return -43;
		  }
		if (!strstr (token, "NOT NULL"))
		  {
		      fprintf (stderr,
			       "Missing NOT NULL constraint for organization_coordsys_id: %s\n",
			       token);
		      return -44;
		  }
		if (strstr (token, "KEY"))
		  {
		      fprintf (stderr,
			       "Unexpected PRIMARY KEY constraint for organization_coordsys_id: %s\n",
			       token);
		      return -45;
		  }
		valid_organization_coordsys_id = true;
	    }
	  if (strstr (token, "definition "))
	    {
		if (!strstr (token, "TEXT"))
		  {
		      fprintf (stderr,
			       "missing COLUMN TYPE TEXT for definition: %s\n",
			       token);
		      return -46;
		  }
		if (!strstr (token, "NOT NULL"))
		  {
		      fprintf (stderr,
			       "Missing NOT NULL constraint for definition: %s\n",
			       token);
		      return -47;
		  }
		if (strstr (token, "KEY"))
		  {
		      fprintf (stderr,
			       "Unexpected KEY constraint for definition: %s\n",
			       token);
		      return -48;
		  }
		valid_definition = true;
	    }
	  if (strstr (token, "description "))
	    {
		if (!strstr (token, "TEXT"))
		  {
		      fprintf (stderr,
			       "missing COLUMN TYPE TEXT for description: %s\n",
			       token);
		      return -49;
		  }
		if (strstr (token, "NOT NULL"))
		  {
		      fprintf (stderr,
			       "Unexpected NOT NULL constraint for description: %s\n",
			       token);
		      return -50;
		  }
		if (strstr (token, "KEY"))
		  {
		      fprintf (stderr,
			       "Unexpected KEY constraint for description: %s\n",
			       token);
		      return -51;
		  }
		valid_description = true;
	    }
      }
    free (sql);

    if (!valid_srs_name)
      {
	  fprintf (stderr,
		   "did not find expected srs_name column, or required properties were not found\n");
	  return -99;
      }
    if (!valid_srs_id)
      {
	  fprintf (stderr,
		   "did not find expected srs_id column, or required properties were not found\n");
	  return -100;
      }
    if (!valid_organization)
      {
	  fprintf (stderr,
		   "did not find expected organization column, or required properties were not found\n");
	  return -101;
      }
    if (!valid_organization_coordsys_id)
      {
	  fprintf (stderr,
		   "did not find expected organization_coordsys_id column, or required properties were not found\n");
	  return -102;
      }
    if (!valid_definition)
      {
	  fprintf (stderr,
		   "did not find expected definition column, or required properties were not found\n");
	  return -103;
      }
    if (!valid_description)
      {
	  fprintf (stderr,
		   "did not find expected description column, or required properties were not found\n");
	  return -104;
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
