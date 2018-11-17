/*

 gg_voronoj.c -- Voronoj Diagram implementation
    
 version 4.3, 2015 June 29

 Author: Sandro Furieri a.furieri@lqt.it

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
 
Portions created by the Initial Developer are Copyright (C) 2008-2015
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

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite_private.h>
#include <spatialite/sqlite.h>

#include <spatialite/gaiageo.h>

#ifdef GEOS_ADVANCED		/* GEOS advanced features */

struct voronoj_triangle
{
/* a struct representing a Delaunay triangle */
    double x1;			/* vertex #1 */
    double y1;
    double x2;			/* vertex #2 */
    double y2;
    double x3;			/* vertex #3 */
    double y3;
    double cx;			/* circumcenter */
    double cy;
    double x_1_2;		/* vertex on the frame - edge #1-#2 */
    double y_1_2;
    double x_2_3;		/* vertex on the frame - edge #2-#3 */
    double y_2_3;
    double x_3_1;		/* vertex on the frame - edge #3-#1 */
    double y_3_1;

    struct voronoj_triangle *tri_1_2;	/* triangle sharing edge #1-#2 */
    struct voronoj_triangle *tri_2_3;	/* triangle sharing edge #2-#3 */
    struct voronoj_triangle *tri_3_1;	/* triangle sharing edge #3-#1 */
    char trace_1_2;		/* flags: to be traced */
    char trace_2_3;
    char trace_3_1;
};

struct voronoj_point
{
/* an auxiliary struct - point on Voronoj's frame */
    double coord;
    struct voronoj_point *next;
};

struct voronoj_aux
{
/* an auxiliary struct supporting Voronoj */
    struct voronoj_triangle *array;	/* array of Triangles */
    int count;			/* number of Triangles */
    double minx;		/* the frame extent */
    double miny;
    double maxx;
    double maxy;
    struct voronoj_point *first_up;	/* linked list: horz-up frame's edge */
    struct voronoj_point *last_up;
    struct voronoj_point *first_low;	/* linked list: horz-down frame's edge */
    struct voronoj_point *last_low;
    struct voronoj_point *first_left;	/* linked list: vert-left frame's edge */
    struct voronoj_point *last_left;
    struct voronoj_point *first_right;	/* linked list: vert-right frame's edge */
    struct voronoj_point *last_right;
};

struct concave_hull_str
{
/* a struct to implement StandardVariation and Variance for Concave Hull */
    double mean;
    double quot;
    double count;
};

static double *
voronoj_sorted_up (struct voronoj_aux *voronoj, int *count)
{
/* returning a sorted array of coordinates */
    double *array = NULL;
    int cnt = 0;
    int ok = 1;
    int i;
    struct voronoj_point *pt = voronoj->first_up;
    while (pt)
      {
	  /* counting how many points are there */
	  cnt++;
	  pt = pt->next;
      }
    *count = cnt;
    if (cnt == 0)
	return NULL;

/* allocating and populating the array */
    array = malloc (sizeof (double) * *count);
    cnt = 0;
    pt = voronoj->first_up;
    while (pt)
      {
	  *(array + cnt++) = pt->coord;
	  pt = pt->next;
      }

/* sorting the array */
    while (ok)
      {
	  ok = 0;
	  for (i = 1; i < *count; i++)
	    {
		if (*(array + i - 1) > *(array + i))
		  {
		      /* swapping two values */
		      double save = *(array + i - 1);
		      *(array + i - 1) = *(array + i);
		      *(array + i) = save;
		      ok = 1;
		  }
	    }
      }
    return array;
}

static double *
voronoj_sorted_low (struct voronoj_aux *voronoj, int *count)
{
/* returning a sorted array of coordinates */
    double *array = NULL;
    int cnt = 0;
    int ok = 1;
    int i;
    struct voronoj_point *pt = voronoj->first_low;
    while (pt)
      {
	  /* counting how many points are there */
	  cnt++;
	  pt = pt->next;
      }
    *count = cnt;
    if (cnt == 0)
	return NULL;

/* allocating and populating the array */
    array = malloc (sizeof (double) * *count);
    cnt = 0;
    pt = voronoj->first_low;
    while (pt)
      {
	  *(array + cnt++) = pt->coord;
	  pt = pt->next;
      }

/* sorting the array */
    while (ok)
      {
	  ok = 0;
	  for (i = 1; i < *count; i++)
	    {
		if (*(array + i - 1) > *(array + i))
		  {
		      /* swapping two values */
		      double save = *(array + i - 1);
		      *(array + i - 1) = *(array + i);
		      *(array + i) = save;
		      ok = 1;
		  }
	    }
      }
    return array;
}

static double *
voronoj_sorted_left (struct voronoj_aux *voronoj, int *count)
{
/* returning a sorted array of coordinates */
    double *array = NULL;
    int cnt = 0;
    int ok = 1;
    int i;
    struct voronoj_point *pt = voronoj->first_left;
    while (pt)
      {
	  /* counting how many points are there */
	  cnt++;
	  pt = pt->next;
      }
    *count = cnt;
    if (cnt == 0)
	return NULL;

/* allocating and populating the array */
    array = malloc (sizeof (double) * *count);
    cnt = 0;
    pt = voronoj->first_left;
    while (pt)
      {
	  *(array + cnt++) = pt->coord;
	  pt = pt->next;
      }

/* sorting the array */
    while (ok)
      {
	  ok = 0;
	  for (i = 1; i < *count; i++)
	    {
		if (*(array + i - 1) > *(array + i))
		  {
		      /* swapping two values */
		      double save = *(array + i - 1);
		      *(array + i - 1) = *(array + i);
		      *(array + i) = save;
		      ok = 1;
		  }
	    }
      }
    return array;
}

static double *
voronoj_sorted_right (struct voronoj_aux *voronoj, int *count)
{
/* returning a sorted array of coordinates */
    double *array = NULL;
    int cnt = 0;
    int ok = 1;
    int i;
    struct voronoj_point *pt = voronoj->first_right;
    while (pt)
      {
	  /* counting how many points are there */
	  cnt++;
	  pt = pt->next;
      }
    *count = cnt;
    if (cnt == 0)
	return NULL;

/* allocating and populating the array */
    array = malloc (sizeof (double) * *count);
    cnt = 0;
    pt = voronoj->first_right;
    while (pt)
      {
	  *(array + cnt++) = pt->coord;
	  pt = pt->next;
      }

/* sorting the array */
    while (ok)
      {
	  ok = 0;
	  for (i = 1; i < *count; i++)
	    {
		if (*(array + i - 1) > *(array + i))
		  {
		      /* swapping two values */
		      double save = *(array + i - 1);
		      *(array + i - 1) = *(array + i);
		      *(array + i) = save;
		      ok = 1;
		  }
	    }
      }
    return array;
}

static void
voronoj_add_frame_point (struct voronoj_aux *voronoj, double x, double y)
{
/* adding some frame point */
    struct voronoj_point *pt = NULL;

/* skipping any corner */
    if (x == voronoj->minx && y == voronoj->miny)
	return;
    if (x == voronoj->minx && y == voronoj->maxy)
	return;
    if (x == voronoj->maxx && y == voronoj->miny)
	return;
    if (x == voronoj->maxx && y == voronoj->maxy)
	return;

    if (x == voronoj->minx)
      {
	  pt = malloc (sizeof (struct voronoj_point));
	  pt->coord = y;
	  pt->next = NULL;
	  if (voronoj->first_left == NULL)
	      voronoj->first_left = pt;
	  if (voronoj->last_left != NULL)
	      voronoj->last_left->next = pt;
	  voronoj->last_left = pt;
      }
    if (x == voronoj->maxx)
      {
	  pt = malloc (sizeof (struct voronoj_point));
	  pt->coord = y;
	  pt->next = NULL;
	  if (voronoj->first_right == NULL)
	      voronoj->first_right = pt;
	  if (voronoj->last_right != NULL)
	      voronoj->last_right->next = pt;
	  voronoj->last_right = pt;
      }
    if (y == voronoj->miny)
      {
	  pt = malloc (sizeof (struct voronoj_point));
	  pt->coord = x;
	  pt->next = NULL;
	  if (voronoj->first_low == NULL)
	      voronoj->first_low = pt;
	  if (voronoj->last_low != NULL)
	      voronoj->last_low->next = pt;
	  voronoj->last_low = pt;
      }
    if (y == voronoj->maxy)
      {
	  pt = malloc (sizeof (struct voronoj_point));
	  pt->coord = x;
	  pt->next = NULL;
	  if (voronoj->first_up == NULL)
	      voronoj->first_up = pt;
	  if (voronoj->last_up != NULL)
	      voronoj->last_up->next = pt;
	  voronoj->last_up = pt;
      }
}

static int
voronoj_same_edge (double ax1, double ay1, double ax2, double ay2, double bx1,
		   double by1, double bx2, double by2)
{
/* testing if two segments are the same */
    if (ax1 == bx1 && ay1 == by1 && ax2 == bx2 && ay2 == by2)
	return 1;
    if (ax1 == bx2 && ay1 == by2 && ax2 == bx1 && ay2 == by1)
	return 1;
    return 0;
}

static int
voronoj_internal (const void *p_cache, struct voronoj_triangle *triangle)
{
/* checking if the circumcenter falls inside the triangle */
    int ret;
    gaiaGeomCollPtr pt = gaiaAllocGeomColl ();
    gaiaGeomCollPtr tri = gaiaAllocGeomColl ();
    gaiaPolygonPtr pg = gaiaAddPolygonToGeomColl (tri, 4, 0);
    gaiaRingPtr rng = pg->Exterior;
    gaiaSetPoint (rng->Coords, 0, triangle->x1, triangle->y1);
    gaiaSetPoint (rng->Coords, 1, triangle->x2, triangle->y2);
    gaiaSetPoint (rng->Coords, 2, triangle->x3, triangle->y3);
    gaiaSetPoint (rng->Coords, 3, triangle->x1, triangle->y1);
    gaiaAddPointToGeomColl (pt, triangle->cx, triangle->cy);
    gaiaMbrGeometry (pt);
    gaiaMbrGeometry (tri);
    if (p_cache != NULL)
	ret = gaiaGeomCollIntersects_r (p_cache, tri, pt);
    else
	ret = gaiaGeomCollIntersects (tri, pt);
    gaiaFreeGeomColl (pt);
    gaiaFreeGeomColl (tri);
    return ret;
}

static double
voronoj_test_point (const void *p_cache, double x1, double y1, double x2,
		    double y2, double x, double y)
{
/* point-segment distance */
    double dist;
    gaiaGeomCollPtr pt = gaiaAllocGeomColl ();
    gaiaGeomCollPtr segm = gaiaAllocGeomColl ();
    gaiaLinestringPtr ln = gaiaAddLinestringToGeomColl (segm, 2);
    gaiaSetPoint (ln->Coords, 0, x1, y1);
    gaiaSetPoint (ln->Coords, 1, x2, y2);
    gaiaAddPointToGeomColl (pt, x, y);
    if (p_cache != NULL)
	gaiaGeomCollDistance_r (p_cache, segm, pt, &dist);
    else
	gaiaGeomCollDistance (segm, pt, &dist);
    gaiaFreeGeomColl (pt);
    gaiaFreeGeomColl (segm);
    return dist;
}

static int
voronoj_check_nearest_edge (const void *p_cache, struct voronoj_triangle *tri,
			    int which)
{
/* testing if direction outside */
    double d_1_2;
    double d_2_3;
    double d_3_1;
    gaiaGeomCollPtr pt = gaiaAllocGeomColl ();
    gaiaGeomCollPtr segm = gaiaAllocGeomColl ();
    gaiaLinestringPtr ln = gaiaAddLinestringToGeomColl (segm, 2);
    gaiaSetPoint (ln->Coords, 0, tri->x1, tri->y1);
    gaiaSetPoint (ln->Coords, 1, tri->x2, tri->y2);
    gaiaAddPointToGeomColl (pt, tri->cx, tri->cy);
    if (p_cache != NULL)
	gaiaGeomCollDistance_r (p_cache, segm, pt, &d_1_2);
    else
	gaiaGeomCollDistance (segm, pt, &d_1_2);
    gaiaFreeGeomColl (segm);
    segm = gaiaAllocGeomColl ();
    ln = gaiaAddLinestringToGeomColl (segm, 2);
    gaiaSetPoint (ln->Coords, 0, tri->x2, tri->y2);
    gaiaSetPoint (ln->Coords, 1, tri->x3, tri->y3);
    if (p_cache != NULL)
	gaiaGeomCollDistance_r (p_cache, segm, pt, &d_2_3);
    else
	gaiaGeomCollDistance (segm, pt, &d_2_3);
    gaiaFreeGeomColl (segm);
    segm = gaiaAllocGeomColl ();
    ln = gaiaAddLinestringToGeomColl (segm, 2);
    gaiaSetPoint (ln->Coords, 0, tri->x3, tri->y3);
    gaiaSetPoint (ln->Coords, 1, tri->x1, tri->y1);
    if (p_cache != NULL)
	gaiaGeomCollDistance_r (p_cache, segm, pt, &d_3_1);
    else
	gaiaGeomCollDistance (segm, pt, &d_3_1);
    gaiaFreeGeomColl (segm);
    gaiaFreeGeomColl (pt);

    if (which == 12 && d_1_2 < d_2_3 && d_1_2 < d_3_1)
	return 0;
    if (which == 23 && d_2_3 < d_1_2 && d_2_3 < d_3_1)
	return 0;
    if (which == 31 && d_3_1 < d_1_2 && d_3_1 < d_2_3)
	return 0;
    return 1;
}

static void
voronoj_minmax (double x, double y, double *minx, double *miny, double *maxx,
		double *maxy)
{
/* updating the frame extent */
    if (x < *minx)
	*minx = x;
    if (y < *miny)
	*miny = y;
    if (x > *maxx)
	*maxx = x;
    if (y > *maxy)
	*maxy = y;
}

static void
voronoj_frame_point (const void *p_cache, double intercept, double slope,
		     struct voronoj_aux *voronoj, double cx, double cy,
		     double mx, double my, int direct, double *x, double *y)
{
/* determining a vertex on the frame */
    double x_up;
    double x_low;
    double y_left;
    double y_right;
    double pre_x1 = DBL_MAX;
    double pre_y1 = DBL_MAX;
    double pre_x2 = DBL_MAX;
    double pre_y2 = DBL_MAX;
    double d1;
    double d2;

    if (slope == DBL_MAX)
      {
	  x_up = cx;
	  x_low = cx;
	  y_left = DBL_MAX;
	  y_right = DBL_MAX;
      }
    else
      {
	  x_up = (voronoj->maxy - intercept) / slope;
	  x_low = (voronoj->miny - intercept) / slope;
	  y_left = (slope * voronoj->minx) + intercept;
	  y_right = (slope * voronoj->maxx) + intercept;
      }

    if (x_up >= voronoj->minx && x_up <= voronoj->maxx)
      {
	  if (pre_x1 == DBL_MAX && pre_y1 == DBL_MAX)
	    {
		pre_x1 = x_up;
		pre_y1 = voronoj->maxy;
	    }
	  else if (pre_x2 == DBL_MAX && pre_y2 == DBL_MAX)
	    {
		pre_x2 = x_up;
		pre_y2 = voronoj->maxy;
	    }
      }
    if (x_low >= voronoj->minx && x_low <= voronoj->maxx)
      {
	  if (pre_x1 == DBL_MAX && pre_y1 == DBL_MAX)
	    {
		pre_x1 = x_low;
		pre_y1 = voronoj->miny;
	    }
	  else if (pre_x2 == DBL_MAX && pre_y2 == DBL_MAX)
	    {
		pre_x2 = x_low;
		pre_y2 = voronoj->miny;
	    }
      }
    if (y_left >= voronoj->miny && y_left <= voronoj->maxy)
      {
	  if (pre_x1 == DBL_MAX && pre_y1 == DBL_MAX)
	    {
		pre_x1 = voronoj->minx;
		pre_y1 = y_left;
	    }
	  else if (pre_x2 == DBL_MAX && pre_y2 == DBL_MAX)
	    {
		pre_x2 = voronoj->minx;
		pre_y2 = y_left;
	    }
      }
    if (y_right >= voronoj->miny && y_right <= voronoj->maxy)
      {
	  if (pre_x1 == DBL_MAX && pre_y1 == DBL_MAX)
	    {
		pre_x1 = voronoj->maxx;
		pre_y1 = y_right;
	    }
	  else if (pre_x2 == DBL_MAX && pre_y2 == DBL_MAX)
	    {
		pre_x2 = voronoj->maxx;
		pre_y2 = y_right;
	    }
      }

/* choosing wich point has to be returned */
    if (direct)
      {
	  /* cutting the edge in two */
	  d1 = voronoj_test_point (p_cache, cx, cy, pre_x1, pre_y1, mx, my);
	  d2 = voronoj_test_point (p_cache, cx, cy, pre_x2, pre_y2, mx, my);
	  if (d1 < d2)
	    {
		*x = pre_x1;
		*y = pre_y1;
	    }
	  else
	    {
		*x = pre_x2;
		*y = pre_y2;
	    }
      }
    else
      {
	  /* going outside */
	  d1 = voronoj_test_point (p_cache, cx, cy, pre_x1, pre_y1, mx, my);
	  d2 = voronoj_test_point (p_cache, cx, cy, pre_x2, pre_y2, mx, my);
	  if (d1 > d2)
	    {
		*x = pre_x1;
		*y = pre_y1;
	    }
	  else
	    {
		*x = pre_x2;
		*y = pre_y2;
	    }
      }
}

SPATIALITE_PRIVATE void *
voronoj_build (int count, void *p_first, double extra_frame_size)
{
    return voronoj_build_r (NULL, count, p_first, extra_frame_size);
}

SPATIALITE_PRIVATE void *
voronoj_build_r (const void *p_cache, int count, void *p_first,
		 double extra_frame_size)
{
/* building the Voronoj auxiliary struct */
    gaiaPolygonPtr first = (gaiaPolygonPtr) p_first;
    struct voronoj_aux *voronoj = NULL;
    struct voronoj_triangle *triangle;
    struct voronoj_triangle *tri2;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    int ind = 0;
    int i2;
    int direct;
    double x;
    double y;
    double z;
    double m;
    double xba;
    double yba;
    double xca;
    double yca;
    double bl;
    double cl;
    double d;
    double mx;
    double my;
    double slope;
    double intercept = 0.0;
    double minx = DBL_MAX;
    double miny = DBL_MAX;
    double maxx = DBL_MIN;
    double maxy = DBL_MIN;
    double ext_x;
    double ext_y;
    double delta;
    double delta2;

/* allocating the Voronoj struct */
    voronoj = malloc (sizeof (struct voronoj_aux));
    voronoj->count = count;
    voronoj->first_up = NULL;
    voronoj->last_up = NULL;
    voronoj->first_low = NULL;
    voronoj->last_low = NULL;
    voronoj->first_left = NULL;
    voronoj->last_left = NULL;
    voronoj->first_right = NULL;
    voronoj->last_right = NULL;
    voronoj->array = malloc (sizeof (struct voronoj_triangle) * count);

/* initializing the Voronoj struct */
    pg = first;
    while (pg)
      {
	  rng = pg->Exterior;
	  triangle = voronoj->array + ind;
	  if (pg->DimensionModel == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (rng->Coords, 0, &x, &y, &z);
		triangle->x1 = x;
		triangle->y1 = y;
		gaiaGetPointXYZ (rng->Coords, 1, &x, &y, &z);
		triangle->x2 = x;
		triangle->y2 = y;
		gaiaGetPointXYZ (rng->Coords, 2, &x, &y, &z);
		triangle->x3 = x;
		triangle->y3 = y;
	    }
	  else if (pg->DimensionModel == GAIA_XY_M)
	    {
		gaiaGetPointXYM (rng->Coords, 0, &x, &y, &m);
		triangle->x1 = x;
		triangle->y1 = y;
		gaiaGetPointXYM (rng->Coords, 1, &x, &y, &m);
		triangle->x2 = x;
		triangle->y2 = y;
		gaiaGetPointXYM (rng->Coords, 2, &x, &y, &m);
		triangle->x3 = x;
		triangle->y3 = y;
	    }
	  else if (pg->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (rng->Coords, 0, &x, &y, &z, &m);
		triangle->x1 = x;
		triangle->y1 = y;
		gaiaGetPointXYZM (rng->Coords, 1, &x, &y, &z, &m);
		triangle->x2 = x;
		triangle->y2 = y;
		gaiaGetPointXYZM (rng->Coords, 2, &x, &y, &z, &m);
		triangle->x3 = x;
		triangle->y3 = y;
	    }
	  else
	    {
		gaiaGetPoint (rng->Coords, 0, &x, &y);
		triangle->x1 = x;
		triangle->y1 = y;
		gaiaGetPoint (rng->Coords, 1, &x, &y);
		triangle->x2 = x;
		triangle->y2 = y;
		gaiaGetPoint (rng->Coords, 2, &x, &y);
		triangle->x3 = x;
		triangle->y3 = y;
	    }

	  /* computing the triangle circumcenter */
	  xba = triangle->x2 - triangle->x1;
	  yba = triangle->y2 - triangle->y1;
	  xca = triangle->x3 - triangle->x1;
	  yca = triangle->y3 - triangle->y1;
	  bl = xba * xba + yba * yba;
	  cl = xca * xca + yca * yca;
	  d = 0.5 / (xba * yca - yba * xca);
	  triangle->cx = triangle->x1 + ((yca * bl - yba * cl) * d);
	  triangle->cy = triangle->y1 + ((xba * cl - xca * bl) * d);
	  /* adjusting the frame extent */
	  voronoj_minmax (triangle->x1, triangle->y1, &minx, &miny, &maxx,
			  &maxy);
	  voronoj_minmax (triangle->x2, triangle->y2, &minx, &miny, &maxx,
			  &maxy);
	  voronoj_minmax (triangle->x3, triangle->y3, &minx, &miny, &maxx,
			  &maxy);
	  voronoj_minmax (triangle->cx, triangle->cy, &minx, &miny, &maxx,
			  &maxy);

	  triangle->tri_1_2 = NULL;
	  triangle->tri_2_3 = NULL;
	  triangle->tri_3_1 = NULL;
	  triangle->trace_1_2 = 0;
	  triangle->trace_2_3 = 0;
	  triangle->trace_3_1 = 0;
	  ind++;
	  pg = pg->Next;
      }

/* setting the frame extent */
    if (extra_frame_size < 0.0)
	extra_frame_size = 5.0;
    ext_x = maxx - minx;
    ext_y = maxy - miny;
    delta = (ext_x * extra_frame_size) / 100.0;
    delta2 = (ext_y * extra_frame_size) / 100.0;
    if (delta2 > delta)
	delta = delta2;
    voronoj->minx = minx - delta;
    voronoj->miny = miny - delta;
    voronoj->maxx = maxx + delta;
    voronoj->maxy = maxy + delta;

/* identifying triangles sharing the same edge */
    for (ind = 0; ind < voronoj->count; ind++)
      {
	  triangle = voronoj->array + ind;
	  for (i2 = ind + 1; i2 < voronoj->count; i2++)
	    {
		tri2 = voronoj->array + i2;
		if (triangle->tri_1_2 == NULL && tri2->tri_1_2 == NULL)
		  {
		      if (voronoj_same_edge
			  (triangle->x1, triangle->y1, triangle->x2,
			   triangle->y2, tri2->x1, tri2->y1, tri2->x2,
			   tri2->y2))
			{
			    triangle->tri_1_2 = tri2;
			    triangle->trace_1_2 = 1;
			    tri2->tri_1_2 = triangle;
			}
		  }
		if (triangle->tri_1_2 == NULL && tri2->tri_2_3 == NULL)
		  {
		      if (voronoj_same_edge
			  (triangle->x1, triangle->y1, triangle->x2,
			   triangle->y2, tri2->x2, tri2->y2, tri2->x3,
			   tri2->y3))
			{
			    triangle->tri_1_2 = tri2;
			    triangle->trace_1_2 = 1;
			    tri2->tri_2_3 = triangle;
			}
		  }
		if (triangle->tri_1_2 == NULL && tri2->tri_3_1 == NULL)
		  {
		      if (voronoj_same_edge
			  (triangle->x1, triangle->y1, triangle->x2,
			   triangle->y2, tri2->x3, tri2->y3, tri2->x1,
			   tri2->y1))
			{
			    triangle->tri_1_2 = tri2;
			    triangle->trace_1_2 = 1;
			    tri2->tri_3_1 = triangle;
			}
		  }
		if (triangle->tri_2_3 == NULL && tri2->tri_1_2 == NULL)
		  {
		      if (voronoj_same_edge
			  (triangle->x2, triangle->y2, triangle->x3,
			   triangle->y3, tri2->x1, tri2->y1, tri2->x2,
			   tri2->y2))
			{
			    triangle->tri_2_3 = tri2;
			    triangle->trace_2_3 = 1;
			    tri2->tri_1_2 = triangle;
			}
		  }
		if (triangle->tri_2_3 == NULL && tri2->tri_2_3 == NULL)
		  {
		      if (voronoj_same_edge
			  (triangle->x2, triangle->y2, triangle->x3,
			   triangle->y3, tri2->x2, tri2->y2, tri2->x3,
			   tri2->y3))
			{
			    triangle->tri_2_3 = tri2;
			    triangle->trace_2_3 = 1;
			    tri2->tri_2_3 = triangle;
			}
		  }
		if (triangle->tri_2_3 == NULL && tri2->tri_3_1 == NULL)
		  {
		      if (voronoj_same_edge
			  (triangle->x2, triangle->y2, triangle->x3,
			   triangle->y3, tri2->x3, tri2->y3, tri2->x1,
			   tri2->y1))
			{
			    triangle->tri_2_3 = tri2;
			    triangle->trace_2_3 = 1;
			    tri2->tri_3_1 = triangle;
			}
		  }
		if (triangle->tri_3_1 == NULL && tri2->tri_1_2 == NULL)
		  {
		      if (voronoj_same_edge
			  (triangle->x3, triangle->y3, triangle->x1,
			   triangle->y1, tri2->x1, tri2->y1, tri2->x2,
			   tri2->y2))
			{
			    triangle->tri_3_1 = tri2;
			    triangle->trace_3_1 = 1;
			    tri2->tri_1_2 = triangle;
			}
		  }
		if (triangle->tri_3_1 == NULL && tri2->tri_2_3 == NULL)
		  {
		      if (voronoj_same_edge
			  (triangle->x3, triangle->y3, triangle->x1,
			   triangle->y1, tri2->x2, tri2->y2, tri2->x3,
			   tri2->y3))
			{
			    triangle->tri_3_1 = tri2;
			    triangle->trace_3_1 = 1;
			    tri2->tri_2_3 = triangle;
			}
		  }
		if (triangle->tri_3_1 == NULL && tri2->tri_3_1 == NULL)
		  {
		      if (voronoj_same_edge
			  (triangle->x3, triangle->y3, triangle->x1,
			   triangle->y1, tri2->x3, tri2->y3, tri2->x1,
			   tri2->y1))
			{
			    triangle->tri_3_1 = tri2;
			    triangle->trace_3_1 = 1;
			    tri2->tri_3_1 = triangle;
			}
		  }
	    }

	  /* identifying vertices on the frame */
	  if (triangle->tri_1_2 == NULL)
	    {
		mx = (triangle->x1 + triangle->x2) / 2.0;
		my = (triangle->y1 + triangle->y2) / 2.0;
		if (mx == triangle->cx)
		    slope = DBL_MAX;
		else
		  {
		      slope = (my - triangle->cy) / (mx - triangle->cx);
		      intercept = my - (slope * mx);
		  }
		direct = 1;
		if (!voronoj_internal (p_cache, triangle))
		    direct = voronoj_check_nearest_edge (p_cache, triangle, 12);
		voronoj_frame_point (p_cache, intercept, slope, voronoj,
				     triangle->cx, triangle->cy, mx, my, direct,
				     &x, &y);
		triangle->x_1_2 = x;
		triangle->y_1_2 = y;
	    }
	  if (triangle->tri_2_3 == NULL)
	    {
		mx = (triangle->x2 + triangle->x3) / 2.0;
		my = (triangle->y2 + triangle->y3) / 2.0;
		if (mx == triangle->cx)
		    slope = DBL_MAX;
		else
		  {
		      slope = (my - triangle->cy) / (mx - triangle->cx);
		      intercept = my - (slope * mx);
		  }
		direct = 1;
		if (!voronoj_internal (p_cache, triangle))
		    direct = voronoj_check_nearest_edge (p_cache, triangle, 23);
		voronoj_frame_point (p_cache, intercept, slope, voronoj,
				     triangle->cx, triangle->cy, mx, my, direct,
				     &x, &y);
		triangle->x_2_3 = x;
		triangle->y_2_3 = y;
	    }
	  if (triangle->tri_3_1 == NULL)
	    {
		mx = (triangle->x3 + triangle->x1) / 2.0;
		my = (triangle->y3 + triangle->y1) / 2.0;
		if (mx == triangle->cx)
		    slope = DBL_MAX;
		else
		  {
		      slope = (my - triangle->cy) / (mx - triangle->cx);
		      intercept = my - (slope * mx);
		  }
		direct = 1;
		if (!voronoj_internal (p_cache, triangle))
		    direct = voronoj_check_nearest_edge (p_cache, triangle, 31);
		voronoj_frame_point (p_cache, intercept, slope, voronoj,
				     triangle->cx, triangle->cy, mx, my, direct,
				     &x, &y);
		triangle->x_3_1 = x;
		triangle->y_3_1 = y;
	    }
      }
    return voronoj;
}

static void *
voronoj_export_common (const void *p_cache, void *p_voronoj, void *p_result,
		       int only_edges)
{
/* building the Geometry representing Voronoj */
    gaiaGeomCollPtr result = (gaiaGeomCollPtr) p_result;
    gaiaGeomCollPtr lines;
    struct voronoj_aux *voronoj = (struct voronoj_aux *) p_voronoj;
    int ind;
    gaiaLinestringPtr ln;
    struct voronoj_triangle *triangle;
    struct voronoj_triangle *tri2;
    int i;
    int count;
    double last;
    double *array;

    for (ind = 0; ind < voronoj->count; ind++)
      {
	  triangle = voronoj->array + ind;

	  /* segments connecting two circumcenters */
	  if (triangle->tri_1_2 && triangle->trace_1_2)
	    {
		tri2 = triangle->tri_1_2;
		ln = gaiaAddLinestringToGeomColl (result, 2);
		if (result->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYZ (ln->Coords, 1, tri2->cx, tri2->cy, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYM (ln->Coords, 1, tri2->cx, tri2->cy, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, 0, triangle->cx,
					triangle->cy, 0.0, 0.0);
		      gaiaSetPointXYZM (ln->Coords, 1, tri2->cx, tri2->cy, 0.0,
					0.0);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, 0, triangle->cx, triangle->cy);
		      gaiaSetPoint (ln->Coords, 1, tri2->cx, tri2->cy);
		  }
	    }
	  if (triangle->tri_2_3 && triangle->trace_2_3)
	    {
		tri2 = triangle->tri_2_3;
		ln = gaiaAddLinestringToGeomColl (result, 2);
		if (result->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYZ (ln->Coords, 1, tri2->cx, tri2->cy, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYM (ln->Coords, 1, tri2->cx, tri2->cy, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, 0, triangle->cx,
					triangle->cy, 0.0, 0.0);
		      gaiaSetPointXYZM (ln->Coords, 1, tri2->cx, tri2->cy, 0.0,
					0.0);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, 0, triangle->cx, triangle->cy);
		      gaiaSetPoint (ln->Coords, 1, tri2->cx, tri2->cy);
		  }
	    }
	  if (triangle->tri_3_1 && triangle->trace_3_1)
	    {
		tri2 = triangle->tri_3_1;
		ln = gaiaAddLinestringToGeomColl (result, 2);
		if (result->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYZ (ln->Coords, 1, tri2->cx, tri2->cy, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYM (ln->Coords, 1, tri2->cx, tri2->cy, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, 0, triangle->cx,
					triangle->cy, 0.0, 0.0);
		      gaiaSetPointXYZM (ln->Coords, 1, tri2->cx, tri2->cy, 0.0,
					0.0);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, 0, triangle->cx, triangle->cy);
		      gaiaSetPoint (ln->Coords, 1, tri2->cx, tri2->cy);
		  }
	    }

	  /* segments connecting a circumcenter to the frame */
	  if (triangle->tri_1_2 == NULL)
	    {
		ln = gaiaAddLinestringToGeomColl (result, 2);
		if (result->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYZ (ln->Coords, 1, triangle->x_1_2,
				       triangle->y_1_2, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYM (ln->Coords, 1, triangle->x_1_2,
				       triangle->y_1_2, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, 0, triangle->cx,
					triangle->cy, 0.0, 0.0);
		      gaiaSetPointXYZM (ln->Coords, 1, triangle->x_1_2,
					triangle->y_1_2, 0.0, 0.0);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, 0, triangle->cx, triangle->cy);
		      gaiaSetPoint (ln->Coords, 1, triangle->x_1_2,
				    triangle->y_1_2);
		  }
		if (!only_edges)
		    voronoj_add_frame_point (voronoj, triangle->x_1_2,
					     triangle->y_1_2);
	    }
	  if (triangle->tri_2_3 == NULL)
	    {
		ln = gaiaAddLinestringToGeomColl (result, 2);
		if (result->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYZ (ln->Coords, 1, triangle->x_2_3,
				       triangle->y_2_3, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYM (ln->Coords, 1, triangle->x_2_3,
				       triangle->y_2_3, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, 0, triangle->cx,
					triangle->cy, 0.0, 0.0);
		      gaiaSetPointXYZM (ln->Coords, 1, triangle->x_2_3,
					triangle->y_2_3, 0.0, 0.0);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, 0, triangle->cx, triangle->cy);
		      gaiaSetPoint (ln->Coords, 1, triangle->x_2_3,
				    triangle->y_2_3);
		  }
		if (!only_edges)
		    voronoj_add_frame_point (voronoj, triangle->x_2_3,
					     triangle->y_2_3);
	    }
	  if (triangle->tri_3_1 == NULL)
	    {
		ln = gaiaAddLinestringToGeomColl (result, 2);
		if (result->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYZ (ln->Coords, 1, triangle->x_3_1,
				       triangle->y_3_1, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, 0, triangle->cx,
				       triangle->cy, 0.0);
		      gaiaSetPointXYM (ln->Coords, 1, triangle->x_3_1,
				       triangle->y_3_1, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, 0, triangle->cx,
					triangle->cy, 0.0, 0.0);
		      gaiaSetPointXYZM (ln->Coords, 1, triangle->x_3_1,
					triangle->y_3_1, 0.0, 0.0);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, 0, triangle->cx, triangle->cy);
		      gaiaSetPoint (ln->Coords, 1, triangle->x_3_1,
				    triangle->y_3_1);
		  }
		if (!only_edges)
		    voronoj_add_frame_point (voronoj, triangle->x_3_1,
					     triangle->y_3_1);
	    }
      }

    if (only_edges)
	return result;

/* setting up the frame's upper edge */
    last = voronoj->minx;
    array = voronoj_sorted_up (voronoj, &count);
    if (array)
      {
	  for (i = 0; i < count; i++)
	    {
		ln = gaiaAddLinestringToGeomColl (result, 2);
		if (result->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, 0, last, voronoj->maxy, 0.0);
		      gaiaSetPointXYZ (ln->Coords, 1, *(array + i),
				       voronoj->maxy, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, 0, last, voronoj->maxy, 0.0);
		      gaiaSetPointXYM (ln->Coords, 1, *(array + i),
				       voronoj->maxy, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, 0, last,
					voronoj->maxy, 0.0, 0.0);
		      gaiaSetPointXYZM (ln->Coords, 1, *(array + i),
					voronoj->maxy, 0.0, 0.0);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, 0, last, voronoj->maxy);
		      gaiaSetPoint (ln->Coords, 1, *(array + i), voronoj->maxy);
		  }
		last = *(array + i);
	    }
	  free (array);
      }

/* closing the frame's upper edge */
    ln = gaiaAddLinestringToGeomColl (result, 2);
    if (result->DimensionModel == GAIA_XY_Z)
      {
	  gaiaSetPointXYZ (ln->Coords, 0, last, voronoj->maxy, 0.0);
	  gaiaSetPointXYZ (ln->Coords, 1, voronoj->maxx, voronoj->maxy, 0.0);
      }
    else if (result->DimensionModel == GAIA_XY_M)
      {
	  gaiaSetPointXYM (ln->Coords, 0, last, voronoj->maxy, 0.0);
	  gaiaSetPointXYM (ln->Coords, 1, voronoj->maxx, voronoj->maxy, 0.0);
      }
    else if (result->DimensionModel == GAIA_XY_Z_M)
      {
	  gaiaSetPointXYZM (ln->Coords, 0, last, voronoj->maxy, 0.0, 0.0);
	  gaiaSetPointXYZM (ln->Coords, 1, voronoj->maxx,
			    voronoj->maxy, 0.0, 0.0);
      }
    else
      {
	  gaiaSetPoint (ln->Coords, 0, last, voronoj->maxy);
	  gaiaSetPoint (ln->Coords, 1, voronoj->maxx, voronoj->maxy);
      }

/* setting up the frame's lower edge */
    last = voronoj->minx;
    array = voronoj_sorted_low (voronoj, &count);
    if (array)
      {
	  for (i = 0; i < count; i++)
	    {
		ln = gaiaAddLinestringToGeomColl (result, 2);
		if (result->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, 0, last, voronoj->miny, 0.0);
		      gaiaSetPointXYZ (ln->Coords, 1, *(array + i),
				       voronoj->miny, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, 0, last, voronoj->miny, 0.0);
		      gaiaSetPointXYM (ln->Coords, 1, *(array + i),
				       voronoj->miny, 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, 0, last,
					voronoj->miny, 0.0, 0.0);
		      gaiaSetPointXYZM (ln->Coords, 1, *(array + i),
					voronoj->miny, 0.0, 0.0);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, 0, last, voronoj->miny);
		      gaiaSetPoint (ln->Coords, 1, *(array + i), voronoj->miny);
		  }
		last = *(array + i);
	    }
	  free (array);
      }

/* closing the frame's lower edge */
    ln = gaiaAddLinestringToGeomColl (result, 2);
    if (result->DimensionModel == GAIA_XY_Z)
      {
	  gaiaSetPointXYZ (ln->Coords, 0, last, voronoj->miny, 0.0);
	  gaiaSetPointXYZ (ln->Coords, 1, voronoj->maxx, voronoj->miny, 0.0);
      }
    else if (result->DimensionModel == GAIA_XY_M)
      {
	  gaiaSetPointXYM (ln->Coords, 0, last, voronoj->miny, 0.0);
	  gaiaSetPointXYM (ln->Coords, 1, voronoj->maxx, voronoj->miny, 0.0);
      }
    else if (result->DimensionModel == GAIA_XY_Z_M)
      {
	  gaiaSetPointXYZM (ln->Coords, 0, last, voronoj->miny, 0.0, 0.0);
	  gaiaSetPointXYZM (ln->Coords, 1, voronoj->maxx,
			    voronoj->miny, 0.0, 0.0);
      }
    else
      {
	  gaiaSetPoint (ln->Coords, 0, last, voronoj->miny);
	  gaiaSetPoint (ln->Coords, 1, voronoj->maxx, voronoj->miny);
      }

/* setting up the frame's left edge */
    last = voronoj->miny;
    array = voronoj_sorted_left (voronoj, &count);
    if (array)
      {
	  for (i = 0; i < count; i++)
	    {
		ln = gaiaAddLinestringToGeomColl (result, 2);
		if (result->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, 0, voronoj->minx, last, 0.0);
		      gaiaSetPointXYZ (ln->Coords, 1, voronoj->minx,
				       *(array + i), 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, 0, voronoj->minx, last, 0.0);
		      gaiaSetPointXYM (ln->Coords, 1, voronoj->minx,
				       *(array + i), 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, 0, voronoj->minx,
					last, 0.0, 0.0);
		      gaiaSetPointXYZM (ln->Coords, 1, voronoj->minx,
					*(array + i), 0.0, 0.0);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, 0, voronoj->minx, last);
		      gaiaSetPoint (ln->Coords, 1, voronoj->minx, *(array + i));
		  }
		last = *(array + i);
	    }
	  free (array);
      }

/* closing the frame's left edge */
    ln = gaiaAddLinestringToGeomColl (result, 2);
    if (result->DimensionModel == GAIA_XY_Z)
      {
	  gaiaSetPointXYZ (ln->Coords, 0, voronoj->minx, last, 0.0);
	  gaiaSetPointXYZ (ln->Coords, 1, voronoj->minx, voronoj->maxy, 0.0);
      }
    else if (result->DimensionModel == GAIA_XY_M)
      {
	  gaiaSetPointXYM (ln->Coords, 0, voronoj->minx, last, 0.0);
	  gaiaSetPointXYM (ln->Coords, 1, voronoj->minx, voronoj->maxy, 0.0);
      }
    else if (result->DimensionModel == GAIA_XY_Z_M)
      {
	  gaiaSetPointXYZM (ln->Coords, 0, voronoj->minx, last, 0.0, 0.0);
	  gaiaSetPointXYZM (ln->Coords, 1, voronoj->minx,
			    voronoj->maxy, 0.0, 0.0);
      }
    else
      {
	  gaiaSetPoint (ln->Coords, 0, voronoj->minx, last);
	  gaiaSetPoint (ln->Coords, 1, voronoj->minx, voronoj->maxy);
      }

    /* setting up the frame's right edge */
    last = voronoj->miny;
    array = voronoj_sorted_right (voronoj, &count);
    if (array)
      {
	  for (i = 0; i < count; i++)
	    {
		ln = gaiaAddLinestringToGeomColl (result, 2);
		if (result->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (ln->Coords, 0, voronoj->maxx, last, 0.0);
		      gaiaSetPointXYZ (ln->Coords, 1, voronoj->maxx,
				       *(array + i), 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (ln->Coords, 0, voronoj->maxx, last, 0.0);
		      gaiaSetPointXYM (ln->Coords, 1, voronoj->maxx,
				       *(array + i), 0.0);
		  }
		else if (result->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (ln->Coords, 0, voronoj->maxx,
					last, 0.0, 0.0);
		      gaiaSetPointXYZM (ln->Coords, 1, voronoj->maxx,
					*(array + i), 0.0, 0.0);
		  }
		else
		  {
		      gaiaSetPoint (ln->Coords, 0, voronoj->maxx, last);
		      gaiaSetPoint (ln->Coords, 1, voronoj->maxx, *(array + i));
		  }
		last = *(array + i);
	    }
	  free (array);
      }

/* closing the frame's right edge */
    ln = gaiaAddLinestringToGeomColl (result, 2);
    if (result->DimensionModel == GAIA_XY_Z)
      {
	  gaiaSetPointXYZ (ln->Coords, 0, voronoj->maxx, last, 0.0);
	  gaiaSetPointXYZ (ln->Coords, 1, voronoj->maxx, voronoj->maxy, 0.0);
      }
    else if (result->DimensionModel == GAIA_XY_M)
      {
	  gaiaSetPointXYM (ln->Coords, 0, voronoj->maxx, last, 0.0);
	  gaiaSetPointXYM (ln->Coords, 1, voronoj->maxx, voronoj->maxy, 0.0);
      }
    else if (result->DimensionModel == GAIA_XY_Z_M)
      {
	  gaiaSetPointXYZM (ln->Coords, 0, voronoj->maxx, last, 0.0, 0.0);
	  gaiaSetPointXYZM (ln->Coords, 1, voronoj->maxx,
			    voronoj->maxy, 0.0, 0.0);
      }
    else
      {
	  gaiaSetPoint (ln->Coords, 0, voronoj->maxx, last);
	  gaiaSetPoint (ln->Coords, 1, voronoj->maxx, voronoj->maxy);
      }

/* building Polygons */
    lines = result;
    if (p_cache != NULL)
	result = gaiaPolygonize_r (p_cache, lines, 1);
    else
	result = gaiaPolygonize (lines, 1);
    gaiaFreeGeomColl (lines);
    return result;
}

SPATIALITE_PRIVATE void *
voronoj_export (void *p_voronoj, void *p_result, int only_edges)
{
    return voronoj_export_common (NULL, p_voronoj, p_result, only_edges);
}

SPATIALITE_PRIVATE void *
voronoj_export_r (const void *p_cache, void *p_voronoj, void *p_result,
		  int only_edges)
{
    return voronoj_export_common (p_cache, p_voronoj, p_result, only_edges);
}

SPATIALITE_PRIVATE void
voronoj_free (void *p_voronoj)
{
/* memory cleanup: destroying the Voronoj auxiliary struct */
    struct voronoj_aux *voronoj = (struct voronoj_aux *) p_voronoj;
    struct voronoj_point *pt;
    struct voronoj_point *ptn;
    free (voronoj->array);
    pt = voronoj->first_up;
    while (pt)
      {
	  ptn = pt->next;
	  free (pt);
	  pt = ptn;
      }
    pt = voronoj->first_low;
    while (pt)
      {
	  ptn = pt->next;
	  free (pt);
	  pt = ptn;
      }
    pt = voronoj->first_left;
    while (pt)
      {
	  ptn = pt->next;
	  free (pt);
	  pt = ptn;
      }
    pt = voronoj->first_right;
    while (pt)
      {
	  ptn = pt->next;
	  free (pt);
	  pt = ptn;
      }
    free (voronoj);
}

SPATIALITE_PRIVATE int
delaunay_triangle_check (void *ppg)
{
/* test if it's really a triangle */
    gaiaPolygonPtr pg = (gaiaPolygonPtr) ppg;
    gaiaRingPtr rng = pg->Exterior;
    if (rng->Points == 4 && pg->NumInteriors == 0)
	return 1;
    return 0;
}

static void
concave_hull_stats (struct concave_hull_str *concave, double length)
{
/* update concave hull statistics */
    if (concave->count == 0)
      {
	  concave->count = 1.0;
	  concave->mean = length;
	  return;
      }

    concave->count += 1.0;
    concave->quot = concave->quot + (((concave->count - 1.0) *
				      ((length - concave->mean) *
				       (length - concave->mean))) /
				     concave->count);
    concave->mean = concave->mean + ((length - concave->mean) / concave->count);
}

static int
concave_hull_filter (const void *p_cache, double x1, double y1,
		     double x2, double y2, double x3, double y3, double limit)
{
/* filtering triangles to be inserted into the Concave Hull */
    gaiaGeomCollPtr segm;
    gaiaLinestringPtr ln;
    double length;

    segm = gaiaAllocGeomColl ();
    ln = gaiaAddLinestringToGeomColl (segm, 2);
    gaiaSetPoint (ln->Coords, 0, x1, y1);
    gaiaSetPoint (ln->Coords, 1, x2, y2);
    if (p_cache != NULL)
	gaiaGeomCollLength_r (p_cache, segm, &length);
    else
	gaiaGeomCollLength (segm, &length);
    gaiaFreeGeomColl (segm);
    if (length >= limit)
	return 0;

    segm = gaiaAllocGeomColl ();
    ln = gaiaAddLinestringToGeomColl (segm, 2);
    gaiaSetPoint (ln->Coords, 0, x2, y2);
    gaiaSetPoint (ln->Coords, 1, x3, y3);
    if (p_cache != NULL)
	gaiaGeomCollLength_r (p_cache, segm, &length);
    else
	gaiaGeomCollLength (segm, &length);
    gaiaFreeGeomColl (segm);
    if (length >= limit)
	return 0;

    segm = gaiaAllocGeomColl ();
    ln = gaiaAddLinestringToGeomColl (segm, 2);
    gaiaSetPoint (ln->Coords, 0, x3, y3);
    gaiaSetPoint (ln->Coords, 1, x1, y1);
    if (p_cache != NULL)
	gaiaGeomCollLength_r (p_cache, segm, &length);
    else
	gaiaGeomCollLength (segm, &length);
    gaiaFreeGeomColl (segm);
    if (length >= limit)
	return 0;

    return 1;
}

static gaiaGeomCollPtr
concave_hull_no_holes (gaiaGeomCollPtr in)
{
/* returning a Polygon surely not containing any hole */
    gaiaGeomCollPtr out = NULL;
    gaiaPolygonPtr pg_in;
    gaiaPolygonPtr pg_out;
    gaiaRingPtr rng_in;
    gaiaRingPtr rng_out;
    int iv;
    double x;
    double y;
    double z;
    double m;

    if (in->DimensionModel == GAIA_XY_Z)
	out = gaiaAllocGeomCollXYZ ();
    else if (in->DimensionModel == GAIA_XY_M)
	out = gaiaAllocGeomCollXYM ();
    else if (in->DimensionModel == GAIA_XY_Z_M)
	out = gaiaAllocGeomCollXYZM ();
    else
	out = gaiaAllocGeomColl ();
    out->Srid = in->Srid;

    pg_in = in->FirstPolygon;
    while (pg_in)
      {
	  rng_in = pg_in->Exterior;
	  pg_out = gaiaAddPolygonToGeomColl (out, rng_in->Points, 0);
	  rng_out = pg_out->Exterior;
	  for (iv = 0; iv < rng_in->Points; iv++)
	    {
		/* copying Exterior Ring vertices */
		if (in->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (rng_in->Coords, iv, &x, &y, &z);
		      gaiaSetPointXYZ (rng_out->Coords, iv, x, y, z);
		  }
		else if (in->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (rng_in->Coords, iv, &x, &y, &m);
		      gaiaSetPointXYM (rng_out->Coords, iv, x, y, m);
		  }
		else if (in->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (rng_in->Coords, iv, &x, &y, &z, &m);
		      gaiaSetPointXYZM (rng_out->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaGetPoint (rng_in->Coords, iv, &x, &y);
		      gaiaSetPoint (rng_out->Coords, iv, x, y);
		  }
	    }
	  pg_in = pg_in->Next;
      }
    return out;
}

static void *
concave_hull_build_common (const void *p_cache, void *p_first,
			   int dimension_model, double factor, int allow_holes)
{
/* building the Concave Hull */
    struct concave_hull_str concave;
    gaiaPolygonPtr first = (gaiaPolygonPtr) p_first;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPolygonPtr pg_out;
    gaiaRingPtr rng_out;
    gaiaGeomCollPtr segm;
    gaiaGeomCollPtr result;
    gaiaLinestringPtr ln;
    double x;
    double y;
    double z;
    double m;
    double x1;
    double y1;
    double x2;
    double y2;
    double x3;
    double y3;
    double length;
    double std_dev;
    int count;

/* initializing the struct for mean and standard deviation */
    concave.mean = 0.0;
    concave.quot = 0.0;
    concave.count = 0.0;

    pg = first;
    while (pg)
      {
	  /* examining each triangle / computing statistics distribution */
	  rng = pg->Exterior;
	  if (pg->DimensionModel == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (rng->Coords, 0, &x, &y, &z);
		x1 = x;
		y1 = y;
		gaiaGetPointXYZ (rng->Coords, 1, &x, &y, &z);
		x2 = x;
		y2 = y;
		gaiaGetPointXYZ (rng->Coords, 2, &x, &y, &z);
		x3 = x;
		y3 = y;
	    }
	  else if (pg->DimensionModel == GAIA_XY_M)
	    {
		gaiaGetPointXYM (rng->Coords, 0, &x, &y, &m);
		x1 = x;
		y1 = y;
		gaiaGetPointXYM (rng->Coords, 1, &x, &y, &m);
		x2 = x;
		y2 = y;
		gaiaGetPointXYM (rng->Coords, 2, &x, &y, &m);
		x3 = x;
		y3 = y;
	    }
	  else if (pg->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (rng->Coords, 0, &x, &y, &z, &m);
		x1 = x;
		y1 = y;
		gaiaGetPointXYZM (rng->Coords, 1, &x, &y, &z, &m);
		x2 = x;
		y2 = y;
		gaiaGetPointXYZM (rng->Coords, 2, &x, &y, &z, &m);
		x3 = x;
		y3 = y;
	    }
	  else
	    {
		gaiaGetPoint (rng->Coords, 0, &x, &y);
		x1 = x;
		y1 = y;
		gaiaGetPoint (rng->Coords, 1, &x, &y);
		x2 = x;
		y2 = y;
		gaiaGetPoint (rng->Coords, 2, &x, &y);
		x3 = x;
		y3 = y;
	    }

	  segm = gaiaAllocGeomColl ();
	  ln = gaiaAddLinestringToGeomColl (segm, 2);
	  gaiaSetPoint (ln->Coords, 0, x1, y1);
	  gaiaSetPoint (ln->Coords, 1, x2, y2);
	  if (p_cache != NULL)
	      gaiaGeomCollLength_r (p_cache, segm, &length);
	  else
	      gaiaGeomCollLength (segm, &length);
	  gaiaFreeGeomColl (segm);
	  concave_hull_stats (&concave, length);

	  segm = gaiaAllocGeomColl ();
	  ln = gaiaAddLinestringToGeomColl (segm, 2);
	  gaiaSetPoint (ln->Coords, 0, x2, y2);
	  gaiaSetPoint (ln->Coords, 1, x3, y3);
	  if (p_cache != NULL)
	      gaiaGeomCollLength_r (p_cache, segm, &length);
	  else
	      gaiaGeomCollLength (segm, &length);
	  gaiaFreeGeomColl (segm);
	  concave_hull_stats (&concave, length);

	  segm = gaiaAllocGeomColl ();
	  ln = gaiaAddLinestringToGeomColl (segm, 2);
	  gaiaSetPoint (ln->Coords, 0, x3, y3);
	  gaiaSetPoint (ln->Coords, 1, x1, y1);
	  if (p_cache != NULL)
	      gaiaGeomCollLength_r (p_cache, segm, &length);
	  else
	      gaiaGeomCollLength (segm, &length);
	  gaiaFreeGeomColl (segm);
	  concave_hull_stats (&concave, length);

	  pg = pg->Next;
      }

    std_dev = sqrt (concave.quot / concave.count);

/* creating the Geometry representing the Concave Hull */
    if (dimension_model == GAIA_XY_Z)
	result = gaiaAllocGeomCollXYZ ();
    else if (dimension_model == GAIA_XY_M)
	result = gaiaAllocGeomCollXYM ();
    else if (dimension_model == GAIA_XY_Z_M)
	result = gaiaAllocGeomCollXYZM ();
    else
	result = gaiaAllocGeomColl ();

    count = 0;
    pg = first;
    while (pg)
      {
	  /* selecting triangles to be inserted */
	  rng = pg->Exterior;
	  if (pg->DimensionModel == GAIA_XY_Z)
	    {
		gaiaGetPointXYZ (rng->Coords, 0, &x, &y, &z);
		x1 = x;
		y1 = y;
		gaiaGetPointXYZ (rng->Coords, 1, &x, &y, &z);
		x2 = x;
		y2 = y;
		gaiaGetPointXYZ (rng->Coords, 2, &x, &y, &z);
		x3 = x;
		y3 = y;
	    }
	  else if (pg->DimensionModel == GAIA_XY_M)
	    {
		gaiaGetPointXYM (rng->Coords, 0, &x, &y, &m);
		x1 = x;
		y1 = y;
		gaiaGetPointXYM (rng->Coords, 1, &x, &y, &m);
		x2 = x;
		y2 = y;
		gaiaGetPointXYM (rng->Coords, 2, &x, &y, &m);
		x3 = x;
		y3 = y;
	    }
	  else if (pg->DimensionModel == GAIA_XY_Z_M)
	    {
		gaiaGetPointXYZM (rng->Coords, 0, &x, &y, &z, &m);
		x1 = x;
		y1 = y;
		gaiaGetPointXYZM (rng->Coords, 1, &x, &y, &z, &m);
		x2 = x;
		y2 = y;
		gaiaGetPointXYZM (rng->Coords, 2, &x, &y, &z, &m);
		x3 = x;
		y3 = y;
	    }
	  else
	    {
		gaiaGetPoint (rng->Coords, 0, &x, &y);
		x1 = x;
		y1 = y;
		gaiaGetPoint (rng->Coords, 1, &x, &y);
		x2 = x;
		y2 = y;
		gaiaGetPoint (rng->Coords, 2, &x, &y);
		x3 = x;
		y3 = y;
	    }

	  if (concave_hull_filter
	      (p_cache, x1, y1, x2, y2, x3, y3, std_dev * factor))
	    {
		/* inserting this triangle into the Concave Hull */
		pg_out = gaiaAddPolygonToGeomColl (result, 4, 0);
		rng_out = pg_out->Exterior;
		if (pg->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (rng->Coords, 0, &x, &y, &z);
		      gaiaSetPointXYZ (rng_out->Coords, 0, x, y, z);
		      gaiaGetPointXYZ (rng->Coords, 1, &x, &y, &z);
		      gaiaSetPointXYZ (rng_out->Coords, 1, x, y, z);
		      gaiaGetPointXYZ (rng->Coords, 2, &x, &y, &z);
		      gaiaSetPointXYZ (rng_out->Coords, 2, x, y, z);
		      gaiaGetPointXYZ (rng->Coords, 3, &x, &y, &z);
		      gaiaSetPointXYZ (rng_out->Coords, 3, x, y, z);
		  }
		else if (pg->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (rng->Coords, 0, &x, &y, &m);
		      gaiaSetPointXYM (rng_out->Coords, 0, x, y, m);
		      gaiaGetPointXYM (rng->Coords, 1, &x, &y, &m);
		      gaiaSetPointXYM (rng_out->Coords, 1, x, y, m);
		      gaiaGetPointXYM (rng->Coords, 2, &x, &y, &m);
		      gaiaSetPointXYM (rng_out->Coords, 2, x, y, m);
		      gaiaGetPointXYM (rng->Coords, 3, &x, &y, &m);
		      gaiaSetPointXYM (rng_out->Coords, 3, x, y, m);
		  }
		else if (pg->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (rng->Coords, 0, &x, &y, &z, &m);
		      gaiaSetPointXYZM (rng_out->Coords, 0, x, y, z, m);
		      gaiaGetPointXYZM (rng->Coords, 1, &x, &y, &z, &m);
		      gaiaSetPointXYZM (rng_out->Coords, 1, x, y, z, m);
		      gaiaGetPointXYZM (rng->Coords, 2, &x, &y, &z, &m);
		      gaiaSetPointXYZM (rng_out->Coords, 2, x, y, z, m);
		      gaiaGetPointXYZM (rng->Coords, 3, &x, &y, &z, &m);
		      gaiaSetPointXYZM (rng_out->Coords, 3, x, y, z, m);
		  }
		else
		  {
		      gaiaGetPoint (rng->Coords, 0, &x, &y);
		      gaiaSetPoint (rng_out->Coords, 0, x, y);
		      gaiaGetPoint (rng->Coords, 1, &x, &y);
		      gaiaSetPoint (rng_out->Coords, 1, x, y);
		      gaiaGetPoint (rng->Coords, 2, &x, &y);
		      gaiaSetPoint (rng_out->Coords, 2, x, y);
		      gaiaGetPoint (rng->Coords, 3, &x, &y);
		      gaiaSetPoint (rng_out->Coords, 3, x, y);
		  }
		count++;
	    }

	  pg = pg->Next;
      }

    if (count == 0)
      {
	  gaiaFreeGeomColl (result);
	  return NULL;
      }

/* merging all triangles into the Concave Hull */
    segm = result;
    if (p_cache != NULL)
	result = gaiaUnaryUnion_r (p_cache, segm);
    else
	result = gaiaUnaryUnion (segm);
    gaiaFreeGeomColl (segm);
    if (!result)
	return NULL;
    if (result->FirstPolygon == NULL)
      {
	  gaiaFreeGeomColl (result);
	  return NULL;
      }
    if (allow_holes)
	return result;

/* suppressing any interior hole */
    segm = result;
    result = concave_hull_no_holes (segm);
    gaiaFreeGeomColl (segm);
    if (!result)
	return NULL;
    if (result->FirstPolygon == NULL)
      {
	  gaiaFreeGeomColl (result);
	  return NULL;
      }
    return result;
}

SPATIALITE_PRIVATE void *
concave_hull_build (void *p_first, int dimension_model, double factor,
		    int allow_holes)
{
    return concave_hull_build_common (NULL, p_first, dimension_model, factor,
				      allow_holes);
}

SPATIALITE_PRIVATE void *
concave_hull_build_r (const void *p_cache, void *p_first, int dimension_model,
		      double factor, int allow_holes)
{
    return concave_hull_build_common (p_cache, p_first, dimension_model, factor,
				      allow_holes);
}

#endif /* end GEOS advanced features */
