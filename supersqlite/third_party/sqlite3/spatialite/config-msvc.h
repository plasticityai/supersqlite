/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */
/* 
/ Sandro: manually adjusted so to support the MSVC compiler 
/         using the OsGeo4W libraries
*/

/* Should be defined in order to enable GCP support. */
/* #undef ENABLE_GCP */

/* Should be defined in order to enable GeoPackage support. */
/* #undef ENABLE_GEOPACKAGE */

/* Should be defined in order to enable LIBXML2 support. */
#define ENABLE_LIBXML2 1

/* Should be defined in order to enable LWGEOM support. */
/* #undef ENABLE_LWGEOM */

/* Should be defined in order to enable GEOS_ADVANCED support. */
#define GEOS_ADVANCED 1

/* Should be defined in order to enable GEOS_TRUNK experimental support. */
/* #undef GEOS_TRUNK */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `fdatasync' function. */
#define HAVE_FDATASYNC 1

/* Define to 1 if you have the <float.h> header file. */
#define HAVE_FLOAT_H 1

/* Define to 1 if you have the <freexl.h> header file. */
#define HAVE_FREEXL_H 1

/* Define to 1 if you have the `ftruncate' function. */
#define HAVE_FTRUNCATE 1

/* Define to 1 if you have the <geos_c.h> header file. */
#define HAVE_GEOS_C_H 1

/* Define to 1 if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the <iconv.h> header file. */
#define HAVE_ICONV_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <liblwgeom.h> header file. */
/* #undef HAVE_LIBLWGEOM_H */

/* Define to 1 if you have the `sqlite3' library (-lsqlite3). */
#define HAVE_LIBSQLITE3 1

/* Define to 1 if you have the `z' library (-lz). */
#define HAVE_LIBZ 1

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if `lstat' has the bug that it succeeds when given the
   zero-length file name argument. */
/* #undef HAVE_LSTAT_EMPTY_STRING_BUG */

/* Define to 1 if you have the <math.h> header file. */
#define HAVE_MATH_H 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the <proj_api.h> header file. */
#define HAVE_PROJ_API_H 1

/* Define to 1 if you have the <sqlite3ext.h> header file. */
#define HAVE_SQLITE3EXT_H 1

/* Define to 1 if you have the <sqlite3.h> header file. */
#define HAVE_SQLITE3_H 1

/* Define to 1 if you have the `sqrt' function. */
/* #undef HAVE_SQRT */

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
/* #undef HAVE_STAT_EMPTY_STRING_BUG */

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <zlib.h> header file. */
#define HAVE_ZLIB_H 1

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
#define LSTAT_FOLLOWS_SLASHED_SYMLINK 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Must be defined in order to disable debug mode. */
#define NDEBUG 1

/* Should be defined in order to disable EPSG full support. */
/* #undef OMIT_EPSG */

/* Should be defined in order to disable FREEXL support. */
/* #undef OMIT_FREEXL */

/* Should be defined in order to disable GEOCALLBACKS support. */
#define OMIT_GEOCALLBACKS 1

/* Should be defined in order to disable GEOS support. */
/* #undef OMIT_GEOS */

/* Should be defined in order to disable ICONV support. */
/* #undef OMIT_ICONV */

/* Should be defined in order to disable MATHSQL support. */
/* #undef OMIT_MATHSQL */

/* Should be defined in order to disable PROJ.4 support. */
/* #undef OMIT_PROJ */

/* Name of package */
#define PACKAGE "libspatialite"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "a.furieri@lqt.it"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libspatialite"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libspatialite 4.3.0-RC1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libspatialite"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.3.0"

/* Should be defined when linking liblwgeom from PostGIS 2.1 (or later). */
/* #undefine POSTGIS_2_1 */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Should contain a text-string describing the intended target CPU */
#define TARGET_CPU "Win32"

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Version number of package */
#define VERSION "4.3.0-RC1"

/* Must be =64 in order to enable huge-file support. */
#define _FILE_OFFSET_BITS 64

/* Must be defined in order to enable huge-file support. */
#define _LARGEFILE_SOURCE 1

/* Must be defined in order to enable huge-file support. */
#define _LARGE_FILE 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */
