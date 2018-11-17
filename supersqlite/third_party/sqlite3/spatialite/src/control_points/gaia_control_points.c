/*

 gaia_control_points.c -- Gaia implementation of RMSE and TPS Control Points
    
 version 4.3, 2015 May 5

 Author: Sandro Furieri a.furieri@lqt.it

 ------------------------------------------------------------------------------
 DISCLAIMER: this source is simply intemded as an interface supporting the
             sources from Grass GIS
			 NOTE: accordingly to the initial license this file is released
			 under GPL2+ terms
 ------------------------------------------------------------------------------
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 
*/

#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#ifdef ENABLE_GCP		/* only if ControlPoints enabled */

#include <spatialite/sqlite.h>

#include <spatialite_private.h>
#include <spatialite/gaiageo.h>
#include <spatialite/gaiamatrix.h>
#include <spatialite/control_points.h>

#include "grass_crs.h"

#define POLYNOMIAL_MAGIC_START		0x00
#define POLYNOMIAL_MAGIC_DELIMITER	0x6a
#define POLYNOMIAL_MAGIC_END		0x63
#define POLYNOMIAL_FIRST_ORDER		0xb1
#define POLYNOMIAL_SECOND_ORDER		0xb2
#define POLYNOMIAL_THIRD_ORDER		0xb3
#define THIN_PLATE_SPLINE			0x3f
#define POLYNOMIAL_2D				0x3e
#define POLYNOMIAL_3D				0x3d

struct cp_coeffs
{
/* Polynomial Coefficients */
    unsigned char type;
    unsigned char order;
    double E[20];
    double N[20];
    double Z[20];
    double *Etps;
    double *Ntps;
    struct Control_Points grass_cp;
};

GAIACP_DECLARE GaiaControlPointsPtr
gaiaCreateControlPoints (int allocation_incr, int has3d, int order, int tps)
{
/* creating a Control Point set container */
    struct gaia_control_points *cp =
	malloc (sizeof (struct gaia_control_points));
    if (cp == NULL)
	return NULL;
    if (order < 1)
	order = 1;
    if (order > 3)
	order = 3;
    cp->order = order;
    cp->has3d = has3d;
    cp->tps = tps;
    cp->affine_valid = 0;
    if (allocation_incr < 64)
	allocation_incr = 64;
    cp->allocation_incr = allocation_incr;
    cp->allocated_items = allocation_incr;
    cp->count = 0;
    cp->x0 = malloc (sizeof (double) * allocation_incr);
    cp->y0 = malloc (sizeof (double) * allocation_incr);
    cp->x1 = malloc (sizeof (double) * allocation_incr);
    cp->y1 = malloc (sizeof (double) * allocation_incr);
    if (has3d)
      {
	  cp->z0 = malloc (sizeof (double) * allocation_incr);
	  cp->z1 = malloc (sizeof (double) * allocation_incr);
      }
    else
      {
	  cp->z0 = NULL;
	  cp->z1 = NULL;
      }
    if (cp->x0 == NULL || cp->y0 == NULL || cp->x1 == NULL || cp->y1 == NULL)
	goto error;
    if (has3d && (cp->z0 == NULL || cp->z1 == NULL))
	goto error;
    return (GaiaControlPointsPtr) cp;

  error:
    if (cp->x0 != NULL)
	free (cp->x0);
    if (cp->y0 != NULL)
	free (cp->y0);
    if (cp->z0 != NULL)
	free (cp->z0);
    if (cp->x1 != NULL)
	free (cp->x1);
    if (cp->y1 != NULL)
	free (cp->y1);
    if (cp->z1 != NULL)
	free (cp->z1);
    return NULL;
}

GAIACP_DECLARE int
gaiaAddControlPoint3D (GaiaControlPointsPtr cp_handle, double x0, double y0,
		       double z0, double x1, double y1, double z1)
{
/* inserting a Control Point 3D into the aggregate container */
    struct gaia_control_points *cp = (struct gaia_control_points *) cp_handle;
    if (cp == NULL)
	return 0;
    if (cp->has3d == 0)
	return 0;
    if (cp->allocated_items == cp->count)
      {
	  /* increasing the size of coord arrays */
	  cp->allocated_items += cp->allocation_incr;
	  cp->x0 = realloc (cp->x0, sizeof (double) * cp->allocated_items);
	  cp->y0 = realloc (cp->y0, sizeof (double) * cp->allocated_items);
	  cp->z0 = realloc (cp->z0, sizeof (double) * cp->allocated_items);
	  cp->x1 = realloc (cp->x1, sizeof (double) * cp->allocated_items);
	  cp->y1 = realloc (cp->y1, sizeof (double) * cp->allocated_items);
	  cp->z1 = realloc (cp->z1, sizeof (double) * cp->allocated_items);
      }
    if (cp->x0 == NULL || cp->y0 == NULL || cp->x1 == NULL || cp->y1 == NULL
	|| cp->z0 == NULL || cp->z1 == NULL)
	return 0;
    cp->x0[cp->count] = x0;
    cp->y0[cp->count] = y0;
    cp->z0[cp->count] = z0;
    cp->x1[cp->count] = x1;
    cp->y1[cp->count] = y1;
    cp->z1[cp->count] = z1;
    cp->count += 1;
    return 1;
}

GAIACP_DECLARE int
gaiaAddControlPoint2D (GaiaControlPointsPtr cp_handle, double x0, double y0,
		       double x1, double y1)
{
/* inserting a Control Point 2D into the aggregate container */
    struct gaia_control_points *cp = (struct gaia_control_points *) cp_handle;
    if (cp == NULL)
	return 0;
    if (cp->has3d)
	return 0;
    if (cp->allocated_items == cp->count)
      {
	  /* increasing the size of coord arrays */
	  cp->allocated_items += 1024;
	  cp->x0 = realloc (cp->x0, sizeof (double) * cp->allocated_items);
	  cp->y0 = realloc (cp->y0, sizeof (double) * cp->allocated_items);
	  cp->x1 = realloc (cp->x1, sizeof (double) * cp->allocated_items);
	  cp->y1 = realloc (cp->y1, sizeof (double) * cp->allocated_items);
      }
    if (cp->x0 == NULL || cp->y0 == NULL || cp->x1 == NULL || cp->y1 == NULL)
	return 0;
    cp->x0[cp->count] = x0;
    cp->y0[cp->count] = y0;
    cp->x1[cp->count] = x1;
    cp->y1[cp->count] = y1;
    cp->count += 1;
    return 1;
}

GAIACP_DECLARE void
gaiaFreeControlPoints (GaiaControlPointsPtr cp_handle)
{
/* memory cleanup */
    struct gaia_control_points *cp = (struct gaia_control_points *) cp_handle;
    if (cp == NULL)
	return;
    free (cp->x0);
    free (cp->y0);
    free (cp->x1);
    free (cp->y1);
    if (cp->has3d)
      {
	  free (cp->z0);
	  free (cp->z1);
      }
    free (cp);
}


static void
copy_control_points_2d (struct gaia_control_points *gaia_cp,
			struct Control_Points *cp)
{
/* initializing Grass 2D Control Points */
    int i;
    cp->count = gaia_cp->count;
    cp->e1 = malloc (sizeof (double) * cp->count);
    cp->e2 = malloc (sizeof (double) * cp->count);
    cp->n1 = malloc (sizeof (double) * cp->count);
    cp->n2 = malloc (sizeof (double) * cp->count);
    cp->status = malloc (sizeof (double) * cp->count);
    for (i = 0; i < cp->count; i++)
      {
	  cp->e1[i] = gaia_cp->x0[i];
	  cp->e2[i] = gaia_cp->x1[i];
	  cp->n1[i] = gaia_cp->y0[i];
	  cp->n2[i] = gaia_cp->y1[i];
	  cp->status[i] = 1;
      }
}

static void
copy_control_points_3d (struct gaia_control_points *gaia_cp,
			struct Control_Points_3D *cp)
{
/* initializing Grass 2D Control Points */
    int i;
    cp->count = gaia_cp->count;
    cp->e1 = malloc (sizeof (double) * cp->count);
    cp->e2 = malloc (sizeof (double) * cp->count);
    cp->n1 = malloc (sizeof (double) * cp->count);
    cp->n2 = malloc (sizeof (double) * cp->count);
    cp->z1 = malloc (sizeof (double) * cp->count);
    cp->z2 = malloc (sizeof (double) * cp->count);
    cp->status = malloc (sizeof (double) * cp->count);
    for (i = 0; i < cp->count; i++)
      {
	  cp->e1[i] = gaia_cp->x0[i];
	  cp->e2[i] = gaia_cp->x1[i];
	  cp->n1[i] = gaia_cp->y0[i];
	  cp->n2[i] = gaia_cp->y1[i];
	  cp->z1[i] = gaia_cp->z0[i];
	  cp->z2[i] = gaia_cp->z1[i];
	  cp->status[i] = 1;
      }
}

static void
free_control_points_2d (struct Control_Points *cp)
{
/* freeing Grass 2D Control Points */
    if (cp->e1 != NULL)
	free (cp->e1);
    if (cp->e2 != NULL)
	free (cp->e2);
    if (cp->n1 != NULL)
	free (cp->n1);
    if (cp->n2 != NULL)
	free (cp->n2);
    if (cp->status != NULL)
	free (cp->status);
}

static void
free_control_points_3d (struct Control_Points_3D *cp)
{
/* freeing Grass 3D Control Points */
    if (cp->e1 != NULL)
	free (cp->e1);
    if (cp->e2 != NULL)
	free (cp->e2);
    if (cp->n1 != NULL)
	free (cp->n1);
    if (cp->n2 != NULL)
	free (cp->n2);
    if (cp->z1 != NULL)
	free (cp->z1);
    if (cp->z2 != NULL)
	free (cp->z2);
    if (cp->status != NULL)
	free (cp->status);
}

static int
blob_encode_3d (double *E, double *N, double *Z, int order,
		unsigned char **blob, int *blob_sz)
{
/* creating a BLOB-Polynomial object - 3D */
    int i;
    int max;
    unsigned char *xblob = NULL;
    unsigned char *ptr;
    int xblob_sz;
    int endian_arch = gaiaEndianArch ();

    *blob = NULL;
    *blob_sz = 0;

    if (order == 2)
	max = 10;
    else if (order == 3)
	max = 20;
    else
	max = 4;
    xblob_sz = 11 + (max * (3 * (sizeof (double) + 1)));

    xblob = malloc (xblob_sz);
    if (xblob == NULL)
	return 0;
    ptr = xblob;
/* encoding the BLOB */
    *ptr = POLYNOMIAL_MAGIC_START;	/* START signature */
    *(ptr + 1) = 1;		/* LITTLE ENDIAN */
    *(ptr + 2) = POLYNOMIAL_3D;
    *(ptr + 3) = POLYNOMIAL_MAGIC_DELIMITER;
    *(ptr + 4) = order;
    *(ptr + 5) = POLYNOMIAL_MAGIC_DELIMITER;
    gaiaExport32 (ptr + 6, 0, 1, endian_arch);
    ptr = xblob + 10;
    for (i = 0; i < max; i++)
      {
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, E[i], 1, endian_arch);
	  ptr += sizeof (double);
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, N[i], 1, endian_arch);
	  ptr += sizeof (double);
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, Z[i], 1, endian_arch);
	  ptr += sizeof (double);
      }
    *ptr = POLYNOMIAL_MAGIC_END;

    *blob = xblob;
    *blob_sz = xblob_sz;
    return 1;
}

static int
blob_encode_2d (double *E, double *N, unsigned char order,
		unsigned char **blob, int *blob_sz)
{
/* creating a BLOB-Polynomial object - 2D */
    int i;
    int max;
    unsigned char *xblob = NULL;
    unsigned char *ptr;
    int xblob_sz;
    int endian_arch = gaiaEndianArch ();

    *blob = NULL;
    *blob_sz = 0;

    if (order == 2)
	max = 6;
    else if (order == 3)
	max = 10;
    else
	max = 3;
    xblob_sz = 11 + (max * (2 * (sizeof (double) + 1)));

    xblob = malloc (xblob_sz);
    if (xblob == NULL)
	return 0;
    ptr = xblob;
/* encoding the BLOB */
    *ptr = POLYNOMIAL_MAGIC_START;	/* START signature */
    *(ptr + 1) = 1;		/* LITTLE ENDIAN */
    *(ptr + 2) = POLYNOMIAL_2D;
    *(ptr + 3) = POLYNOMIAL_MAGIC_DELIMITER;
    *(ptr + 4) = order;
    *(ptr + 5) = POLYNOMIAL_MAGIC_DELIMITER;
    gaiaExport32 (ptr + 6, 0, 1, endian_arch);
    ptr = xblob + 10;
    for (i = 0; i < max; i++)
      {
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, E[i], 1, endian_arch);
	  ptr += sizeof (double);
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, N[i], 1, endian_arch);
	  ptr += sizeof (double);
      }
    *ptr = POLYNOMIAL_MAGIC_END;

    *blob = xblob;
    *blob_sz = xblob_sz;
    return 1;
}

static int
blob_encode_tps (double *E, double *N, struct Control_Points *cp,
		 unsigned char **blob, int *blob_sz)
{
/* creating a BLOB-Polynomial object - 2D */
    int i;
    unsigned char *xblob = NULL;
    unsigned char *ptr;
    int xblob_sz;
    int endian_arch = gaiaEndianArch ();

    *blob = NULL;
    *blob_sz = 0;

    xblob_sz = 11 + ((cp->count + 3) * (2 * (sizeof (double) + 1)));
    xblob_sz += (cp->count * (4 * (sizeof (double) + 1)));

    xblob = malloc (xblob_sz);
    if (xblob == NULL)
	return 0;
    ptr = xblob;
/* encoding the BLOB */
    *ptr = POLYNOMIAL_MAGIC_START;	/* START signature */
    *(ptr + 1) = 1;		/* LITTLE ENDIAN */
    *(ptr + 2) = THIN_PLATE_SPLINE;
    *(ptr + 3) = POLYNOMIAL_MAGIC_DELIMITER;
    *(ptr + 4) = 1;
    *(ptr + 5) = POLYNOMIAL_MAGIC_DELIMITER;
    gaiaExport32 (ptr + 6, cp->count, 1, endian_arch);
    ptr = xblob + 10;
    for (i = 0; i < cp->count + 3; i++)
      {
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, E[i], 1, endian_arch);
	  ptr += sizeof (double);
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, N[i], 1, endian_arch);
	  ptr += sizeof (double);
      }
    for (i = 0; i < cp->count; i++)
      {
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, cp->e1[i], 1, endian_arch);
	  ptr += sizeof (double);
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, cp->n1[i], 1, endian_arch);
	  ptr += sizeof (double);
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, cp->e2[i], 1, endian_arch);
	  ptr += sizeof (double);
	  *ptr++ = POLYNOMIAL_MAGIC_DELIMITER;
	  gaiaExport64 (ptr, cp->n2[i], 1, endian_arch);
	  ptr += sizeof (double);
      }
    *ptr = POLYNOMIAL_MAGIC_END;

    *blob = xblob;
    *blob_sz = xblob_sz;
    return 1;
}

static int
blob_decode (struct cp_coeffs *coeffs, const unsigned char *blob, int blob_sz)
{
/* decoding a BLOB-Polynomial coeffs object */
    int endian;
    int endian_arch = gaiaEndianArch ();
    unsigned char type;
    unsigned char order;
    int count;
    int i;
    int max;
    int xblob_sz;
    const unsigned char *ptr = blob;

    coeffs->Etps = NULL;
    coeffs->Ntps = NULL;
    coeffs->grass_cp.count = 0;
    coeffs->grass_cp.e1 = NULL;
    coeffs->grass_cp.n1 = NULL;
    coeffs->grass_cp.e2 = NULL;
    coeffs->grass_cp.n2 = NULL;
    coeffs->grass_cp.status = NULL;
    if (blob == NULL)
	return 0;
    if (blob_sz < 11)
	return 0;

    if (*ptr != POLYNOMIAL_MAGIC_START)
	return 0;
    if (*(ptr + 1) == 1)
	endian = 1;
    else if (*(ptr + 1) == 0)
	endian = 0;
    else
	return 0;
    type = *(ptr + 2);
    order = *(ptr + 4);
    coeffs->type = type;
    coeffs->order = order;
    if (order < 1 || order > 3)
	return 0;
    if (type == THIN_PLATE_SPLINE)
	max = 0;
    else if (type == POLYNOMIAL_2D)
      {
	  if (order == 2)
	      max = 6;
	  else if (order == 3)
	      max = 10;
	  else
	      max = 3;
      }
    else if (type == POLYNOMIAL_3D)
      {
	  if (order == 2)
	      max = 10;
	  else if (order == 3)
	      max = 20;
	  else
	      max = 4;
      }
    else
	return 0;
    count = gaiaImport32 (ptr + 6, endian, endian_arch);
    if (type == POLYNOMIAL_3D)
	xblob_sz = 11 + (max * (3 * (sizeof (double) + 1)));
    else
	xblob_sz = 11 + (max * (2 * (sizeof (double) + 1)));
    if (type == THIN_PLATE_SPLINE)
      {
	  xblob_sz += ((count + 3) * (2 * (sizeof (double) + 1)));
	  xblob_sz += (count * (4 * (sizeof (double) + 1)));
      }
    if (blob_sz != xblob_sz)
	return 0;

    ptr = blob + 11;
    for (i = 0; i < max; i++)
      {
	  coeffs->E[i] = gaiaImport64 (ptr, endian, endian_arch);
	  ptr += sizeof (double) + 1;
	  coeffs->N[i] = gaiaImport64 (ptr, endian, endian_arch);
	  ptr += sizeof (double) + 1;
	  if (type == POLYNOMIAL_3D)
	    {
		coeffs->Z[i] = gaiaImport64 (ptr, endian, endian_arch);
		ptr += sizeof (double) + 1;
	    }
      }

    if (type == THIN_PLATE_SPLINE)
      {
	  /* extracting the Control Points for Grass TPS code */
	  coeffs->Etps = malloc (sizeof (double) * (3 + count));
	  coeffs->Ntps = malloc (sizeof (double) * (3 + count));
	  coeffs->grass_cp.count = count;
	  coeffs->grass_cp.e1 = malloc (sizeof (double) * count);
	  coeffs->grass_cp.n1 = malloc (sizeof (double) * count);
	  coeffs->grass_cp.e2 = malloc (sizeof (double) * count);
	  coeffs->grass_cp.n2 = malloc (sizeof (double) * count);
	  coeffs->grass_cp.status = malloc (sizeof (int) * count);
	  for (i = 0; i < count + 3; i++)
	    {
		coeffs->Etps[i] = gaiaImport64 (ptr, endian, endian_arch);
		ptr += sizeof (double) + 1;
		coeffs->Ntps[i] = gaiaImport64 (ptr, endian, endian_arch);
		ptr += sizeof (double) + 1;
	    }
	  for (i = 0; i < count; i++)
	    {
		coeffs->grass_cp.e1[i] =
		    gaiaImport64 (ptr, endian, endian_arch);
		ptr += sizeof (double) + 1;
		coeffs->grass_cp.n1[i] =
		    gaiaImport64 (ptr, endian, endian_arch);
		ptr += sizeof (double) + 1;
		coeffs->grass_cp.e2[i] =
		    gaiaImport64 (ptr, endian, endian_arch);
		ptr += sizeof (double) + 1;
		coeffs->grass_cp.n2[i] =
		    gaiaImport64 (ptr, endian, endian_arch);
		ptr += sizeof (double) + 1;
		coeffs->grass_cp.status[i] = 1;
	    }
      }
    return 1;
}


GAIACP_DECLARE int
gaiaCreatePolynomialCoeffs (GaiaControlPointsPtr cp_handle,
			    unsigned char **blob, int *blob_sz)
{
/*
 * computes the Control Points and return a BLOB-serialized 
 * Polynomial coeffs object
*/
    unsigned char *xblob;
    int xblob_sz;
    struct Control_Points cp;
    struct Control_Points_3D cp3;
    int ret = 0;
    int ret2;
    int use3d;
    int orthorot = 0;
    int order = 1;
    int order_pnts[2][3] = { {3, 6, 10}, {4, 10, 20} };

    double E12[20];
    double N12[20];
    double Z12[20];
    double E21[20];
    double N21[20];
    double Z21[20];
    double *E12_t = NULL;
    double *N12_t = NULL;
    double *E21_t = NULL;
    double *N21_t = NULL;
    struct gaia_control_points *gaia_cp =
	(struct gaia_control_points *) cp_handle;

    *blob = NULL;
    *blob_sz = 0;
    if (gaia_cp == NULL)
	return 0;

    cp.count = 0;
    cp.e1 = NULL;
    cp.e2 = NULL;
    cp.n1 = NULL;
    cp.n2 = NULL;
    cp.status = NULL;

    cp3.count = 0;
    cp3.e1 = NULL;
    cp3.e2 = NULL;
    cp3.n1 = NULL;
    cp3.n2 = NULL;
    cp3.z1 = NULL;
    cp3.z2 = NULL;
    cp3.status = NULL;

    use3d = gaia_cp->has3d;
    order = gaia_cp->order;
    if (use3d)
      {
	  /* 3D control points */
	  copy_control_points_3d (gaia_cp, &cp3);
	  ret =
	      gcp_CRS_compute_georef_equations_3d (&cp3, E12, N12, Z12, E21,
						   N21, Z21, order);
      }
    else
      {
	  /* 2D control points */
	  copy_control_points_2d (gaia_cp, &cp);
	  if (gaia_cp->tps)
	      ret =
		  gcp_I_compute_georef_equations_tps (&cp, &E12_t, &N12_t,
						      &E21_t, &N21_t);
	  else
	      ret =
		  gcp_I_compute_georef_equations (&cp, E12, N12, E21, N21,
						  order);
      }

    switch (ret)
      {
      case 0:
	  fprintf (stderr,
		   "Not enough active control points for current order, %d are required.\n",
		   (orthorot ? 3 : order_pnts[use3d != 0][order - 1]));
	  break;
      case -1:
	  fprintf (stderr,
		   "Poorly placed control points.\nCan not generate the transformation equation.\n");
	  break;
      case -2:
	  fprintf (stderr,
		   "Not enough memory to solve for transformation equation\n");
	  break;
      case -3:
	  fprintf (stderr, "Invalid order\n");
	  break;
      default:
	  break;
      }

    if (ret > 0)
      {
	  if (use3d)
	      ret2 = blob_encode_3d (E12, N12, Z12, order, &xblob, &xblob_sz);
	  else
	    {
		if (gaia_cp->tps)
		    ret2 =
			blob_encode_tps (E12_t, N12_t, &cp, &xblob, &xblob_sz);
		else
		    ret2 = blob_encode_2d (E12, N12, order, &xblob, &xblob_sz);
	    }
      }

    if (use3d)
	free_control_points_3d (&cp3);
    else
	free_control_points_2d (&cp);
    if (E12_t != NULL)
	free (E12_t);
    if (N12_t != NULL)
	free (N12_t);
    if (E21_t != NULL)
	free (E21_t);
    if (N21_t != NULL)
	free (N21_t);

    if (ret > 0 && ret2)
      {
	  *blob = xblob;
	  *blob_sz = xblob_sz;
	  return 1;
      }
    return 0;
}

GAIACP_DECLARE int
gaiaPolynomialIsValid (const unsigned char *blob, int blob_sz)
{
/* checking a BLOB-Polynomial coeffs object for validity */
    unsigned char type;
    unsigned char order;
    int i;
    int max;
    int xblob_sz;
    int endian;
    int endian_arch = gaiaEndianArch ();
    int count;
    const unsigned char *ptr = blob;
    if (blob == NULL)
	return 0;
    if (blob_sz < 11)
	return 0;

    if (*ptr != POLYNOMIAL_MAGIC_START)
	return 0;
    if (*(ptr + 1) == 1)
	endian = 1;
    else if (*(ptr + 1) == 0)
	endian = 0;
    else
	return 0;
    type = *(ptr + 2);
    order = *(ptr + 4);
    if (order > 3)
	return 0;
    if (type == THIN_PLATE_SPLINE)
	max = 0;
    else if (type == POLYNOMIAL_2D)
      {
	  if (order == 2)
	      max = 6;
	  else if (order == 3)
	      max = 10;
	  else
	      max = 3;
      }
    else if (type == POLYNOMIAL_3D)
      {
	  if (order == 2)
	      max = 10;
	  else if (order == 3)
	      max = 20;
	  else
	      max = 4;
      }
    else
	return 0;
    count = gaiaImport32 (ptr + 6, endian, endian_arch);
    if (type == POLYNOMIAL_3D)
	xblob_sz = 11 + (max * (3 * (sizeof (double) + 1)));
    else
	xblob_sz = 11 + (max * (2 * (sizeof (double) + 1)));
    if (type == THIN_PLATE_SPLINE)
      {
	  xblob_sz += ((count + 3) * (2 * (sizeof (double) + 1)));
	  xblob_sz += (count * (4 * (sizeof (double) + 1)));
      }
    if (blob_sz != xblob_sz)
	return 0;

    ptr = blob + 10;
    for (i = 0; i < max; i++)
      {
	  if (*ptr != POLYNOMIAL_MAGIC_DELIMITER)
	      return 0;
	  ptr++;
	  ptr += sizeof (double);
	  if (*ptr != POLYNOMIAL_MAGIC_DELIMITER)
	      return 0;
	  ptr++;
	  ptr += sizeof (double);
	  if (type == POLYNOMIAL_3D)
	    {
		if (*ptr != POLYNOMIAL_MAGIC_DELIMITER)
		    return 0;
		ptr++;
		ptr += sizeof (double);
	    }
      }
    if (type == THIN_PLATE_SPLINE)
      {
	  for (i = 0; i < count + 3; i++)
	    {
		if (*ptr != POLYNOMIAL_MAGIC_DELIMITER)
		    return 0;
		ptr++;
		ptr += sizeof (double);
		if (*ptr != POLYNOMIAL_MAGIC_DELIMITER)
		    return 0;
		ptr++;
		ptr += sizeof (double);
	    }
	  for (i = 0; i < count; i++)
	    {
		if (*ptr != POLYNOMIAL_MAGIC_DELIMITER)
		    return 0;
		ptr++;
		ptr += sizeof (double);
		if (*ptr != POLYNOMIAL_MAGIC_DELIMITER)
		    return 0;
		ptr++;
		ptr += sizeof (double);
		if (*ptr != POLYNOMIAL_MAGIC_DELIMITER)
		    return 0;
		ptr++;
		ptr += sizeof (double);
		if (*ptr != POLYNOMIAL_MAGIC_DELIMITER)
		    return 0;
		ptr++;
		ptr += sizeof (double);
	    }
      }
    if (*ptr != POLYNOMIAL_MAGIC_END)
	return 0;
    return 1;
}

