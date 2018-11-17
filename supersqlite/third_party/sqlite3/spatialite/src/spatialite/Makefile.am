
AM_CPPFLAGS = @CFLAGS@ @GEOS_CFLAGS@ @LIBXML2_CFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/src/headers

noinst_LTLIBRARIES = libsplite.la splite.la

SPATIALITE_COMMON_SOURCES = mbrcache.c \
	spatialite.c \
	spatialite_init.c \
	metatables.c \
	statistics.c \
	extra_tables.c \
	se_helpers.c \
	srid_aux.c \
	table_cloner.c \
	virtualdbf.c \
	virtualXL.c \
	virtualfdo.c \
	virtualgpkg.c \
	virtualbbox.c \
	virtualspatialindex.c \
	virtualnetwork.c \
	virtualshape.c \
	virtualxpath.c \
	virtualelementary.c

libsplite_la_SOURCES = $(SPATIALITE_COMMON_SOURCES)

libsplite_la_CFLAGS = -fvisibility=hidden

splite_la_SOURCES = $(SPATIALITE_COMMON_SOURCES)

splite_la_CPPFLAGS = @CFLAGS@ @GEOS_CFLAGS@ @LIBXML2_CFLAGS@
splite_la_CPPFLAGS += -I$(top_srcdir)/src/headers -I.
splite_la_CPPFLAGS += -DLOADABLE_EXTENSION
splite_la_CPPFLAGS += -fvisibility=hidden
splite_la_LDFLAGS = -module
splite_la_LIBTOOLFLAGS = --tag=disable-static

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda
