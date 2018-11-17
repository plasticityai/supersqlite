
SUBDIRS = epsg_update

AM_CPPFLAGS = @CFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/src/headers

noinst_LTLIBRARIES = libsrsinit.la srsinit.la

SRSINIT_COMMON_SOURCES = srs_init.c \
	epsg_inlined_00.c \
	epsg_inlined_01.c \
	epsg_inlined_02.c \
	epsg_inlined_03.c \
	epsg_inlined_04.c \
	epsg_inlined_05.c \
	epsg_inlined_06.c \
	epsg_inlined_07.c \
	epsg_inlined_08.c \
	epsg_inlined_09.c \
	epsg_inlined_10.c \
	epsg_inlined_11.c \
	epsg_inlined_12.c \
	epsg_inlined_13.c \
	epsg_inlined_14.c \
	epsg_inlined_15.c \
	epsg_inlined_16.c \
	epsg_inlined_17.c \
	epsg_inlined_18.c \
	epsg_inlined_19.c \
	epsg_inlined_20.c \
	epsg_inlined_21.c \
	epsg_inlined_22.c \
	epsg_inlined_23.c \
	epsg_inlined_24.c \
	epsg_inlined_25.c \
	epsg_inlined_26.c \
	epsg_inlined_27.c \
	epsg_inlined_28.c \
	epsg_inlined_29.c \
	epsg_inlined_30.c \
	epsg_inlined_31.c \
	epsg_inlined_32.c \
	epsg_inlined_33.c \
	epsg_inlined_34.c \
	epsg_inlined_35.c \
	epsg_inlined_36.c \
	epsg_inlined_37.c \
	epsg_inlined_38.c \
	epsg_inlined_39.c \
	epsg_inlined_40.c \
	epsg_inlined_41.c \
	epsg_inlined_42.c \
	epsg_inlined_43.c \
	epsg_inlined_44.c \
	epsg_inlined_45.c \
	epsg_inlined_46.c \
	epsg_inlined_wgs84_00.c \
	epsg_inlined_wgs84_01.c \
	epsg_inlined_prussian.c \
	epsg_inlined_extra.c

libsrsinit_la_SOURCES = $(SRSINIT_COMMON_SOURCES)

libsrsinit_la_CFLAGS = -fvisibility=hidden

srsinit_la_SOURCES = $(SRSINIT_COMMON_SOURCES)

srsinit_la_CPPFLAGS = @CFLAGS@
srsinit_la_CPPFLAGS += -I$(top_srcdir)/src/headers -I.
srsinit_la_CPPFLAGS += -DLOADABLE_EXTENSION
srsinit_la_CPPFLAGS += -fvisibility=hidden
srsinit_la_LDFLAGS = -module
srsinit_la_LIBTOOLFLAGS = --tag=disable-static

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda


