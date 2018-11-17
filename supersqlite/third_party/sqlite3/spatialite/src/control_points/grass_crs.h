/*

 grass_crs.h -- private header file required by source file derived from
                Grass GIS
    
 version 4.3, 2015 May 5

 Author: Sandro Furieri a.furieri@lqt.it

 ------------------------------------------------------------------------------
 DISCLAIMER: this source is strictly derived from Grass GIS code and simply
             contains very trivial adjustments required in order to compile
			 smoothly on libspatialite.
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

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define GCP_PRIVATE __attribute__ ((visibility("hidden")))
#endif

/* STRUCTURE FOR USE INTERNALLY WITH THESE FUNCTIONS.  THESE FUNCTIONS EXPECT
   SQUARE MATRICES SO ONLY ONE VARIABLE IS GIVEN (N) FOR THE MATRIX SIZE */

struct MATRIX
{
    int n;			/* SIZE OF THIS MATRIX (N x N) */
    double *v;
};

/* CALCULATE OFFSET INTO ARRAY BASED ON R/C */

#define M(row,col) m->v[(((row)-1)*(m->n))+(col)-1]

#define MSUCCESS     1		/* SUCCESS */
#define MNPTERR      0		/* NOT ENOUGH POINTS */
#define MUNSOLVABLE -1		/* NOT SOLVABLE */
#define MMEMERR     -2		/* NOT ENOUGH MEMORY */
#define MPARMERR    -3		/* PARAMETER ERROR */
#define MINTERR     -4		/* INTERNAL ERROR */

#define MAXORDER 3		/* HIGHEST SUPPORTED ORDER OF TRANSFORMATION */

#define GRASS_EPSILON 1.0e-15

struct Control_Points
{
    int count;
    double *e1;
    double *n1;
    double *e2;
    double *n2;
    int *status;
};

struct Control_Points_3D
{
    int count;
    double *e1;
    double *n1;
    double *z1;
    double *e2;
    double *n2;
    double *z2;
    int *status;
};

GCP_PRIVATE int
gcp_CRS_compute_georef_equations_3d (struct Control_Points_3D *,
				     double *, double *, double *,
				     double *, double *, double *, int);

GCP_PRIVATE int
gcp_I_compute_georef_equations (struct Control_Points *cp, double E12[],
				double N12[], double E21[], double N21[],
				int order);

GCP_PRIVATE int
gcp_I_compute_georef_equations_tps (struct Control_Points *cp,
				    double **E12tps, double **N12tps,
				    double **E21tps, double **N21tps);

GCP_PRIVATE int
gcp_I_georef (double e1, double n1, double *e, double *n, double E[],
	      double N[], int order);

GCP_PRIVATE int
gcp_CRS_georef_3d (double e1, double n1, double z1, double *e,
		   double *n, double *z, double E[], double N[],
		   double Z[], int order);

GCP_PRIVATE int
gcp_I_georef_tps (double e1, double n1, double *e, double *n, double *E,
		  double *N, struct Control_Points *cp, int fwd);
