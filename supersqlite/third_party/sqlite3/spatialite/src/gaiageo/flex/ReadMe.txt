The Vanuatu WKT lexer:
======================
1) in order to make any change to the lexer
you have to edit first the definitions file
vanuatuLexer.l

2) then run:
flex -L vanuatuLexer.l

3) a source file named "lex.VanuatuWkt.c" will be
generated during the above step.

3.1) IMPORTANT NOTICE: carefully check the generated *.c 
code; you should manually replace any occurrence of:
fprintf(stderr, .....
with:
spatialite_e(.....

3.2) now you should copy this file into the parent dir:
cp lex.VanuatuWkt.c ..




The EWKT lexer:
================
1) in order to make any change to the lexer
you have to edit first the definitions file
ewktLexer.l

2) then run:
flex -L ewktLexer.l

3) a source file named "lex.Ewkt.c" will be
generated during the above step.

3.1) IMPORTANT NOTICE: carefully check the generated *.c 
code; you should manually replace any occurrence of:
fprintf(stderr, .....
with:
spatialite_e(.....

3.2) now you should copy this file into the parent dir:
cp lex.Ewkt.c ..




The GeoJSON lexer:
================
1) in order to make any change to the lexer
you have to edit first the definitions file
geoJsonLexer.l

2) then run:
flex -L geoJsonLexer.l

3) a source file named "lex.GeoJson.c" will be
generated during the above step.

3.1) IMPORTANT NOTICE: carefully check the generated *.c 
code; you should manually replace any occurrence of:
fprintf(stderr, .....
with:
spatialite_e(.....

3.2) now you should copy this file into the parent dir:
cp lex.geoJsonLexer.c ..




The KML lexer:
================
1) in order to make any change to the lexer
you have to edit first the definitions file
kmlLexer.l

2) then run:
flex -L kmlLexer.l

3) a source file named "lex.Kml.c" will be
generated during the above step.

3.1) IMPORTANT NOTICE: carefully check the generated *.c 
code; you should manually replace any occurrence of:
fprintf(stderr, .....
with:
spatialite_e(.....

3.2) now you should copy this file into the parent dir:
cp lex.Kml.c ..




The GML lexer:
================
1) in order to make any change to the lexer
you have to edit first the definitions file
gmlLexer.l

2) then run:
flex -L gmlLexer.l

3) a source file named "lex.Gml.c" will be
generated during the above step.

3.1) IMPORTANT NOTICE: carefully check the generated *.c 
code; you should manually replace any occurrence of:
fprintf(stderr, .....
with:
spatialite_e(.....

3.2) now you should copy this file into the parent dir:
cp lex.Gml.c ..
