HOW-TO: UPDATE spatial_ref_sys SELF-INITIALIZING C CODE
============================================================================
When updating is required: each time a new GDAL version will be released.
============================================================================

STEP #0: building the most recent GDAL
--------
- download the latest GDAL sources
- build and install (no special settings are required)
- CAVEAT: set the LD_LIBRARY_PATH env variable so to be
  absolutely sure to target this custom built GDAL and
  not the default system installation
  

STEP #1: compiling the C GDAL utility
--------
# cd {libspatialite-source}/src/srsinit/epsg_update

Linux:
# gcc epsg_from_gdal.c -o epsg_from_gdal -lgdal

Windows [MinGW]:
# gcc -I/usr/local/include epsg_from_gdal.c -o epsg_from_gdal.exe \
      -L/usr/local/lib -lgdal



STEP #2: getting the basic EPSG file
--------
# rm epsg
# epsg_from_gdal >epsg

all right: this "epsg" output file will be used as a "seed" 
into the next step



STEP #3: compiling the C generator tool
--------
# cd {libspatialite-source}/src/srsinit/epsg_update

Linux:
# gcc auto_epsg_ext.c -o auto_epsg_ext

Windows [MinGW]:
# gcc auto_epsg_ext.c -o auto_epsg_ext.exe



STEP #4: generating the C code [inlined EPSG dataset]
--------
# rm epsg_inlined_*.c
# ./auto_epsg_ext

at the end of this step several "epsg_inlined_*.c" files will be generated



STEP #5: final setup
--------
- copy the generated file into the parent dir:
  rm ../epsg_inlined*.c
  cp epsg_inlined*.c ..
- be sure to update as required the repository (ADD/DEL)
- be sure to update as required Makefile.am
- and finally commit into the repository
