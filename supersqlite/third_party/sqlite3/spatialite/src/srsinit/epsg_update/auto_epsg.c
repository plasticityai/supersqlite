/* 
/ auto_epsg
/
/ a tool autogenerating C code for SpatiaLite
/ [spatial_ref_sys self-initialization routines]
/
/ version 1.0, 2012 January 18
/
/ Author: Sandro Furieri a.furieri@lqt.it
/
/ Copyright (C) 2011  Alessandro Furieri
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
#include <string.h>
#include <time.h>

struct epsg_entry
{
/* a struct wrapping an EPSG entry */
    int srid;
    char *ref_sys_name;
    char *proj4text;
    char *srs_wkt;
    struct epsg_entry *next;
};

struct epsg_dict
{
/* the EPSG dictionary */
    struct epsg_entry *first;
    struct epsg_entry *last;
    struct epsg_entry **sorted;
    int count;
};

static void
free_epsg_entry (struct epsg_entry *p)
{
/* destroying an epsg_entry */
    if (p->ref_sys_name)
	free (p->ref_sys_name);
    if (p->proj4text)
	free (p->proj4text);
    if (p->srs_wkt)
	free (p->srs_wkt);
    free (p);
}

static void
free_epsg (struct epsg_dict *epsg)
{
/* destroying the EPSG dictionary */
    struct epsg_entry *pE;
    struct epsg_entry *pEn;
    pE = epsg->first;
    while (pE)
      {
	  pEn = pE->next;
	  free_epsg_entry (pE);
	  pE = pEn;
      }
    if (epsg->sorted)
	free (epsg->sorted);
}

static void
epsg_insert (struct epsg_dict *epsg, int srid, const char *name,
	     const char *proj4text)
{
/* inserting an entry into the EPSG dictionary */
    int len;
    struct epsg_entry *p = malloc (sizeof (struct epsg_entry));
    p->srid = srid;
    len = strlen (name);
    p->ref_sys_name = malloc (len + 1);
    strcpy (p->ref_sys_name, name);
    len = strlen (proj4text);
    p->proj4text = malloc (len + 1);
    strcpy (p->proj4text, proj4text);
    p->srs_wkt = NULL;
    p->next = NULL;

/* updating the linked list */
    if (epsg->first == NULL)
	epsg->first = p;
    if (epsg->last != NULL)
	epsg->last->next = p;
    epsg->last = p;
}

static void
add_wkt (struct epsg_dict *epsg, int srid, const char *wkt)
{
/* adding the WKT def to some EPSG entry */
    struct epsg_entry *p = epsg->first;
    while (p)
      {
	  if (p->srid == srid)
	    {
		int len = strlen (wkt);
		if (p->srs_wkt)
		    free (p->srs_wkt);
		p->srs_wkt = malloc (len + 1);
		strcpy (p->srs_wkt, wkt);
		return;
	    }
	  p = p->next;
      }
}

static int
parse_epsg (FILE * fl_epsg, struct epsg_dict *epsg)
{
/* parsing the EPSG input file */
    char line[1024];
    char name[512];
    char *out = line;
    int odd_even = 0;
    int c;

    while ((c = getc (fl_epsg)) != EOF)
      {
	  if (c == '\r')
	      continue;
	  if (c == '\n')
	    {
		*out = '\0';
		if (odd_even)
		  {
		      int i;
		      int len = strlen (line);
		      char srid[16];
		      char *p = srid;
		      const char *proj4text = NULL;
		      if (odd_even == 2)
			{
			    if (*line == '#')
			      {
				  /* closing a pending invalid EPSG def */
				  odd_even = 0;
				  *name = '\0';
				  out = line;
				  continue;
			      }
			    return 0;
			}
		      if (*name == '\0')
			  return 0;
		      if (strncmp (line, "# ", 2) == 0)
			{
			    /* probably an invalid EPSG def: skipping */
			    odd_even = 2;
			    *name = '\0';
			    out = line;
			    continue;
			}
		      if (*line != '<')
			  return 0;
		      if (strcmp (line + len - 2, "<>") != 0)
			  return 0;
		      line[len - 2] = '\0';
		      for (i = 1; i < 10; i++)
			{
			    if (line[i] == '>')
			      {
				  *p = '\0';
				  proj4text = line + i + 2;
			      }
			    *p++ = line[i];
			}
		      if (!proj4text)
			  return 0;
		      len = strlen (line);
		      for (i = len - 1; i > 1; i--)
			{
			    if (line[i] == ' ')
				line[i] = '\0';
			    else
				break;
			}
		      epsg_insert (epsg, atoi (srid), name, proj4text);
		      odd_even = 0;
		      *name = '\0';
		  }
		else
		  {
		      if (strncmp (line, "# ", 2) != 0)
			  return 0;
		      strcpy (name, line + 2);
		      odd_even = 1;
		  }
		out = line;
		continue;
	    }
	  *out++ = c;
      }
    return 1;
}

static int
fetch_srid (const char *line)
{
/* attempting to retrieve the WKT own SRID */
    int len = strlen (line);
    int i;
    int cnt = 0;
    for (i = len - 1; i > 1; i--)
      {
	  if (line[i] == ']')
	      cnt++;
	  if (cnt == 3)
	    {
		if (strncmp (line + i, "],AUTHORITY[\"EPSG\",\"", 20) == 0)
		    return atoi (line + i + 20);
		return -1;
	    }
      }
    return -1;
}

static int
parse_wkt (FILE * fl_wkt, struct epsg_dict *epsg)
{
/* parsing the WKT input file */
    char line[8192];
    int srid;
    char *out = line;
    int c;

    while ((c = getc (fl_wkt)) != EOF)
      {
	  if (c == '\r')
	      continue;
	  if (c == '\n')
	    {
		*out = '\0';
		srid = fetch_srid (line);
		if (srid >= 0)
		    add_wkt (epsg, srid, line);
		out = line;
		continue;
	    }
	  *out++ = c;
      }
    return 1;
}

static int
cmp_sort (const void *p1, const void *p2)
{
/* comparison function for QSORT */
    struct epsg_entry *e1 = *(struct epsg_entry **) p1;
    struct epsg_entry *e2 = *(struct epsg_entry **) p2;
    if (e1->srid == e2->srid)
	return 0;
    if (e1->srid < e2->srid)
	return -1;
    return 1;
}

static int
sort_epsg (struct epsg_dict *epsg)
{
/* sorting the EPSG defs by ascending SRID */
    int count = 0;
    int i;
    struct epsg_entry *p = epsg->first;
    while (p)
      {
	  /* counting how many entries are there */
	  count++;
	  p = p->next;
      }
    if (count == 0)
	return 0;
    if (epsg->sorted)
	free (epsg->sorted);
    epsg->sorted = malloc (sizeof (struct epsg_dict *) * count);
    epsg->count = count;
    i = 0;
    p = epsg->first;
    while (p)
      {
	  /* feeding the pointer array */
	  *(epsg->sorted + i) = p;
	  i++;
	  p = p->next;
      }
/* sorting the pointer array by ascending SRID */
    qsort (epsg->sorted, count, sizeof (struct epsg_dict *), cmp_sort);
    return 1;
}

static FILE *
open_file (int file_no)
{
/* opening an output file */
    char path[1024];
    FILE *out = NULL;

    sprintf (path, "epsg_inlined_%02d.c", file_no);
    out = fopen (path, "wb");
    if (out == NULL)
      {
	  fprintf (stderr,
		   "ERROR: unable to open the \"%s\" output file\n", path);
	  return NULL;
      }
    return out;
}

static FILE *
open_file2 ()
{
/* opening the "extra" output file */
    FILE *out = NULL;

    out = fopen ("epsg_inlined_extra.c", "wb");
    if (out == NULL)
      {
	  fprintf (stderr,
		   "ERROR: unable to open the \"epsg_inlined_extra.c\" output file\n");
	  return NULL;
      }
    return out;
}

static FILE *
open_file3 ()
{
/* opening the "prussian" output file */
    FILE *out = NULL;

    out = fopen ("epsg_inlined_prussian.c", "wb");
    if (out == NULL)
      {
	  fprintf (stderr,
		   "ERROR: unable to open the \"epsg_inlined_prussian.c\" output file\n");
	  return NULL;
      }
    return out;
}

static FILE *
open_file_wgs84 (int file_no)
{
/* opening an output file "wgs84" */
    char path[1024];
    FILE *out = NULL;

    sprintf (path, "epsg_inlined_wgs84_%02d.c", file_no);
    out = fopen (path, "wb");
    if (out == NULL)
      {
	  fprintf (stderr,
		   "ERROR: unable to open the \"%s\" output file\n", path);
	  return NULL;
      }
    return out;
}

static void
do_prussian (FILE * out)
{
/* writing the "prussian" body */
    fprintf (out,
	     "SPATIALITE_PRIVATE void\ninitialize_epsg_prussian(int filter,struct epsg_defs **first, struct epsg_defs **last)\n");
    fprintf (out,
	     "{\n/* old Prussian based on Cassini-Soldner - kindly contributed by Mark Johnson */\n");
    fprintf (out, "    struct epsg_defs *p;\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187900,\"mj10777.de\",187900,\"DHDN / Soldner Rathausnetz 660\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=52.41864827777778 +lon_0=13.62720366666667 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=14741.77 +y_0=-11288.28 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner Rathausnetz 660\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",52.41864827777778],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",13.62720366666667],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",14741.77],PARAMETER[\\\"false_northing\\\",-11288.28],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187900\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187901,\"mj10777.de\",187901,\"DHDN / Soldner 1 Kucklinsberg\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=54.46022375 +lon_0=21.95509736 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 1 Kucklinsberg\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",54.46022375],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",21.95509736],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187901\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187902,\"mj10777.de\",187902,\"DHDN / Soldner 2 Paulinen\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=54.28921067 +lon_0=20.73315431 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 2 Paulinen\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",54.28921067],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",20.73315431],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187902\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187903,\"mj10777.de\",187903,\"DHDN / Soldner 3 Markushof I\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=54.05881333 +lon_0=19.37343583 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 3 Markushof I\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",54.05881333],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",19.37343583],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187903\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187904,\"mj10777.de\",187904,\"DHDN / Soldner 4 Thurmberg\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=54.22552092 +lon_0=18.12569375 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 4 Thurmberg\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",54.22552092],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",18.12569375],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187904\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187905,\"mj10777.de\",187905,\"DHDN / Soldner 5 Kauernick I\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=53.38933197 +lon_0=19.59810438 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 5 Kauernick I\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",53.38933197],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",19.59810438],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187905\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187906,\"mj10777.de\",187906,\"DHDN / Soldner 6 Thorn, Rathausturm\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=53.01181586 +lon_0=18.60725428 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 6 Thorn, Rathausturm\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",53.01181586],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",18.60725428],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187906\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187907,\"mj10777.de\",187907,\"DHDN / Soldner 7 Heinrichsthal\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=53.71289217 +lon_0=17.49676781 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 7 Heinrichsthal\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",53.71289217],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",17.49676781],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187907\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187908,\"mj10777.de\",187908,\"DHDN / Soldner 8 Gollenberg, Denkmal\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=54.20857178 +lon_0=16.22956781 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 8 Gollenberg, Denkmal\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",54.20857178],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",16.22956781],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187908\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187909,\"mj10777.de\",187909,\"DHDN / Soldner 9 Gnesen\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=52.53820406 +lon_0=17.59450500 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 9 Gnesen\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",52.53820406],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",17.59450500],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187909\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187910,\"mj10777.de\",187910,\"DHDN / Soldner 10 Josefsberg\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.98768806 +lon_0=16.20044389 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 10 Josefsberg\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.98768806],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",16.20044389],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187910\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187911,\"mj10777.de\",187911,\"DHDN / Soldner 11 Schroda\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=52.23137372 +lon_0=17.27795372 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 11 Schroda\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",52.23137372],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",17.27795372],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187911\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187912,\"mj10777.de\",187912,\"DHDN / Soldner 12 Pschow\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=50.04206544 +lon_0=18.39610692 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 12 Pschow\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",50.04206544],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",18.39610692],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187912\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187913,\"mj10777.de\",187913,\"DHDN / Soldner 13 Rummelsberg\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=50.70352314 +lon_0=17.11233917 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 13 Rummelsberg\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",50.70352314],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",17.11233917],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187913\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187914,\"mj10777.de\",187914,\"DHDN / Soldner 14 Gröditzberg I\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.17819342 +lon_0=15.76127086 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 14 Gröditzberg I\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.17819342],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",15.76127086],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187914\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187915,\"mj10777.de\",187915,\"DHDN / Soldner 15 Kaltenborn\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.92903708 +lon_0=14.66212942 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 15 Kaltenborn\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.92903708],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",14.66212942],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187915\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187917,\"mj10777.de\",187917,\"DHDN / Soldner 17 Greifswald, Nikolaikirche\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=54.09698872 +lon_0=13.37880703 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 17 Greifswald, Nikolaikirche\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",54.09698872],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",13.37880703],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187917\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187916,\"mj10777.de\",187916,\"DHDN / Soldner 16 Bahn I\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=53.10184583 +lon_0=14.70144539 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 16 Bahn I\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",53.10184583],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",14.70144539],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187916\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187918,\"mj10777.de\",187918,\"DHDN / Soldner 18 Müggelberg 600\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=52.41864827777778 +lon_0=13.62720366666667 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 18 Müggelberg 600\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",52.41864827777778],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",13.62720366666667],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187918\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187919,\"mj10777.de\",187919,\"DHDN / Soldner 19 Götzer Berg 650\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=52.43725961111112 +lon_0=12.72882972222223 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 19 Götzer Berg 650\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",52.43725961111112],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",12.72882972222223],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187919\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187920,\"mj10777.de\",187920,\"DHDN / Soldner 20 Torgau, Stadtkirche\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.56136217 +lon_0=13.00760264 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 20 Torgau, Stadtkirche\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.56136217],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",13.00760264],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187920\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187921,\"mj10777.de\",187921,\"DHDN / Soldner 21 Burkersroda, Kirchturm\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.17656322 +lon_0=11.64139367 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 21 Burkersroda, Kirchturm\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.17656322],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",11.64139367],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187921\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187922,\"mj10777.de\",187922,\"DHDN / Soldner 22 Inselsberg\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=50.85237983 +lon_0=10.46776506 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 22 Inselsberg\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",50.85237983],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",10.46776506],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187922\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187923,\"mj10777.de\",187923,\"DHDN / Soldner 23 Magdeburg\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=52.12625311 +lon_0=11.63550325 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 23 Magdeburg\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",52.12625311],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",11.63550325],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187923\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187924,\"mj10777.de\",187924,\"DHDN / Soldner 24 Ostenfeld I\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=54.47018733 +lon_0=9.23411097 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 24 Ostenfeld I\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",54.47018733],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",9.23411097],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187924\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187925,\"mj10777.de\",187925,\"DHDN / Soldner 25 Rathkrügen\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=53.81839364 +lon_0=10.04220189 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 25 Rathkrügen\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",53.81839364],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",10.04220189],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187925\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187926,\"mj10777.de\",187926,\"DHDN / Soldner 26 Bungsberg, Aussichtsturm\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=54.21110653 +lon_0=10.72636431 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 26 Bungsberg, Aussichtsturm\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",54.21110653],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",10.72636431],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187926\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187927,\"mj10777.de\",187927,\"DHDN / Soldner 27 Celle, Stadtkirche\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=52.62574192 +lon_0=10.08190214 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 27 Celle, Stadtkirche\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",52.62574192],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",10.08190214],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187927\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187928,\"mj10777.de\",187928,\"DHDN / Soldner 28 Kaltenborn\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.79646722 +lon_0=10.27322494 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 28 Kaltenborn\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.79646722],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",10.27322494],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187928\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187929,\"mj10777.de\",187929,\"DHDN / Soldner 29 Silberberg\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=53.73123461 +lon_0=9.05774925 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 29 Silberberg\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",53.73123461],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",9.05774925],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187929\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187930,\"mj10777.de\",187930,\"DHDN / Soldner 30 Windberg\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=52.88088372 +lon_0=7.53062114 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 30 Windberg\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",52.88088372],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",7.53062114],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187930\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187931,\"mj10777.de\",187931,\"DHDN / Soldner 31 Hermannsdenkmal\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.91301647 +lon_0=8.84051853 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 31 Hermannsdenkmal\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.91301647],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",8.84051853],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187931\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187932,\"mj10777.de\",187932,\"DHDN / Soldner 32 Münster\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.96547642 +lon_0=7.62334994 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 32 Münster\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.96547642],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",7.62334994],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187932\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187933,\"mj10777.de\",187933,\"DHDN / Soldner 33 Bochum, Peter-Paul-Kirche\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.48373533 +lon_0=7.22115822 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 33 Bochum, Peter-Paul-Kirche\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.48373533],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",7.22115822],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187933\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187934,\"mj10777.de\",187934,\"DHDN / Soldner 34 Homert\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.26480147 +lon_0=8.10687050 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 34 Homert\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.26480147],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",8.10687050],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187934\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187935,\"mj10777.de\",187935,\"DHDN / Soldner 35 Kassel, Martinskirche\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=51.31846489 +lon_0=9.50203072 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 35 Kassel, Martinskirche\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",51.31846489],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",9.50203072],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187935\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187936,\"mj10777.de\",187936,\"DHDN / Soldner 36 Schaumburg, Schloßturm\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=50.34048964 +lon_0=7.97808156 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 36 Schaumburg, Schloßturm\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",50.34048964],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",7.97808156],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187936\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187937,\"mj10777.de\",187937,\"DHDN / Soldner 37 Fleckert\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=50.18762544 +lon_0=7.60594289 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 37 Fleckert\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",50.18762544],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",7.60594289],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187937\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187938,\"mj10777.de\",187938,\"DHDN / Soldner 38 Cöln, Dom\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=50.94257242 +lon_0=6.95897600 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 38 Cöln, Dom\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",50.94257242],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",6.95897600],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187938\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187939,\"mj10777.de\",187939,\"DHDN / Soldner 39 Langschoß\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=50.66738711 +lon_0=6.28935703 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 39 Langschoß\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",50.66738711],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",6.28935703],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187939\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187940,\"mj10777.de\",187940,\"DHDN / Soldner 40 Rissenthal\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=49.47801819 +lon_0=6.75864339 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner 40 Rissenthal\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",49.47801819],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",6.75864339],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187940\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187941,\"mj10777.de\",187941,\"DHDN / Soldner Bayern\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=48.13959111 +lon_0=11.57437083 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=0 +y_0=0 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner Bayern\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",48.13959111],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",11.57437083],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",0],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187941\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def (filter,first, last, 187998, \"mj10777.de\", 187998,\"DHDN / Ferro / Deutsches_Hauptdreiecksnetz\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=longlat +ellps=bessel +datum=potsdam +pm=ferro +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Ferro / Deutsches_Hauptdreiecksnetz\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"SPHEROID[\\\"Bessel 1841\\\",6377397.155,299.1528128,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]], AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"PRIMEM[\\\"ferro\\\",-17.66666666666667],UNIT[\\\"Degree\\\",0.017453292519943295,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187998\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]]\");\n");
    fprintf (out,
	     "/* mj10777: Not exsiting System, used for testing purposes. */\n");
    fprintf (out,
	     "    p = add_epsg_def(filter,first,last,187999,\"mj10777.de\",187999,\"DHDN / Brandenburger Tor\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=cass +lat_0=52.41864827777778 +lon_0=13.62720366666667 \");\n");
    fprintf (out,
	     "    add_proj4text(p,1,\"+x_0=16819.76033675660074 +y_0=-11046.08218553455117 +ellps=bessel +datum=potsdam +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"DHDN / Soldner Brandenburger Tor\\\",\");\n");
    fprintf (out, "    add_srs_wkt(p,1,\"GEOGCS[\\\"DHDN\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"DATUM[\\\"Deutsches_Hauptdreiecksnetz\\\",SPHEROID[\\\"Bessel 1841\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"6377397.155,299.1528128,AUTHORITY[\\\"EPSG\\\",\\\"7004\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"AUTHORITY[\\\"EPSG\\\",\\\"6314\\\"]],PRIMEM[\\\"Greenwich\\\",0,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],UNIT[\\\"degree\\\",0.01745329251994328,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"4314\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"UNIT[\\\"metre\\\",1,AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PROJECTION[\\\"Cassini_Soldner\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"latitude_of_origin\\\",52.41864827777778],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"central_meridian\\\",13.62720366666667],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"PARAMETER[\\\"false_easting\\\",16819.76033675660074],PARAMETER[\\\"false_northing\\\",-11046.08218553455117],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AUTHORITY[\\\"mj10777.de\\\",\\\"187999\\\"],AXIS[\\\"x\\\",NORTH],AXIS[\\\"y\\\",EAST]]\");\n");
    fprintf (out,
	     "    p = add_epsg_def (filter,first, last, 325833, \"mj10777.de\", 325833,\"ETRS89 / UTM zone 33N (Brandenburg)\");\n");
    fprintf (out,
	     "    add_proj4text(p,0,\"+proj=utm +zone=33 +ellps=GRS80 +units=m +no_defs\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,0,\"PROJCS[\\\"ETRS89 / UTM zone 33N (Brandenburg)\\\",GEOGCS[\\\"ETRS89\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,1,\"DATUM[\\\"European_Terrestrial_Reference_System_1989\\\",\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,2,\"SPHEROID[\\\"GRS 1980\\\",6378137,298.257222101,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,3,\"AUTHORITY[\\\"EPSG\\\",\\\"7019\\\"]],AUTHORITY[\\\"EPSG\\\",\\\"6258\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,4,\"PRIMEM[\\\"Greenwich\\\",0,AUTHORITY[\\\"EPSG\\\",\\\"8901\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,5,\"UNIT[\\\"degree\\\",0.01745329251994328,AUTHORITY[\\\"EPSG\\\",\\\"9122\\\"]],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,6,\"AUTHORITY[\\\"EPSG\\\",\\\"4258\\\"]],UNIT[\\\"metre\\\",1,\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,7,\"AUTHORITY[\\\"EPSG\\\",\\\"9001\\\"]],PROJECTION[\\\"Transverse_Mercator\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,8,\"PARAMETER[\\\"latitude_of_origin\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,9,\"PARAMETER[\\\"central_meridian\\\",15],PARAMETER[\\\"scale_factor\\\",0.9996],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,10,\"PARAMETER[\\\"false_easting\\\",3500000],PARAMETER[\\\"false_northing\\\",0],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,11,\"AUTHORITY[\\\"EPSG\\\",\\\"325833\\\"],\");\n");
    fprintf (out,
	     "    add_srs_wkt(p,12,\"AXIS[\\\"Easting\\\",EAST],AXIS[\\\"Northing\\\",NORTH]]\");\n");
}

static void
do_header (FILE * out, int macro)
{
/* writing a file header */
    time_t tm;
    time (&tm);
    fprintf (out, "/*\n\n");
    fprintf (out,
	     " helper function for srs_init.c -- populating the SPATIAL_REF_SYS table\n\n");
    fprintf (out, " this code was autogenerated by \"auto_epsg\" on %s\n",
	     ctime (&tm));
    fprintf (out, " Author: Sandro Furieri a.furieri@lqt.it\n\n");
    fprintf (out,
	     " -----------------------------------------------------------------------------\n\n");
    fprintf (out, " Version: MPL 1.1/GPL 2.0/LGPL 2.1\n\n");
    fprintf (out,
	     " The contents of this file are subject to the Mozilla Public License Version\n");
    fprintf (out,
	     " 1.1 (the \"License\"); you may not use this file except in compliance with\n");
    fprintf (out, " the License. You may obtain a copy of the License at\n");
    fprintf (out, " http://www.mozilla.org/MPL/\n\n");
    fprintf (out,
	     "Software distributed under the License is distributed on an \"AS IS\" basis,\n");
    fprintf (out,
	     "WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License\n");
    fprintf (out,
	     "for the specific language governing rights and limitations under the\n");
    fprintf (out,
	     "License.\n\nThe Original Code is the SpatiaLite library\n\n");
    fprintf (out,
	     "The Initial Developer of the Original Code is Alessandro Furieri\n\n");
    fprintf (out,
	     "Portions created by the Initial Developer are Copyright (C) 2008\n");
    fprintf (out,
	     "the Initial Developer. All Rights Reserved.\n\nContributor(s):\n\n");
    fprintf (out,
	     "Alternatively, the contents of this file may be used under the terms of\n");
    fprintf (out,
	     "either the GNU General Public License Version 2 or later (the \"GPL\"), or\n");
    fprintf (out,
	     "the GNU Lesser General Public License Version 2.1 or later (the \"LGPL\"),\n");
    fprintf (out,
	     "in which case the provisions of the GPL or the LGPL are applicable instead\n");
    fprintf (out,
	     "of those above. If you wish to allow use of your version of this file only\n");
    fprintf (out,
	     "under the terms of either the GPL or the LGPL, and not to allow others to\n");
    fprintf (out,
	     "use your version of this file under the terms of the MPL, indicate your\n");
    fprintf (out,
	     "decision by deleting the provisions above and replace them with the notice\n");
    fprintf (out,
	     "and other provisions required by the GPL or the LGPL. If you do not delete\n");
    fprintf (out,
	     "the provisions above, a recipient may use your version of this file under\n");
    fprintf (out,
	     "the terms of any one of the MPL, the GPL or the LGPL.\n\n*/\n\n");
    fprintf (out, "#include \"config.h\"\n\n");
    fprintf (out, "#include <spatialite_private.h>\n\n");

    if (macro)
	fprintf (out,
		 "#ifndef OMIT_EPSG    /* full EPSG initialization enabled */\n\n");
}


static void
do_footer (FILE * out, int macro)
{
/* writing a file footer */
    fprintf (out, "}\n\n");
    if (macro)
	fprintf (out,
		 "#endif /* full EPSG initialization enabled/disabled */\n\n");
}

static void
output_c_code (FILE * out, struct epsg_dict *epsg)
{
/* generating the C code supporting spatial_ref_sys self-initialization */
    struct epsg_entry *p = epsg->first;
    int n;
    int sect = 0;
    int wgs84_sect = 0;
    int def_cnt = 1000;
    int out_cnt;
    const char *in;
    int i;
    int pending_footer = 0;

    for (i = 0; i < epsg->count; i++)
      {
	  p = *(epsg->sorted + i);
	  if (p->srid == 4326 || (p->srid >= 32601 && p->srid <= 32766))
	    {
		/* skipping WGS84 defs */
		continue;
	    }

	  if (def_cnt > 100)
	    {
		if (out != NULL)
		  {
		      if (pending_footer)
			{
			    do_footer (out, 1);
			    pending_footer = 0;
			}

		      /* closing the output file */
		      fclose (out);
		      out = NULL;
		  }

		/* opening the next output file */
		out = open_file (sect);
		if (out == NULL)
		    return;

		/* function header */
		do_header (out, 1);
		fprintf (out, "SPATIALITE_PRIVATE void\n");
		fprintf (out,
			 "initialize_epsg_%02d (int filter, struct epsg_defs **first, struct epsg_defs **last)\n",
			 sect++);
		fprintf (out, "{\n/* initializing the EPSG defs list */\n");
		fprintf (out, "    struct epsg_defs *p;\n");
		def_cnt = 0;
	    }
	  pending_footer = 1;

	  /* inserting the main EPSG def */
	  def_cnt++;
	  fprintf (out,
		   "    p = add_epsg_def (filter, first, last, %d, \"epsg\", %d,\n",
		   p->srid, p->srid);
	  fprintf (out, "        \"%s\");\n", p->ref_sys_name);

	  /* inserting the proj4text string */
	  n = 0;
	  in = p->proj4text;
	  while (*in != '\0')
	    {
		fprintf (out, "    add_proj4text (p, %d,\n        \"", n);
		out_cnt = 0;
		while (*in != '\0')
		  {
		      if (*in == '"')
			{
			    fprintf (out, "\\%c", *in++);
			    out_cnt += 2;
			}
		      else
			{
			    fprintf (out, "%c", *in++);
			    out_cnt++;
			}
		      if (out_cnt >= 56)
			  break;
		  }
		fprintf (out, "\");\n");
		n++;
	    }

	  if (p->srs_wkt != NULL)
	    {
		/* inserting the srs_wkt string */ n = 0;
		n = 0;
		in = p->srs_wkt;
		while (*in != '\0')
		  {
		      fprintf (out, "    add_srs_wkt (p, %d,\n        \"", n);
		      out_cnt = 0;
		      while (*in != '\0')
			{
			    if (*in == '"')
			      {
				  fprintf (out, "\\%c", *in++);
				  out_cnt += 2;
			      }
			    else
			      {
				  fprintf (out, "%c", *in++);
				  out_cnt++;
			      }
			    if (out_cnt >= 56)
				break;
			}
		      fprintf (out, "\");\n");
		      n++;
		  }
	    }
	  else
	      fprintf (out, "    add_srs_wkt (p, 0, \"\");\n");
      }
    if (out != NULL)
      {
	  /* closing the output file */
	  if (pending_footer)
	    {
		do_footer (out, 1);
		pending_footer = 0;
	    }

	  /* closing the output file */
	  fclose (out);
	  out = NULL;
      }

    def_cnt = 1000;
    pending_footer = 0;
    for (i = 0; i < epsg->count; i++)
      {
	  p = *(epsg->sorted + i);
	  if (p->srid == 4326 || (p->srid >= 32601 && p->srid <= 32766))
	      ;
	  else
	    {
		/* skipping not-WGS84 defs */
		continue;
	    }

	  if (def_cnt > 100)
	    {
		if (out != NULL)
		  {
		      if (pending_footer)
			{
			    do_footer (out, 0);
			    pending_footer = 0;
			}

		      /* closing the output file */
		      fclose (out);
		      out = NULL;
		  }

		/* opening the next output file */
		out = open_file_wgs84 (wgs84_sect);
		if (out == NULL)
		    return;

		/* function header */
		do_header (out, 0);
		fprintf (out, "SPATIALITE_PRIVATE void\n");
		fprintf (out,
			 "initialize_epsg_wgs84_%02d (int filter, struct epsg_defs **first, struct epsg_defs **last)\n",
			 wgs84_sect++);
		fprintf (out,
			 "{\n/* initializing the EPSG defs list [WGS84] */\n");
		fprintf (out, "    struct epsg_defs *p;\n");
		def_cnt = 0;
	    }
	  pending_footer = 1;

	  /* inserting the main EPSG def */
	  def_cnt++;
	  fprintf (out,
		   "    p = add_epsg_def (filter, first, last, %d, \"epsg\", %d,\n",
		   p->srid, p->srid);
	  fprintf (out, "        \"%s\");\n", p->ref_sys_name);

	  /* inserting the proj4text string */
	  n = 0;
	  in = p->proj4text;
	  while (*in != '\0')
	    {
		fprintf (out, "    add_proj4text (p, %d,\n        \"", n);
		out_cnt = 0;
		while (*in != '\0')
		  {
		      if (*in == '"')
			{
			    fprintf (out, "\\%c", *in++);
			    out_cnt += 2;
			}
		      else
			{
			    fprintf (out, "%c", *in++);
			    out_cnt++;
			}
		      if (out_cnt >= 56)
			  break;
		  }
		fprintf (out, "\");\n");
		n++;
	    }

	  if (p->srs_wkt != NULL)
	    {
		/* inserting the srs_wkt string */
		n = 0;
		in = p->srs_wkt;
		while (*in != '\0')
		  {
		      fprintf (out, "    add_srs_wkt (p, %d,\n        \"", n);
		      out_cnt = 0;
		      while (*in != '\0')
			{
			    if (*in == '"')
			      {
				  fprintf (out, "\\%c", *in++);
				  out_cnt += 2;
			      }
			    else
			      {
				  fprintf (out, "%c", *in++);
				  out_cnt++;
			      }
			    if (out_cnt >= 56)
				break;
			}
		      fprintf (out, "\");\n");
		      n++;
		  }
	    }
	  else
	      fprintf (out, "    add_srs_wkt (p, 0, \"\");\n");
      }
    if (out != NULL)
      {
	  /* closing the output file */
	  if (pending_footer)
	    {
		do_footer (out, 0);
		pending_footer = 0;
	    }

	  /* closing the output file */
	  fclose (out);
	  out = NULL;
      }

/* opening the "prussian" output file */
    out = open_file3 ();
    if (out == NULL)
	return;

/* function header */
    do_header (out, 1);
    do_prussian (out);

/* function footer */
    do_footer (out, 1);

/* closing the "prussian" output file */
    fclose (out);
    out = NULL;

/* opening the "extra" output file */
    out = open_file2 ();
    if (out == NULL)
	return;

/* function header */
    do_header (out, 0);
    fprintf (out, "/* declaring prototypes */\n");
    for (i = 0; i < sect; i++)
	fprintf (out,
		 "SPATIALITE_PRIVATE void initialize_epsg_%02d (\n\tint filter, struct epsg_defs **first, struct epsg_defs **last);\n",
		 i);
    for (i = 0; i < wgs84_sect; i++)
	fprintf (out,
		 "SPATIALITE_PRIVATE void initialize_epsg_wgs84_%02d (\n\tint filter, struct epsg_defs **first, struct epsg_defs **last);\n",
		 i);
    fprintf (out,
	     "SPATIALITE_PRIVATE void initialize_epsg_prussian (\n\tint filter, struct epsg_defs **first, struct epsg_defs **last);\n");
    fprintf (out,
	     "SPATIALITE_PRIVATE void initialize_epsg_extra (\n\tint filter, struct epsg_defs **first, struct epsg_defs **last);\n\n");

    fprintf (out, "SPATIALITE_PRIVATE void\n");
    fprintf (out,
	     "initialize_epsg_extra (int filter, struct epsg_defs **first, struct epsg_defs **last)\n\n");
    fprintf (out, "{\n/* initializing the EPSG defs list [EXTRA] */\n");
    fprintf (out, "    struct epsg_defs *p;\n");

    fprintf (out,
	     "    p = add_epsg_def (filter, first, last, 40000, \"gfoss.it\", 1,\n");
    fprintf (out, "        \"Italy mainland zone 1 GB Roma40\");\n");
    fprintf (out, "    add_proj4text (p, 0,\n");
    fprintf (out,
	     "        \"+proj=tmerc+lat_0=0 +lon_0=9  +k=0.9996 +x_0=1500000 +y_\");\n");
    fprintf (out, "    add_proj4text (p, 1,\n");
    fprintf (out,
	     "        \"0=0 +ellps=intl +units=m +towgs84=-104.1,-49.1,-9.9,0.97\");\n");
    fprintf (out,
	     "    add_proj4text (p, 2, \"1,-2.917,0.714,-11.68 +no_defs\");\n");
    fprintf (out, "    add_srs_wkt (p, 0, \"\");\n");
    fprintf (out,
	     "    p = add_epsg_def (filter, first, last, 40001, \"gfoss.it\", 2,\n");
    fprintf (out, "        \"Italy mainland zone 2 GB Roma40\");\n");
    fprintf (out, "    add_proj4text (p, 0,\n");
    fprintf (out,
	     "        \"+proj=tmerc +lat_0=0 +lon_0=15 +k=0.9996 +x_0=2520000 +y\");\n");
    fprintf (out, "    add_proj4text (p, 1,\n");
    fprintf (out,
	     "        \"_0=0 +ellps=intl +units=m +towgs84=-104.1,-49.1,-9.9,0.9\");\n");
    fprintf (out,
	     "    add_proj4text (p, 2, \"71,-2.917,0.714,-11.68 +no_defs\");\n");
    fprintf (out, "    add_srs_wkt (p, 0, \"\");\n");
    fprintf (out,
	     "    p = add_epsg_def (filter, first, last, 40002, \"gfoss.it\", 3,\n");
    fprintf (out, "        \"Italy Sardinia GB Roma40\");\n");
    fprintf (out, "    add_proj4text (p, 0,\n");
    fprintf (out,
	     "        \"+proj=tmerc +lat_0=0 +lon_0=9  +k=0.9996 +x_0=1500000 +y\");\n");
    fprintf (out, "    add_proj4text (p, 1,\n");
    fprintf (out,
	     "        \"_0=0 +ellps=intl +units=m +towgs84=-168.6,-34.0,38.6,-0.\");\n");
    fprintf (out,
	     "    add_proj4text (p, 2, \"374,-0.679,-1.379,-9.48 +no_defs\");\n");
    fprintf (out, "    add_srs_wkt (p, 0, \"\");\n");
    fprintf (out,
	     "    p = add_epsg_def (filter, first, last, 40003, \"gfoss.it\", 4,\n");
    fprintf (out, "        \"Italy Sicily GB Roma40\");\n");
    fprintf (out, "    add_proj4text (p, 0,\n");
    fprintf (out,
	     "        \"+proj=tmerc +lat_0=0 +lon_0=9  +k=0.9996 +x_0=1500000 +y\");\n");
    fprintf (out, "    add_proj4text (p, 1,\n");
    fprintf (out,
	     "        \"_0=0 +ellps=intl +units=m +towgs84=-50.2,-50.4,84.8,-0.6\");\n");
    fprintf (out,
	     "    add_proj4text (p, 2, \"90,-2.012,0.459,-28.08  +no_defs\");\n");
    fprintf (out, "    add_srs_wkt (p, 0, \"\");\n");

/* function footer */
    do_footer (out, 0);

/* inserting the pilot function */
    fprintf (out, "SPATIALITE_PRIVATE void\n");
    fprintf (out,
	     "initialize_epsg (int filter, struct epsg_defs **first, struct epsg_defs **last)\n");
    fprintf (out, "{\n/* initializing the EPSG defs list */\n");
    fprintf (out, "    struct epsg_defs *p;\n ");
    fprintf (out, "/* initializing the EPSG UNKNOWN def [-1] */\n");
    fprintf (out,
	     "    p = add_epsg_def (filter, first, last, -1, \"NONE\", -1, \"Undefined - Cartesian\");\n");
    fprintf (out, "    add_proj4text (p, 0, \"\");\n");
    fprintf (out, "    add_srs_wkt (p, 0, \"\");\n");
    fprintf (out,
	     "    p = add_epsg_def (filter, first, last, 0, \"NONE\", 0, \"Undefined - Geographic Long/Lat\");\n");
    fprintf (out, "    add_proj4text (p, 0, \"\");\n");
    fprintf (out, "    add_srs_wkt (p, 0, \"\");\n\n");
    fprintf (out, "    if (filter != GAIA_EPSG_WGS84_ONLY)\n    {\n");
    fprintf (out,
	     "#ifndef OMIT_EPSG    /* full EPSG initialization enabled */\n");
    for (i = 0; i < sect; i++)
	fprintf (out,
		 "        initialize_epsg_%02d (filter, first, last);\n", i);
    fprintf (out, "        initialize_epsg_prussian (filter, first, last);\n");
    fprintf (out, "        initialize_epsg_extra (filter, first, last);\n");
    fprintf (out, "#endif /* full EPSG initialization enabled/disabled */\n");

    fprintf (out, "    }\n");
    for (i = 0; i < wgs84_sect; i++)
	fprintf (out,
		 "    initialize_epsg_wgs84_%02d (filter, first, last);\n", i);
    fprintf (out, "}\n");

/* closing the "extra" output file */
    fclose (out);
    out = NULL;
}

int
main (int argc, char *argv[])
{
/*
/
/ Please note: no args are supported !!!
/
/ we'll expect to find two input files respectively named:
/ - epsg
/ - wkt
/
/ the C code will be generated into: epsg_inlined.c
/
*/
    FILE *fl_epsg = NULL;
    FILE *fl_wkt = NULL;
    FILE *fl_out = NULL;
    struct epsg_dict epsg;
/* initializing the EPSG dictionary */
    epsg.first = NULL;
    epsg.last = NULL;
    epsg.sorted = NULL;
    epsg.count = 0;
/* opening the EPSG input file */
    fl_epsg = fopen ("epsg", "rb");
    if (fl_epsg == NULL)
      {
	  fprintf (stderr, "ERROR: unable to open the \"epsg\" input file\n");
	  goto stop;
      }

/* opening the WKT input file */
    fl_wkt = fopen ("wkt", "rb");
    if (fl_wkt == NULL)
      {
	  fprintf (stderr, "ERROR: unable to open the \"wkt\" input file\n");
	  goto stop;
      }

    if (!parse_epsg (fl_epsg, &epsg))
      {
	  fprintf (stderr, "ERROR: malformed EPSG input file\n");
	  goto stop;
      }

    if (!parse_wkt (fl_wkt, &epsg))
      {
	  fprintf (stderr, "ERROR: malformed WKT input file\n");
	  goto stop;
      }

    if (!sort_epsg (&epsg))
      {
	  fprintf (stderr, "ERROR: unable to sort EPSG entries\n");
	  goto stop;
      }

    output_c_code (fl_out, &epsg);
  stop:
    free_epsg (&epsg);
    if (fl_epsg)
	fclose (fl_epsg);
    if (fl_wkt)
	fclose (fl_wkt);
    if (fl_out)
	fclose (fl_out);
    return 0;
}
