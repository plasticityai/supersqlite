/*

    GeoPackage extensions for SpatiaLite / SQLite
 
Version: MPL 1.1/GPL 2.0/LGPL 2.1

The contents of this file are subject to the Mozilla Public License Version
1.1 (the "License"); you may not use this file except in compliance with
the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/
 
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is GeoPackage Extensions

The Initial Developer of the Original Code is Brad Hards (bradh@frogmouth.net)
 
Portions created by the Initial Developer are Copyright (C) 2012-2015
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

#include "spatialite/geopackage.h"
#include <spatialite/gaiaexif.h>
#include "config.h"
#include "geopackage_internal.h"

#ifdef ENABLE_GEOPACKAGE

GEOPACKAGE_PRIVATE void
fnct_gpkgGetImageType (sqlite3_context * context, int argc UNUSED,
		       sqlite3_value ** argv)
{
/* SQL function:
/ gpkgGetImageType(blob)
/
/ Gets the image type (as a string) of the blob argument, or "unknown" if
/ the image type is not one of the PNG, JPEG, TIFF or WebP format types that
/ are supported in GeoPackage. This function raises exception on error (e.g.
/ wrong argument type).
/
/ The result will be one of:
/ - "png" - for PNG
/ - "jpeg" - for JPEG
/ - "tiff" - for TIFF
/ - "x-webp" - for WebP
/ These are the mime type for the image format (without the "image/" prefix)
/
*/
    unsigned char *p_blob = NULL;
    int n_bytes = 0;
    int blobType;

    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_error (context,
				"gpkgGetImageType() error: argument 1 [image blob] is not of the BLOB type",
				-1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);

    blobType = gaiaGuessBlobType (p_blob, n_bytes);
    switch (blobType)
      {
      case GAIA_TIFF_BLOB:
	  sqlite3_result_text (context, "tiff", strlen ("tiff"),
			       SQLITE_TRANSIENT);
	  break;
      case GAIA_PNG_BLOB:
	  sqlite3_result_text (context, "png", strlen ("png"),
			       SQLITE_TRANSIENT);
	  break;
      case GAIA_JPEG_BLOB:
	  sqlite3_result_text (context, "jpeg", strlen ("jpeg"),
			       SQLITE_TRANSIENT);
	  break;
      case GAIA_WEBP_BLOB:
	  sqlite3_result_text (context, "x-webp", strlen ("x-webp"),
			       SQLITE_TRANSIENT);
	  break;
      default:
	  sqlite3_result_text (context, "unknown", strlen ("unknown"),
			       SQLITE_TRANSIENT);
	  break;
      }
}
#endif
