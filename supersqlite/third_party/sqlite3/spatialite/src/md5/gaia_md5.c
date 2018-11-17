/*

 gaia_md5.c -- a thin wrapper built around the original MD5
               implementation from Alexander Peslyak 
               (released on the Public Domain)

 version 4.3, 2015 June 29

 Author: Sandro Furieri a.furieri@lqt.it

 -----------------------------------------------------------------------------
 
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#include <spatialite/sqlite.h>
#include <spatialite/gaiaaux.h>

#include "md5.h"

GAIAAUX_DECLARE void *
gaiaCreateMD5Checksum (void)
{
/* Creates and initializes an MD5 checksum object */
    MD5_CTX *md5 = malloc (sizeof (MD5_CTX));
    splite_MD5_Init (md5);
    return md5;
}

GAIAAUX_DECLARE void
gaiaFreeMD5Checksum (void *p_md5)
{
/* memory cleanup - destroying an MD5 checksum object */
    unsigned char result[32];
    MD5_CTX *md5 = (MD5_CTX *) p_md5;
    if (md5 == NULL)
	return;
    splite_MD5_Final (result, md5);
    free (md5);
}

GAIAAUX_DECLARE void
gaiaUpdateMD5Checksum (void *p_md5, const unsigned char *blob, int blob_len)
{
/* progressively updating the MD5 checksum */
    MD5_CTX *md5 = (MD5_CTX *) p_md5;
    if (md5 == NULL || blob == NULL)
	return;
    splite_MD5_Update (md5, (void *)blob, blob_len);
}

GAIAAUX_DECLARE char *
gaiaFinalizeMD5Checksum (void *p_md5)
{
/* return the current MD5 checksum and resets the MD5 object */
    int i;
    char *hex;
    char hex_byte[8];
    unsigned char result[32];
    MD5_CTX *md5 = (MD5_CTX *) p_md5;
    if (md5 == NULL)
	return NULL;
    splite_MD5_Final (result, md5);
    splite_MD5_Init (md5);
/* formatting the MD5 checksum as hex-text */
    hex = malloc (33);
    *hex = '\0';
    for (i = 0; i < 16; i++)
      {
	  sprintf (hex_byte, "%02x", result[i]);
	  strcat (hex, hex_byte);
      }
    return hex;
}
