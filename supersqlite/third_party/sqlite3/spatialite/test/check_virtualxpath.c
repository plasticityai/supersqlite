/*

 check_virtualxpath.c -- SpatiaLite Test Case

 Author: Sandro Furieri <a.furieri@lqt.it>

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
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

static int
load_xml (void *cache, sqlite3 * db_handle, sqlite3_stmt * stmt,
	  const char *path)
{
#ifdef ENABLE_LIBXML2		/* only if LIBXML2 is supported */
/* loading an XMLDocument into the DB */
    FILE *fl;
    int sz = 0;
    int rd;
    unsigned char *xml = NULL;
    unsigned char *p_result = NULL;
    int len;
    int ret;

/* loading the XMLDocument */
    fl = fopen (path, "rb");
    if (!fl)
      {
	  fprintf (stderr, "cannot open \"%s\"\n", path);
	  return 0;
      }
    if (fseek (fl, 0, SEEK_END) == 0)
	sz = ftell (fl);
    xml = malloc (sz);
    rewind (fl);
    rd = fread (xml, 1, sz, fl);
    if (rd != sz)
      {
	  fprintf (stderr, "read error \"%s\"\n", path);
	  return 0;
      }
    fclose (fl);

/* parsing the XMLDocument */
    gaiaXmlToBlob (cache, xml, rd, 1, NULL, &p_result, &len, NULL, NULL);
    if (p_result == NULL)
      {
	  fprintf (stderr, "\"%s\": not a well-formed XML !!!\n", path);
	  return 0;
      }
    free (xml);

/* inserting into the DB */
    sqlite3_reset (stmt);
    sqlite3_clear_bindings (stmt);
    sqlite3_bind_blob (stmt, 1, p_result, len, free);
    sqlite3_bind_text (stmt, 2, path, strlen (path), SQLITE_STATIC);
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	return 1;
    fprintf (stderr, "INSERT error: %s\n", sqlite3_errmsg (db_handle));
    return 0;
#else
    return -1;			/* no support */
#endif
}

int
main (int argc, char *argv[])
{
#ifdef ENABLE_LIBXML2		/* only if LIBXML2 is supported */
    sqlite3 *db_handle = NULL;
    sqlite3_stmt *stmt;
    char *sql_statement;
    int ret;
    char *err_msg = NULL;
    char **results;
    int rows;
    int columns;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret =
	sqlite3_open_v2 (":memory:", &db_handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (db_handle));
	  sqlite3_close (db_handle);
	  db_handle = NULL;
	  return -1;
      }
    spatialite_init_ex (db_handle, cache, 0);

/* creating the base XML table */
    sql_statement = "CREATE TABLE test (id INTEGER PRIMARY "
	"KEY AUTOINCREMENT, xml BLOB, name TEXT)";
    ret = sqlite3_exec (db_handle, sql_statement, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CreateTable error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

/* loading the XMLDocuments into the DB */
    sql_statement = "INSERT INTO test (id, xml, name) VALUES (NULL, ?, ?)";
    ret =
	sqlite3_prepare_v2 (db_handle, sql_statement, strlen (sql_statement),
			    &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "PrepareStatement: error \"%s\"\n",
		   sqlite3_errmsg (db_handle));
	  return -3;
      }
    if (!load_xml (cache, db_handle, stmt, "books.xml"))
      {
	  fprintf (stderr, "Unable to load \"books.xml\"");
	  return -4;
      }
    if (!load_xml (cache, db_handle, stmt, "opera.xml"))
      {
	  fprintf (stderr, "Unable to load \"opera.xml\"");
	  return -5;
      }
    if (!load_xml (cache, db_handle, stmt, "movies.xml"))
      {
	  fprintf (stderr, "Unable to load \"movies.xml\"");
	  return -6;
      }
    if (!load_xml (cache, db_handle, stmt, "books.xml"))
      {
	  fprintf (stderr, "Unable to re-load yet again \"books.xml\"");
	  return -7;
      }
    if (!load_xml (cache, db_handle, stmt, "inspire-data-example.xml"))
      {
	  fprintf (stderr, "Unable to load \"books-bad.xml\"");
	  return -8;
      }
    ret = sqlite3_finalize (stmt);

/* checking for parse errors */
    sql_statement = "SELECT XB_GetLastParseError()";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -10;
      }
    sqlite3_free_table (results);

/* validating the XMLDocuments */
    sql_statement = "UPDATE test SET xml = XB_SchemaValidate(xml, "
	"XB_GetInternalSchemaURI(XB_GetPayload(xml))) "
	"WHERE XB_GetInternalSchemaURI(XB_GetPayload(xml)) IS NOT NULL "
	"AND name <> 'inspire-data-example.xml'";
    ret = sqlite3_exec (db_handle, sql_statement, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "SchemaValidation error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

/* checking for validation errors */
    sql_statement = "SELECT XB_GetLastValidateError()";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -13;
      }
    sqlite3_free_table (results);

/* check #1: validity */
    sql_statement = "SELECT Count(*) FROM test WHERE XB_IsValid(xml) = 1";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -15;
      }
    if (strcmp (results[1], "5") != 0)
      {
	  fprintf (stderr, "Unexpected error: XB_IsValid() bad result: %s.\n",
		   results[1]);
	  return -16;
      }
    sqlite3_free_table (results);

/* check #2: validated */
    sql_statement =
	"SELECT Count(*) FROM test WHERE XB_IsSchemaValidated(xml) = 1";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -17;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -18;
      }
    if (strcmp (results[1], "4") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: XB_IsSchemaValidated() bad result: %s.\n",
		   results[1]);
	  return -19;
      }
    sqlite3_free_table (results);

/* freeing the Schema cache */
    sql_statement = "SELECT XB_CacheFlush()";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -20;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -21;
      }
    if (strcmp (results[1], "1") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: XB_CacheFlush() bad result: %s.\n",
		   results[1]);
	  return -22;
      }
    sqlite3_free_table (results);

/* check #3: validated */
    sql_statement =
	"SELECT XB_GetFileId(xml), XB_GetParentId(xml), XB_GetTitle(xml), "
	"XB_GetAbstract(xml), ST_Srid(XB_GetGeometry(xml)) "
	"FROM test WHERE XB_IsIsoMetadata(xml) = 1";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }
    if ((rows != 1) || (columns != 5))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -24;
      }
    if (strcmp (results[5], "029097fd-2ef2-487c-a5ca-6ec7a3dbac53") != 0)
      {
	  fprintf (stderr, "Unexpected error: XB_GetFileId() bad result: %s.\n",
		   results[5]);
	  return -25;
      }
    if (strcmp (results[6], "024027fd-3ef2-487c-a8ca-6ec8a3dfac57") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: XB_GetParentId() bad result: %s.\n",
		   results[6]);
	  return -26;
      }
    if (strcmp (results[7], "Image2000 Product 1 (nl2) Multispectral") != 0)
      {
	  fprintf (stderr, "Unexpected error: XB_GetTitle() bad result: %s.\n",
		   results[7]);
	  return -27;
      }
    if (strcmp
	(results[8],
	 "IMAGE2000 product 1 individual orthorectified scenes.") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: XB_GetAbstract() bad result: %s.\n",
		   results[8]);
	  return -28;
      }
    if (strcmp (results[9], "4326") != 0)
      {
	  fprintf (stderr,
		   "Unexpected error: ST_Srid(XB_Geometry()) bad result: %s.\n",
		   results[9]);
	  return -29;
      }
    sqlite3_free_table (results);

/* creating the VirtualXPath table */
    sql_statement =
	"CREATE VIRTUAL TABLE test_xp USING VirtualXPath ('test', 'xml')";
    ret = sqlite3_exec (db_handle, sql_statement, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CreateTable error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }

/* test XPAth expression #1 */
    sql_statement = "SELECT Count(*) FROM test_xp WHERE xpath_expr = '/*'";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -31;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -32;
      }
    if (strcmp (results[1], "5") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #1 bad result: %s.\n",
		   results[1]);
	  return -33;
      }
    sqlite3_free_table (results);

/* test XPAth expression #2 */
    sql_statement = "SELECT DISTINCT a.value, b.name FROM test_xp AS a "
	"JOIN test AS b ON (b.id = a.pkid) "
	"WHERE a.xpath_expr = '//author[@lastName=\"Conrad\"]/title/text()'";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -34;
      }
    if ((rows != 3) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -35;
      }
    if (strcmp (results[0], "value") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #2 bad result: %s.\n",
		   results[0]);
	  return -36;
      }
    if (strcmp (results[1], "name") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #2 bad result: %s.\n",
		   results[1]);
	  return -37;
      }
    if (strcmp (results[2], "Lord Jim") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #2 bad result: %s.\n",
		   results[2]);
	  return -38;
      }
    if (strcmp (results[3], "books.xml") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #2 bad result: %s.\n",
		   results[3]);
	  return -39;
      }
    if (strcmp (results[4], "The Secret Agent") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #2 bad result: %s.\n",
		   results[4]);
	  return -40;
      }
    if (strcmp (results[6], "Heart of Darkness") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #2 bad result: %s.\n",
		   results[6]);
	  return -41;
      }
    sqlite3_free_table (results);

/* test XPAth expression #3 */
    sql_statement = "SELECT value FROM test_xp WHERE xpath_expr = "
	"'//dflt:title[text()=\"Pulp Fiction\"]/../@lastName'";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -42;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -43;
      }
    if (strcmp (results[1], "Tarantino") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #3 bad result: %s.\n",
		   results[1]);
	  return -44;
      }
    sqlite3_free_table (results);

/* test XPAth expression #4 */
    sql_statement =
	"SELECT node FROM test_xp WHERE pkid = 2 AND xpath_expr = '/*'";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -45;
      }
    if ((rows != 1) || (columns != 1))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -46;
      }
    if (strcmp (results[1], "op:Opera") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #4 bad result: %s.\n",
		   results[1]);
	  return -47;
      }
    sqlite3_free_table (results);

/* test XPAth expression #5 */
    sql_statement = "SELECT ROWID, pkid, sub, parent, attribute FROM test_xp "
	"WHERE pkid > 1 AND pkid < 3 AND xpath_expr = '/*'";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -48;
      }
    if ((rows != 1) || (columns != 5))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -49;
      }
    if (strcmp (results[5], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #5 bad result: %s.\n",
		   results[5]);
	  return -50;
      }
    if (strcmp (results[6], "2") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #5 bad result: %s.\n",
		   results[6]);
	  return -51;
      }
    if (strcmp (results[7], "0") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #5 bad result: %s.\n",
		   results[7]);
	  return -52;
      }
    if (results[8] != NULL)
      {
	  fprintf (stderr, "Unexpected error: XPath #5 bad result: %s.\n",
		   results[8]);
	  return -53;
      }
    if (results[9] != NULL)
      {
	  fprintf (stderr, "Unexpected error: XPath #5 bad result: %s.\n",
		   results[9]);
	  return -54;
      }
    sqlite3_free_table (results);

/* test XPAth expression #6 */
    sql_statement = "SELECT parent, attribute FROM test_xp "
	"WHERE pkid >= 1 AND pkid <= 3 AND "
	"xpath_expr = '//op:author[@lastName=\"Rossini\"]/@firstName'";
    ret =
	sqlite3_get_table (db_handle, sql_statement, &results, &rows, &columns,
			   &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -55;
      }
    if ((rows != 1) || (columns != 2))
      {
	  fprintf (stderr,
		   "Unexpected error: select columns bad result: %i/%i.\n",
		   rows, columns);
	  return -56;
      }
    if (strcmp (results[2], "op:Opera") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #6 bad result: %s.\n",
		   results[2]);
	  return -57;
      }
    if (strcmp (results[3], "firstName") != 0)
      {
	  fprintf (stderr, "Unexpected error: XPath #6 bad result: %s.\n",
		   results[3]);
	  return -58;
      }
    sqlite3_free_table (results);

/* dropping the VirtualXPathL table */
    sql_statement = "DROP TABLE test_xp";
    ret = sqlite3_exec (db_handle, sql_statement, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Drop VirtualXPath Table error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  return -59;
      }

    sqlite3_close (db_handle);
    spatialite_cleanup_ex (cache);

#endif /* end LIBXML2 conditional */

    spatialite_shutdown ();
    return 0;
}
