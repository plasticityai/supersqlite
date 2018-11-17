/* 
 geoJSON.y -- GeoJSON parser - LEMON config
  
 version 2.4, 2011 May 16

 Author: Sandro Furieri a.furieri@lqt.it

 ------------------------------------------------------------------------------
 
 Version: MPL 1.1/GPL 2.0/LGPL 2.1
 
 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/
 
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is the SpatiaLite library

The Initial Developer of the Original Code is Alessandro Furieri
 
Portions created by the Initial Developer are Copyright (C) 2011
the Initial Developer. All Rights Reserved.

Alternatively, the contents of this file may be used under the terms of
either the GNU General Public License Version 2 or later (the "GPL"), or
the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
in which case the provisions of the GPL or the LGPL are applicable instead
of those above. If you wish to allow use of your version of this file only
under the terms of either the GPL or the LGPL, and not to allow others to
use your version of this file under the terms of the MPL, indicate your
decision by deleting the provisions above and replace them with the notice
and other provisions required by the GPL or the LGPL. If you do not delete
the provisions above, a recipient may use your version of this file under
the terms of any one of the MPL, the GPL or the LGPL.
 
*/

// Tokens are void pointers (so we can cast them to whatever we want)
%token_type {void *}

// Output to stderr when stack overflows
%stack_overflow {
     spatialite_e( "Giving up.  Parser stack overflow\n");
}

// Increase this number if necessary
%stack_size 1000000

// Header files to be included in geoJSON.c
%include {
}

// Set the return value of gaiaParseGeoJSON in the following pointer:
%extra_argument { struct geoJson_data *p_data }

// Invalid syntax (ie. no rules matched)
%syntax_error {
/* 
** when the LEMON parser encounters an error
** then this global variable is set 
*/
	p_data->geoJson_parse_error = 1;
	p_data->result = NULL;
}
 
 /* This is to terminate with a new line */
 main ::= in.
 in ::= .
 in ::= in state GEOJSON_NEWLINE.
 
 state ::= program.
 
 /* 
 * program is the start node. All strings matched by this CFG must be one of 
 * geo_text (text describing a geometry)
 */
 
program ::= geo_text.

// geometries (2D and 3D):
geo_text ::= point(P). { p_data->result = P; }			// P is a geometry collection containing a point
geo_text ::= pointz(P). { p_data->result = P; }			// P is a geometry collection containing a point
geo_text ::= linestring(L). { p_data->result = L; }		// L is a geometry collection containing a linestring
geo_text ::= linestringz(L). { p_data->result = L; }		// L is a geometry collection containing a linestring
geo_text ::= polygon(P). { p_data->result = P; }		// P is a geometry collection containing a polygon
geo_text ::= polygonz(P). { p_data->result = P; }		// P is a geometry collection containing a polygon
geo_text ::= multipoint(M). { p_data->result = M; }		// M is a geometry collection containing a multipoint
geo_text ::= multipointz(M). { p_data->result = M; }		// M is a geometry collection containing a multipoint
geo_text ::= multilinestring(M). { p_data->result = M; }	// M is a geometry collection containing a multilinestring
geo_text ::= multilinestringz(M). { p_data->result = M; }	// M is a geometry collection containing a multilinestring
geo_text ::= multipolygon(M). { p_data->result = M; }		// M is a geometry collection containing a multipolygon
geo_text ::= multipolygonz(M). { p_data->result = M; }		// M is a geometry collection containing a multipolygon
geo_text ::= geocoll(H). { p_data->result = H; }		// H is a geometry collection created from user input
geo_text ::= geocollz(H). { p_data->result = H; }		// H is a geometry collection created from user input


// Syntax for a "point" object:
// The functions called build a geometry collection from a gaiaPointPtr
point(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON point_coordxy(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPoint( p_data, (gaiaPointPtr)Q); } // Point (2D) [simple]
point(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON point_coordxy(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPoint( p_data, (gaiaPointPtr)Q); } // Point (2D) [with BBOX] 
point(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON point_coordxy(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPointSrid( p_data, (gaiaPointPtr)Q, (int *)S); } // Point (2D) [with short SRS]
point(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON point_coordxy(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPointSrid( p_data, (gaiaPointPtr)Q, (int *)S); } // Point (2D) [with long SRS]
point(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON point_coordxy(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPointSrid( p_data, (gaiaPointPtr)Q, (int *)S); } // Point (2D) [with BBOX & short SRS]
point(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON point_coordxy(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPointSrid( p_data, (gaiaPointPtr)Q, (int *)S); } // Point (2D) [with BBOX & long SRS]
pointz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON point_coordxyz(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPoint( p_data, (gaiaPointPtr)Q); } // Point (3D) [simple]
pointz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON point_coordxyz(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPoint( p_data, (gaiaPointPtr)Q); } // Point (3D) [with BBOX] 
pointz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON point_coordxyz(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPointSrid( p_data, (gaiaPointPtr)Q, (int *)S); } // Point (3D) [with short SRS] 
pointz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON point_coordxyz(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPointSrid( p_data, (gaiaPointPtr)Q, (int *)S); } // Point (3D) [with long SRS]
point(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON point_coordxyz(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPointSrid( p_data, (gaiaPointPtr)Q, (int *)S); } // Point (3D) [with BBOX & short SRS]
point(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON point_coordxyz(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPointSrid( p_data, (gaiaPointPtr)Q, (int *)S); } // Point (3D) [with BBOX & long SRS] 

// GeoJSON Bounding Box
bbox ::= coord GEOJSON_COMMA coord GEOJSON_COMMA coord GEOJSON_COMMA coord.
	
// GeoJSON short-format SRS
short_crs(A) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_NAME GEOJSON_COMMA
	GEOJSON_PROPS GEOJSON_COLON GEOJSON_OPEN_BRACE GEOJSON_NAME GEOJSON_COLON
	short_srid(B) GEOJSON_CLOSE_BRACE GEOJSON_CLOSE_BRACE.
	{ A = B; }	
	
// GeoJSON long-format SRS
long_crs(A) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_NAME GEOJSON_COMMA
	GEOJSON_PROPS GEOJSON_COLON GEOJSON_OPEN_BRACE GEOJSON_NAME GEOJSON_COLON
	long_srid(B) GEOJSON_CLOSE_BRACE GEOJSON_CLOSE_BRACE.
	{ A = B; }

// Point coordinates in different dimensions.
// Create the point by calling the proper function in SpatiaLite :
point_coordxy(P) ::= GEOJSON_OPEN_BRACKET coord(X) GEOJSON_COMMA coord(Y) GEOJSON_CLOSE_BRACKET. 
	{ P = (void *) geoJSON_point_xy( p_data, (double *)X, (double *)Y); }
point_coordxyz(P) ::= GEOJSON_OPEN_BRACKET coord(X) GEOJSON_COMMA coord(Y) GEOJSON_COMMA coord(Z) GEOJSON_CLOSE_BRACKET. 
	{ P = (void *) geoJSON_point_xyz( p_data, (double *)X, (double *)Y, (double *)Z); }

// All coordinates are assumed to be doubles (guaranteed by the flex tokenizer).
coord(A) ::= GEOJSON_NUM(B). { A = B; }

// short-format SRID.
short_srid(A) ::= GEOJSON_SHORT_SRID(B). { A = B; }

// long-format SRID.
long_srid(A) ::= GEOJSON_LONG_SRID(B). { A = B; }
 
 
// Rules to match an infinite number of points:
// Also links the generated gaiaPointPtrs together
extra_pointsxy(A) ::=  . { A = NULL; }
extra_pointsxy(A) ::= GEOJSON_COMMA point_coordxy(P) extra_pointsxy(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_pointsxyz(A) ::=  .  { A = NULL; }
extra_pointsxyz(A) ::= GEOJSON_COMMA point_coordxyz(P) extra_pointsxyz(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

	
// Syntax for a "linestring" object:
// The functions called build a geometry collection from a gaiaLinestringPtr
linestring(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON linestring_text(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); } // LineString (2D) [simple]
linestring(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON linestring_text(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); } // LineString (2D) [with BBOX] 
linestring(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON linestring_text(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestringSrid( p_data, (gaiaLinestringPtr)X, (int *)S); } // LineString (2D) [with short SRS]
linestring(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON linestring_text(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestringSrid( p_data, (gaiaLinestringPtr)X, (int *)S); } // LineString (2D) [with long SRS]
linestring(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON linestring_text(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestringSrid( p_data, (gaiaLinestringPtr)X, (int *)S); } // LineString (2D) [with BBOX & short SRS]
linestring(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON linestring_text(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestringSrid( p_data, (gaiaLinestringPtr)X, (int *)S); } // LineString (2D) [with BBOX & long SRS]
linestringz(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON linestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); } // LineString (3D) [simple]
linestringz(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON linestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); } // LineString (3D) [with BBOX] 
linestringz(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON linestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestringSrid( p_data, (gaiaLinestringPtr)X, (int *)S); } // LineString (3D) [with short SRS] 
linestringz(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON linestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestringSrid( p_data, (gaiaLinestringPtr)X, (int *)S); } // LineString (3D) [with long SRS]
linestringz(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON linestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestringSrid( p_data, (gaiaLinestringPtr)X, (int *)S); } // LineString (3D) [with BBOX & short SRS]
linestringz(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON linestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestringSrid( p_data, (gaiaLinestringPtr)X, (int *)S); } // LineString (3D) [with BBOX & long SRS] 

// A valid linestring must have at least two vertices:
// The functions called build a gaiaLinestring from a linked list of points
linestring_text(L) ::= GEOJSON_OPEN_BRACKET point_coordxy(P) GEOJSON_COMMA point_coordxy(Q) extra_pointsxy(R) GEOJSON_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)Q)->Next = (gaiaPointPtr)R; 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q;
	   L = (void *) geoJSON_linestring_xy( p_data, (gaiaPointPtr)P);
	}

linestring_textz(L) ::= GEOJSON_OPEN_BRACKET point_coordxyz(P) GEOJSON_COMMA point_coordxyz(Q) extra_pointsxyz(R) GEOJSON_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)Q)->Next = (gaiaPointPtr)R; 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q;
	   L = (void *) geoJSON_linestring_xyz( p_data, (gaiaPointPtr)P);
	}


// Syntax for a "polygon" object:
// The functions called build a geometry collection from a gaiaPolygonPtr
polygon(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON polygon_text(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); } // Polygon (2D) [simple]
polygon(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON polygon_text(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); } // Polygon (2D) [with BBOX] 
polygon(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON polygon_text(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygonSrid( p_data, (gaiaPolygonPtr)X, (int *)S); } // Polygon (2D) [with short SRS]
polygon(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON polygon_text(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygonSrid( p_data, (gaiaPolygonPtr)X, (int *)S); } // Polygon (2D) [with long SRS]
polygon(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON polygon_text(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygonSrid( p_data, (gaiaPolygonPtr)X, (int *)S); } // Polygon (2D) [with BBOX & short SRS]
polygon(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON polygon_text(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygonSrid( p_data, (gaiaPolygonPtr)X, (int *)S); } // Polygon (2D) [with BBOX & long SRS]
polygonz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON polygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); } // Polygon (3D) [simple]
polygonz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON polygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); } // Polygon (3D) [with BBOX] 
polygonz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON polygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygonSrid( p_data, (gaiaPolygonPtr)X, (int *)S); } // Polygon (3D) [with short SRS] 
polygonz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON polygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygonSrid( p_data, (gaiaPolygonPtr)X, (int *)S); } // Polygon (3D) [with long SRS]
polygonz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON polygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygonSrid( p_data, (gaiaPolygonPtr)X, (int *)S); } // Polygon (3D) [with BBOX & short SRS]
polygonz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON polygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygonSrid( p_data, (gaiaPolygonPtr)X, (int *)S); } // Polygon (3D) [with BBOX & long SRS] 

// A valid polygon must have at least one ring:
// The functions called build a gaiaPolygonPtr from a linked list of gaiaRingPtrs
polygon_text(P) ::= GEOJSON_OPEN_BRACKET ring(R) extra_rings(E) GEOJSON_CLOSE_BRACKET.
	{ 
		((gaiaRingPtr)R)->Next = (gaiaRingPtr)E;
		P = (void *) geoJSON_polygon_xy(p_data, (gaiaRingPtr)R);
	}

polygon_textz(P) ::= GEOJSON_OPEN_BRACKET ringz(R) extra_ringsz(E) GEOJSON_CLOSE_BRACKET.
	{  
		((gaiaRingPtr)R)->Next = (gaiaRingPtr)E;
		P = (void *) geoJSON_polygon_xyz(p_data, (gaiaRingPtr)R);
	}

// A valid ring must have at least 4 points
// The functions called build a gaiaRingPtr from a linked list of gaiaPointPtrs
ring(R) ::= GEOJSON_OPEN_BRACKET point_coordxy(A) GEOJSON_COMMA point_coordxy(B) 
	GEOJSON_COMMA point_coordxy(C) GEOJSON_COMMA point_coordxy(D) extra_pointsxy(E) GEOJSON_CLOSE_BRACKET.
	{
		((gaiaPointPtr)A)->Next = (gaiaPointPtr)B; 
		((gaiaPointPtr)B)->Next = (gaiaPointPtr)C;
		((gaiaPointPtr)C)->Next = (gaiaPointPtr)D; 
		((gaiaPointPtr)D)->Next = (gaiaPointPtr)E;
		R = (void *) geoJSON_ring_xy(p_data, (gaiaPointPtr)A);
	}

// To match more than one 2D ring:
extra_rings(R) ::=  . { R = NULL; }
extra_rings(R) ::= GEOJSON_COMMA ring(S) extra_rings(T).
	{
		((gaiaRingPtr)S)->Next = (gaiaRingPtr)T;
		R = S;
	}
	
ringz(R) ::= GEOJSON_OPEN_BRACKET point_coordxyz(A) GEOJSON_COMMA point_coordxyz(B) 
	GEOJSON_COMMA point_coordxyz(C) GEOJSON_COMMA point_coordxyz(D) extra_pointsxyz(E) GEOJSON_CLOSE_BRACKET.
	{
		((gaiaPointPtr)A)->Next = (gaiaPointPtr)B; 
		((gaiaPointPtr)B)->Next = (gaiaPointPtr)C;
		((gaiaPointPtr)C)->Next = (gaiaPointPtr)D; 
		((gaiaPointPtr)D)->Next = (gaiaPointPtr)E;
		R = (void *) geoJSON_ring_xyz(p_data, (gaiaPointPtr)A);
	}

// To match more than one 3D ring:
extra_ringsz(R) ::=  . { R = NULL; }
extra_ringsz(R) ::= GEOJSON_COMMA ringz(S) extra_ringsz(T).
	{
		((gaiaRingPtr)S)->Next = (gaiaRingPtr)T;
		R = S;
	}

// Syntax for a "multipoint" object:
// X in the following lines is a geometry collection containing a multipoint
multipoint(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON multipoint_text(X) GEOJSON_CLOSE_BRACE. 
	{ M = X; } // MultiPoint (2D) [simple]
multipoint(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipoint_text(X) GEOJSON_CLOSE_BRACE.
	{ M = X; } // MultiPoint (2D) [with BBOX]  
multipoint(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipoint_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPoint (2D) [with short SRS]
multipoint(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipoint_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPoint (2D) [with long SRS]
multipoint(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multipoint_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPoint (2D) [with BBOX & short SRS]
multipoint(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multipoint_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPoint (2D) [with BBOX & long SRS]
multipointz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON multipoint_textz(X) GEOJSON_CLOSE_BRACE. 
	{ M = X; } // MultiPoint (3D) [simple]
multipointz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipoint_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = X; } // MultiPoint (3D) [with BBOX] 
multipointz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipoint_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPoint (3D) [with short SRS]
multipointz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipoint_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPoint (3D) [with long SRS]
multipointz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multipoint_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPoint (3D) [with BBOX & short SRS]
multipointz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOINT GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multipoint_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPoint (3D) [with BBOX & long SRS]

// Multipoints can contain any number of points (but at least one):
// The functions called build a geometry collection containing a multipoint
multipoint_text(M) ::= GEOJSON_OPEN_BRACKET point_coordxy(P) extra_pointsxy(Q) GEOJSON_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) geoJSON_multipoint_xy(p_data, (gaiaPointPtr)P);
	}
multipoint_textz(M) ::= GEOJSON_OPEN_BRACKET point_coordxyz(P) extra_pointsxyz(Q) GEOJSON_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) geoJSON_multipoint_xyz(p_data, (gaiaPointPtr)P);
	}


// Syntax for a "multilinestring" object:
// X in the following lines refers to a geometry collection containing a multilinestring
multilinestring(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON multilinestring_text(X) GEOJSON_CLOSE_BRACE. 
	{ M = X; } // MultiLineString (2D) [simple]
multilinestring(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multilinestring_text(X) GEOJSON_CLOSE_BRACE.
	{ M = X; } // MultiLineString (2D) [with BBOX]   
multilinestring(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multilinestring_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiLineString (2D) [with short SRS]
multilinestring(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multilinestring_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiLineString (2D) [with long SRS]
multilinestring(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multilinestring_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiLineString (2D) [with BBOX & short SRS]
multilinestring(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multilinestring_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiLineString (2D) [with BBOX & long SRS]
multilinestringz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON multilinestring_textz(X) GEOJSON_CLOSE_BRACE. 
	{ M = X; } // MultiLineString (3D) [simple]
multilinestringz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multilinestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = X; } // MultiLineString (3D) [with BBOX] 
multilinestringz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multilinestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiLineString (3D) [with short SRS]
multilinestringz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multilinestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiLineString (3D) [with long SRS]
multilinestringz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multilinestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiLineString (3D) [with BBOX & short SRS]
multilinestringz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTILINESTRING GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multilinestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiLineString (3D) [with BBOX & long SRS]

// Multilinestrings can contain any number of linestrings (but at least one):
// The functions called build a geometry collection containing a multilinestring
multilinestring_text(M) ::= GEOJSON_OPEN_BRACKET linestring_text(L) multilinestring_text2(X) GEOJSON_CLOSE_BRACKET.
	{ 
	   ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)X; 
	   M = (void *) geoJSON_multilinestring_xy( p_data, (gaiaLinestringPtr)L);
	}

// Extra linestrings
multilinestring_text2(X) ::=  . { X = NULL; }
multilinestring_text2(X) ::= GEOJSON_COMMA linestring_text(L) multilinestring_text2(Y).
	{ ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)Y;  X = L; }

multilinestring_textz(M) ::= GEOJSON_OPEN_BRACKET linestring_textz(L) multilinestring_textz2(X) GEOJSON_CLOSE_BRACKET.
	{ 
	   ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)X; 
	   M = (void *) geoJSON_multilinestring_xyz(p_data, (gaiaLinestringPtr)L);
	}

multilinestring_textz2(X) ::=  . { X = NULL; }
multilinestring_textz2(X) ::= GEOJSON_COMMA linestring_textz(L) multilinestring_textz2(Y).
	{ ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)Y;  X = L; }


// Syntax for a "multipolygon" object
// X in the following lines refers to a geometry collection containing a multipolygon
multipolygon(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON multipolygon_text(X) GEOJSON_CLOSE_BRACE. 
	{ M = X; } // MultiPolygon (2D) [simple]
multipolygon(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipolygon_text(X) GEOJSON_CLOSE_BRACE.
	{ M = X; } // MultiPolygon (2D) [with BBOX] 
multipolygon(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipolygon_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPolygon (2D) [with short SRS]
multipolygon(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipolygon_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPolygon (2D) [with long SRS]
multipolygon(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multipolygon_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPolygon (2D) [with BBOX & short SRS]
multipolygon(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multipolygon_text(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPolygon (2D) [with BBOX & long SRS]
multipolygonz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON multipolygon_textz(X) GEOJSON_CLOSE_BRACE. 
	{ M = X; } // MultiPolygon (3D) [simple]
multipolygonz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipolygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = X; } // MultiPolygon (3D) [with BBOX] 
multipolygonz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipolygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPolygon (3D) [with short SRS]
multipolygonz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_COORDS GEOJSON_COLON multipolygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPolygon (3D) [with long SRS]
multipolygonz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multipolygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPolygon (3D) [with BBOX & short SRS]
multipolygonz(M) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_MULTIPOLYGON GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_COORDS 
	GEOJSON_COLON multipolygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ M = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // MultiPolygon (3D) [with BBOX & long SRS]

// Multipolygons can contain any number of polygons (but at least one):
// The functions called build a geometry collection containing a multipolygon
multipolygon_text(M) ::= GEOJSON_OPEN_BRACKET polygon_text(P) multipolygon_text2(Q) GEOJSON_CLOSE_BRACKET.
	{ 
	   ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)Q; 
	   M = (void *) geoJSON_multipolygon_xy(p_data, (gaiaPolygonPtr)P);
	}

// Extra polygons
multipolygon_text2(Q) ::=  . { Q = NULL; }
multipolygon_text2(A) ::= GEOJSON_COMMA polygon_text(P) multipolygon_text2(B).
	{ ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)B;  A = P; }

multipolygon_textz(M) ::= GEOJSON_OPEN_BRACKET polygon_textz(P) multipolygon_textz2(Q) GEOJSON_CLOSE_BRACKET.
	{ 
	   ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)Q; 
	   M = (void *) geoJSON_multipolygon_xyz(p_data, (gaiaPolygonPtr)P);
	}

multipolygon_textz2(Q) ::=  . { Q = NULL; }
multipolygon_textz2(A) ::= GEOJSON_COMMA polygon_textz(P) multipolygon_textz2(B).
	{ ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)B;  A = P; }


// Syntax for a "geometrycollection" object:
// X in the following lines refers to a geometry collection generated based on user input
geocoll(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_GEOMS GEOJSON_COLON geocoll_text(X) GEOJSON_CLOSE_BRACE. 
	{ G = X; } // GeomColl (2D) [simple]
geocoll(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_GEOMS GEOJSON_COLON geocoll_text(X) GEOJSON_CLOSE_BRACE.
	{ G = X; } // GeomColl (2D) [with BBOX] 
geocoll(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_GEOMS GEOJSON_COLON geocoll_text(X) GEOJSON_CLOSE_BRACE.
	{ G = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // GeomColl (2D) [with short SRS]
geocoll(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_GEOMS GEOJSON_COLON geocoll_text(X) GEOJSON_CLOSE_BRACE.
	{ G = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // GeomColl (2D) [with long SRS]
geocoll(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS 
	GEOJSON_COLON geocoll_text(X) GEOJSON_CLOSE_BRACE.
	{ G = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // GeomColl (2D) [with BBOX & short SRS]
geocoll(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS 
	GEOJSON_COLON geocoll_text(X) GEOJSON_CLOSE_BRACE.
	{ G = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // GeomColl (2D) [with BBOX & long SRS]
geocollz(G) ::= GEOJSON_GEOMETRYCOLLECTION geocoll_textz(X). 
	{ G = X; } // GeomColl (3D) [simple]
geocollz(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_BBOX GEOJSON_COLON GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA
	GEOJSON_GEOMS GEOJSON_COLON geocoll_textz(X) GEOJSON_CLOSE_BRACE.
	{ G = X; } // GeomColl (3D) [with BBOX] 
geocollz(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA
	GEOJSON_GEOMS GEOJSON_COLON geocoll_textz(X) GEOJSON_CLOSE_BRACE.
	{ G = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // GeomColl (3D) [with short SRS]
geocollz(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA
	GEOJSON_GEOMS GEOJSON_COLON geocoll_textz(X) GEOJSON_CLOSE_BRACE.
	{ G = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // GeomColl (3D) [with long SRS]
geocollz(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON short_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS 
	GEOJSON_COLON geocoll_textz(X) GEOJSON_CLOSE_BRACE.
	{ G = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // GeomColl (3D) [with BBOX & short SRS]
geocollz(G) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_GEOMETRYCOLLECTION GEOJSON_COMMA 
	GEOJSON_CRS GEOJSON_COLON long_crs(S) GEOJSON_COMMA GEOJSON_BBOX GEOJSON_COLON 
	GEOJSON_OPEN_BRACKET bbox GEOJSON_CLOSE_BRACKET GEOJSON_COMMA GEOJSON_GEOMS 
	GEOJSON_COLON geocoll_textz(X) GEOJSON_CLOSE_BRACE.
	{ G = (void *) geoJSON_setSrid((gaiaGeomCollPtr)X, (int *)S); } // GeomColl (3D) [with BBOX & long SRS]

// Geometry collections can contain any number of points, linestrings, or polygons (but at least one):
geocoll_text(G) ::= GEOJSON_OPEN_BRACKET coll_point(P) geocoll_text2(X) GEOJSON_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) geoJSON_geomColl_xy(p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_text(G) ::= GEOJSON_OPEN_BRACKET coll_linestring(L) geocoll_text2(X) GEOJSON_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) geoJSON_geomColl_xy(p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_text(G) ::= GEOJSON_OPEN_BRACKET coll_polygon(P) geocoll_text2(X) GEOJSON_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) geoJSON_geomColl_xy(p_data, (gaiaGeomCollPtr)P);
	}

// Extra points, linestrings, or polygons
geocoll_text2(X) ::=  . { X = NULL; }
geocoll_text2(X) ::= GEOJSON_COMMA coll_point(P) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_text2(X) ::= GEOJSON_COMMA coll_linestring(L) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_text2(X) ::= GEOJSON_COMMA coll_polygon(P) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}


geocoll_textz(G) ::= GEOJSON_OPEN_BRACKET coll_pointz(P) geocoll_textz2(X) GEOJSON_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) geoJSON_geomColl_xyz(p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textz(G) ::= GEOJSON_OPEN_BRACKET coll_linestringz(L) geocoll_textz2(X) GEOJSON_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) geoJSON_geomColl_xyz(p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textz(G) ::= GEOJSON_OPEN_BRACKET coll_polygonz(P) geocoll_textz2(X) GEOJSON_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) geoJSON_geomColl_xyz(p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textz2(X) ::=  . { X = NULL; }
geocoll_textz2(X) ::= GEOJSON_COMMA coll_pointz(P) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textz2(X) ::= GEOJSON_COMMA coll_linestringz(L) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textz2(X) ::= GEOJSON_COMMA coll_polygonz(P) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

coll_point(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON point_coordxy(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPoint(p_data, (gaiaPointPtr)Q); } 
	
coll_pointz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POINT GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON point_coordxyz(Q) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPoint(p_data, (gaiaPointPtr)Q); }
	
coll_linestring(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON linestring_text(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestring(p_data, (gaiaLinestringPtr)X); }
	
coll_linestringz(L) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_LINESTRING GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON linestring_textz(X) GEOJSON_CLOSE_BRACE.
	{ L = geoJSON_buildGeomFromLinestring(p_data, (gaiaLinestringPtr)X); } 
	
coll_polygon(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON polygon_text(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygon(p_data, (gaiaPolygonPtr)X); }
	
coll_polygonz(P) ::= GEOJSON_OPEN_BRACE GEOJSON_TYPE GEOJSON_COLON GEOJSON_POLYGON GEOJSON_COMMA 
	GEOJSON_COORDS GEOJSON_COLON polygon_textz(X) GEOJSON_CLOSE_BRACE.
	{ P = geoJSON_buildGeomFromPolygon(p_data, (gaiaPolygonPtr)X); }
