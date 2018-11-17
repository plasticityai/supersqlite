#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libspatialite, 4.3.0a, a.furieri@lqt.it)
AC_LANG(C)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
AM_CONFIG_HEADER(config.h)

# enabling huge-file support (64 bit file pointers)
AH_TEMPLATE([_LARGE_FILE],
            [Must be defined in order to enable huge-file support.])
AC_DEFINE(_LARGE_FILE)
AH_TEMPLATE([_FILE_OFFSET_BITS],
            [Must be =64 in order to enable huge-file support.])
AC_DEFINE(_FILE_OFFSET_BITS, 64)
AH_TEMPLATE([_LARGEFILE_SOURCE],
            [Must be defined in order to enable huge-file support.])
AC_DEFINE(_LARGEFILE_SOURCE)

# disabling debug support
AH_TEMPLATE([NDEBUG],
            [Must be defined in order to disable debug mode.])
AC_DEFINE(NDEBUG)

# config depending options
AH_TEMPLATE([OMIT_GEOS],
            [Should be defined in order to disable GEOS support.])
AH_TEMPLATE([GEOS_ADVANCED],
            [Should be defined in order to enable GEOS_ADVANCED support.])
AH_TEMPLATE([GEOS_TRUNK],
            [Should be defined in order to enable GEOS_TRUNK experimental support.])
AH_TEMPLATE([ENABLE_LWGEOM],
            [Should be defined in order to enable LWGEOM support.])
AH_TEMPLATE([ENABLE_GCP],
            [Should be defined in order to enable GCP support.])
AH_TEMPLATE([OMIT_PROJ],
            [Should be defined in order to disable PROJ.4 support.])
AH_TEMPLATE([OMIT_ICONV],
            [Should be defined in order to disable ICONV support.])
AH_TEMPLATE([OMIT_MATHSQL],
            [Should be defined in order to disable MATHSQL support.])
AH_TEMPLATE([OMIT_EPSG],
            [Should be defined in order to disable EPSG full support.])
AH_TEMPLATE([OMIT_GEOCALLBACKS],
            [Should be defined in order to disable GEOCALLBACKS support.])
AH_TEMPLATE([OMIT_FREEXL],
            [Should be defined in order to disable FREEXL support.])
AH_TEMPLATE([ENABLE_LIBXML2],
            [Should be defined in order to enable LIBXML2 support.])
AH_TEMPLATE([ENABLE_GEOPACKAGE],
            [Should be defined in order to enable GeoPackage support.])
AH_TEMPLATE([POSTGIS_2_1],
            [Should be defined when linking liblwgeom from PostGIS 2.1 (or later).])
AH_TEMPLATE([TARGET_CPU],
            [Should contain a text-string describing the intended target CPU])

# Checks for header files.
AC_CHECK_HEADERS(stdlib.h,, [AC_MSG_ERROR([cannot find stdlib.h, bailing out])])
AC_CHECK_HEADERS(stdio.h,, [AC_MSG_ERROR([cannot find stdio.h, bailing out])])
AC_CHECK_HEADERS(string.h,, [AC_MSG_ERROR([cannot find string.h, bailing out])])
AC_CHECK_HEADERS(memory.h,, [AC_MSG_ERROR([cannot find memory.h, bailing out])])
AC_CHECK_HEADERS(math.h,, [AC_MSG_ERROR([cannot find math.h, bailing out])])
AC_CHECK_HEADERS(float.h,, [AC_MSG_ERROR([cannot find float.h, bailing out])])
AC_CHECK_HEADERS(fcntl.h,, [AC_MSG_ERROR([cannot find fcntl.h, bailing out])])
AC_CHECK_HEADERS(inttypes.h,, [AC_MSG_ERROR([cannot find inttypes.h, bailing out])])
AC_CHECK_HEADERS(stddef.h,, [AC_MSG_ERROR([cannot find stddef.h, bailing out])])
AC_CHECK_HEADERS(stdint.h,, [AC_MSG_ERROR([cannot find stdint.h, bailing out])])
AC_CHECK_HEADERS(sys/time.h,, [AC_MSG_ERROR([cannot find sys/time.h, bailing out])])
AC_CHECK_HEADERS(unistd.h,, [AC_MSG_ERROR([cannot find unistd.h, bailing out])])
AC_CHECK_HEADERS(sqlite3.h,, [AC_MSG_ERROR([cannot find sqlite3.h, bailing out])])
AC_CHECK_HEADERS(sqlite3ext.h,, [AC_MSG_ERROR([cannot find sqlite3ext.h, bailing out])])
AC_CHECK_HEADERS(zlib.h,, [AC_MSG_ERROR([cannot find zlib.h, bailing out])])


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MEMCMP
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memset sqrt strcasecmp strerror strncasecmp strstr fdatasync ftruncate getcwd gettimeofday localtime_r memmove strerror])

