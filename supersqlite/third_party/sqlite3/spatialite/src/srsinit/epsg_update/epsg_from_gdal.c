/* 
/ epsg_from_gdal
/
/ a tool autogenerating C code for SpatiaLite
/ [spatial_ref_sys self-initialization routines]
/
/ version 1.0, 2014 December 2014
/
/ Author: Sandro Furieri a.furieri@lqt.it
/
/ Copyright (C) 2014  Alessandro Furieri
/
/    This program is free software: you can redistribute it and/or modify
/    it under the terms of the GNU General Public License as published by
/    the Free Software Foundation, either version 3 of the License, or
/    (at your option) any later version.
/
/    This program is distributed in the hope that it will be useful,
/    but WITHOUT ANY WARRANTY; without even the implied warranty of
/    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/    GNU General Public License for more details.
/
/    You should have received a copy of the GNU General Public License
/    along with this program.  If not, see <http://www.gnu.org/licenses/>.
/
*/

#include <stdlib.h>
#include <stdio.h>

#include <ogr_srs_api.h>

static void
print_wkt (int srid)
{
    OGRSpatialReferenceH handle;
    handle = OSRNewSpatialReference (NULL);
    if (handle == NULL)
	;
    else
      {
	  OGRErr err;
	  char urn[1024];
	  char *proj4 = NULL;
	  char *wkt = NULL;
	  const char *name = NULL;
	  const char *unit = NULL;
	  const char *axis_1 = NULL;
	  const char *axis_2 = NULL;
	  const char *orient_1;
	  const char *orient_2;
	  const char *spheroid = NULL;
	  const char *prime_meridian = NULL;
	  const char *datum = NULL;
	  const char *projection = NULL;
	  OGRAxisOrientation orientation_1 = OAO_Other;
	  OGRAxisOrientation orientation_2 = OAO_Other;
	  int flipped_axes = 0;
	  int is_geographic = 0;

	  sprintf (urn, "urn:ogc:def:crs:EPSG::%d", srid);
	  err = OSRSetFromUserInput (handle, urn);
	  if (err == 0)
	    {
		if (OSRIsGeocentric (handle))
		    goto skip;
		if (OSREPSGTreatsAsLatLong (handle))
		    flipped_axes = 1;
		if (OSREPSGTreatsAsNorthingEasting (handle))
		    flipped_axes = 1;
		if (OSRIsGeographic (handle))
		    is_geographic = 1;
		OSRExportToProj4 (handle, &proj4);
		OSRExportToWkt (handle, &wkt);
		name = OSRGetAttrValue (handle, "PROJCS", 0);
		if (name == NULL)
		    name = OSRGetAttrValue (handle, "GEOGCS", 0);
		unit = OSRGetAttrValue (handle, "UNIT", 0);
		spheroid = OSRGetAttrValue (handle, "SPHEROID", 0);
		prime_meridian = OSRGetAttrValue (handle, "PRIMEM", 0);
		datum = OSRGetAttrValue (handle, "DATUM", 0);
		projection = OSRGetAttrValue (handle, "PROJECTION", 0);
		if (is_geographic)
		  {
		      axis_1 = OSRGetAxis (handle, "GEOGCS", 0, &orientation_1);
		      axis_2 = OSRGetAxis (handle, "GEOGCS", 1, &orientation_2);
		  }
		else
		  {
		      axis_1 = OSRGetAxis (handle, "PROJCS", 0, &orientation_1);
		      axis_2 = OSRGetAxis (handle, "PROJCS", 1, &orientation_2);
		  }
	    }
	  if (proj4 != NULL || wkt != NULL)
	    {
		if (strlen (proj4) > 0)
		  {
		      int i;
		      int len = strlen (proj4);
		      for (i = len - 1; i >= 0; i--)
			{
			    if (*(proj4 + i) == ' ')
				*(proj4 + i) = '\0';
			    else
				break;
			}
		      if (unit == NULL)
			  unit = "unknown";
		      if (spheroid == NULL)
			  spheroid = "unknown";
		      if (prime_meridian == NULL)
			  prime_meridian = "unknown";
		      if (datum == NULL)
			  datum = "unknown";
		      if (is_geographic)
			  projection = "none";
		      else
			{
			    if (projection == NULL)
				projection = "unknown";
			}
		      if (axis_1 == NULL)
			{
			    axis_1 = "unknown";
			    orient_1 = "unknown";
			}
		      else
			{
			    switch (orientation_1)
			      {
			      case OAO_North:
				  orient_1 = "North";
				  break;
			      case OAO_South:
				  orient_1 = "South";
				  break;
			      case OAO_East:
				  orient_1 = "East";
				  break;
			      case OAO_West:
				  orient_1 = "West";
				  break;
			      case OAO_Up:
				  orient_1 = "Up";
				  break;
			      case OAO_Down:
				  orient_1 = "Down";
				  break;
			      default:
				  orient_1 = "unknown";
				  break;
			      };
			}
		      if (axis_2 == NULL)
			{
			    axis_2 = "unknown";
			    orient_2 = "unknown";
			}
		      else
			{
			    switch (orientation_2)
			      {
			      case OAO_North:
				  orient_2 = "North";
				  break;
			      case OAO_South:
				  orient_2 = "South";
				  break;
			      case OAO_East:
				  orient_2 = "East";
				  break;
			      case OAO_West:
				  orient_2 = "West";
				  break;
			      case OAO_Up:
				  orient_2 = "Up";
				  break;
			      case OAO_Down:
				  orient_2 = "Down";
				  break;
			      default:
				  orient_2 = "unknown";
				  break;
			      };
			}
		      printf
			  ("%d\t%d\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
			   srid, is_geographic, flipped_axes, unit,
			   axis_1, orient_1, axis_2, orient_2,
			   spheroid, prime_meridian, datum, projection, name,
			   proj4, wkt);
		  }
	    }
	skip:
	  if (proj4 != NULL)
	      OGRFree (proj4);
	  if (wkt != NULL)
	      OGRFree (wkt);
	  OSRRelease (handle);
      }
}

int
main (void)
{
    int srid;
    for (srid = 1; srid <= 32766; srid++)
	print_wkt (srid);
    return 0;
}
