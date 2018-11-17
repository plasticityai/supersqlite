What is the LEMON parser ?
==========================
Please see: http://www.hwaci.com/sw/lemon/

in a very few words: the LEMON parser is
internally used by the SQLite's development
team, so using it for SpatiaLite as well
seems to be fully appropriate.


How to get the LEMON parser executable:
=======================================
you can get the latest LEMON sources consulting
the above URL: anyway, for the sake of simplicity,
you can directly found a copy of LEMON sources
into the "lemon_src" directory.

in order to get the LEMON executable you have
to build it by yourself: that's not at all difficult,
simply type:

cd lemon_src
gcc lemon.c -o lemon




The Vanuatu WKT parser:
=======================
1) in order to make any change to the  WKT parser
you have to edit first the definitions file
vanuatuWkt.y

2) then run:
lemon_src/lemon -l vanuatuWkt.y

3) during the above step the following files will be
generated:
vanuatuWkt.c [the C code implementing the parser]
vanuatuWkt.h [C header file]
vanuatuWkt.out [check file - useful for debugging]

3.1] IMPORTANT NOTICE: carefully check the generated *.c 
code; you should manually replace any occurrence of:
fprintf(stderr, .....
with:
spatialite_e(.....

3.2] and finally you must copy both generated files
into the parent dir:
cp vanuatuWkt.h ..
cp vanuatuWkt.c ..


The EWKT parser:
================
1) in order to make any change to the  EWKT parser
you have to edit first the definitions file
Ewkt.y

2) then run:
lemon_src/lemon -l Ewkt.y

3) during the above step the following files will be
generated:
Ewkt.c [the C code implementing the parser]
Ewkt.h [C header file]
Ewkt.out [check file - useful for debugging]

3.1] IMPORTANT NOTICE: carefully check the generated *.c 
code; you should manually replace any occurrence of:
fprintf(stderr, .....
with:
spatialite_e(.....

3.2] and finally you must copy both generated files
into the parent dir:
cp Ewkt.h ..
cp Ewkt.c ..


The GeoJSON parser:
================
1) in order to make any change to the  GeoJSON parser
you have to edit first the definitions file
geoJSON.y

2) then run:
lemon_src/lemon -l geoJSON.y

3) during the above step the following files will be
generated:
geoJSON.c [the C code implementing the parser]
geoJSON.h [C header file]
geoJSON.out [check file - useful for debugging]

3.1] IMPORTANT NOTICE: carefully check the generated *.c 
code; you should manually replace any occurrence of:
fprintf(stderr, .....
with:
spatialite_e(.....

3.2] and finally you must copy both generated files
into the parent dir:
cp geoJSON.h ..
cp geoJSON.c ..


The KML parser:
================
1) in order to make any change to the  KML parser
you have to edit first the definitions file
Kml.y

2) then run:
lemon_src/lemon -l Kml.y

3) during the above step the following files will be
generated:
Kml.c [the C code implementing the parser]
Kml.h [C header file]
Kml.out [check file - useful for debugging]

3.1] IMPORTANT NOTICE: carefully check the generated *.c 
code; you should manually replace any occurrence of:
fprintf(stderr, .....
with:
spatialite_e(.....

3.2] and finally you must copy both generated files
into the -/lemon/include dir:
cp Kml.h ..
cp Kml.c ..


The GML parser:
================
1) in order to make any change to the  GML parser
you have to edit first the definitions file
Gml.y

2) then run:
lemon_src/lemon -l Gml.y

3) during the above step the following files will be
generated:
Gml.c [the C code implementing the parser]
Gml.h [C header file]
Gml.out [check file - useful for debugging]

3.1] IMPORTANT NOTICE: carefully check the generated *.c 
code; you should manually replace any occurrence of:
fprintf(stderr, .....
with:
spatialite_e(.....

3.2] and finally you must copy both generated files
into the -/lemon/include dir:
cp Gml.h ..
cp Gml.c ..

