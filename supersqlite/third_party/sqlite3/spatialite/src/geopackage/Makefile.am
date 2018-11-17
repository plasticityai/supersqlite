AM_CFLAGS = $(GCOV_FLAGS)

AM_CPPFLAGS = @CFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/src/headers

noinst_LTLIBRARIES = libgeopackage.la geopackage.la

SHARED_SOURCES = \
	gpkg_add_tile_triggers.c \
	gpkg_get_normal_row.c \
	gpkg_get_normal_zoom.c \
	gpkgGetImageType.c \
	gpkgCreateBaseTables.c \
	gpkgInsertEpsgSRID.c \
	gpkgCreateTilesTable.c \
	gpkgCreateTilesZoomLevel.c \
	gpkgAddGeometryColumn.c \
	gpkgMakePoint.c \
	gpkgBinary.c \
	gpkg_add_geometry_triggers.c \
	gpkg_add_spatial_index.c \
	gaia_cvt_gpkg.c

libgeopackage_la_SOURCES = $(SHARED_SOURCES)

libgeopackage_la_CFLAGS = -fvisibility=hidden

geopackage_la_SOURCES = $(SHARED_SOURCES)

geopackage_la_CPPFLAGS = @CFLAGS@
geopackage_la_CPPFLAGS += -I$(top_srcdir)/src/headers -I.
geopackage_la_CPPFLAGS += -DLOADABLE_EXTENSION
geopackage_la_CPPFLAGS += -fvisibility=hidden
geopackage_la_LDFLAGS = -module
geopackage_la_LIBTOOLFLAGS = --tag=disable-static

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda

EXTRA_DIST = geopackage_internal.h
