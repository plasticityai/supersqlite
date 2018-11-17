/* 
/ code_generator
/
/ a tool autogenerating C code for SpatiaLite
/ [#include snippets for alloc_cache.c]
/
/ version 1.0, 2013 October 30
/
/ Author: Sandro Furieri a.furieri@lqt.it
/
/ Copyright (C) 2013  Alessandro Furieri
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

int
main (int argc, char *argv[])
{
/*
    this tool autogenerates few #include code snippets
    required by ../cache_alloc.c

    the MAX argument corresponds to the max number of
    concurrent connections supported by libspatialite
    at runtime.
    this value is required to be in the range between
    4 and 1024
*/
    int i;
    int max;
    FILE *out;

/* checking the MAX arg for validity */
    if (argc != 2)
      {
	  fprintf (stderr, "usage: code_generator MAX\n");
	  return -1;
      }
    max = atoi (argv[1]);
    if (max < 4)
      {
	  fprintf (stderr, "error: MAX has to be >= 4 !!!!\n");
	  return -1;
      }
    if (max > 1024)
      {
	  fprintf (stderr, "error: MAX has to be <= 1024 !!!!\n");
	  return -1;
      }

/* generating "cache_aux_1.h" */
    out = fopen ("cache_aux_1.h", "w");
    if (out == NULL)
      {
	  fprintf (stderr, "Unable to create/open \"cache_aux_1.h\"\n");
	  return -1;
      }
    fprintf (out, "#define SPATIALITE_MAX_CONNECTIONS\t%d\n\n", max);
    fprintf (out, "struct splite_connection\n{\n");
    fprintf (out, "/* connections pool */\n");
    fprintf (out, "\tvoid *conn_ptr;\n");
    fprintf (out, "\tchar *gaia_geos_error_msg;\n");
    fprintf (out, "\tchar *gaia_geos_warning_msg;\n");
    fprintf (out, "\tchar *gaia_geosaux_error_msg;\n");
    fprintf (out,
	     "} splite_connection_pool[" "SPATIALITE_MAX_CONNECTIONS] =\n{\n");
    for (i = 0; i < max; i++)
      {
	  if (i == (max - 1))
	      fprintf (out, "\t{NULL, NULL, NULL, NULL}\n");
	  else
	      fprintf (out, "\t{NULL, NULL, NULL, NULL},\n");
      }
    fprintf (out, "};\n");
    fclose (out);

/* generating "cache_aux_2.h" */
    out = fopen ("cache_aux_2.h", "w");
    if (out == NULL)
      {
	  fprintf (stderr, "Unable to create/open \"cache_aux_2.h\"\n");
	  return -1;
      }
    for (i = 0; i < max; i++)
      {
	  fprintf (out, "static void\n");
	  fprintf (out, "geos_error_%d (const char *fmt, ...)\n{\n", i);
	  fprintf (out, "\tva_list ap;\n");
	  fprintf (out, "\tva_start (ap, fmt);\n");
	  fprintf (out, "\tgeos_error_r (%d, fmt, ap);\n", i);
	  fprintf (out, "\tva_end (ap);\n}\n\n");
      }
    for (i = 0; i < max; i++)
      {
	  fprintf (out, "static void\n");
	  fprintf (out, "geos_warning_%d (const char *fmt, ...)\n{\n", i);
	  fprintf (out, "\tva_list ap;\n");
	  fprintf (out, "\tva_start (ap, fmt);\n");
	  fprintf (out, "\tgeos_warning_r (%d, fmt, ap);\n", i);
	  fprintf (out, "\tva_end (ap);\n}\n\n");
      }
    fclose (out);

/* generating "cache_aux_3.h" */
    out = fopen ("cache_aux_3.h", "w");
    if (out == NULL)
      {
	  fprintf (stderr, "Unable to create/open \"cache_aux_3.h\"\n");
	  return -1;
      }
    fprintf (out, "/* initializing all function pointers */\n");
    fprintf (out, "\tswitch (cache->pool_index)\n\t{\n");
    for (i = 0; i < max; i++)
      {
	  fprintf (out, "\tcase %d:\n", i);
	  fprintf (out, "\t\tcache->geos_warning = geos_warning_%d;\n", i);
	  fprintf (out, "\t\tcache->geos_error = geos_error_%d;\n", i);
	  fprintf (out, "\t\tbreak;\n");
      }
    fprintf (out, "\t};\n\n");
    fclose (out);

    return 0;
}
