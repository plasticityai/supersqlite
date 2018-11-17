AM_CPPFLAGS = @CFLAGS@ @GEOS_CFLAGS@ @LIBXML2_CFLAGS@

check_PROGRAMS = check_endian \
		check_version \
		check_init \
		check_init2 \
		check_geom_aux \
		check_geometry_cols \
		check_create \
		check_bufovflw \
		check_fdo1 \
		check_fdo2 \
		check_fdo3 \
		check_fdo_bufovflw \
		check_md5 \
		check_dbf_load \
		check_shp_load \
		check_shp_load_3d \
		shape_cp1252 \
		shape_primitives \
		shape_utf8_1 \
		shape_utf8_1ex \
		shape_utf8_2 \
		shape_3d \
		check_clone_table \
		check_xls_load \
		check_math_funcs \
		check_gaia_util \
		check_gaia_utf8 \
		check_extension \
		check_recover_geom \
		check_sql_stmt \
		check_multithread \
		check_virtualtable1 \
		check_virtualtable2 \
		check_virtualtable3 \
		check_virtualtable4 \
		check_virtualtable5 \
		check_virtualtable6 \
		check_virtual_ovflw \
		check_mbrcache \
		check_spatialindex \
		check_exif \
		check_exif2 \
		check_relations_fncts \
		check_extra_relations_fncts \
		check_geoscvt_fncts \
		check_libxml2 \
		check_styling \
		check_virtualxpath \
		check_virtualbbox \
		check_wfsin \
		check_dxf \
		check_metacatalog \
		check_virtualelem \
		check_srid_fncts \
		check_control_points
		
if ENABLE_GEOPACKAGE
check_PROGRAMS += \
		check_createBaseTables \
		check_gpkgCreateTilesTable \
		check_gpkgCreateTilesTableMissingSRID \
		check_gpkgCreateTilesZoomLevel \
		check_gpkgInsertEpsgSRID check_gpkgMode \
		check_gpkgCreateFeaturesTable \
		check_gpkg_base_core_container_data_file_format_application_id \
		check_gpkg_base_core_spatial_ref_sys_data_table_def \
		check_gpkg_base_core_spatial_ref_sys_data_values_default \
		check_gpkg_base_core_contents_data_table_def \
		check_gpkg_opt_extension_mechanism_extensions_data_table_def \
		check_add_tile_triggers \
		check_add_tile_triggers_bad_table_name \
		check_get_normal_row \
		check_get_normal_row_bad_geopackage \
		check_get_normal_row_bad_geopackage2 \
		check_get_normal_zoom \
		check_get_normal_zoom_extension_load \
		check_get_normal_zoom_bad_geopackage \
		check_get_normal_zoom_bad_geopackage2 \
		check_gpkgGetImageFormat \
		check_gpkgGetImageFormat_nonimage \
		check_gpkgGetImageFormat_nonblob \
		check_gpkgGetImageFormat_png \
		check_gpkgGetImageFormat_tiff \
		check_gpkgGetImageFormat_webp \
		check_gpkgConvert \
		check_gpkgVirtual
endif

AM_CFLAGS = -I@srcdir@/../src/headers -I@srcdir@

if MINGW
AM_LDFLAGS = -L../src -lspatialite -lm -lxml2 $(GCOV_FLAGS)
else 
AM_LDFLAGS = -L../src -lpthread -lspatialite -lm -lxml2 $(GCOV_FLAGS)
endif

TESTS = $(check_PROGRAMS)

MOSTLYCLEANFILES = *.gcna *.gcno *.gcda

