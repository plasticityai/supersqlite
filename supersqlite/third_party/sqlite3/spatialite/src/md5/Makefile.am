
AM_CPPFLAGS = @CFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/src/headers -I.

noinst_HEADERS = md5.h
noinst_LTLIBRARIES = libmd5.la md5.la

libmd5_la_SOURCES = md5.c gaia_md5.c

md5_la_SOURCES = md5.c gaia_md5.c

md5_la_CPPFLAGS = @CFLAGS@
md5_la_CPPFLAGS += -I$(top_srcdir)/src/headers -I.
md5_la_CPPFLAGS += -DLOADABLE_EXTENSION
md5_la_LDFLAGS = -module
md5_la_LIBTOOLFLAGS = --tag=disable-static

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda
