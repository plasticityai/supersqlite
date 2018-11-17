****************************************************************************
***
*** WARNING     WARNING     WARNING     WARNING     WARNING
***
*** this procedure was discontinued since 4.2.1 and is now obsolete
*** the currecnt procedure is documented in README.txt
***
****************************************************************************


HOW-TO: UPDATE spatial_ref_sys SELF-INITIALIZING C CODE
============================================================================
When updating is required: each time a new GDAL version will be released.
============================================================================

STEP #1: getting the basic EPSG files
--------
- download the latest GDAL sources
- build and install 
  be sure to set: ./configure --with-python=yes

# cd {gdal-sources}/data 
# rm epsg
# epsg_tr.py --config OVERRIDE_PROJ_DATUM_WITH_TOWGS84 FALSE \
#    -proj4 -skip -list gcs.csv > epsg
# epsg_tr.py --config OVERRIDE_PROJ_DATUM_WITH_TOWGS84 FALSE \
#    -proj4 -skip -list pcs.csv >> epsg
# rm wkt
# epsg_tr.py -wkt -skip -list gcs.csv > wkt
# epsg_tr.py -wkt -skip -list pcs.csv >> wkt

all right: these "epsg" and "wkt" files will be used as "seeds" into the
next step:
- copy both "epsg" and "wkt" files into: 
  {libspatialite-source}/src/srcinit/epsg_update



STEP #2: compiling the C generator tool
--------
# cd {libspatialite-source}/src/srsinit/epsg_update

Linux:
# gcc auto_epsg.c -o auto_epsg

Windows [MinGW]:
# gcc auto_epsg.c -o auto_epsg.exe



STEP #3: generating the C code [inlined EPSG dataset]
--------
# rm epsg_inlined_*.c
# ./auto_epsg

at the end of this step several "epsg_inlined_*.c" files will be generated



STEP #4: final setup
--------
- copy the generated file into the parent dir:
  rm ../epsg_inlined*.c
  cp epsg_inlined*.c ..
- be sure to update as required the repository (ADD/DEL)
- be sure to update as required Makefile.am
- and finally commit into the repository
