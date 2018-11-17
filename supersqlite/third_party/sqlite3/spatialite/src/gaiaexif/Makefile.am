
AM_CPPFLAGS = @CFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/src/headers

noinst_LTLIBRARIES = libgaiaexif.la gaiaexif.la

libgaiaexif_la_SOURCES = gaia_exif.c

gaiaexif_la_SOURCES = gaia_exif.c

gaiaexif_la_CPPFLAGS = @CFLAGS@
gaiaexif_la_CPPFLAGS += -I$(top_srcdir)/src/headers -I.
gaiaexif_la_CPPFLAGS += -DLOADABLE_EXTENSION
gaiaexif_la_LDFLAGS = -module
gaiaexif_la_LIBTOOLFLAGS = --tag=disable-static

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda
