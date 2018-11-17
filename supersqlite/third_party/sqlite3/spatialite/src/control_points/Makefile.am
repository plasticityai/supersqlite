
AM_CPPFLAGS = @CFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/src/headers -I.

EXTRA_DIST = COPYING

noinst_HEADERS = grass_crs.h
noinst_LTLIBRARIES = libcontrol_points.la control_points.la

libcontrol_points_la_SOURCES = gaia_control_points.c grass_crs3d.c \
	grass_georef.c grass_georef_tps.c

control_points_la_SOURCES = gaia_control_points.c grass_crs3d.c \
	grass_georef.c grass_georef_tps.c

control_points_la_CPPFLAGS = @CFLAGS@
control_points_la_CPPFLAGS += -I$(top_srcdir)/src/headers -I.
control_points_la_CPPFLAGS += -DLOADABLE_EXTENSION
control_points_la_LDFLAGS = -module
control_points_la_LIBTOOLFLAGS = --tag=disable-static

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda
