LOCAL_PATH:= $(call my-dir)/..

include ${CLEAR_VARS}

LOCAL_SRC_FILES := \
 src/gaiaaux/gg_sqlaux.c \
  src/gaiaexif/gaia_exif.c \
  src/gaiageo/gg_advanced.c \
  src/gaiageo/gg_endian.c \
  src/gaiageo/gg_ewkt.c \
  src/gaiageo/gg_geodesic.c \
  src/gaiageo/gg_geoJSON.c \
  src/gaiageo/gg_geometries.c \
  src/gaiageo/gg_geoscvt.c \
  src/gaiageo/gg_gml.c \
  src/gaiageo/gg_kml.c \
  src/gaiageo/gg_relations.c \
  src/gaiageo/gg_transform.c \
  src/gaiageo/gg_vanuatu.c \
  src/gaiageo/gg_wkb.c \
  src/gaiageo/gg_wkt.c \
  src/spatialite/mbrcache.c \
  src/spatialite/spatialite.c \
  src/spatialite/virtualfdo.c \
  src/spatialite/virtualnetwork.c \
  src/spatialite/virtualspatialindex.c \
  src/srsinit/srs_init.c \
  src/versioninfo/version.c

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/jni/src/headers \
    $(LOCAL_PATH)/src/headers \
    $(LOCAL_PATH)/../SQLite

LOCAL_MODULE := spatialite

LOCAL_LDLIBS := -ldl -llog

LOCAL_CFLAGS := \
  -fvisibility=hidden \
  -DOMIT_GEOCALLBACKS \
  -DOMIT_GEOS \
  -DOMIT_PROJ \
  -DOMIT_EPSG \
  -DOMIT_ICONV \
  -DVERSION="\"3.0.2\""

include $(BUILD_SHARED_LIBRARY)