EXTRA_DIST = asprintf4win.h \
	fnmatch_impl4win.h \
	fnmatch4win.h \
	scandir4win.h \
	test_helpers.h \
	shapetest1.dbf \
	shapetest1.prj \
	shapetest1.qpj \
	shapetest1.shp \
	shapetest1.shx \
	test_under_valgrind.sh \
	WritingSQLTestCase.txt \
	test-legacy-2.3.1.sqlite \
	test-legacy-3.0.1.sqlite \
	test-invalid.sqlite \
	test_geos.sqlite \
	shp/new-caledonia/buildings.dbf \
	shp/new-caledonia/buildings.prj \
	shp/new-caledonia/buildings.shp \
	shp/new-caledonia/buildings.shx \
	shp/new-caledonia/points.dbf \
	shp/new-caledonia/points.prj \
	shp/new-caledonia/points.shp \
	shp/new-caledonia/points.shx \
	shp/new-caledonia/railways.dbf \
	shp/new-caledonia/railways.shp \
	shp/new-caledonia/railways.shx \
	shp/gaza/aeroway.dbf \
	shp/gaza/aeroway.prj \
	shp/gaza/aeroway.shp \
	shp/gaza/aeroway.shx \
	shp/gaza/barrier.dbf \
	shp/gaza/barrier.prj \
	shp/gaza/barrier.shp \
	shp/gaza/barrier.shx \
	shp/gaza/route.dbf \
	shp/gaza/route.prj \
	shp/gaza/route.shp \
	shp/gaza/route.shx \
	shp/taiwan/hystoric.dbf \
	shp/taiwan/hystoric.prj \
	shp/taiwan/hystoric.shp \
	shp/taiwan/hystoric.shx \
	shp/taiwan/leisure.dbf \
	shp/taiwan/leisure.prj \
	shp/taiwan/leisure.shp \
	shp/taiwan/leisure.shx \
	shp/taiwan/route.dbf \
	shp/taiwan/route.prj \
	shp/taiwan/route.shp \
	shp/taiwan/route.shx \
	shp/merano-3d/points.dbf \
	shp/merano-3d/points.prj \
	shp/merano-3d/points.shp \
	shp/merano-3d/points.shx \
	shp/merano-3d/polygons.dbf \
	shp/merano-3d/polygons.prj \
	shp/merano-3d/polygons.shp \
	shp/merano-3d/polygons.shx \
	shp/merano-3d/roads.dbf \
	shp/merano-3d/roads.prj \
	shp/merano-3d/roads.shp \
	shp/merano-3d/roads.shx \
	shp/foggia/local_councils.dbf \
	shp/foggia/local_councils.prj \
	shp/foggia/local_councils.shp \
	shp/foggia/local_councils.shx \
	testcase1.xls \
	testcase1.csv \
	books.xml books.xsd opera.xml opera.xsd \
	movies.xml movies.xsd books-bad.xml books-bad.xsd \
	inspire-data-example.xml stazioni_se.xml \
	stazioni2_se.xml raster_se.xml raster2_se.xml \
	sld_sample.xml sld_sample2.xml thunderstorm_mild.svg \
	tile000.jpeg  tile101.jpeg  tile111.jpeg \
	test.webp tile100.jpeg  tile110.jpeg \
	Apple-iPhone-4.jpg empty.png  empty.tif \
	test.wfs testDescribeFeatureType.wfs \
	getcapabilities-1.0.0.wfs \
	getcapabilities-1.1.0.wfs \
	describefeaturetype.wfs	\
	22.dxf f06.dxf l02.dxf p05.dxf \
	archaic.dxf linked.dxf hatch.dxf \
	symbol.dxf gpkg_test.sqlite gpkg_test.gpkg \
	gpkg_test_broken.gpkg gpkg_test_extrasrid.gpkg

SUBDIRS = sql_stmt_geosadvanced_tests sql_stmt_geos_tests \
	sql_stmt_libxml2_tests sql_stmt_lwgeom_tests \
	sql_stmt_mathsql_tests sql_stmt_proj_tests \
	sql_stmt_security_tests sql_stmt_tests \
	sql_stmt_xmlsec_tests sql_stmt_geopackage_tests \
	sql_stmt_freexl_tests sql_stmt_cache_tests \
	sql_stmt_nocache_tests
