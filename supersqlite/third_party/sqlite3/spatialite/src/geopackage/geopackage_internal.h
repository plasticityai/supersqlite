#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <spatialite/sqlite.h>

#include "spatialite/debug.h"

#include "spatialite.h"
#include <spatialite_private.h>

#define GEOPACKAGE_HEADER_LEN 8
#define GEOPACKAGE_2D_ENVELOPE_LEN 32
#define GEOPACKAGE_3D_ENVELOPE_LEN 48
#define GEOPACKAGE_4D_ENVELOPE_LEN 64
#define GEOPACKAGE_MAGIC1 0x47
#define GEOPACKAGE_MAGIC2 0x50
#define GEOPACKAGE_VERSION 0x00
#define GEOPACKAGE_WKB_LITTLEENDIAN 0x01
#define GEOPACKAGE_WKB_EXTENDEDGEOMETRY_FLAG (0x01 << 5)
#define GEOPACKAGE_WKB_EMPTY_FLAG (0x01 << 4)
#define GEOPACKAGE_2D_ENVELOPE 0x01
#define GEOPACKAGE_3D_ENVELOPE 0x02
#define GEOPACKAGE_2DM_ENVELOPE 0x03
#define GEOPACKAGE_3DM_ENVELOPE 0x04
#define GEOPACKAGE_FLAGS_2D_LITTLEENDIAN ((GEOPACKAGE_2D_ENVELOPE << 1) | GEOPACKAGE_WKB_LITTLEENDIAN)
#define GEOPACKAGE_FLAGS_2DM_LITTLEENDIAN ((GEOPACKAGE_2DM_ENVELOPE << 1) | GEOPACKAGE_WKB_LITTLEENDIAN)
#define GEOPACKAGE_FLAGS_3D_LITTLEENDIAN ((GEOPACKAGE_3D_ENVELOPE << 1) | GEOPACKAGE_WKB_LITTLEENDIAN)
#define GEOPACKAGE_FLAGS_3DM_LITTLEENDIAN ((GEOPACKAGE_3DM_ENVELOPE << 1) | GEOPACKAGE_WKB_LITTLEENDIAN)
#define GEOPACKAGE_WKB_POINT 1
#define GEOPACKAGE_WKB_POINTZ 1001
#define GEOPACKAGE_WKB_POINTM 2001
#define GEOPACKAGE_WKB_POINTZM 3001
#define GEOPACKAGE_WKB_HEADER_LEN  ((sizeof(char) + sizeof(int)))

#define GEOPACKAGE_DEFAULT_UNDEFINED_SRID 0

GEOPACKAGE_DECLARE void gpkgSetHeader2DLittleEndian (unsigned char *ptr,
						     int srid, int endian_arch);

GEOPACKAGE_DECLARE void gpkgSetHeader2DMbr (unsigned char *ptr, double min_x,
					    double min_y, double max_x,
					    double max_y, int endian_arch);
