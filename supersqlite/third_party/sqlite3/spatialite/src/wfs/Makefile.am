
AM_CPPFLAGS= @CFLAGS@ @LIBXML2_CFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/src/headers

noinst_LTLIBRARIES = libwfs.la wfs.la

libwfs_la_SOURCES = wfs_in.c

wfs_la_SOURCES = wfs_in.c

wfs_la_CPPFLAGS = @CFLAGS@ @LIBXML2_CFLAGS@
wfs_la_CPPFLAGS += -I$(top_srcdir)/src/headers -I.
wfs_la_CPPFLAGS += -DLOADABLE_EXTENSION
wfs_la_LDFLAGS = -module
wfs_la_LIBTOOLFLAGS = --tag=disable-static

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda
