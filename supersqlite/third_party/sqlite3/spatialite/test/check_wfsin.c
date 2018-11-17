/*

 check_wfsin.c -- SpatiaLite Test Case

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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gg_wfs.h"

int
main (int argc, char *argv[])
{
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    int row_count;
    int count;
    int srid;
    gaiaWFScatalogPtr catalog = NULL;
    gaiaWFSitemPtr lyr;
    gaiaWFSschemaPtr schema;
    gaiaWFScolumnPtr column;
    char *str;
    const char *name;
    int type;
    int dims;
    int nillable;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory database: %s\n",
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

#ifdef ENABLE_LIBXML2		/* only if LIBXML2 is supported */
    ret =
	load_from_wfs (handle, "./test.wfs", NULL, "topp:p02", 0, "test_wfs1",
		       "objectid", 1, &row_count, &err_msg, NULL, NULL);
    if (!ret)
      {
	  fprintf (stderr, "load_from_wfs() error for test.wfs (1): %s\n",
		   err_msg);
	  free (err_msg);
	  sqlite3_close (handle);
	  return -3;
      }
    if (row_count != 3)
      {
	  fprintf (stderr, "unexpected row count for test_wfs: %i\n",
		   row_count);
	  sqlite3_close (handle);
	  return -4;
      }

    ret =
	load_from_wfs (handle, "./test.wfs", NULL, "topp:p02", 0, "test_wfs2",
		       NULL, 0, &row_count, &err_msg, NULL, NULL);
    if (!ret)
      {
	  fprintf (stderr, "load_from_wfs() error for test.wfs (2): %s\n",
		   err_msg);
	  free (err_msg);
	  sqlite3_close (handle);
	  return -5;
      }
    if (row_count != 3)
      {
	  fprintf (stderr, "unexpected row count for test_wfs: %i\n",
		   row_count);
	  sqlite3_close (handle);
	  return -6;
      }

    catalog = create_wfs_catalog ("./getcapabilities-1.0.0.wfs", &err_msg);
    if (catalog == NULL)
      {
	  fprintf (stderr,
		   "create_wfs_catalog() error for getcapabilities-1.0.0.wfs: %s\n",
		   err_msg);
	  free (err_msg);
	  sqlite3_close (handle);
	  return -7;
      }

    count = get_wfs_catalog_count (catalog);
    if (count != 49)
      {
	  fprintf (stderr,
		   "get_wfs_catalog_count() 1.0.0 error: got %d, expected 49\n",
		   count);
	  sqlite3_close (handle);
	  return -8;
      }

    lyr = get_wfs_catalog_item (catalog, 2);
    if (lyr == NULL)
      {
	  fprintf (stderr, "get_wfs_catalog_item() 1.0.0 error (NULL)\n");
	  sqlite3_close (handle);
	  return -9;
      }

    str = (char *) get_wfs_item_name (lyr);
    if (str == NULL)
      {
	  fprintf (stderr, "get_wfs_item_name() 1.0.0 error (NULL)\n");
	  sqlite3_close (handle);
	  return -10;
      }
    if (strcmp (str, "tiger:tiger_roads") != 0)
      {
	  fprintf (stderr,
		   "get_wfs_item_name() 1.0.0 error: got \"%s\", expected \"tiger:tiger_roads\"\n",
		   str);
	  sqlite3_close (handle);
	  return -11;
      }

    str = (char *) get_wfs_item_title (lyr);
    if (str == NULL)
      {
	  fprintf (stderr, "get_wfs_item_title() 1.0.0 error (NULL)\n");
	  sqlite3_close (handle);
	  return -12;
      }
    if (strcmp (str, "Manhattan (NY) roads") != 0)
      {
	  fprintf (stderr,
		   "get_wfs_item_title() 1.0.0 error: got \"%s\", expected \"Manhattan (NY) roads\"\n",
		   str);
	  sqlite3_close (handle);
	  return -13;
      }

    str = (char *) get_wfs_item_abstract (lyr);
    if (str == NULL)
      {
	  fprintf (stderr, "get_wfs_item_abstract() error (NULL)\n");
	  sqlite3_close (handle);
	  return -14;
      }
    if (strcmp (str, "Highly simplified road layout of Manhattan in New York..")
	!= 0)
      {
	  fprintf (stderr,
		   "get_wfs_item_abstract() 1.0.0 error: got \"%s\", expected \"Highly simplified road layout of Manhattan in New York..\"\n",
		   str);
	  sqlite3_close (handle);
	  return -15;
      }

    count = get_wfs_layer_srid_count (lyr);
    if (count != 1)
      {
	  fprintf (stderr,
		   "get_wfs_layer_srid_count() 1.0.0 error: got %d, expected 1\n",
		   count);
	  sqlite3_close (handle);
	  return -16;
      }

    srid = get_wfs_layer_srid (lyr, 0);
    if (srid != 4326)
      {
	  fprintf (stderr,
		   "get_wfs_layer_srid() 1.0.0 error: got %d, expected 4326\n",
		   srid);
	  sqlite3_close (handle);
	  return -17;
      }

    count = get_wfs_keyword_count (lyr);
    if (count != 3)
      {
	  fprintf (stderr,
		   "get_wfs_keyword_count() 1.0.0 error: got %d, expected 3\n",
		   count);
	  sqlite3_close (handle);
	  return -18;
      }

    str = (char *) get_wfs_keyword (lyr, 2);
    if (strcmp (str, "roads") != 0)
      {
	  fprintf (stderr,
		   "get_wfs_keyword() 1.0.0 error: got \"%s\", expected \"roads\"\n",
		   str);
	  sqlite3_close (handle);
	  return -19;
      }

    str = (char *) get_wfs_base_request_url (catalog);
    if (strcmp (str, "http://www.gaia-gis.it:8080/geoserver/wfs?") != 0)
      {
	  fprintf (stderr,
		   "get_wfs_base_request_url() 1.0.0 error: got \"%s\", expected \"http://www.gaia-gis.it:8080/geoserver/wfs?\"\n",
		   str);
	  sqlite3_close (handle);
	  return -20;
      }

    str =
	(char *) get_wfs_request_url (catalog, "sf:roads", "1.0.0", 26713, -1);
    if (strcmp
	(str,
	 "http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.0.0&request=GetFeature&typeName=sf:roads&srsName=EPSG:26713")
	!= 0)
      {
	  fprintf (stderr,
		   "get_wfs_request_url() 1.0.0 error: got \"%s\", expected \"http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.0.0&request=GetFeature&typeName=sf:roads&srsName=EPSG:26713\"\n",
		   str);
	  sqlite3_close (handle);
	  return -21;
      }
    free (str);

    str = (char *) get_wfs_describe_url (catalog, "sf:roads", "1.0.0");
    if (strcmp
	(str,
	 "http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.0.0&request=DescribeFeatureType&typeName=sf:roads")
	!= 0)
      {
	  fprintf (stderr,
		   "get_wfs_describe_url() 1.0.0 error: got \"%s\", expected \"http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.0.0&request=DescribeFeatureType&typeName=sf:roads\"\n",
		   str);
	  sqlite3_close (handle);
	  return -22;
      }
    free (str);
    destroy_wfs_catalog (catalog);

    catalog = create_wfs_catalog ("./getcapabilities-1.1.0.wfs", &err_msg);
    if (catalog == NULL)
      {
	  fprintf (stderr,
		   "create_wfs_catalog() error for getcapabilities-1.1.0.wfs: %s\n",
		   err_msg);
	  free (err_msg);
	  sqlite3_close (handle);
	  return -23;
      }

    count = get_wfs_catalog_count (catalog);
    if (count != 49)
      {
	  fprintf (stderr,
		   "get_wfs_catalog_count() 1.1.0 error: got %d, expected 49\n",
		   count);
	  sqlite3_close (handle);
	  return -24;
      }

    lyr = get_wfs_catalog_item (catalog, 3);
    if (lyr == NULL)
      {
	  fprintf (stderr, "get_wfs_catalog_item() 1.1.0 error (NULL)\n");
	  sqlite3_close (handle);
	  return -25;
      }

    str = (char *) get_wfs_item_name (lyr);
    if (str == NULL)
      {
	  fprintf (stderr, "get_wfs_item_name() 1.1.0 error (NULL)\n");
	  sqlite3_close (handle);
	  return -26;
      }
    if (strcmp (str, "sf:archsites") != 0)
      {
	  fprintf (stderr,
		   "get_wfs_item_name() 1.1.0 error: got \"%s\", expected \"sf:archsites\"\n",
		   str);
	  sqlite3_close (handle);
	  return -27;
      }

    str = (char *) get_wfs_item_title (lyr);
    if (str == NULL)
      {
	  fprintf (stderr, "get_wfs_item_title() error (NULL)\n");
	  sqlite3_close (handle);
	  return -28;
      }
    if (strcmp (str, "Spearfish archeological sites") != 0)
      {
	  fprintf (stderr,
		   "get_wfs_item_title() 1.1.0 error: got \"%s\", expected \"Spearfish archeological sites\"\n",
		   str);
	  sqlite3_close (handle);
	  return -29;
      }

    str = (char *) get_wfs_item_abstract (lyr);
    if (str == NULL)
      {
	  fprintf (stderr, "get_wfs_item_abstract() 1.1.0 error (NULL)\n");
	  sqlite3_close (handle);
	  return -30;
      }
    if (strcmp
	(str,
	 "Sample data from GRASS, archeological sites location, Spearfish, South Dakota, USA")
	!= 0)
      {
	  fprintf (stderr,
		   "get_wfs_item_abstract() 1.1.0 error: got \"%s\", expected \"Sample data from GRASS, archeological sites location, Spearfish, South Dakota, USA\"\n",
		   str);
	  sqlite3_close (handle);
	  return -31;
      }

    count = get_wfs_layer_srid_count (lyr);
    if (count != 1)
      {
	  fprintf (stderr,
		   "get_wfs_layer_srid_count() 1.1.0 error: got %d, expected 1\n",
		   count);
	  sqlite3_close (handle);
	  return -32;
      }

    srid = get_wfs_layer_srid (lyr, 0);
    if (srid != 26713)
      {
	  fprintf (stderr,
		   "get_wfs_layer_srid() 1.1.0 error: got %d, expected 26713\n",
		   srid);
	  sqlite3_close (handle);
	  return -33;
      }

    srid = get_wfs_layer_srid (lyr, 10);
    if (srid != -1)
      {
	  fprintf (stderr,
		   "get_wfs_layer_srid() 1.1.0 error: got %d, expected -1\n",
		   srid);
	  sqlite3_close (handle);
	  return -34;
      }

    count = get_wfs_keyword_count (lyr);
    if (count != 4)
      {
	  fprintf (stderr,
		   "get_wfs_keyword_count() 1.1.0 error: got %d, expected 4\n",
		   count);
	  sqlite3_close (handle);
	  return -35;
      }

    str = (char *) get_wfs_keyword (lyr, 1);
    if (strcmp (str, "spearfish") != 0)
      {
	  fprintf (stderr,
		   "get_wfs_keyword() 1.1.0 error: got \"%s\", expected \"spearfish\"\n",
		   str);
	  sqlite3_close (handle);
	  return -36;
      }

    str = (char *) get_wfs_keyword (lyr, 100);
    if (str != NULL)
      {
	  fprintf (stderr, "get_wfs_keyword() 1.1.0 error: expected NULL\n");
	  sqlite3_close (handle);
	  return -37;
      }

    str = (char *) get_wfs_base_request_url (catalog);
    if (strcmp (str, "http://www.gaia-gis.it:8080/geoserver/wfs?") != 0)
      {
	  fprintf (stderr,
		   "get_wfs_base_request_url() 1.1.0 error: got \"%s\", expected \"http://www.gaia-gis.it:8080/geoserver/wfs?\"\n",
		   str);
	  sqlite3_close (handle);
	  return -38;
      }

    str = get_wfs_request_url (catalog, "sf:roads", "1.1.0", 26713, 100);
    if (strcmp
	(str,
	 "http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.1.0&request=GetFeature&typeName=sf:roads&srsName=urn:x-ogc:def:crs:EPSG:26713&maxFeatures=100")
	!= 0)
      {
	  fprintf (stderr,
		   "get_wfs_request_url() 1.1.0 error: got \"%s\", expected \"http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.1.0&request=GetFeature&typeName=sf:roads&srsName=urn:x-ogc:def:crs:EPSG:26713&maxFeatures=100\"\n",
		   str);
	  sqlite3_close (handle);
	  return -39;
      }
    free (str);

    str = get_wfs_request_url (catalog, NULL, "1.1.0", 26713, -1);
    if (str != NULL)
      {
	  fprintf (stderr,
		   "get_wfs_request_url() 1.1.0 error: expected NULL\n");
	  sqlite3_close (handle);
	  return -40;
      }
    free (str);

    str = get_wfs_request_url (catalog, "sf:roads", NULL, -1, 100);
    if (strcmp
	(str,
	 "http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.1.0&request=GetFeature&typeName=sf:roads&maxFeatures=100")
	!= 0)
      {
	  fprintf (stderr,
		   "get_wfs_request_url() 1.1.0 error: got \"%s\", expected \"http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.1.0&request=GetFeature&typeName=sf:roads&maxFeatures=100\"\n",
		   str);
	  sqlite3_close (handle);
	  return -41;
      }
    free (str);

    str = get_wfs_request_url (catalog, "sf:roads", NULL, 1234, 100);
    if (strcmp
	(str,
	 "http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.1.0&request=GetFeature&typeName=sf:roads&maxFeatures=100")
	!= 0)
      {
	  fprintf (stderr,
		   "get_wfs_request_url() 1.1.0 error: got \"%s\", expected \"http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.1.0&request=GetFeature&typeName=sf:roads&maxFeatures=100\"\n",
		   str);
	  sqlite3_close (handle);
	  return -42;
      }
    free (str);

    str = get_wfs_describe_url (catalog, "sf:roads", "1.1.0");
    if (strcmp
	(str,
	 "http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.1.0&request=DescribeFeatureType&typeName=sf:roads")
	!= 0)
      {
	  fprintf (stderr,
		   "get_wfs_describe_url() 1.1.0 error: got \"%s\", expected \"http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.1.0&request=DescribeFeatureType&typeName=sf:roads\"\n",
		   str);
	  sqlite3_close (handle);
	  return -43;
      }
    free (str);

    str = get_wfs_describe_url (catalog, NULL, "1.1.0");
    if (str != NULL)
      {
	  fprintf (stderr,
		   "get_wfs_describe_url() 1.1.0 error: : expected NULL\n");
	  sqlite3_close (handle);
	  return -44;
      }
    free (str);

    str = get_wfs_describe_url (catalog, "sf:roads", NULL);
    if (strcmp
	(str,
	 "http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.1.0&request=DescribeFeatureType&typeName=sf:roads")
	!= 0)
      {
	  fprintf (stderr,
		   "get_wfs_describe_url() 1.1.0 error: got \"%s\", expected \"http://www.gaia-gis.it:8080/geoserver/wfs?service=WFS&version=1.1.0&request=DescribeFeatureType&typeName=sf:roads\"\n",
		   str);
	  sqlite3_close (handle);
	  return -45;
      }
    free (str);
    destroy_wfs_catalog (catalog);

    lyr = get_wfs_catalog_item (NULL, 3);
    if (lyr != NULL)
      {
	  fprintf (stderr, "get_wfs_catalog_item() 1.1.0 error (NULL)\n");
	  sqlite3_close (handle);
	  return -46;
      }

    str = (char *) get_wfs_item_name (NULL);
    if (str != NULL)
      {
	  fprintf (stderr, "get_wfs_item_name() 1.1.0 error (NULL)\n");
	  sqlite3_close (handle);
	  return -47;
      }

    str = (char *) get_wfs_item_title (NULL);
    if (str != NULL)
      {
	  fprintf (stderr, "get_wfs_item_title() error (NULL)\n");
	  sqlite3_close (handle);
	  return -48;
      }

    str = (char *) get_wfs_item_abstract (NULL);
    if (str != NULL)
      {
	  fprintf (stderr, "get_wfs_item_abstract() 1.1.0 error (NULL)\n");
	  sqlite3_close (handle);
	  return -49;
      }

    count = get_wfs_layer_srid_count (NULL);
    if (count != -1)
      {
	  fprintf (stderr,
		   "get_wfs_layer_srid_count() 1.1.0 error: got %d, expected -1\n",
		   count);
	  sqlite3_close (handle);
	  return -50;
      }

    srid = get_wfs_layer_srid (NULL, 0);
    if (srid != -1)
      {
	  fprintf (stderr,
		   "get_wfs_layer_srid() 1.1.0 error: got %d, expected -1\n",
		   srid);
	  sqlite3_close (handle);
	  return -51;
      }

    count = get_wfs_keyword_count (NULL);
    if (count != -1)
      {
	  fprintf (stderr,
		   "get_wfs_keyword_count() 1.1.0 error: got %d, expected -1\n",
		   count);
	  sqlite3_close (handle);
	  return -52;
      }

    str = (char *) get_wfs_keyword (NULL, 1);
    if (str != NULL)
      {
	  fprintf (stderr, "get_wfs_keyword() 1.1.0 error: expected NULL\n");
	  sqlite3_close (handle);
	  return -53;
      }

    str = (char *) get_wfs_keyword (NULL, 100);
    if (str != NULL)
      {
	  fprintf (stderr, "get_wfs_keyword() 1.1.0 error: expected NULL\n");
	  sqlite3_close (handle);
	  return -54;
      }

    str = (char *) get_wfs_base_request_url (NULL);
    if (str != NULL)
      {
	  fprintf (stderr, "get_wfs_base_request_url() error: expected NULL\n");
	  sqlite3_close (handle);
	  return -55;
      }

    str = (char *) get_wfs_base_describe_url (NULL);
    if (str != NULL)
      {
	  fprintf (stderr,
		   "get_wfs_base_describe_url() error: expected NULL\n");
	  sqlite3_close (handle);
	  return -56;
      }

    str = (char *) get_wfs_request_url (NULL, "sf:roads", "1.1.0", 26713, -1);
    if (str != NULL)
      {
	  fprintf (stderr, "get_wfs_request_url() error: expected NULL\n");
	  sqlite3_close (handle);
	  return -57;
      }

    str = (char *) get_wfs_describe_url (NULL, "sf:roads", "1.1.0");
    if (str != NULL)
      {
	  fprintf (stderr, "get_wfs_describe_url() error: expected NULL\n");
	  sqlite3_close (handle);
	  return -58;
      }
    destroy_wfs_catalog (NULL);

    schema =
	create_wfs_schema ("./describefeaturetype.wfs", "sf:roads", &err_msg);
    if (schema == NULL)
      {
	  fprintf (stderr,
		   "create_wfs_schema() error for describefeaturetype.wfs: %s\n",
		   err_msg);
	  free (err_msg);
	  sqlite3_close (handle);
	  return -59;
      }

    count = get_wfs_schema_column_count (schema);
    if (count != 8)
      {
	  fprintf (stderr,
		   "get_wfs_schema_column_count() error: got %d, expected 8\n",
		   count);
	  sqlite3_close (handle);
	  return -60;
      }

    column = get_wfs_schema_column (schema, 5);
    if (column == NULL)
      {
	  fprintf (stderr, "get_wfs_schema_column() error (NULL)\n");
	  sqlite3_close (handle);
	  return -61;
      }

    if (get_wfs_schema_column_info (column, &name, &type, &nillable) == 0)
      {
	  fprintf (stderr, "get_wfs_schema_column_info() error\n");
	  sqlite3_close (handle);
	  return -62;
      }

    if (strcmp (name, "codcom") != 0)
      {
	  fprintf (stderr,
		   "get_wfs_schema_column_info() NAME error: got \"%s\", expected \"codcom\"\n",
		   name);
	  sqlite3_close (handle);
	  return -63;
      }

    if (type != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "get_wfs_schema_column_info() TYPE error: got %d, expected %d\n",
		   type, SQLITE_TEXT);
	  sqlite3_close (handle);
	  return -64;
      }

    if (nillable == 0)
      {
	  fprintf (stderr,
		   "get_wfs_schema_column_info() NILLABLE error: got %d, expected 1\n",
		   nillable);
	  sqlite3_close (handle);
	  return -65;
      }

    if (get_wfs_schema_geometry_info
	(schema, &name, &type, &srid, &dims, &nillable) == 0)
      {
	  fprintf (stderr, "get_wfs_schema_geometry_info() error\n");
	  sqlite3_close (handle);
	  return -66;
      }

    if (strcmp (name, "geometry") != 0)
      {
	  fprintf (stderr,
		   "get_wfs_schema_geometry_info() NAME error: got \"%s\", expected \"geometry\"\n",
		   name);
	  sqlite3_close (handle);
	  return -67;
      }

    if (type != GAIA_MULTIPOLYGON)
      {
	  fprintf (stderr,
		   "get_wfs_schema_geometry_info() TYPE error: got %d, expected %d\n",
		   type, GAIA_MULTIPOLYGON);
	  sqlite3_close (handle);
	  return -68;
      }

    if (srid != -1)
      {
	  fprintf (stderr,
		   "get_wfs_schema_geometry_info() TYPE error: got %d, expected -1\n",
		   srid);
	  sqlite3_close (handle);
	  return -69;
      }

    if (dims != 2)
      {
	  fprintf (stderr,
		   "get_wfs_schema_geometry_info() DIMS error: got %d, expected 2\n",
		   dims);
	  sqlite3_close (handle);
	  return -70;
      }

    if (nillable == 0)
      {
	  fprintf (stderr,
		   "get_wfs_schema_geometry_info() NILLABLE error: got %d, expected 1\n",
		   nillable);
	  sqlite3_close (handle);
	  return -71;
      }
    destroy_wfs_schema (schema);

    schema = create_wfs_schema (NULL, NULL, &err_msg);
    if (schema != NULL)
      {
	  fprintf (stderr, "create_wfs_schema() unexpected result for NULL\n");
	  free (err_msg);
	  sqlite3_close (handle);
	  return -72;
      }

    count = get_wfs_schema_column_count (NULL);
    if (count != -1)
      {
	  fprintf (stderr,
		   "get_wfs_schema_column_count() unexpected result for NULL\n");
	  sqlite3_close (handle);
	  return -73;
      }

    column = get_wfs_schema_column (NULL, 5);
    if (column != NULL)
      {
	  fprintf (stderr,
		   "get_wfs_schema_column() unexpected result for NULL\n");
	  sqlite3_close (handle);
	  return -74;
      }

    if (get_wfs_schema_column_info (NULL, &name, &type, &nillable) != 0)
      {
	  fprintf (stderr,
		   "get_wfs_schema_column_info() unexpected result for NULL\n");
	  sqlite3_close (handle);
	  return -75;
      }

    if (get_wfs_schema_geometry_info
	(NULL, &name, &type, &srid, &dims, &nillable) != 0)
      {
	  fprintf (stderr,
		   "get_wfs_schema_geometry_info() unexpected result for NULL\n");
	  sqlite3_close (handle);
	  return -76;
      }
    destroy_wfs_schema (NULL);

#endif /* end LIBXML2 conditional */

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -77;
      }

    spatialite_cleanup_ex (cache);
    spatialite_shutdown ();

    return 0;
}
