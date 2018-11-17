HOW-TO: UPDATE connection_cache.c auto-generated C CODE
============================================================================
When updating is required: 
every time you wish to change the max number of concurrent connections.
the basic default setting supported by any standard distribution always
is 64; anyway you could eventually change this to any other valure
in the range 4 - 1024
============================================================================

STEP #1: compiling the C generator tool
--------
# cd {libspatialite-source}/src/connection_cache/generator

Linux:
# gcc code_generator.c -o code_generator

Windows [MinGW]:
# gcc code_generator.c -o code_generator.exe



STEP #3: generating the C code (#include snippets)
--------
# rm cache_aux_*.h
# ./code_generator MAX

at the end of this step several "cache_aux_*.h" files will be generated



STEP #4: final setup
--------
- copy the generated file into the parent dir:
  rm ../cache_aux_*.h
  cp cache_aux_*.h ..