# Checks for installed libraries
AC_CHECK_LIB(sqlite3,sqlite3_prepare_v2,,AC_MSG_ERROR(['libsqlite3' is required but it doesn't seem to be installed on this system.]),-lm)
AC_CHECK_LIB(z,inflateInit_,,AC_MSG_ERROR(['libz' is required but it doesn't seem to be installed on this system.]),-lm)

AC_CONFIG_FILES([Makefile \
		src/Makefile \
		src/headers/Makefile \
		src/gaiaaux/Makefile \
		src/gaiaexif/Makefile \
		src/gaiageo/Makefile \
		src/gaiageo/flex/Makefile \
		src/gaiageo/lemon/Makefile \
		src/gaiageo/lemon/lemon_src/Makefile \
		src/geopackage/Makefile \
		src/spatialite/Makefile \
		src/shapefiles/Makefile \
		src/dxf/Makefile \
		src/md5/Makefile \
		src/control_points/Makefile \
		src/srsinit/Makefile \
		src/srsinit/epsg_update/Makefile \
		src/connection_cache/Makefile \
		src/connection_cache/generator/Makefile \
		src/virtualtext/Makefile \
		src/wfs/Makefile \
		test/Makefile \
		test/sql_stmt_tests/Makefile \
		test/sql_stmt_geos_tests/Makefile \
		test/sql_stmt_geosadvanced_tests/Makefile \
		test/sql_stmt_geopackage_tests/Makefile \
		test/sql_stmt_proj_tests/Makefile \
		test/sql_stmt_mathsql_tests/Makefile \
		test/sql_stmt_lwgeom_tests/Makefile \
		test/sql_stmt_libxml2_tests/Makefile \
		test/sql_stmt_security_tests/Makefile \
		test/sql_stmt_xmlsec_tests/Makefile \
		test/sql_stmt_freexl_tests/Makefile \
		test/sql_stmt_cache_tests/Makefile \
		test/sql_stmt_nocache_tests/Makefile \
		examples/Makefile \
		Doxyfile \
		spatialite.pc])

# exporting the TARGET_CPU string
splite_cpu=`$CC -dumpmachine`
AC_DEFINE_UNQUOTED([TARGET_CPU], ["$splite_cpu"])


#-----------------------------------------------------------------------
#   --enable-mathsql
#
AC_ARG_ENABLE(mathsql, [AS_HELP_STRING(
  [--enable-mathsql], [enables SQL math functions [default=yes]])],
  [], [enable_mathsql=yes])
if test x"$enable_mathsql" != "xyes"; then
  AC_DEFINE(OMIT_MATHSQL)
fi
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
#   --enable-geocallbacks
#
AC_ARG_ENABLE(geocallbacks, [AS_HELP_STRING(
  [--enable-geocallbacks], [enables geometry callbacks [default=no]])],
  [], [enable_geocallbacks=no])
if test x"$enable_geocallbacks" == "xyes"; then
  AC_CHECK_LIB(sqlite3,sqlite3_rtree_geometry_callback,,AC_MSG_ERROR([obsolete 'libsqlite3' (< v.3.7.3). please retry specifying: --disable-geocallbacks]),-lm)
else
  AC_DEFINE(OMIT_GEOCALLBACKS)
fi
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
#   --enable-proj
#
AC_ARG_ENABLE(proj, [AS_HELP_STRING(
  [--enable-proj], [enables PROJ.4 inclusion [default=yes]])],
  [], [enable_proj=yes])
if test x"$enable_proj" != "xno"; then
  AC_CHECK_HEADERS(proj_api.h,, [AC_MSG_ERROR([cannot find proj_api.h, bailing out])])
  AC_SEARCH_LIBS(pj_init_plus,proj,,AC_MSG_ERROR(['libproj' is required but it doesn't seem to be installed on this system.]),-lm)
  AC_SEARCH_LIBS(pj_init_ctx,proj,,AC_MSG_ERROR(['libproj' is required but the version on this system does not support the required features (Version 4.8.0 or later is needed).]),-lm)
else
  AC_DEFINE(OMIT_PROJ)
fi
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
#   --enable-iconv
#
AC_ARG_ENABLE(iconv, [AS_HELP_STRING(
  [--enable-iconv], [enables ICONV inclusion [default=yes]])],
  [], [enable_iconv=yes])
if test x"$enable_iconv" != "xno"; then
  AC_CHECK_HEADERS(iconv.h,, [AC_MSG_ERROR([cannot find iconv.h, bailing out])])
# on some systems "iconv()" lives in libc. On others it lives in libiconv
#  on older systems "libiconv()" lives in libiconv
  AC_SEARCH_LIBS(iconv,iconv,,
    AC_SEARCH_LIBS(libiconv,iconv,,AC_MSG_ERROR(['libiconv' is required but it doesn't seem to be installed on this system.]),))
  AC_SEARCH_LIBS(locale_charset,charset,,
    AC_SEARCH_LIBS(nl_langinfo,c,,AC_MSG_ERROR(['libcharset' is required but it doesn't seem to be installed on this system.]),))
else
  AC_DEFINE(OMIT_ICONV)
fi

#-----------------------------------------------------------------------
#   --enable-freexl
#
AC_ARG_ENABLE(freexl, [AS_HELP_STRING(
  [--enable-freexl], [enables FreeXL inclusion [default=yes]])],
  [], [enable_freexl=yes])
if test x"$enable_freexl" != "xno"; then
  AC_CHECK_HEADERS(freexl.h,, [AC_MSG_ERROR([cannot find freexl.h, bailing out])])
  AC_SEARCH_LIBS(freexl_version,freexl,,AC_MSG_ERROR([could not find 'freexl' (or obsolete 'freexl' < v.1.0.1 found).]),-lm)
else
  AC_DEFINE(OMIT_FREEXL)
fi
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
#   --enable-epsg
#
AC_ARG_ENABLE(epsg, [AS_HELP_STRING(
  [--enable-epsg], [enables full EPSG dataset support [default=yes]])],
  [], [enable_epsg=yes])
if test x"$enable_epsg" != "xyes"; then
  AC_DEFINE(OMIT_EPSG)
fi
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
#   --enable-geos
#
AC_ARG_ENABLE(geos, [AS_HELP_STRING(
  [--enable-geos], [enables GEOS inclusion [default=yes]])],
  [], [enable_geos=yes])
if test x"$enable_geos" != "xno"; then
  #-----------------------------------------------------------------------
  #   --with-geosconfig
  #
  AC_ARG_WITH([geosconfig],
        [AS_HELP_STRING([--with-geosconfig=FILE], [specify an alternative geos-config file])],
	[GEOSCONFIG="$withval"], [GEOSCONFIG=""])
  if test "x$GEOSCONFIG" = "x"; then
        # GEOSCONFIG was not specified, so search within the current path
        AC_PATH_PROG([GEOSCONFIG], [geos-config])	
        # If we couldn't find geos-config, display an error
        if test "x$GEOSCONFIG" = "x"; then
                AC_MSG_ERROR([could not find geos-config within the current path. You may need to try re-running configure with a --with-geosconfig parameter.])
        fi
  else
        # GEOSCONFIG was specified; display a message to the user
        if test "x$GEOSCONFIG" = "xyes"; then
                AC_MSG_ERROR([you must specify a parameter to --with-geosconfig, e.g. --with-geosconfig=/path/to/geos-config])
        else
                if test -f $GEOSCONFIG; then
                        AC_MSG_RESULT([Using user-specified geos-config file: $GEOSCONFIG])
                else
                        AC_MSG_ERROR([the user-specified geos-config file $GEOSCONFIG does not exist])
                fi     
        fi
  fi
  # Extract the linker and include flags
  GEOS_LDFLAGS=`$GEOSCONFIG --ldflags`
  GEOS_CFLAGS=-I`$GEOSCONFIG --includes`
  AC_SUBST([GEOS_LDFLAGS])
  AC_SUBST([GEOS_CFLAGS])	
  # Ensure that we can parse geos_c.h
  CPPFLAGS_SAVE="$CPPFLAGS"
  CPPFLAGS="$GEOS_CFLAGS"
  AC_CHECK_HEADERS([geos_c.h],, [AC_MSG_ERROR([could not find geos_c.h - you may need to specify the directory of a geos-config file using --with-geosconfig])])
  CPPFLAGS="$CPPFLAGS_SAVE"	
  # Ensure we can link against libgeos_c
  LIBS_SAVE="$LIBS"
  LIBS="$GEOS_LDFLAGS"
  AC_SEARCH_LIBS(GEOSCoveredBy,geos_c,,AC_MSG_ERROR([could not find libgeos_c (or obsolete 'libgeos_c' < v.3.3.0 found) - you may need to specify the directory of a geos-config file using --with-geosconfig]))
  LIBS="$LIBS_SAVE"
  LIBS="$LIBS $GEOS_LDFLAGS -lgeos_c"

  #-----------------------------------------------------------------------
  #   --enable-controlpoints
  #
  AC_ARG_ENABLE(gcp, [AS_HELP_STRING(
	  [--enable-gcp], [enables Control Points (from Grass GIS) [default=no]])],
	  [], [enable_gcp=no])
  if test x"$enable_gcp" != "xno"; then
	  AC_DEFINE(ENABLE_GCP)
  fi

  #-----------------------------------------------------------------------
  #   --enable-geosadvanced
  #
  AC_ARG_ENABLE(geosadvanced, [AS_HELP_STRING(
	  [--enable-geosadvanced], [enables GEOS advanced features [default=yes]])],
	  [], [enable_geosadvanced=yes])
  if test x"$enable_geosadvanced" != "xno"; then
	  AC_SEARCH_LIBS(GEOSDelaunayTriangulation,geos_c,,AC_MSG_ERROR(['libgeos_c' (>= v.3.4.0) is required but it doesn't seem to be installed on this system.]))
	  AC_DEFINE(GEOS_ADVANCED)
  fi

  #-----------------------------------------------------------------------
  #   --enable-lwgeom
  #
  AC_ARG_ENABLE(lwgeom, [AS_HELP_STRING(
	  [--enable-lwgeom], [enables LWGEOM support [default=no]])],
	  [], [enable_lwgeom=no])
  if test x"$enable_lwgeom" != "xno"; then
	  AC_CHECK_HEADERS(liblwgeom.h,, [AC_MSG_ERROR([cannot find liblwgeom.h, bailing out])])
	  _save_libs="$LIBS"
	  LIBS=$LIBS' -llwgeom -lgeos_c'
	  AC_LINK_IFELSE([AC_LANG_SOURCE([int main() { return lwgeom_make_valid(); return 0; }
	    void lwgeom_init_allocators(void) { return; } ])],
	    [AC_DEFINE(ENABLE_LWGEOM)],
            [AC_MSG_ERROR(['liblwgeom' doesn't seem to be installed on this system; unsupported lwgeom_make_valid().])])
	  LIBS="$_save_libs"
	  LIBS=$LIBS' -llwgeom -lgeos_c'
	#
	# testing for PostGIS 2.1 - lwgeom_set_handlers
	#
	  AC_SEARCH_LIBS(lwgeom_set_handlers,geos_c,AC_DEFINE(POSTGIS_2_1))
  fi
else
  AC_DEFINE(OMIT_GEOS)
fi

#-----------------------------------------------------------------------
#   --enable-libxml2
#
AC_ARG_ENABLE(libxml2, [AS_HELP_STRING(
  [--enable-libxml2], [enables libxml2 inclusion [default=yes]])],
  [], [enable_libxml2=yes])
if test x"$enable_libxml2" != "xno"; then
  PKG_CHECK_MODULES([LIBXML2], [libxml-2.0], , AC_MSG_ERROR(['libxml2' is required but it doesn't seem to be installed on this system.]))
  AC_SUBST(LIBXML2_CFLAGS)
  AC_SUBST(LIBXML2_LIBS)
  AC_DEFINE(ENABLE_LIBXML2)
fi

#-----------------------------------------------------------------------
#   --enable-geopackage
#
AC_ARG_ENABLE(geopackage, [AS_HELP_STRING(
  [--enable-geopackage], [enables GeoPackage support [default=yes]])],
  [], [enable_geopackage=yes])
if test x"$enable_geopackage" != "xno"; then
  AC_DEFINE(ENABLE_GEOPACKAGE)
fi
AM_CONDITIONAL([ENABLE_GEOPACKAGE], [test x"$enable_geopackage" == "xyes"])

#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
#    --enable-gcov
#
AC_ARG_ENABLE(gcov, AC_HELP_STRING([--enable-gcov],[turn on code coverage analysis tools]))
if test "x$enable_gcov" = "xyes"; then
    dnl Check for -fprofile-arcs and -ftest-coverage option
    CFLAGS=$CFLAGS" -fprofile-arcs -ftest-coverage -g"
    GCOV_FLAGS="-lgcov"
fi
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
#   --enable-examples
#
AC_ARG_ENABLE(examples, [AS_HELP_STRING(
       [--enable-examples], [enables building examples [default=yes]])],
       [], [enable_examples=yes])
AM_CONDITIONAL(ENABLE_EXAMPLES, [test x"$enable_examples" != "xno"])
#-----------------------------------------------------------------------

# Checking for MinGW
AM_CONDITIONAL([MINGW], [test "$target_alias" = "mingw32"])
# Checking for Mac OsX
AM_CONDITIONAL([MACOSX], [test "$target_alias" = "macosx"])
# Checking for Android
AM_CONDITIONAL([ANDROID], [test "$target_alias" = "android"])

AC_OUTPUT

#-----------------------------------------------------------------------
# printing an eventual message reporting about GPL escalation
#-----------------------------------------------------------------------
gpl_escalation=no;
if test x"$enable_lwgeom" != "xno"; then
    gpl_escalation=yes
fi
if test x"$enable_gcp" != "xno"; then
    gpl_escalation=yes
fi
if test x"$gpl_escalation" != xno; then
    echo 
    echo
    echo "=============================================================="
    echo "                        IMPORTANT NOTICE"
    echo "=============================================================="
    echo "You have selected --enable-lwgeom and/or --enable-gcp"
    echo
    echo "Both modules strictly depend on code released under the GPLv2+"
    echo "license, wich is virally infective by definition."
    echo "Consequently the copy of libspatialite you are going to build"
    echo "if configured this way *must* be released under the GPLv2+ license."
    echo
    echo "If you wish better preserving the initial MPL tri-license you"
    echo "simply have to reconfigure by specifying the following options:"
    echo "        --disable-lwgeom --disable-gcp"
    echo "=============================================================="
    
fi
