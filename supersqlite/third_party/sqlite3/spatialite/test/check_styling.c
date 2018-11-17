
/*

 check_styling.c -- SpatiaLite Test Case

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
 
Portions created by the Initial Developer are Copyright (C) 2013
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

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

int
execute_check (sqlite3 * sqlite, const char *sql, char **error)
{
/* executing an SQL statement returning True/False */
    sqlite3_stmt *stmt;
    int ret;
    int retcode = 0;

    if (error != NULL)
	*error = NULL;
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  if (error != NULL)
	      *error = sqlite3_mprintf ("%s", sqlite3_errmsg (sqlite));
	  return SQLITE_ERROR;
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
      {
	  if (sqlite3_column_int (stmt, 0) == 1)
	      retcode = 1;
      }
    sqlite3_finalize (stmt);
    if (retcode == 1)
	return SQLITE_OK;
    return SQLITE_ERROR;
}

static unsigned char *
load_blob (const char *path, int *blob_len)
{
/* loading an external image */
    unsigned char *blob;
    int sz = 0;
    int rd;
    FILE *fl = fopen (path, "rb");
    if (!fl)
      {
	  fprintf (stderr, "cannot open \"%s\"\n", path);
	  return NULL;
      }
    if (fseek (fl, 0, SEEK_END) == 0)
	sz = ftell (fl);
    blob = (unsigned char *) malloc (sz);
    *blob_len = sz;
    rewind (fl);
    rd = fread (blob, 1, sz, fl);
    if (rd != sz)
      {
	  fprintf (stderr, "read error \"%s\"\n", path);
	  return NULL;
      }
    fclose (fl);
    return blob;
}

static unsigned char *
load_xml (const char *path, int *len)
{
/* loading an external XML */
    unsigned char *xml;
    int sz = 0;
    int rd;
    FILE *fl = fopen (path, "rb");
    if (!fl)
      {
	  fprintf (stderr, "cannot open \"%s\"\n", path);
	  return NULL;
      }
    if (fseek (fl, 0, SEEK_END) == 0)
	sz = ftell (fl);
    xml = malloc (sz + 1);
    *len = sz;
    rewind (fl);
    rd = fread (xml, 1, sz, fl);
    if (rd != sz)
      {
	  fprintf (stderr, "read error \"%s\"\n", path);
	  return NULL;
      }
    fclose (fl);
    xml[rd] = '\0';
    return xml;
}

static char *
build_hex_blob (const unsigned char *blob, int blob_len)
{
/* building an HEX blob */
    int i;
    const unsigned char *p_in = blob;
    char *hex = malloc ((blob_len * 2) + 1);
    char *p_out = hex;
    for (i = 0; i < blob_len; i++)
      {
	  sprintf (p_out, "%02x", *p_in);
	  p_in++;
	  p_out += 2;
      }
    return hex;
}

static int
check_vector (sqlite3 * handle, void *cache)
{
/* testing Vector Styles */
    int ret;
    char *err_msg = NULL;
    char *sql;
    unsigned char *blob;
    int blob_len;
    char *hexBlob;
    unsigned char *xml;
    int len;

/* testing External Graphic */
    blob = load_blob ("empty.png", &blob_len);
    if (blob == NULL)
	return -4;
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -5;
    sql =
	sqlite3_mprintf ("SELECT SE_RegisterExternalGraphic('url-A', x%Q)",
			 hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterExternalGraphic #1: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterExternalGraphic('url-A', x%Q, 'title', 'abstract', 'file_name')",
	 hexBlob);
    free (hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterExternalGraphic #2: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }

    xml = load_xml ("thunderstorm_mild.svg", &len);
    if (xml == NULL)
	return -8;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -9;
    sql =
	sqlite3_mprintf ("SELECT SE_RegisterExternalGraphic('url-A', x%Q)",
			 hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterExternalGraphic #3: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterExternalGraphic('url-B', x%Q, 'title', 'abstract', 'file_name')",
	 hexBlob);
    free (hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterExternalGraphic #4: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

/* creating two  vector Tables */
    sql = "CREATE TABLE table1 (id INTEGER PRIMARY KEY AUTOINCREMENT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error Create Table table1: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }
    sql = "CREATE TABLE table2 (id INTEGER PRIMARY KEY AUTOINCREMENT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error Create Table table2: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }
    sql = "SELECT AddGeometryColumn('table1', 'geom', 4326, 'POINT', 'XY')";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error AddGeometryColumn table1: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }
    sql = "SELECT AddGeometryColumn('table2', 'geom', 32632, 'POINT', 'XY')";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error AddGeometryColumn table2: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -15;
      }

/* registering two Vector Coverages */
    sql = "SELECT SE_RegisterVectorCoverage('table1', 'table1', 'geom')";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverage table1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -16;
      }
    sql =
	"SELECT SE_RegisterVectorCoverage('table2', 'table2', 'geom', 'title-2', 'abstract-2')";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverage table2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -17;
      }
    sql = "SELECT SE_SetVectorCoverageInfos('table1', 'title-1', 'abstract-1')";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverage table1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -18;
      }

/* testing Vector Styles */
    xml = load_xml ("stazioni_se.xml", &len);
    if (xml == NULL)
	return -19;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -20;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -21;

/* Register Vector Styled Layer */
    sql = sqlite3_mprintf ("SELECT SE_RegisterVectorStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorStyle #1: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }

    sql = sqlite3_mprintf ("SELECT SE_RegisterVectorStyledLayer('table1',  1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorStyledLayer #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterVectorStyledLayer('table2', 'Railway Stations')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorStyledLayer #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }
    free (hexBlob);

    xml = load_xml ("stazioni2_se.xml", &len);
    if (xml == NULL)
	return -25;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -26;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -27;

    sql = sqlite3_mprintf ("SELECT SE_RegisterVectorStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorStyle #2: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -29;
      }

    sql = sqlite3_mprintf ("SELECT SE_RegisterVectorStyledLayer('table2', 2)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorStyledLayer #3: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -30;
      }

    sql = sqlite3_mprintf ("SELECT SE_RegisterVectorStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorStyle #3: %s\n\n",
		   "expected failure");
	  return -31;
      }

/* Reload Vector Style */
    sql = sqlite3_mprintf ("SELECT SE_ReloadVectorStyle(12, x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadVectorStyle #1: %s\n\n",
		   "expected failure");
	  return -33;
      }

    sql = sqlite3_mprintf ("SELECT SE_ReloadVectorStyle(1, x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadVectorStyle #2: %s\n\n",
		   "expected failure");
	  return -34;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_ReloadVectorStyle('Railway Stations', x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadVectorStyle #4: %s\n\n",
		   "expected failure");
	  return -36;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_ReloadVectorStyle('Railway Stations', x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadVectorStyle #5: %s\n\n",
		   "expected failure");
	  return -37;
      }
    free (hexBlob);

    xml = load_xml ("stazioni_se.xml", &len);
    if (xml == NULL)
	return -35;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -39;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -40;
    sql = sqlite3_mprintf ("SELECT SE_ReloadVectorStyle(1, x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadVectorStyle #7: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -41;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_ReloadVectorStyle('Railway Stations 2', x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadVectorStyle #8: %s\n\n",
		   "expected failure");
	  return -42;
      }
    free (hexBlob);

/* Unregister Vector Style */
    sql = sqlite3_mprintf ("SELECT SE_UnRegisterVectorStyle(5)");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterVectorStyle #1: %s\n\n",
		   "expected failure");
	  return -43;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterVectorStyle('alpha')");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterVectorStyle #2: %s\n\n",
		   "expected failure");
	  return -44;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_UnRegisterVectorStyle('Railway Stations 2')");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterVectorStyle #3: %s\n\n",
		   "expected failure");
	  return -45;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_UnRegisterVectorStyle('Railway Stations 2', 1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterVectorStyle #5: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -47;
      }

/* Register Vector Styled Layer: again */
    xml = load_xml ("stazioni2_se.xml", &len);
    if (xml == NULL)
	return -48;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -49;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -50;

    sql = sqlite3_mprintf ("SELECT SE_RegisterVectorStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorStyle #3: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -51;
      }
    free (hexBlob);

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterVectorStyledLayer('table1', 'Railway Stations 2')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorStyledLayer #5: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -53;
      }

/* Unregister Vector Style Layer */
    sql =
	sqlite3_mprintf
	("SELECT SE_UnRegisterVectorStyledLayer('table1', 'Railway Stations 2')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterVectorStyledLayer #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -54;
      }

    sql =
	sqlite3_mprintf ("SELECT SE_UnRegisterVectorStyledLayer('table1', 1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterVectorStyledLayer #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -55;
      }

/* unregister External Graphic */
    sql = sqlite3_mprintf ("SELECT SE_UnRegisterExternalGraphic('url-A')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterExternalGraphic #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -56;
      }
    sql = sqlite3_mprintf ("SELECT SE_UnRegisterExternalGraphic('jeroboam')");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterExternalGraphic #2: %s\n\n",
		   "expected failure");
	  return -57;
      }

    return 0;
}

static int
check_raster (sqlite3 * handle, void *cache)
{
/* testing Raster Styles */
    int ret;
    char *err_msg = NULL;
    char *sql;
    unsigned char *blob;
    int blob_len;
    char *hexBlob;
    unsigned char *xml;
    int len;

/* testing Raster Styles */
    xml = load_xml ("raster_se.xml", &len);
    if (xml == NULL)
	return -4;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -5;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -6;

/* Register Raster Styled Layer */
    sql = sqlite3_mprintf ("SELECT SE_RegisterRasterStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterRasterStyle #1: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterRasterStyledLayer('coverage_srtm1', 1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterRasterStyledLayer #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -8;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterRasterStyledLayer('coverage_srtm2', 'srtm_style')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterRasterStyledLayer #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }
    free (hexBlob);

    xml = load_xml ("raster2_se.xml", &len);
    if (xml == NULL)
	return -10;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -11;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -12;

    sql = sqlite3_mprintf ("SELECT SE_RegisterRasterStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterRasterStyle #2: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterRasterStyledLayer('srtm2_style', 2)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterRasterStyledLayer #3: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }

    sql = sqlite3_mprintf ("SELECT SE_RegisterRasterStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterRasterStyle #3: %s\n\n",
		   "expected failure");
	  return -15;
      }

/* Reload Raster Style */
    sql = sqlite3_mprintf ("SELECT SE_ReloadRasterStyle(12, x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadRasterStyle #1: %s\n\n",
		   "expected failure");
	  return -17;
      }

    sql = sqlite3_mprintf ("SELECT SE_ReloadRasterStyle(1, x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadRasterStyle #2: %s\n\n",
		   "expected failure");
	  return -18;
      }

    sql =
	sqlite3_mprintf ("SELECT SE_ReloadRasterStyle('srtm_style', x%Q)",
			 hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadRasterStyle #4: %s\n\n",
		   "expected failure");
	  return -20;
      }
    free (hexBlob);

    xml = load_xml ("raster_se.xml", &len);
    if (xml == NULL)
	return -22;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -23;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -24;
    sql = sqlite3_mprintf ("SELECT SE_ReloadRasterStyle(1, x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadRasterStyle #6: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_ReloadRasterrStyle('Railway Stations 2', x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadVRasterStyle #4: %s\n\n",
		   "expected failure");
	  return -26;
      }
    free (hexBlob);

/* Unregister Raster Style */
    sql = sqlite3_mprintf ("SELECT SE_UnRegisterRasterStyle(5)");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterRasterStyle #1: %s\n\n",
		   "expected failure");
	  return -27;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterRasterStyle('alpha')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterRasterStyle #2: %s\n\n",
		   "expected failure");
	  return -28;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterRasterStyle('srtm2_style')");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterRasterStyle #3: %s\n\n",
		   "expected failure");
	  return -29;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterRasterStyle('srtm2_style', 1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterRasterStyle #4: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -31;
      }

/* Register Raster Styled Layer: again */
    xml = load_xml ("raster2_se.xml", &len);
    if (xml == NULL)
	return -32;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -33;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -34;

    sql = sqlite3_mprintf ("SELECT SE_RegisterRasterStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterRasterStyle #5: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -35;
      }
    free (hexBlob);

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterRasterStyledLayer('coverage_srtm1', 'srtm2_style')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterRasterStyledLayer #5: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -37;
      }

/* Unregister Raster Style Layer */
    sql =
	sqlite3_mprintf
	("SELECT SE_UnRegisterRasterStyledLayer('coverage_srtm1', 'srtm2_style')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterRasterStyledLayer #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -39;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_UnRegisterRasterStyledLayer('coverage_srtm1', 1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterRasterStyledLayer #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -40;
      }

    return 0;
}

static int
check_group (sqlite3 * handle, void *cache)
{
/* testing Group Styles */
    int ret;
    char *err_msg = NULL;
    char *sql;
    unsigned char *blob;
    int blob_len;
    char *hexBlob;
    unsigned char *xml;
    int len;

/* testing Groups */
    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupRaster('group1', 'coverage_srtm1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupRaster #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -4;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_SetStyledGroupInfos('group1', 'title', 'abstract')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SetStyledGroupInfos #1: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -5;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupVector('group1', 'table1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupVector #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -6;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupVector('group1', 'table2')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupVector #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_SetStyledGroupInfos('group1', 'changed title', 'changed abstract')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SetStyledGroupInfos #2: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -8;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_SetStyledGroupInfos('group2', 'title2', 'abstract2')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SetStyledGroupInfos #3: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupRaster('group2', 'coverage_srtm2')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupRaster #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupVector('group2', 'table1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupVector #3: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_UnregisterStyledGroupVector('group2', 'table1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterStyledGroupVector #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupVector('group2', 'table2')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupVector #4: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupVector('group2', 'table1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupVector #5: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_UnregisterStyledGroupRaster('group2', 'coverage_srtm2')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterStyledGroupRaster #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -15;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupRaster('group2', 'coverage_srtm1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupRaster #3: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -16;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupRaster('group2', 'coverage_srtm2')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupRaster #4: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -17;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnregisterStyledGroupLayer(8)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterStyledGroupLayer #3: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -18;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_SetStyledGroupInfos('group3', 'title', 'abstract')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SetStyledGroupInfos #4: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -19;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupRaster('group3', 'coverage_srtm1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroup #10: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -20;
      }

/* testing Paint Order */
    sql = sqlite3_mprintf ("SELECT SE_SetStyledGroupLayerPaintOrder(7, 11)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SetStyledGroupLayerPaintOrder #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_SetStyledGroupVectorPaintOrder('group2', 'table2', 12)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SetStyledGroupVectorPaintOrder #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_SetStyledGroupRasterPaintOrder('group2', 'coverage_srtm2', 10)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SetStyledGroupRasterPaintOrder #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }

    sql = sqlite3_mprintf ("SELECT SE_SetStyledGroupLayerPaintOrder(3, -1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SetStyledGroupLayerPaintOrder #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_SetStyledGroupVectorPaintOrder('group1', 'table1', -1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SetStyledGroupLayerPaintOrder #3: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_SetStyledGroupRasterPaintOrder('group1', 'coverage_srtm1', -1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error SetStyledGroupRasterPaintOrder #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -26;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnregisterStyledGroup('group2')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterStyledGroup #1: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -27;
      }

    xml = load_xml ("sld_sample.xml", &len);
    if (xml == NULL)
	return -29;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -30;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -31;

/* Register Styled Group Style */
    sql = sqlite3_mprintf ("SELECT SE_RegisterGroupStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterGroupStyle #1: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -32;
      }
    free (hexBlob);

    xml = load_xml ("sld_sample2.xml", &len);
    if (xml == NULL)
	return -33;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -34;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -35;

    sql = sqlite3_mprintf ("SELECT SE_RegisterGroupStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterGroupStyle #2: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -36;
      }

    sql = sqlite3_mprintf ("SELECT SE_ReloadGroupStyle(1, x%Q)", hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadGroupStyle #1: %s\n\n",
		   "expected failure");
	  return -37;
      }

    sql =
	sqlite3_mprintf ("SELECT SE_ReloadGroupStyle('group style 1', x%Q)",
			 hexBlob);
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadGroupStyle #2: %s\n\n",
		   "expected failure");
	  return -38;
      }

    sql =
	sqlite3_mprintf ("SELECT SE_ReloadGroupStyle('group style 2', x%Q)",
			 hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error ReloadGroupStyle #3: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -39;
      }
    free (hexBlob);

    xml = load_xml ("sld_sample.xml", &len);
    if (xml == NULL)
	return -42;
    gaiaXmlToBlob (cache, xml, len, 1, NULL, &blob, &blob_len, NULL, NULL);
    free (xml);
    if (blob == NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return -43;
      }
    hexBlob = build_hex_blob (blob, blob_len);
    free (blob);
    if (hexBlob == NULL)
	return -44;

    sql = sqlite3_mprintf ("SELECT SE_UnregisterGroupStyle('group style 1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterGroupStyle #2: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -46;
      }

    sql = sqlite3_mprintf ("SELECT SE_RegisterGroupStyle(x%Q)", hexBlob);
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterGroupStyle #4: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -47;
      }
    free (hexBlob);

    /* testing Styled Group Styles */
    sql = sqlite3_mprintf ("SELECT SE_RegisterStyledGroupStyle('group1', 2)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupStyle #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -50;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupStyle('group1', 'group style 1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupStyle #3: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -51;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterStyledGroupStyle('group2', 'group style 1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterStyledGroupStyle #3: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -52;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterStyledGroupStyle('group1', 3)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterStyledGroupStyle #1: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -53;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_UnRegisterStyledGroupStyle('group2', 'group style 1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterStyledGroupStyle #2: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -54;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterGroupStyle(2, 1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterGroupStyle #3: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -55;
      }

    sql =
	sqlite3_mprintf ("SELECT SE_UnRegisterGroupStyle('group style 1', 1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterGroupStyle #4: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -56;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterVectorStyle(1, 1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterVectorStyle #6: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -57;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterRasterStyle(1)");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnregisterRasterStyle #5: %s\n\n",
		   "expected failure");
	  return -58;
      }

    return 0;
}

static int
check_extent (sqlite3 * handle)
{
/* testing Vector Coverage Extents */
    int ret;
    char *err_msg = NULL;
    char *sql;

/* inserting more alternative SRIDs */
    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterVectorCoverageSrid('table1', 32632)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverageSrid #1 %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -1;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterVectorCoverageSrid('table2', 4326)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverageSrid #2 %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterVectorCoverageSrid('table2', 32633)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverageSrid #3 %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterVectorCoverageSrid('table2', 32632)");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverageSrid #4 %s\n\n",
		   "expected failure");
	  return -4;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterVectorCoverageSrid('table99', 4326)");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverageSrid #5 %s\n\n",
		   "expected failure");
	  return -5;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterVectorCoverageSrid('table99', 4326)");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverageSrid #6 %s\n\n",
		   "expected failure");
	  return -6;
      }

/* inserting few Points just to set an Extent */
    sql =
	sqlite3_mprintf
	("INSERT INTO table1 (id, geom) VALUES (NULL, MakePoint(11.4, 42.3, 4326))");
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error Insert Point #1 %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -7;
      }

    sql =
	sqlite3_mprintf
	("INSERT INTO table1 (id, geom) VALUES (NULL, MakePoint(11.8, 42.3, 4326))");
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error Insert Point #2 %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -8;
      }

    sql =
	sqlite3_mprintf
	("INSERT INTO table1 (id, geom) VALUES (NULL, MakePoint(11.8, 43.2, 4326))");
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error Insert Point #3 %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -9;
      }

    sql =
	sqlite3_mprintf
	("INSERT INTO table2 (id, geom) VALUES (NULL, MakePoint(697831.5121, 4685875.1570, 32632))");
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error Insert Point #4 %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -10;
      }

    sql =
	sqlite3_mprintf
	("INSERT INTO table2 (id, geom) VALUES (NULL, MakePoint(696831.123, 4685875.456, 32632))");
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error Insert Point #5 %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -11;
      }

    sql =
	sqlite3_mprintf
	("INSERT INTO table2 (id, geom) VALUES (NULL, MakePoint(696531.9876, 4688875.4321, 32632))");
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error Insert Point #6 %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -12;
      }

    sql = sqlite3_mprintf ("SELECT SE_UpdateVectorCoverageExtent(1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UpdateVectorCoverageExtent #1 %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -13;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_UnRegisterVectorCoverageSrid('table2', 4326)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterVectorCoverageSrid #1 %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -14;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_UnRegisterVectorCoverageSrid('table2', 32632)");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterVectorCoverageSrid #2 %s\n\n",
		   "expected failure");
	  return -15;
      }

    sql =
	sqlite3_mprintf
	("SELECT SE_UnRegisterVectorCoverageSrid('table99', 4326)");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterVectorCoverageSrid #3 %s\n\n",
		   "expected failure");
	  return -16;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterVectorCoverage('table1')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterVectorCoverage #1 %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -17;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterVectorCoverage('table99')");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterVectorCoverage #2 %s\n\n",
		   "expected failure");
	  return -18;
      }

    sql = sqlite3_mprintf ("SELECT SE_UpdateVectorCoverageExtent('table2', 1)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UpdateVectorCoverageExtent #2 %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -19;
      }

    sql = sqlite3_mprintf ("SELECT SE_UnRegisterVectorCoverage('table1')");
    ret = execute_check (handle, sql, NULL);
    sqlite3_free (sql);
    if (ret == SQLITE_OK)
      {
	  fprintf (stderr, "Error UnRegisterVectorCoverage #3 %s\n\n",
		   "expected failure");
	  return -20;
      }

/* creating a furher vector Tables */
    sql = "CREATE TABLE tablex (id INTEGER PRIMARY KEY AUTOINCREMENT)";
    ret = sqlite3_exec (handle, sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error Create Table table3: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -21;
      }
    sql = "SELECT AddGeometryColumn('tablex', 'geom', 4326, 'POINT', 'XY')";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error AddGeometryColumn tablex: %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -22;
      }
    sql = "SELECT SE_RegisterVectorCoverage('tablex', 'tablex', 'geom')";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverage tablex: %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -23;
      }
    sql =
	sqlite3_mprintf
	("SELECT SE_RegisterVectorCoverageSrid('tablex', 32632)");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error RegisterVectorCoverageSrid #7 %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -24;
      }
    sql = sqlite3_mprintf ("SELECT SE_UpdateVectorCoverageExtent('tablex')");
    ret = execute_check (handle, sql, &err_msg);
    sqlite3_free (sql);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error UpdateVectorCoverageExtent #3 %s\n\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  return -25;
      }

    return 0;
}

int
main (int argc, char *argv[])
{
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    char *sql;
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

    sql = "SELECT InitSpatialMetadata(1, 'WGS84')";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr,
		   "Unexpected InitSpatialMetadata result: %i, (%s)\n", ret,
		   err_msg);
	  sqlite3_free (err_msg);
	  return -2;
      }

#ifdef ENABLE_LIBXML2		/* only if LIBXML2 is supported */

/* creating the Styling Tables */
    sql = "SELECT CreateStylingTables(1)";
    ret = execute_check (handle, sql, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "Error CreateStylingTables %s\n\n", err_msg);
	  sqlite3_free (err_msg);
	  return -3;
      }

    ret = check_vector (handle, cache);
    if (ret != 0)
	return -100 - ret;

    ret = check_raster (handle, cache);
    if (ret != 0)
	return -200 - ret;

    ret = check_group (handle, cache);
    if (ret != 0)
	return -300 - ret;

    ret = check_extent (handle);
    if (ret != 0)
	return -400 - ret;

#endif

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -57;
      }

    spatialite_cleanup_ex (cache);

    spatialite_shutdown ();
    return 0;
}
