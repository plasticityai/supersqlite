ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = dist-zip foreign

EXTRA_DIST = makefile.vc nmake.opt Android_3.0.2.mk Android_4.3.0.mk \
	spatialite-4.3.0.mk mainpage.doxy README.coverage \
	spatialite-sql-latest.html config-msvc.h

if ENABLE_EXAMPLES
EXAMPLES = examples
endif

SUBDIRS = src test $(EXAMPLES)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = spatialite.pc

coverage-init:
	lcov --directory src --capture --initial --output-file libspatialite_cov.info

coverage::
	lcov --rc lcov_branch_coverage=1 --directory src --output-file libspatialite_cov.info --capture
	genhtml --rc lcov_branch_coverage=1 -o covresults libspatialite_cov.info

MOSTLYCLEANFILES = libspatialite_cov.info
