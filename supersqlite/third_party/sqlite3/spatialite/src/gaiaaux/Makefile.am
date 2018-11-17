
AM_CPPFLAGS = @CFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/src/headers

noinst_LTLIBRARIES = libgaiaaux.la gaiaaux.la

GAIAAUX_COMMON_SOURCES = gg_sqlaux.c gg_utf8.c

libgaiaaux_la_SOURCES = $(GAIAAUX_COMMON_SOURCES)

gaiaaux_la_SOURCES = $(GAIAAUX_COMMON_SOURCES)

gaiaaux_la_CPPFLAGS = @CFLAGS@
gaiaaux_la_CPPFLAGS += -I$(top_srcdir)/src/headers -I.
gaiaaux_la_CPPFLAGS += -DLOADABLE_EXTENSION
gaiaaux_la_LDFLAGS = -module
gaiaaux_la_LIBTOOLFLAGS = --tag=disable-static

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda
