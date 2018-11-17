/*

 shape_primitives.c -- SpatiaLite Test Case

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
Ahmadou Dicko <dicko.ahmadou@gmail.com>

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

void
cleanup_shapefile (const char *filename)
{
    char nam[1000];

    if (!filename)
      {
	  return;
      }

    snprintf (nam, 1000, "%s.dbf", filename);
    unlink (nam);
    snprintf (nam, 1000, "%s.prj", filename);
    unlink (nam);
    snprintf (nam, 1000, "%s.shp", filename);
    unlink (nam);
    snprintf (nam, 1000, "%s.shx", filename);
    unlink (nam);
}

int
do_test (sqlite3 * handle)
{
/* testing some DB */
#ifndef OMIT_ICONV		/* only if ICONV is supported */
    char *dumpname = __FILE__ "dump";
    char *err_msg = NULL;
    int row_count;
    int ret;
    gaiaVectorLayersListPtr list;

    ret =
	sqlite3_exec (handle,
		      "CREATE TABLE Point_Test (Name TEXT, Description TEXT)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -3;
      }

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Point_Test', 'geomZ', 4326, 'POINT', 'XYZ', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn POINT XYZ error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -4;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Point_Test (Name, Description, geomZ) VALUES ('Point 1', 'Some point', GeomFromText('POINTZ(136 -33 365)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT POINT XYZ error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -5;
      }

    ret =
	dump_shapefile (handle, "Point_Test", "geomZ", dumpname, "UTF-8",
			"POINT", 1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for POINT XYZ: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -6;
      }
    cleanup_shapefile (dumpname);

    ret =
	dump_shapefile (handle, "Point_Test", "geomZ", dumpname, "UTF-8",
			"MULTIPOINT", 1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for POINT XYZ: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -7;
      }
    cleanup_shapefile (dumpname);

    ret =
	dump_shapefile (handle, "Point_Test", "geomZ", dumpname, "UTF-8", "", 1,
			&row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for POINT XYZ: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -7;		/* dupe */
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Point_Test', 'geomM', 4326, 'POINT', 'XYM', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn POINT XYM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -8;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Point_Test (Name, Description, geomM) VALUES ('Point 2', 'Some pointM', GeomFromText('POINTM(136 -33 26.7)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT POINT XYM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -9;
      }

    ret =
	dump_shapefile (handle, "Point_Test", "geomM", dumpname, "UTF-8", "", 1,
			&row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for POINT XYM: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -10;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Point_Test', 'geomXY', 4326, 'POINT', 'XY', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn POINT XY error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -11;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Point_Test (Name, Description, geomXY) VALUES ('Point 3', 'Some point', GeomFromText('POINT(136 -33)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT POINT XY error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -12;
      }

    ret =
	dump_shapefile (handle, "Point_Test", "geomXY", dumpname, "UTF-8", "",
			1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for POINT XY: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -13;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Point_Test', 'geomXYZM', 4326, 'POINT', 'XYZM', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn POINT XYZM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -14;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Point_Test (Name, Description, geomXYZM) VALUES ('Point 4', 'Some pointXYZM', GeomFromText('POINT(136 -33 424 233.2)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT POINT XYZM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -15;
      }

    ret =
	dump_shapefile (handle, "Point_Test", "geomXYZM", dumpname, "UTF-8", "",
			1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for POINT XYZM: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -16;
      }
    cleanup_shapefile (dumpname);

    ret = sqlite3_exec (handle, "DROP TABLE Point_Test", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE Point_Test error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -20;
      }

    ret =
	sqlite3_exec (handle,
		      "CREATE TABLE MPoint_Test (Name TEXT, Description TEXT)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE MPoint_Test error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -21;
      }

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('MPoint_Test', 'geom', 4326, 'MULTIPOINT', 'XY', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn MPOINT XY error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -22;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO MPoint_Test (Name, Description, geom) VALUES ('Point 1', 'Some point', GeomFromText('MULTIPOINT(136 -33, 47 2, -20, 12)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT MPOINT XY error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -23;
      }

    ret =
	dump_shapefile (handle, "MPoint_Test", "geom", dumpname, "UTF-8",
			"MULTIPOINT", 1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for MPOINT XY: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -24;
      }
    cleanup_shapefile (dumpname);

    ret =
	dump_shapefile (handle, "MPoint_Test", "geom", dumpname, "UTF-8", "", 1,
			&row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for MPOINT XY: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -25;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('MPoint_Test', 'geomZ', 4326, 'MULTIPOINT', 'XYZ', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn MPOINT XYZ error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -26;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO MPoint_Test (Name, Description, geomZ) VALUES ('Point 2', 'Some pointZ', GeomFromText('MULTIPOINTZ(136 -33 1, 47 2 2, -20, 12 3)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT MPOINT XYZ error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -27;
      }

    ret =
	dump_shapefile (handle, "MPoint_Test", "geomZ", dumpname, "UTF-8", "",
			1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for MPOINT XYZ: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -28;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('MPoint_Test', 'geomM', 4326, 'MULTIPOINT', 'XYM', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn MPOINT XYM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -29;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO MPoint_Test (Name, Description, geomM) VALUES ('Point 3', 'Some pointM', GeomFromText('MULTIPOINTM(136 -33 1, 47 2 2, -20, 12 3)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT MPOINT XYM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -30;
      }

    ret =
	dump_shapefile (handle, "MPoint_Test", "geomM", dumpname, "UTF-8", "",
			1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for MPOINT XYM: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -31;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('MPoint_Test', 'geomMZ', 4326, 'MULTIPOINT', 'XYZM', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn MPOINT XYZM error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -32;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO MPoint_Test (Name, Description, geomMZ) VALUES ('Point 4', 'Some pointMZ', GeomFromText('MULTIPOINTMZ(136 -33 1 4.2, 47 2 2.3 1, -20, 12 3 -2)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT MPOINT XYZM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -33;
      }

    ret =
	dump_shapefile (handle, "MPoint_Test", "geomMZ", dumpname, "UTF-8", "",
			1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for MPOINT XYZM %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -34;
      }
    cleanup_shapefile (dumpname);

    ret = sqlite3_exec (handle, "DROP TABLE MPoint_Test", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE MPoint_Test error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -35;
      }

    ret =
	sqlite3_exec (handle,
		      "CREATE TABLE Linestring_Test (Name TEXT, Description TEXT)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE Linestring_Test error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -36;
      }

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Linestring_Test', 'geom', 4326, 'LINESTRING', 'XY', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn LINESTRING XY error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -37;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Linestring_Test (Name, Description, geom) VALUES ('Point 1', 'Some linestring', GeomFromText('LINESTRING(136 -33, 47 2, -20, 12)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT LINESTRING XY error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -38;
      }

    ret =
	dump_shapefile (handle, "Linestring_Test", "geom", dumpname, "UTF-8",
			"", 1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for LINESTRING XY: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -39;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Linestring_Test', 'geomZ', 4326, 'LINESTRING', 'XYZ', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn LINESTRING XYZ error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -40;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Linestring_Test (Name, Description, geomZ) VALUES ('Point 2', 'Some linestringZ', GeomFromText('LINESTRINGZ(136 -33 3, 47 2 3.8, -20 12 10.1)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT LINESTRING XYZ error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -41;
      }

    ret =
	dump_shapefile (handle, "Linestring_Test", "geomZ", dumpname, "UTF-8",
			"", 1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for LINESTRING XYZ: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -42;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Linestring_Test', 'geomM', 4326, 'LINESTRING', 'XYM', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn LINESTRING XYM error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -43;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Linestring_Test (Name, Description, geomM) VALUES ('Point 3', 'Some linestringM', GeomFromText('LINESTRINGM(136 -33 3, 47 2 3.8, -20 12 10.1)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT LINESTRING XYM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -44;
      }

    ret =
	dump_shapefile (handle, "Linestring_Test", "geomM", dumpname, "UTF-8",
			"", 1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for LINESTRING XYM: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -45;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Linestring_Test', 'geomMZ', 4326, 'LINESTRING', 'XYZM', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn LINESTRING XYZM error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -46;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Linestring_Test (Name, Description, geomMZ) VALUES ('Point 3', 'Some linestringZM', GeomFromText('LINESTRINGZM(136 -33 3 4.2, 47 2 3.8 1, -20 12 10.1 321)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT LINESTRING XYZM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -47;
      }

    ret =
	dump_shapefile (handle, "Linestring_Test", "geomMZ", dumpname, "UTF-8",
			"", 1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for LINESTRING XYZM: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -48;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle, "DROP TABLE Linestring_Test", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE Linestring_Test error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -49;
      }

    ret =
	sqlite3_exec (handle,
		      "CREATE TABLE Polygon_Test (Name TEXT, Description TEXT, thing1 INTEGER, thing2 REAL, thing3 DOUBLE, thing4 BLOB)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "CREATE TABLE Polygon_Test error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -50;
      }

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Polygon_Test', 'geom', 4326, 'POLYGON', 'XY', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn POLYGON XY error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -51;
      }
    /* TODO: try dumping an empty table */

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Polygon_Test (Name, Description, thing1, thing2, thing3, thing4, geom) VALUES ('Polygon 1', 'Some polygon', 2, 4.25, 343.343, zeroblob(40), GeomFromText('POLYGON((136 -33, 47 2, -20 -1, 136 -33),(10 -2, -20 -0.4, 40 0.1, 10 -2))', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT POLYGON XY error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -54;
      }

    ret =
	dump_shapefile (handle, "Polygon_Test", "geom", dumpname, "UTF-8", "",
			1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for POLYGON XY: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -55;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Polygon_Test', 'geomZ', 4326, 'POLYGON', 'XYZ', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn POLYGON XYZ error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -56;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Polygon_Test (Name, Description, thing1, thing2, thing3, thing4, geomZ) VALUES ('Polygon 2', 'Some polygonZ', 12, 14.25, 1343.343, zeroblob(140), GeomFromText('POLYGONZ((136 -33 4, 47 2 4.2, -20 -1 1, 136 -33 4),(10 -2 4, 40 0.1 1.6, -20 -0.4 1, 10 -2 4)', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT POLYGON XYZ error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -57;
      }
    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Polygon_Test (Name, Description, geomZ) VALUES ('Polygon 2', 'wrong direction', GeomFromText('POLYGONZ((136 -33 4, 47 2 4.2, -20 -1 1, 136 -33 4),(10 -2 4, -20 -0.4 1, 40 0.1 1.6, 10 -2 4))', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT POLYGON XYZ 2 error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -57;		/* dupe */
      }

    ret =
	dump_shapefile (handle, "Polygon_Test", "geomZ", dumpname, "UTF-8", "",
			1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for POLYGON XYZ: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -58;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Polygon_Test', 'geomM', 4326, 'POLYGON', 'XYM', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn POLYGON XYM error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -59;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Polygon_Test (Name, Description, geomM) VALUES ('Polygon 3', 'Some polygonM', GeomFromText('POLYGONM((136 -33 4, 47 2 4.2, -20 -1 1, 136 -33 4),(10 -2 4, -20 -0.4 1, 40 0.1 1.6, 10 -2 4))', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT POLYGON XYM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -60;
      }

    ret =
	dump_shapefile (handle, "Polygon_Test", "geomM", dumpname, "UTF-8", "",
			1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for POLYGON XYM: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -61;
      }
    cleanup_shapefile (dumpname);

    ret =
	sqlite3_exec (handle,
		      "SELECT AddGeometryColumn('Polygon_Test', 'geomZM', 4326, 'POLYGON', 'XYZM', 0)",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn POLYGON XYZM error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -62;
      }

    ret =
	sqlite3_exec (handle,
		      "INSERT INTO Polygon_Test (Name, Description, geomZM) VALUES ('Polygon 4', 'Some polygonZM', GeomFromText('POLYGONZM((136 -33 4 4, 47 2 4.2 4.4, -20 -1 1 1, 136 -33 4 4),(10 -2 4 1, -20 -0.4 1 1, 40 0.1 1.6 1, 10 -2 4 1))', 4326))",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "INSERT POLYGON XYZM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -63;
      }

    ret =
	dump_shapefile (handle, "Polygon_Test", "geomZM", dumpname, "UTF-8", "",
			1, &row_count, err_msg);
    if (!ret)
      {
	  fprintf (stderr, "dump_shapefile() error for POLYGON XYZM: %s\n",
		   err_msg);
	  sqlite3_close (handle);
	  return -64;
      }
    cleanup_shapefile (dumpname);

/* testing VectorLayersList (several flavors) */
    list =
	gaiaGetVectorLayersList (handle, NULL, NULL,
				 GAIA_VECTORS_LIST_OPTIMISTIC);
    gaiaFreeVectorLayersList (list);
    list =
	gaiaGetVectorLayersList (handle, NULL, NULL,
				 GAIA_VECTORS_LIST_PESSIMISTIC);
    gaiaFreeVectorLayersList (list);
    list =
	gaiaGetVectorLayersList (handle, "Polygon_Test", "geomZM",
				 GAIA_VECTORS_LIST_OPTIMISTIC);
    gaiaFreeVectorLayersList (list);
    gaiaStatisticsInvalidate (handle, NULL, NULL);
    list =
	gaiaGetVectorLayersList (handle, NULL, NULL,
				 GAIA_VECTORS_LIST_PESSIMISTIC);
    gaiaFreeVectorLayersList (list);
    gaiaStatisticsInvalidate (handle, "Polygon_Test", NULL);
    list =
	gaiaGetVectorLayersList (handle, NULL, NULL,
				 GAIA_VECTORS_LIST_PESSIMISTIC);
    gaiaFreeVectorLayersList (list);
    gaiaStatisticsInvalidate (handle, "Polygon_Test", "geomZM");
    list =
	gaiaGetVectorLayersList (handle, NULL, NULL,
				 GAIA_VECTORS_LIST_PESSIMISTIC);
    gaiaFreeVectorLayersList (list);
    gaiaStatisticsInvalidate (handle, "Palygon_Tost", "ZMgeom");

    ret =
	sqlite3_exec (handle, "DROP TABLE Polygon_Test", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DROP TABLE Polygon_Test error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -65;
      }

/* final DB cleanup */
    ret =
	sqlite3_exec (handle, "DELETE FROM geometry_columns", NULL, NULL,
		      &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DELETE FROM geometry_columns error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -66;
      }
    ret =
	sqlite3_exec (handle,
		      "DELETE FROM spatialite_history WHERE geometry_column IS NOT NULL",
		      NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "DELETE FROM spatialite_history error: %s\n",
		   err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -66;
      }
    ret = sqlite3_exec (handle, "VACUUM", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "VACUUM error: %s\n", err_msg);
	  sqlite3_free (err_msg);
	  sqlite3_close (handle);
	  return -66;
      }
#endif /* end ICONV conditional */

/* ok, succesfull termination */
    return 0;
}

int
main (int argc, char *argv[])
{
#ifndef OMIT_ICONV		/* only if ICONV is supported */
#ifdef ENABLE_LWGEOM		/* only if LWGEOM is supported */
    int ret;
    sqlite3 *handle;
    char *err_msg = NULL;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

/* testing current style metadata layout >= v.4.0.0 */
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

    ret = do_test (handle);
    if (ret != 0)
      {
	  fprintf (stderr,
		   "error while testing current style metadata layout\n");
	  return ret;
      }

    spatialite_cleanup_ex (cache);
    sqlite3_close (handle);

/* testing legacy style metadata layout <= v.3.1.0 */
    cache = spatialite_alloc_connection ();
    ret =
	system
	("cp test-legacy-3.0.1.sqlite copy-primitives-legacy-3.0.1.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot copy legacy v.3.0.1 database\n");
	  return -1;
      }
    ret =
	sqlite3_open_v2 ("copy-primitives-legacy-3.0.1.sqlite", &handle,
			 SQLITE_OPEN_READWRITE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open legacy v.3.0.1 database: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }

    spatialite_init_ex (handle, cache, 0);

    ret = do_test (handle);
    if (ret != 0)
      {
	  fprintf (stderr,
		   "error while testing legacy style metadata layout\n");
	  return ret;
      }

    spatialite_cleanup_ex (cache);
    sqlite3_close (handle);
    ret = unlink ("copy-primitives-legacy-3.0.1.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot remove legacy v.3.0.1 database\n");
	  return -1;
      }

/* testing invalid geometries [check/repair] */
    cache = spatialite_alloc_connection ();
    ret = system ("cp test-invalid.sqlite copy-invalid.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot copy invalid-geoms database\n");
	  return -1;
      }
    ret =
	sqlite3_open_v2 ("copy-invalid.sqlite", &handle, SQLITE_OPEN_READWRITE,
			 NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open invalid-geoms database: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }

    spatialite_init_ex (handle, cache, 0);

    ret = check_all_geometry_columns_r (cache, handle, "./report", NULL, NULL);
    if (!ret)
      {
	  fprintf (stderr, "check_all_geometry_columns() error\n");
	  sqlite3_close (handle);
	  return -61;
      }

    ret =
	sanitize_all_geometry_columns_r (cache, handle, "tmp_", "./report",
					 NULL, NULL);
    if (!ret)
      {
	  fprintf (stderr, "sanitize_all_geometry_columns() error\n");
	  sqlite3_close (handle);
	  return -62;
      }

    spatialite_cleanup_ex (cache);
    sqlite3_close (handle);
    ret = unlink ("copy-invalid.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot remove invalid-geoms database\n");
	  return -1;
      }

/* testing invalid geometries [check/repair] - legacy mode */
    spatialite_init (0);
    ret = system ("cp test-invalid.sqlite copy-invalid.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot copy invalid-geoms database\n");
	  return -1;
      }
    ret =
	sqlite3_open_v2 ("copy-invalid.sqlite", &handle, SQLITE_OPEN_READWRITE,
			 NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open invalid-geoms database: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }

    ret = check_all_geometry_columns (handle, "./report", NULL, NULL);
    if (!ret)
      {
	  fprintf (stderr, "check_all_geometry_columns() error\n");
	  sqlite3_close (handle);
	  return -91;
      }

    ret =
	sanitize_all_geometry_columns (handle, "tmp_", "./report", NULL, NULL);
    if (!ret)
      {
	  fprintf (stderr, "sanitize_all_geometry_columns() error\n");
	  sqlite3_close (handle);
	  return -92;
      }

    spatialite_cleanup ();
    sqlite3_close (handle);
    ret = unlink ("copy-invalid.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot remove invalid-geoms database\n");
	  return -1;
      }

/* testing legacy style metadata layout (v.2.3.1) */
    cache = spatialite_alloc_connection ();
    ret = system ("cp test-legacy-2.3.1.sqlite copy-legacy-2.3.1.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot copy legacy v.2.3.1 database\n");
	  return -1;
      }
    ret =
	sqlite3_open_v2 ("copy-legacy-2.3.1.sqlite", &handle,
			 SQLITE_OPEN_READWRITE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open legacy v.2.3.1 database: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }

    spatialite_init_ex (handle, cache, 0);

    ret = do_test (handle);
    if (ret != 0)
      {
	  fprintf (stderr,
		   "error while testing legacy (2.3.1) style metadata layout\n");
	  return ret;
      }

    spatialite_cleanup_ex (cache);
    sqlite3_close (handle);
    ret = unlink ("copy-legacy-2.3.1.sqlite");
    if (ret != 0)
      {
	  fprintf (stderr, "cannot remove legacy v.2.3.1 database\n");
	  return -1;
      }

#endif /* end LWGEOM conditionals */
#endif /* end ICONV conditional */

    spatialite_shutdown ();
    return 0;
}
