
AM_CPPFLAGS = @CFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/src/headers

noinst_LTLIBRARIES = libshapefiles.la shapefiles.la

libshapefiles_la_SOURCES = shapefiles.c validator.c

shapefiles_la_SOURCES = shapefiles.c validator.c

shapefiles_la_CPPFLAGS = @CFLAGS@
shapefiles_la_CPPFLAGS += -I$(top_srcdir)/src/headers -I.
shapefiles_la_CPPFLAGS += -DLOADABLE_EXTENSION
shapefiles_la_LDFLAGS = -module
shapefiles_la_LIBTOOLFLAGS = --tag=disable-static

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda
