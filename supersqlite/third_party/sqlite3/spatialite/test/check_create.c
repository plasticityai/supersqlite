/*

 check_create.c -- SpatiaLite Test Case

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

The Original Code is the SpatiaLite library

The Initial Developer of the Original Code is Alessandro Furieri
 
Portions created by the Initial Developer are Copyright (C) 2011
the Initial Developer. All Rights Reserved.

Contributor(s):
Brad Hards <bradh@frogmouth.net>

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

#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gaiaaux.h"

static int
checkCache (void)
{
    int i;
    int max = 1024;
    void *cache[1024];
    char msg[64];
    const char *m;

    for (i = 0; i < max; i++)
      {
	  cache[i] = spatialite_alloc_connection ();
      }

    for (i = 0; i < max; i++)
      {
	  if (cache[i] != NULL)
	    {
		sprintf (msg, "Err%d", i);
		gaiaSetGeosErrorMsg_r (cache[i], msg);
		sprintf (msg, "Warn%d", i);
		gaiaSetGeosWarningMsg_r (cache[i], msg);
		sprintf (msg, "Aux%d", i);
		gaiaSetGeosAuxErrorMsg_r (cache[i], msg);
	    }
      }

    for (i = 0; i < max; i++)
      {
	  if (cache[i] != NULL)
	    {
		sprintf (msg, "Err%d", i);
		m = gaiaGetGeosErrorMsg_r (cache[i]);
		if (m == NULL)
		  {
		      fprintf (stderr, "unexpected NULL: GeosErrorMsg %d\n", i);
		      return -200 + i;
		  }
		if (strcmp (m, msg) != 0)
		  {
		      fprintf (stderr, "unexpected GeosErrorMsg %d \"%s\"\n", i,
			       m);
		      return -300 + i;
		  }
		sprintf (msg, "Warn%d", i);
		m = gaiaGetGeosWarningMsg_r (cache[i]);
		if (m == NULL)
		  {
		      fprintf (stderr, "unexpected NULL: GeosWarningMsg %d\n",
			       i);
		      return -400 + i;
		  }
		if (strcmp (m, msg) != 0)
		  {
		      fprintf (stderr,
			       "unexpected GeosWarningErrorMsg %d \"%s\"\n", i,
			       m);
		      return -500 + i;
		  }
		sprintf (msg, "Aux%d", i);
		m = gaiaGetGeosAuxErrorMsg_r (cache[i]);
		if (m == NULL)
		  {
		      fprintf (stderr, "unexpected NULL: GeosAuxErrorMsg %d\n",
			       i);
		      return -600 + i;
		  }
		if (strcmp (m, msg) != 0)
		  {
		      fprintf (stderr, "unexpected GeosAuxErrorMsg %d \"%s\"\n",
			       i, m);
		      return -700 + i;
		  }
	    }
      }

    for (i = 0; i < max; i++)
      {
	  if (cache[i] != NULL)
	      spatialite_cleanup_ex (cache[i]);
      }
    return 0;
}

int
main (int argc, char *argv[])
{
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    sqlite3_int64 log_pk;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }

    spatialite_init_ex (handle, cache, 0);

    ret =
	sqlite3_exec (handle, "SELECT InitSpatialMetadata(1)", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "InitSpatialMetadata() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -2;
      }
    ret = sqlite3_exec (handle, "SELECT HasProj()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasProj() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -3;
      }
    ret = sqlite3_exec (handle, "SELECT HasGeos()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasGeos() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -4;
      }
    ret =
	sqlite3_exec (handle, "SELECT HasGeosAdvanced()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasGeosAdvanced() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -5;
      }
    ret = sqlite3_exec (handle, "SELECT HasIconv()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasIconv() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -6;
      }
    ret = sqlite3_exec (handle, "SELECT HasMathSql()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasMathSql() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -7;
      }
    ret =
	sqlite3_exec (handle, "SELECT HasGeoCallbacks()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasGeoCallbacks() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -8;
      }
    ret = sqlite3_exec (handle, "SELECT HasFreeXL()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasFreeXL() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -9;
      }
    ret = sqlite3_exec (handle, "SELECT HasEpsg()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasEpsg() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -10;
      }
    ret = sqlite3_exec (handle, "SELECT HasGeosTrunk()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasGeoTrunk() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -11;
      }
    ret = sqlite3_exec (handle, "SELECT HasLwGeom()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasLwGeom() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -12;
      }
    ret = sqlite3_exec (handle, "SELECT HasLibXml2()", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "HasLibXml2() error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -13;
      }

    gaiaInsertIntoSqlLog (handle, "test", "sql_statement_ok", &log_pk);
    gaiaUpdateSqlLog (handle, log_pk, 1, NULL);
    gaiaInsertIntoSqlLog (handle, "test", "sql_statement_no", &log_pk);
    gaiaUpdateSqlLog (handle, log_pk, 0, "some error message");

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -11;
      }

    spatialite_cleanup_ex (cache);

    ret = checkCache ();
    if (ret != 0)
	return ret;
    spatialite_shutdown ();

    return 0;
}
