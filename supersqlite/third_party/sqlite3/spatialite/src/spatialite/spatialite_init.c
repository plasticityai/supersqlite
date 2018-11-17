/*

 spatialite_init.c -- SQLite3 spatial extension

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
Pepijn Van Eeckhoudt <pepijnvaneeckhoudt@luciad.com>
(implementing Android support)

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

/*
 
CREDITS:

this module has been partly funded by:
Regione Toscana - Settore Sistema Informativo Territoriale ed Ambientale
(exposing liblwgeom APIs as SpatiaLite own SQL functions) 

*/

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>

#include <spatialite/spatialite.h>
#include <spatialite.h>
#include <spatialite_private.h>

#ifndef OMIT_GEOS		/* including GEOS */
#include <geos_c.h>
#endif

#ifndef LOADABLE_EXTENSION	/* ordinary library, not loadable-extension */

static int
init_spatialite_extension (sqlite3 * db, char **pzErrMsg, const void *pApi)
{
    if (pApi == NULL)
	pApi = NULL;		/* suppressing stupid compiler warnings */

/* setting the POSIX locale for numeric */
    setlocale (LC_NUMERIC, "POSIX");
    *pzErrMsg = NULL;

    register_spatialite_sql_functions (db, NULL);

    init_spatialite_virtualtables (db, NULL);

/* setting a timeout handler */
    sqlite3_busy_timeout (db, 5000);

    return 0;
}

SPATIALITE_DECLARE void
spatialite_init (int verbose)
{
/* used when SQLite initializes as an ordinary lib 

   OBSOLETE - strongly discouraged !!!!!
   always using spatialite_init_ex() as a replacement
   is warmly reccomended
*/
    spatialite_initialize ();

#ifndef OMIT_GEOS		/* initializing GEOS */
    initGEOS (geos_warning, geos_error);
#endif /* end GEOS  */

#ifdef POSTGIS_2_1		/* initializing liblwgeom from PostGIS 2.1.x (or later) */
    splite_lwgeom_init ();
#endif /* end POSTGIS_2_1 */

    sqlite3_auto_extension ((void (*)(void)) init_spatialite_extension);
    spatialite_splash_screen (verbose);
}

SPATIALITE_DECLARE void
spatialite_cleanup ()
{
/* OBSOLETE - strongly discouraged !!!!!
   always using spatialite_cleanup_ex() as a replacement
   is warmly reccomended
*/

#ifndef OMIT_GEOS
    finishGEOS ();
#endif

#ifdef ENABLE_LWGEOM
    gaiaResetLwGeomMsg ();
#endif

    sqlite3_reset_auto_extension ();
}
#endif
