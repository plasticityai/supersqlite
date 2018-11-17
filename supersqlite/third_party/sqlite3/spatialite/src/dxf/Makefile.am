
AM_CPPFLAGS = @CFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/src/headers -I.

noinst_HEADERS = dxf_private.h
noinst_LTLIBRARIES = libdxf.la dxf.la

DXF_COMMON_SOURCES = dxf_parser.c dxf_loader.c \
	dxf_load_distinct.c dxf_load_mixed.c \
	dxf_writer.c

libdxf_la_SOURCES = $(DXF_COMMON_SOURCES)

dxf_la_SOURCES = $(DXF_COMMON_SOURCES)

dxf_la_CPPFLAGS = @CFLAGS@
dxf_la_CPPFLAGS += -I$(top_srcdir)/src/headers -I.
dxf_la_CPPFLAGS += -DLOADABLE_EXTENSION
dxf_la_LDFLAGS = -module
dxf_la_LIBTOOLFLAGS = --tag=disable-static

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda
