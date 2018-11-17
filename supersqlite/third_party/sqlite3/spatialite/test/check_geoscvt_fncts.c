/*

 check_geoscvt_fncts.c -- SpatiaLite Test Case
 
 This does "boundary conditions" and error checks for gg_geoscvt
 functions that are hard to test with SQL.

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
#include "spatialite/gaiageo.h"

int
main (int argc, char *argv[])
{
#ifndef OMIT_GEOS		/* only if GEOS is supported */
    gaiaGeomCollPtr result;
    void *resultVoid;
    int returnValue = 0;

    /* Common setup */
    gaiaGeomCollPtr emptyGeometry = gaiaAllocGeomColl ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    /* Tests start here */

    /* null input test */
    result = gaiaFromGeos_XY ((const void *) NULL);
    if (result != NULL)
      {
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -1;
	  goto exit;
      }

    resultVoid = gaiaToGeos ((gaiaGeomCollPtr) NULL);
    if (resultVoid != NULL)
      {
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -2;
	  goto exit;
      }

    /* unknown type geometry collection */
    resultVoid = gaiaToGeos (emptyGeometry);
    if (resultVoid != NULL)
      {
	  fprintf (stderr, "bad result at %s:%i\n", __FILE__, __LINE__);
	  returnValue = -3;
	  goto exit;
      }

    /* Cleanup and exit */
  exit:
    gaiaFreeGeomColl (emptyGeometry);
    spatialite_shutdown ();
    return returnValue;

#endif /* end GEOS conditional */

    spatialite_shutdown ();
    return 0;
}
