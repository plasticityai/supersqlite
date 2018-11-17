/*

 check_endian.c -- SpatiaLite Test Case

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sqlite3.h"
#include "spatialite.h"
#include <spatialite/gaiageo.h>

#define LITTLE_ENDIAN_ENCODED	1
#define BIG_ENDIAN_ENCODED	0
#define LITTLE_ENDIAN_ARCH	1
#define BIG_ENDIAN_ARCH		0

int
main (int argc, char *argv[])
{
    unsigned char buf_in[8];
    unsigned char buf_out[8];
    short shrt_val;
    int int_val;
    sqlite3_int64 i64_val;
    float flt_val;
    double dbl_val;

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

/* testing short values [16 bit] */
    buf_in[0] = 0xdf;
    buf_in[1] = 0xfd;
    shrt_val = gaiaImport16 (buf_in, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExport16 (buf_out, shrt_val, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xfd)
      {
	  fprintf (stderr,
		   "endian INT16 (1): got 0x%02x%02x, expected 0xdffd\n",
		   buf_out[0], buf_out[1]);
	  return -1;
      }
    shrt_val = gaiaImport16 (buf_in, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExport16 (buf_out, shrt_val, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xfd)
      {
	  fprintf (stderr,
		   "endian INT16 (2): got 0x%02x%02x, expected 0xdffd\n",
		   buf_out[0], buf_out[1]);
	  return -2;
      }
    shrt_val = gaiaImport16 (buf_in, BIG_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExport16 (buf_out, shrt_val, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT16 (3): got 0x%02x%02x, expected 0xfddf\n",
		   buf_out[0], buf_out[1]);
	  return -3;
      }
    shrt_val = gaiaImport16 (buf_in, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExport16 (buf_out, shrt_val, LITTLE_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT16 (4): got 0x%02x%02x, expected 0xfddf\n",
		   buf_out[0], buf_out[1]);
	  return -4;
      }
    shrt_val = gaiaImport16 (buf_in, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExport16 (buf_out, shrt_val, BIG_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xfd)
      {
	  fprintf (stderr,
		   "endian 16 bit (5): got 0x%02x%02x, expected 0xdffd\n",
		   buf_out[0], buf_out[1]);
	  return -5;
      }
    shrt_val = gaiaImport16 (buf_in, LITTLE_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExport16 (buf_out, shrt_val, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT16 (6): got 0x%02x%02x, expected 0xfddf\n",
		   buf_out[0], buf_out[1]);
	  return -6;
      }

/* testing int values [32 bit] */
    buf_in[0] = 0xdf;
    buf_in[1] = 0xec;
    buf_in[2] = 0xce;
    buf_in[3] = 0xfd;
    int_val = gaiaImport32 (buf_in, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExport32 (buf_out, int_val, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xec || buf_out[2] != 0xce
	|| buf_out[3] != 0xfd)
      {
	  fprintf (stderr,
		   "endian INT32 (1): got 0x%02x%02x%02x%02x, expected 0xdfeccefd\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -7;
      }
    int_val = gaiaImport32 (buf_in, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExport32 (buf_out, int_val, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xec || buf_out[2] != 0xce
	|| buf_out[3] != 0xfd)
      {
	  fprintf (stderr,
		   "endian INT32 (2): got 0x%02x%02x%02x%02x, expected 0xdfeccefd\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -8;
      }
    int_val = gaiaImport32 (buf_in, BIG_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExport32 (buf_out, int_val, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0xec
	|| buf_out[3] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT32 (3): got 0x%02x%02x%02x%02x, expected 0xfdceecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -9;
      }
    int_val = gaiaImport32 (buf_in, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExport32 (buf_out, int_val, LITTLE_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0xec
	|| buf_out[3] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT32 (4): got 0x%02x%02x%02x%02x, expected 0xfdceecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -10;
      }
    int_val = gaiaImport32 (buf_in, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExport32 (buf_out, int_val, BIG_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0xec
	|| buf_out[3] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT32 (5): got 0x%02x%02x%02x%02x, expected 0xfdceecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -11;
      }
    int_val = gaiaImport32 (buf_in, LITTLE_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExport32 (buf_out, int_val, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0xec
	|| buf_out[3] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT32 (6): got 0x%02x%02x%02x%02x, expected 0xfdceecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -12;
      }

/* testing long values [64 bit] */
    buf_in[0] = 0xdf;
    buf_in[1] = 0xec;
    buf_in[2] = 0xa8;
    buf_in[3] = 0x63;
    buf_in[4] = 0x36;
    buf_in[5] = 0x8a;
    buf_in[6] = 0xce;
    buf_in[7] = 0xfd;
    i64_val = gaiaImportI64 (buf_in, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExportI64 (buf_out, i64_val, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xec || buf_out[2] != 0xa8
	|| buf_out[3] != 0x63 || buf_out[4] != 0x36 || buf_out[5] != 0x8a
	|| buf_out[6] != 0xce || buf_out[7] != 0xfd)
      {
	  fprintf (stderr,
		   "endian INT64 (1): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xdfeca863368acefd\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -13;
      }
    i64_val = gaiaImportI64 (buf_in, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExportI64 (buf_out, i64_val, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xec || buf_out[2] != 0xa8
	|| buf_out[3] != 0x63 || buf_out[4] != 0x36 || buf_out[5] != 0x8a
	|| buf_out[6] != 0xce || buf_out[7] != 0xfd)
      {
	  fprintf (stderr,
		   "endian INT64 (2): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xdfeca863368acefd\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -14;
      }
    i64_val = gaiaImportI64 (buf_in, BIG_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExportI64 (buf_out, i64_val, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0x8a
	|| buf_out[3] != 0x36 || buf_out[4] != 0x63 || buf_out[5] != 0xa8
	|| buf_out[6] != 0xec || buf_out[7] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT64 (3): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xfdce8a3663a8ecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -15;
      }
    i64_val = gaiaImportI64 (buf_in, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExportI64 (buf_out, i64_val, LITTLE_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0x8a
	|| buf_out[3] != 0x36 || buf_out[4] != 0x63 || buf_out[5] != 0xa8
	|| buf_out[6] != 0xec || buf_out[7] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT64 (4): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xfdce8a3663a8ecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -16;
      }
    i64_val = gaiaImportI64 (buf_in, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExportI64 (buf_out, i64_val, BIG_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0x8a
	|| buf_out[3] != 0x36 || buf_out[4] != 0x63 || buf_out[5] != 0xa8
	|| buf_out[6] != 0xec || buf_out[7] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT64 (5): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xfdce8a3663a8ecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -17;
      }
    i64_val = gaiaImportI64 (buf_in, LITTLE_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExportI64 (buf_out, i64_val, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0x8a
	|| buf_out[3] != 0x36 || buf_out[4] != 0x63 || buf_out[5] != 0xa8
	|| buf_out[6] != 0xec || buf_out[7] != 0xdf)
      {
	  fprintf (stderr,
		   "endian INT64 (6): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xfdce8a3663a8ecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -18;
      }

/* testing float values */
    buf_in[0] = 0xdf;
    buf_in[1] = 0xec;
    buf_in[2] = 0xce;
    buf_in[3] = 0xfd;
    flt_val = gaiaImportF32 (buf_in, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExportF32 (buf_out, flt_val, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xec || buf_out[2] != 0xce
	|| buf_out[3] != 0xfd)
      {
	  fprintf (stderr,
		   "endian FLOAT (1): got 0x%02x%02x%02x%02x, expected 0xdfeccefd\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -19;
      }
    flt_val = gaiaImportF32 (buf_in, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExportF32 (buf_out, flt_val, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xec || buf_out[2] != 0xce
	|| buf_out[3] != 0xfd)
      {
	  fprintf (stderr,
		   "endian FLOAT (2): got 0x%02x%02x%02x%02x, expected 0xdfeccefd\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -20;
      }
    flt_val = gaiaImportF32 (buf_in, BIG_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExportF32 (buf_out, flt_val, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0xec
	|| buf_out[3] != 0xdf)
      {
	  fprintf (stderr,
		   "endian FLOAT (3): got 0x%02x%02x%02x%02x, expected 0xfdceecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -21;
      }
    flt_val = gaiaImportF32 (buf_in, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExportF32 (buf_out, flt_val, LITTLE_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0xec
	|| buf_out[3] != 0xdf)
      {
	  fprintf (stderr,
		   "endian FLOAT (4): got 0x%02x%02x%02x%02x, expected 0xfdceecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -22;
      }
    flt_val = gaiaImportF32 (buf_in, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExportF32 (buf_out, flt_val, BIG_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0xec
	|| buf_out[3] != 0xdf)
      {
	  fprintf (stderr,
		   "endian FLOAT (5): got 0x%02x%02x%02x%02x, expected 0xfdceecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -23;
      }
    flt_val = gaiaImportF32 (buf_in, LITTLE_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExportF32 (buf_out, flt_val, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0xec
	|| buf_out[3] != 0xdf)
      {
	  fprintf (stderr,
		   "endian FLOAT (6): got 0x%02x%02x%02x%02x, expected 0xfdceecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3]);
	  return -24;
      }

/* testing double values */
    buf_in[0] = 0xdf;
    buf_in[1] = 0xec;
    buf_in[2] = 0xa8;
    buf_in[3] = 0x63;
    buf_in[4] = 0x36;
    buf_in[5] = 0x8a;
    buf_in[6] = 0xce;
    buf_in[7] = 0xfd;
    dbl_val = gaiaImport64 (buf_in, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExport64 (buf_out, dbl_val, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xec || buf_out[2] != 0xa8
	|| buf_out[3] != 0x63 || buf_out[4] != 0x36 || buf_out[5] != 0x8a
	|| buf_out[6] != 0xce || buf_out[7] != 0xfd)
      {
	  fprintf (stderr,
		   "endian DOUBLE (1): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xdfeca863368acefd\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -25;
      }
    dbl_val = gaiaImport64 (buf_in, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExport64 (buf_out, dbl_val, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xdf || buf_out[1] != 0xec || buf_out[2] != 0xa8
	|| buf_out[3] != 0x63 || buf_out[4] != 0x36 || buf_out[5] != 0x8a
	|| buf_out[6] != 0xce || buf_out[7] != 0xfd)
      {
	  fprintf (stderr,
		   "endian DOUBLE (2): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xdfeca863368acefd\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -26;
      }
    dbl_val = gaiaImport64 (buf_in, BIG_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExport64 (buf_out, dbl_val, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0x8a
	|| buf_out[3] != 0x36 || buf_out[4] != 0x63 || buf_out[5] != 0xa8
	|| buf_out[6] != 0xec || buf_out[7] != 0xdf)
      {
	  fprintf (stderr,
		   "endian DOUBLE (3): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xfdce8a3663a8ecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -27;
      }
    dbl_val = gaiaImport64 (buf_in, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExport64 (buf_out, dbl_val, LITTLE_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0x8a
	|| buf_out[3] != 0x36 || buf_out[4] != 0x63 || buf_out[5] != 0xa8
	|| buf_out[6] != 0xec || buf_out[7] != 0xdf)
      {
	  fprintf (stderr,
		   "endian DOUBLE (4): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xfdce8a3663a8ecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -28;
      }
    dbl_val = gaiaImport64 (buf_in, LITTLE_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    gaiaExport64 (buf_out, dbl_val, BIG_ENDIAN_ENCODED, LITTLE_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0x8a
	|| buf_out[3] != 0x36 || buf_out[4] != 0x63 || buf_out[5] != 0xa8
	|| buf_out[6] != 0xec || buf_out[7] != 0xdf)
      {
	  fprintf (stderr,
		   "endian DOUBLE (5): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xfdce8a3663a8ecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -29;
      }
    dbl_val = gaiaImport64 (buf_in, LITTLE_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    gaiaExport64 (buf_out, dbl_val, BIG_ENDIAN_ENCODED, BIG_ENDIAN_ARCH);
    if (buf_out[0] != 0xfd || buf_out[1] != 0xce || buf_out[2] != 0x8a
	|| buf_out[3] != 0x36 || buf_out[4] != 0x63 || buf_out[5] != 0xa8
	|| buf_out[6] != 0xec || buf_out[7] != 0xdf)
      {
	  fprintf (stderr,
		   "endian DOUBLE (6): got 0x%02x%02x%02x%02x%02x%02x%02x%02x, expected 0xfdce8a3663a8ecdf\n",
		   buf_out[0], buf_out[1], buf_out[2], buf_out[3], buf_out[4],
		   buf_out[5], buf_out[6], buf_out[7]);
	  return -30;
      }

    spatialite_shutdown ();
    return 0;
}
