include $(CLEAR_VARS)
# ./configure  --enable-lwgeom=no --enable-gcp --enable-examples=no --build=x86_64-pc-linux-gnu --host=arm-linux-eabi
# ./configure  --build=x86_64-pc-linux-gnu --host=arm-linux-eabi --without-grib --prefix=$PROJECT/external/gdal
# 20150607.libspatialite-4.3.0-dev
LOCAL_MODULE    := spatialite

# SQLite flags copied from ASOP
common_sqlite_flags := \
 -DHAVE_USLEEP=1 \
 -DSQLITE_DEFAULT_JOURNAL_SIZE_LIMIT=1048576 \
 -DSQLITE_THREADSAFE=1 \
 -DNDEBUG=1 \
 -DSQLITE_ENABLE_MEMORY_MANAGEMENT=1 \
 -DSQLITE_DEFAULT_AUTOVACUUM=1 \
 -DSQLITE_TEMP_STORE=3 \
 -DSQLITE_ENABLE_FTS3 \
 -DSQLITE_ENABLE_FTS3_BACKWARDS \
 -DSQLITE_ENABLE_RTREE=1 \
 -DSQLITE_DEFAULT_FILE_FORMAT=4


# spatialite flags
# comment out TARGET_CPU in config.h - will be replaced with TARGET_ARCH_ABI
spatialite_flags := \
 -DOMIT_FREEXL \
 -DTARGET_CPU=\"$(TARGET_ARCH_ABI)\" \
 -Dfdatasync=fsync \
 -DSQLITE_ENABLE_RTREE=1 \
 -DENABLE_GCP=1 \
 -DENABLE_GEOPACKAGE=1 \
 -DENABLE_LIBXML2=1 \
 -DSQLITE_OMIT_BUILTIN_TEST=1 

LOCAL_CFLAGS    := \
 $(common_sqlite_flags) \
 $(spatialite_flags)

# LOCAL_LDLIBS is always ignored for static libraries
# LOCAL_LDLIBS    := -llog -lz
# LOADABLE_EXTENSION must NOT be defined
# For Spatialite with VirtualShapes,VirtualXL support iconv is needed
# 2014-07-26 - adapted based on ls -1 result in all directories
# Note: not included are: /src/gaiageo/
# --> Ewkt.c:2071:24: error: expected ')' before 'yymsp'
# - Ewkt.c,geoJSON.c,Gml.c,Kml.c,vanuatuWkt.c
# - lex.Ewkt.c,lex.geoJSON.c,lex.Gml.c,lex.Kml.c,lex.VanuatuWkt.c
# 20150607 - ENABLE_GCP=1: 'GPL v2.0 or any subsequent version'
# 'srsinit/epsg_update' is not included, since it is not needed in the library [tools to create the epsg_inlined_*.c files]
LOCAL_C_INCLUDES := \
 $(SQLITE_PATH) \
 $(SPATIALITE_PATH) \
 $(SPATIALITE_PATH)/src/headers \
 $(ICONV_PATH)/include \
 $(ICONV_PATH)/libcharset/include \
 $(GEOS_PATH)/include \
 $(GEOS_PATH)/capi \
 $(PROJ4_PATH)/src \
 $(LZMA_PATH)/src/liblzma/api \
 $(XML2_PATH)/include
LOCAL_SRC_FILES := \
 $(SPATIALITE_PATH)/src/connection_cache/alloc_cache.c \
 $(SPATIALITE_PATH)/src/connection_cache/generator/code_generator.c \
 $(SPATIALITE_PATH)/src/control_points/gaia_control_points.c \
 $(SPATIALITE_PATH)/src/control_points/grass_crs3d.c \
 $(SPATIALITE_PATH)/src/control_points/grass_georef.c \
 $(SPATIALITE_PATH)/src/control_points/grass_georef_tps.c \
 $(SPATIALITE_PATH)/src/dxf/dxf_load_distinct.c \
 $(SPATIALITE_PATH)/src/dxf/dxf_loader.c \
 $(SPATIALITE_PATH)/src/dxf/dxf_load_mixed.c \
 $(SPATIALITE_PATH)/src/dxf/dxf_parser.c \
 $(SPATIALITE_PATH)/src/dxf/dxf_writer.c \
 $(SPATIALITE_PATH)/src/gaiaaux/gg_sqlaux.c \
 $(SPATIALITE_PATH)/src/gaiaaux/gg_utf8.c \
 $(SPATIALITE_PATH)/src/gaiaexif/gaia_exif.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_advanced.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_endian.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_ewkt.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_extras.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_geodesic.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_geoJSON.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_geometries.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_geoscvt.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_gml.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_kml.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_lwgeom.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_matrix.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_relations.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_relations_ext.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_shape.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_transform.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_vanuatu.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_voronoj.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_wkb.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_wkt.c \
 $(SPATIALITE_PATH)/src/gaiageo/gg_xml.c \
 $(SPATIALITE_PATH)/src/geopackage/gaia_cvt_gpkg.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkgAddGeometryColumn.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkg_add_geometry_triggers.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkg_add_spatial_index.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkg_add_tile_triggers.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkgBinary.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkgCreateBaseTables.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkgCreateTilesTable.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkgCreateTilesZoomLevel.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkgGetImageType.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkg_get_normal_row.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkg_get_normal_zoom.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkgInsertEpsgSRID.c \
 $(SPATIALITE_PATH)/src/geopackage/gpkgMakePoint.c \
 $(SPATIALITE_PATH)/src/md5/gaia_md5.c \
 $(SPATIALITE_PATH)/src/md5/md5.c \
 $(SPATIALITE_PATH)/src/shapefiles/shapefiles.c \
 $(SPATIALITE_PATH)/src/shapefiles/validator.c \
 $(SPATIALITE_PATH)/src/spatialite/extra_tables.c \
 $(SPATIALITE_PATH)/src/spatialite/mbrcache.c \
 $(SPATIALITE_PATH)/src/spatialite/metatables.c \
 $(SPATIALITE_PATH)/src/spatialite/se_helpers.c \
 $(SPATIALITE_PATH)/src/spatialite/spatialite.c \
 $(SPATIALITE_PATH)/src/spatialite/spatialite_init.c \
 $(SPATIALITE_PATH)/src/spatialite/srid_aux.c \
 $(SPATIALITE_PATH)/src/spatialite/statistics.c \
 $(SPATIALITE_PATH)/src/spatialite/table_cloner.c \
 $(SPATIALITE_PATH)/src/spatialite/virtualbbox.c \
 $(SPATIALITE_PATH)/src/spatialite/virtualdbf.c \
 $(SPATIALITE_PATH)/src/spatialite/virtualelementary.c \
 $(SPATIALITE_PATH)/src/spatialite/virtualfdo.c \
 $(SPATIALITE_PATH)/src/spatialite/virtualgpkg.c \
 $(SPATIALITE_PATH)/src/spatialite/virtualnetwork.c \
 $(SPATIALITE_PATH)/src/spatialite/virtualshape.c \
 $(SPATIALITE_PATH)/src/spatialite/virtualspatialindex.c \
 $(SPATIALITE_PATH)/src/spatialite/virtualXL.c \
 $(SPATIALITE_PATH)/src/spatialite/virtualxpath.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_00.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_01.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_02.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_03.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_04.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_05.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_06.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_07.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_08.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_09.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_10.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_11.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_12.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_13.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_14.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_15.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_16.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_17.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_18.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_19.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_20.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_21.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_22.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_23.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_24.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_25.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_26.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_27.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_28.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_29.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_30.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_31.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_32.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_33.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_34.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_35.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_36.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_37.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_38.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_39.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_40.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_41.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_42.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_43.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_44.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_45.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_46.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_extra.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_prussian.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_wgs84_00.c \
 $(SPATIALITE_PATH)/src/srsinit/epsg_inlined_wgs84_01.c \
 $(SPATIALITE_PATH)/src/srsinit/srs_init.c \
 $(SPATIALITE_PATH)/src/versioninfo/version.c \
 $(SPATIALITE_PATH)/src/virtualtext/virtualtext.c \
 $(SPATIALITE_PATH)/src/wfs/wfs_in.c
LOCAL_STATIC_LIBRARIES := iconv proj geos libxml2
include $(BUILD_STATIC_LIBRARY)