static void
clean_tps_coeffs (struct cp_coeffs *cp)
{
/* memory cleanup - TPS coefficients */
    if (cp->Etps != NULL)
	free (cp->Etps);
    if (cp->Ntps != NULL)
	free (cp->Ntps);
}

GAIACP_DECLARE char *
gaiaPolynomialAsText (const unsigned char *blob, int blob_sz)
{
/* printing a BLOB-Polynomial coeffs object as a text string */
    char *text = NULL;
    struct cp_coeffs coeffs;
    if (!gaiaPolynomialIsValid (blob, blob_sz))
	return NULL;
    if (!blob_decode (&coeffs, blob, blob_sz))
	return NULL;

    free_control_points_2d (&(coeffs.grass_cp));
/* printing the Polynommial coeffs as text */
    if (coeffs.type == POLYNOMIAL_3D)
      {
	  if (coeffs.order == 3)
	      text =
		  sqlite3_mprintf
		  ("E{%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f}, "
		   "N{%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f}, "
		   "Z{%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f}",
		   coeffs.E[0], coeffs.E[1], coeffs.E[2], coeffs.E[3],
		   coeffs.E[4], coeffs.E[5], coeffs.E[6], coeffs.E[6],
		   coeffs.E[7], coeffs.E[8], coeffs.E[9], coeffs.E[10],
		   coeffs.E[11], coeffs.E[12], coeffs.E[13], coeffs.E[14],
		   coeffs.E[15], coeffs.E[16], coeffs.E[16], coeffs.E[17],
		   coeffs.E[18], coeffs.E[19], coeffs.N[0], coeffs.N[1],
		   coeffs.N[2], coeffs.N[3], coeffs.N[4], coeffs.N[5],
		   coeffs.N[6], coeffs.N[6], coeffs.N[7], coeffs.N[8],
		   coeffs.N[9], coeffs.N[10], coeffs.N[11], coeffs.N[12],
		   coeffs.N[13], coeffs.N[14], coeffs.N[15], coeffs.N[16],
		   coeffs.N[16], coeffs.N[17], coeffs.N[18], coeffs.N[19],
		   coeffs.Z[0], coeffs.Z[1], coeffs.Z[2], coeffs.Z[3],
		   coeffs.Z[4], coeffs.Z[5], coeffs.Z[6], coeffs.Z[6],
		   coeffs.Z[7], coeffs.Z[8], coeffs.Z[9], coeffs.Z[10],
		   coeffs.Z[11], coeffs.Z[12], coeffs.Z[13], coeffs.Z[14],
		   coeffs.Z[15], coeffs.Z[16], coeffs.Z[16], coeffs.Z[17],
		   coeffs.Z[18], coeffs.Z[19]);
	  else if (coeffs.order == 2)
	      text =
		  sqlite3_mprintf
		  ("E{%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f}, "
		   "N{%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f}, "
		   "Z{%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f}",
		   coeffs.E[0], coeffs.E[1], coeffs.E[2], coeffs.E[3],
		   coeffs.E[4], coeffs.E[5], coeffs.E[6], coeffs.E[6],
		   coeffs.E[7], coeffs.E[8], coeffs.E[9], coeffs.N[0],
		   coeffs.N[1], coeffs.N[2], coeffs.N[3], coeffs.N[4],
		   coeffs.N[5], coeffs.N[6], coeffs.N[6], coeffs.N[7],
		   coeffs.N[8], coeffs.N[9], coeffs.Z[0], coeffs.Z[1],
		   coeffs.Z[2], coeffs.Z[3], coeffs.Z[4], coeffs.Z[5],
		   coeffs.Z[6], coeffs.Z[6], coeffs.Z[7], coeffs.Z[8],
		   coeffs.Z[9]);
	  else
	      text =
		  sqlite3_mprintf
		  ("E{%1.10f,%1.10f,%1.10f,%1.10f}, N{%1.10f,%1.10f,%1.10f,%1.10f}, Z{%1.10f,%1.10f,%1.10f,%1.10f}",
		   coeffs.E[0], coeffs.E[1], coeffs.E[2], coeffs.E[3],
		   coeffs.N[0], coeffs.N[1], coeffs.N[2], coeffs.N[3],
		   coeffs.Z[0], coeffs.Z[1], coeffs.Z[2], coeffs.Z[3]);
      }
    else
      {
	  if (coeffs.order == 3)
	      text =
		  sqlite3_mprintf
		  ("E{%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f}, "
		   "N{%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f}",
		   coeffs.E[0], coeffs.E[1], coeffs.E[2], coeffs.E[3],
		   coeffs.E[4], coeffs.E[5], coeffs.E[6], coeffs.E[6],
		   coeffs.E[7], coeffs.E[8], coeffs.E[9], coeffs.N[0],
		   coeffs.N[1], coeffs.N[2], coeffs.N[3], coeffs.N[4],
		   coeffs.N[5], coeffs.N[6], coeffs.N[6], coeffs.N[7],
		   coeffs.N[8], coeffs.N[9]);
	  else if (coeffs.order == 2)
	      text =
		  sqlite3_mprintf
		  ("E{%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f}, "
		   "N{%1.10f,%1.10f,%1.10f,%1.10f,%1.10f,%1.10f}",
		   coeffs.E[0], coeffs.E[1], coeffs.E[2], coeffs.E[3],
		   coeffs.E[4], coeffs.E[5], coeffs.N[0], coeffs.N[1],
		   coeffs.N[2], coeffs.N[3], coeffs.N[4], coeffs.N[5]);
	  else
	      text =
		  sqlite3_mprintf
		  ("E{%1.10f,%1.10f,%1.10f}, N{%1.10f,%1.10f,%1.10f}",
		   coeffs.E[0], coeffs.E[1], coeffs.E[2], coeffs.N[0],
		   coeffs.N[1], coeffs.N[2]);
      }
    clean_tps_coeffs (&coeffs);
    return text;
}

static void
gaia_point_transform3D (struct cp_coeffs *coeffs, double *x, double *y,
			double *z)
{
/* Affine Transform 3D */
    double x0 = *x;
    double y0 = *y;
    double z0 = *z;
    double x1;
    double y1;
    double z1;
    if (coeffs->type == THIN_PLATE_SPLINE)
      {
	  gcp_I_georef_tps (x0, y0, &x1, &y1, coeffs->Etps, coeffs->Ntps,
			    &(coeffs->grass_cp), 1);
	  z1 = z0;
      }
    else if (coeffs->type == POLYNOMIAL_2D)
      {
	  gcp_I_georef (x0, y0, &x1, &y1, coeffs->E, coeffs->N, coeffs->order);
	  z1 = z0;
      }
    else
	gcp_CRS_georef_3d (x0, y0, z0, &x1, &y1, &z1, coeffs->E, coeffs->N,
			   coeffs->Z, coeffs->order);
    *x = x1;
    *y = y1;
    *z = z1;
}


static void
gaia_point_transform2D (struct cp_coeffs *coeffs, double *x, double *y)
{
/* Affine Transform 2D */
    double x0 = *x;
    double y0 = *y;
    double x1;
    double y1;
    if (coeffs->type == THIN_PLATE_SPLINE)
	gcp_I_georef_tps (x0, y0, &x1, &y1, coeffs->Etps, coeffs->Ntps,
			  &(coeffs->grass_cp), 1);
    else
	gcp_I_georef (x0, y0, &x1, &y1, coeffs->E, coeffs->N, coeffs->order);
    *x = x1;
    *y = y1;
}

GAIACP_DECLARE gaiaGeomCollPtr
gaiaPolynomialTransformGeometry (gaiaGeomCollPtr geom,
				 const unsigned char *blob, int blob_sz)
{
/* transforming a Geometry by applying Polynomial coefficients */
    int iv;
    int ib;
    double x;
    double y;
    double z;
    double m;
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaLinestringPtr new_line;
    gaiaPolygonPtr polyg;
    gaiaPolygonPtr new_polyg;
    gaiaGeomCollPtr new_geom;
    gaiaRingPtr i_ring;
    gaiaRingPtr o_ring;
    struct cp_coeffs coeffs;
    if (!gaiaPolynomialIsValid (blob, blob_sz))
	return NULL;
    if (!blob_decode (&coeffs, blob, blob_sz))
	return NULL;
    if (geom == NULL)
      {
	  free_control_points_2d (&(coeffs.grass_cp));
	  return NULL;
      }

/* creating the output Geometry */
    if (geom->DimensionModel == GAIA_XY_Z)
	new_geom = gaiaAllocGeomCollXYZ ();
    else if (geom->DimensionModel == GAIA_XY_M)
	new_geom = gaiaAllocGeomCollXYM ();
    else if (geom->DimensionModel == GAIA_XY_Z_M)
	new_geom = gaiaAllocGeomCollXYZM ();
    else
	new_geom = gaiaAllocGeomColl ();
    new_geom->Srid = geom->Srid;
    new_geom->DeclaredType = geom->DeclaredType;

/* cloning and transforming all individual items */
    point = geom->FirstPoint;
    while (point)
      {
	  /* copying POINTs */
	  if (geom->DimensionModel == GAIA_XY_Z)
	    {
		x = point->X;
		y = point->Y;
		z = point->Z;
		gaia_point_transform3D (&coeffs, &x, &y, &z);
		gaiaAddPointToGeomCollXYZ (new_geom, x, y, z);
	    }
	  else if (geom->DimensionModel == GAIA_XY_M)
	    {
		x = point->X;
		y = point->Y;
		m = point->M;
		gaia_point_transform2D (&coeffs, &x, &y);
		gaiaAddPointToGeomCollXYM (new_geom, x, y, m);
	    }
	  else if (geom->DimensionModel == GAIA_XY_Z_M)
	    {
		x = point->X;
		y = point->Y;
		z = point->Z;
		m = point->M;
		gaia_point_transform3D (&coeffs, &x, &y, &z);
		gaiaAddPointToGeomCollXYZM (new_geom, x, y, z, m);
	    }
	  else
	    {
		x = point->X;
		y = point->Y;
		gaia_point_transform2D (&coeffs, &x, &y);
		gaiaAddPointToGeomColl (new_geom, x, y);
	    }
	  point = point->Next;
      }

    line = geom->FirstLinestring;
    while (line)
      {
	  /* copying LINESTRINGs */
	  new_line = gaiaAddLinestringToGeomColl (new_geom, line->Points);
	  for (iv = 0; iv < line->Points; iv++)
	    {
		z = 0.0;
		m = 0.0;
		if (line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (line->Coords, iv, &x, &y, &z);
		  }
		else if (line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (line->Coords, iv, &x, &y, &m);
		  }
		else if (line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (line->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		  }
		if (new_line->DimensionModel == GAIA_XY_Z
		    || new_line->DimensionModel == GAIA_XY_Z_M)
		    gaia_point_transform3D (&coeffs, &x, &y, &z);
		else
		    gaia_point_transform2D (&coeffs, &x, &y);
		if (new_line->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (new_line->Coords, iv, x, y, z);
		  }
		else if (new_line->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (new_line->Coords, iv, x, y, m);
		  }
		else if (new_line->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (new_line->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (new_line->Coords, iv, x, y);
		  }
	    }
	  line = line->Next;
      }

    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* copying POLYGONs */
	  i_ring = polyg->Exterior;
	  new_polyg =
	      gaiaAddPolygonToGeomColl (new_geom, i_ring->Points,
					polyg->NumInteriors);
	  o_ring = new_polyg->Exterior;
	  /* copying points for the EXTERIOR RING */
	  for (iv = 0; iv < o_ring->Points; iv++)
	    {
		z = 0.0;
		m = 0.0;
		if (i_ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (i_ring->Coords, iv, &x, &y, &z);
		  }
		else if (i_ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (i_ring->Coords, iv, &x, &y, &m);
		  }
		else if (i_ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (i_ring->Coords, iv, &x, &y, &z, &m);
		  }
		else
		  {
		      gaiaGetPoint (i_ring->Coords, iv, &x, &y);
		  }
		if (o_ring->DimensionModel == GAIA_XY_Z
		    || o_ring->DimensionModel == GAIA_XY_Z_M)
		    gaia_point_transform3D (&coeffs, &x, &y, &z);
		else
		    gaia_point_transform2D (&coeffs, &x, &y);
		if (o_ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaSetPointXYZ (o_ring->Coords, iv, x, y, z);
		  }
		else if (o_ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaSetPointXYM (o_ring->Coords, iv, x, y, m);
		  }
		else if (o_ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaSetPointXYZM (o_ring->Coords, iv, x, y, z, m);
		  }
		else
		  {
		      gaiaSetPoint (o_ring->Coords, iv, x, y);
		  }
	    }
	  for (ib = 0; ib < new_polyg->NumInteriors; ib++)
	    {
		/* copying each INTERIOR RING [if any] */
		i_ring = polyg->Interiors + ib;
		o_ring = gaiaAddInteriorRing (new_polyg, ib, i_ring->Points);
		for (iv = 0; iv < o_ring->Points; iv++)
		  {
		      z = 0.0;
		      m = 0.0;
		      if (i_ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (i_ring->Coords, iv, &x, &y, &z);
			}
		      else if (i_ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (i_ring->Coords, iv, &x, &y, &m);
			}
		      else if (i_ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (i_ring->Coords, iv, &x, &y, &z,
					      &m);
			}
		      else
			{
			    gaiaGetPoint (i_ring->Coords, iv, &x, &y);
			}
		      if (o_ring->DimensionModel == GAIA_XY_Z
			  || o_ring->DimensionModel == GAIA_XY_Z_M)
			  gaia_point_transform3D (&coeffs, &x, &y, &z);
		      else
			  gaia_point_transform2D (&coeffs, &x, &y);
		      if (o_ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (o_ring->Coords, iv, x, y, z);
			}
		      else if (o_ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaSetPointXYM (o_ring->Coords, iv, x, y, m);
			}
		      else if (o_ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (o_ring->Coords, iv, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (o_ring->Coords, iv, x, y);
			}
		  }
	    }
	  polyg = polyg->Next;
      }
    free_control_points_2d (&(coeffs.grass_cp));
    clean_tps_coeffs (&coeffs);
    return new_geom;
}

GAIACP_DECLARE int
gaiaPolynomialToMatrix (const unsigned char *iblob, int iblob_sz,
			unsigned char **oblob, int *oblob_sz)
{
/*
* converting a BLOB-Polynomial coeffs object into a
* BLOB Affine Transformation matrix */
    double a = 1.0;
    double b = 0.0;
    double c = 0.0;
    double d = 0.0;
    double e = 1.0;
    double f = 0.0;
    double g = 0.0;
    double h = 0.0;
    double i = 1.0;
    double xoff = 0.0;
    double yoff = 0.0;
    double zoff = 0.0;
    struct cp_coeffs coeffs;

    *oblob = NULL;
    *oblob_sz = 0;
    if (!gaiaPolynomialIsValid (iblob, iblob_sz))
	return 0;
    if (!blob_decode (&coeffs, iblob, iblob_sz))
	return 0;

    if (coeffs.type == THIN_PLATE_SPLINE)
      {
	  free_control_points_2d (&(coeffs.grass_cp));
	  clean_tps_coeffs (&coeffs);
	  return 0;		/* Thin Plate Spline coefficients can't be converted */
      }
    if (coeffs.order != 1)
	return 0;		/* only 1st order coefficients can be converted */

/* converting */
    if (coeffs.type == POLYNOMIAL_3D)
      {
	  a = coeffs.E[1];
	  b = coeffs.E[2];
	  c = coeffs.E[3];
	  d = coeffs.N[1];
	  e = coeffs.N[2];
	  f = coeffs.N[3];
	  g = coeffs.Z[1];
	  h = coeffs.Z[2];
	  i = coeffs.Z[3];
	  xoff = coeffs.E[0];
	  yoff = coeffs.N[0];
	  zoff = coeffs.Z[0];
      }
    else
      {
	  a = coeffs.E[1];
	  b = coeffs.E[2];
	  d = coeffs.N[1];
	  e = coeffs.N[2];
	  xoff = coeffs.E[0];
	  yoff = coeffs.N[0];
      }
    if (gaia_matrix_create
	(a, b, c, d, e, f, g, h, i, xoff, yoff, zoff, oblob, oblob_sz))
	return 1;
    return 0;
}

#endif /* end including GCO */
