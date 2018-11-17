/*

 check_math_funcs.c -- SpatiaLite Test Case

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

#include "sqlite3.h"
#include "spatialite.h"

static const double double_eps = 0.00000001;

int
main (int argc, char *argv[])
{
    double result;
    sqlite3_int64 result64;

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    result = math_round (3.4);
    if (abs (result - 3) > double_eps)
      {
	  fprintf (stderr, "bad math_round() result for 3.4: %f\n", result);
	  return -1;
      }

    result = math_round (3.6);
    if (abs (result - 4) > double_eps)
      {
	  fprintf (stderr, "bad math_round() result for 3.6: %f\n", result);
	  return -2;
      }

    result = math_round (-3.4);
    if (abs (result + 3) > double_eps)
      {
	  fprintf (stderr, "bad math_round() result for -3.4: %f\n", result);
	  return -3;
      }

    result = math_round (-3.6);
    if (abs (result + 4) > double_eps)
      {
	  fprintf (stderr, "bad math_round() result for -3.6: %f\n", result);
	  return -4;
      }

    result64 = math_llabs ((sqlite3_int64) 26);
    if (result64 != 26)
      {
	  fprintf (stderr, "bad math_llabs() result for 26");
	  return -5;
      }

    result64 = math_llabs ((sqlite3_int64) - 26);
    if (result64 != 26)
      {
	  fprintf (stderr, "bad math_llabs() result for -26");
	  return -6;
      }

    return 0;
}
