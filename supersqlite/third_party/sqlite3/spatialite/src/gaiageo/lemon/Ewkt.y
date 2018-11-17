/* 
 Ewkt.y -- EWKT parser - LEMON config
  
 version 2.4, 2011 May 14

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

// Header files to be included in Ewkt.c
%include {
}

// Set the return value of gaiaParseEWKT in the following pointer:
%extra_argument { struct ewkt_data *p_data }

// Invalid syntax (ie. no rules matched)
%syntax_error {
/* 
** when the LEMON parser encounters an error
** then this global variable is set 
*/
	p_data->ewkt_parse_error = 1;
	p_data->result = NULL;
}
 
 /* This is to terminate with a new line */
 main ::= in.
 in ::= .
 in ::= in state EWKT_NEWLINE.
 
 state ::= program.
 
 /* 
 * program is the start node. All strings matched by this CFG must be one of 
 * geo_text (text describing a geometry), 
 * geo_textm (text describing a 2D geometry with a measure)
 */
 
program ::= geo_text.
program ::= geo_textm.

// geometries (2D, 3D and 3D with a measure):
geo_text ::= point(P). { p_data->result = P; }			// P is a geometry collection containing a point
geo_text ::= pointz(P). { p_data->result = P; }			// P is a geometry collection containing a point
geo_text ::= pointzm(P). { p_data->result = P; }		// P is a geometry collection containing a point
geo_text ::= linestring(L). { p_data->result = L; }		// L is a geometry collection containing a linestring
geo_text ::= linestringz(L). { p_data->result = L; }		// L is a geometry collection containing a linestring
geo_text ::= linestringzm(L). { p_data->result = L; }		// L is a geometry collection containing a linestring
geo_text ::= polygon(P). { p_data->result = P; }		// P is a geometry collection containing a polygon
geo_text ::= polygonz(P). { p_data->result = P; }		// P is a geometry collection containing a polygon
geo_text ::= polygonzm(P). { p_data->result = P; }		// P is a geometry collection containing a polygon
geo_text ::= multipoint(M). { p_data->result = M; }		// M is a geometry collection containing a multipoint
geo_text ::= multipointz(M). { p_data->result = M; }		// M is a geometry collection containing a multipoint
geo_text ::= multipointzm(M). { p_data->result = M; }		// M is a geometry collection containing a multipoint
geo_text ::= multilinestring(M). { p_data->result = M; }	// M is a geometry collection containing a multilinestring
geo_text ::= multilinestringz(M). { p_data->result = M; }	// M is a geometry collection containing a multilinestring
geo_text ::= multilinestringzm(M). { p_data->result = M; }	// M is a geometry collection containing a multilinestring
geo_text ::= multipolygon(M). { p_data->result = M; }		// M is a geometry collection containing a multipolygon
geo_text ::= multipolygonz(M). { p_data->result = M; }		// M is a geometry collection containing a multipolygon
geo_text ::= multipolygonzm(M). { p_data->result = M; }		// M is a geometry collection containing a multipolygon
geo_text ::= geocoll(H). { p_data->result = H; }		// H is a geometry collection created from user input
geo_text ::= geocollz(H). { p_data->result = H; }		// H is a geometry collection created from user input
geo_text ::= geocollzm(H). { p_data->result = H; }		// H is a geometry collection created from user input

// 2D geometries (with a measure):
geo_textm ::= pointm(P). { p_data->result = P; }
geo_textm ::= linestringm(L). { p_data->result = L; }
geo_textm ::= polygonm(P). { p_data->result = P; }
geo_textm ::= multipointm(M). { p_data->result = M; }
geo_textm ::= multilinestringm(M). { p_data->result = M; }
geo_textm ::= multipolygonm(M). { p_data->result = M; }
geo_textm ::= geocollm(H). { p_data->result = H; }


// Syntax for a "point" object:
// The functions called build a geometry collection from a gaiaPointPtr
point(P) ::= EWKT_POINT EWKT_OPEN_BRACKET point_coordxy(Q) EWKT_CLOSE_BRACKET. 
	{ P = ewkt_buildGeomFromPoint( p_data, (gaiaPointPtr)Q); }
pointz(P) ::= EWKT_POINT EWKT_OPEN_BRACKET point_coordxyz(Q) EWKT_CLOSE_BRACKET. 
	{ P = ewkt_buildGeomFromPoint( p_data, (gaiaPointPtr)Q); }
pointm(P) ::= EWKT_POINT_M EWKT_OPEN_BRACKET point_coordxym(Q) EWKT_CLOSE_BRACKET. 
	{ P = ewkt_buildGeomFromPoint( p_data, (gaiaPointPtr)Q);  }
pointzm(P) ::= EWKT_POINT EWKT_OPEN_BRACKET point_coordxyzm(Q) EWKT_CLOSE_BRACKET. 
	{ P = ewkt_buildGeomFromPoint( p_data, (gaiaPointPtr)Q); }

// Point coordinates in different dimensions: MultiPoint((pt),(pt))
// Create the point by calling the proper function in SpatiaLite :
point_brkt_coordxy(P) ::= EWKT_OPEN_BRACKET coord(X) coord(Y) EWKT_CLOSE_BRACKET.  
	{ P = (void *) ewkt_point_xy( p_data, (double *)X, (double *)Y); }
point_brkt_coordxym(P) ::= EWKT_OPEN_BRACKET coord(X) coord(Y) coord(M) EWKT_CLOSE_BRACKET.  
	{ P = (void *) ewkt_point_xym( p_data, (double *)X, (double *)Y, (double *)M); }
point_brkt_coordxyz(P) ::= EWKT_OPEN_BRACKET coord(X) coord(Y) coord(Z) EWKT_CLOSE_BRACKET.  
	{ P = (void *) ewkt_point_xyz( p_data, (double *)X, (double *)Y, (double *)Z); }
point_brkt_coordxyzm(P) ::= EWKT_OPEN_BRACKET coord(X) coord(Y) coord(Z) coord(M) EWKT_CLOSE_BRACKET.  
	{ P = (void *) ewkt_point_xyzm( p_data, (double *)X, (double *)Y, (double *)Z, (double *)M); }

// Point coordinates in different dimensions.
// Create the point by calling the proper function in SpatiaLite :
point_coordxy(P) ::= coord(X) coord(Y). 
	{ P = (void *) ewkt_point_xy( p_data, (double *)X, (double *)Y); }
point_coordxym(P) ::= coord(X) coord(Y) coord(M). 
	{ P = (void *) ewkt_point_xym( p_data, (double *)X, (double *)Y, (double *)M); }
point_coordxyz(P) ::= coord(X) coord(Y) coord(Z). 
	{ P = (void *) ewkt_point_xyz( p_data, (double *)X, (double *)Y, (double *)Z); }
point_coordxyzm(P) ::= coord(X) coord(Y) coord(Z) coord(M). 
	{ P = (void *) ewkt_point_xyzm( p_data, (double *)X, (double *)Y, (double *)Z, (double *)M); }

// All coordinates are assumed to be doubles (guaranteed by the flex tokenizer).
coord(A) ::= EWKT_NUM(B). { A = B; } 


// Rules to match an infinite number of points: MultiPoint((pt), (pt))
// Also links the generated gaiaPointPtrs together
extra_brkt_pointsxy(A) ::=  . { A = NULL; }
extra_brkt_pointsxy(A) ::= EWKT_COMMA point_brkt_coordxy(P) extra_brkt_pointsxy(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_brkt_pointsxym(A) ::=  . { A = NULL; }
extra_brkt_pointsxym(A) ::= EWKT_COMMA point_brkt_coordxym(P) extra_brkt_pointsxym(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_brkt_pointsxyz(A) ::=  .  { A = NULL; }
extra_brkt_pointsxyz(A) ::= EWKT_COMMA point_brkt_coordxyz(P) extra_brkt_pointsxyz(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_brkt_pointsxyzm(A) ::=  .  { A = NULL; }
extra_brkt_pointsxyzm(A) ::= EWKT_COMMA point_brkt_coordxyzm(P) extra_brkt_pointsxyzm(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }


// Rules to match an infinite number of points:
// Also links the generated gaiaPointPtrs together
extra_pointsxy(A) ::=  . { A = NULL; }
extra_pointsxy(A) ::= EWKT_COMMA point_coordxy(P) extra_pointsxy(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_pointsxym(A) ::=  . { A = NULL; }
extra_pointsxym(A) ::= EWKT_COMMA point_coordxym(P) extra_pointsxym(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_pointsxyz(A) ::=  .  { A = NULL; }
extra_pointsxyz(A) ::= EWKT_COMMA point_coordxyz(P) extra_pointsxyz(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_pointsxyzm(A) ::=  .  { A = NULL; }
extra_pointsxyzm(A) ::= EWKT_COMMA point_coordxyzm(P) extra_pointsxyzm(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }


// Syntax for a "linestring" object:
// The functions called build a geometry collection from a gaiaLinestringPtr
linestring(L) ::= EWKT_LINESTRING linestring_text(X). 
	{ L = ewkt_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); }
linestringm(L) ::= EWKT_LINESTRING_M linestring_textm(X).
	{ L = ewkt_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); }
linestringz(L) ::= EWKT_LINESTRING linestring_textz(X).
	{ L = ewkt_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); }
linestringzm(L) ::= EWKT_LINESTRING linestring_textzm(X).
	{ L = ewkt_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); }

// A valid linestring must have at least two vertices:
// The functions called build a gaiaLinestring from a linked list of points
linestring_text(L) ::= EWKT_OPEN_BRACKET point_coordxy(P) EWKT_COMMA point_coordxy(Q) extra_pointsxy(R) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)Q)->Next = (gaiaPointPtr)R; 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q;
	   L = (void *) ewkt_linestring_xy( p_data, (gaiaPointPtr)P);
	}

linestring_textm(L) ::= EWKT_OPEN_BRACKET point_coordxym(P) EWKT_COMMA point_coordxym(Q) extra_pointsxym(R) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)Q)->Next = (gaiaPointPtr)R; 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q;
	   L = (void *) ewkt_linestring_xym( p_data, (gaiaPointPtr)P);
	}

linestring_textz(L) ::= EWKT_OPEN_BRACKET point_coordxyz(P) EWKT_COMMA point_coordxyz(Q) extra_pointsxyz(R) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)Q)->Next = (gaiaPointPtr)R; 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q;
	   L = (void *) ewkt_linestring_xyz( p_data, (gaiaPointPtr)P);
	}

linestring_textzm(L) ::= EWKT_OPEN_BRACKET point_coordxyzm(P) EWKT_COMMA point_coordxyzm(Q) extra_pointsxyzm(R) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)Q)->Next = (gaiaPointPtr)R; 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q;
	   L = (void *) ewkt_linestring_xyzm( p_data, (gaiaPointPtr)P);
	}


// Syntax for a "polygon" object:
// The functions called build a geometry collection from a gaiaPolygonPtr
polygon(P) ::= EWKT_POLYGON polygon_text(X).
	{ P = ewkt_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); }
polygonm(P) ::= EWKT_POLYGON_M polygon_textm(X).
	{ P = ewkt_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); }
polygonz(P) ::= EWKT_POLYGON polygon_textz(X).
	{ P = ewkt_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); }
polygonzm(P) ::= EWKT_POLYGON polygon_textzm(X).
	{ P = ewkt_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); }

// A valid polygon must have at least one ring:
// The functions called build a gaiaPolygonPtr from a linked list of gaiaRingPtrs
polygon_text(P) ::= EWKT_OPEN_BRACKET ring(R) extra_rings(E) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaRingPtr)R)->Next = (gaiaRingPtr)E;
		P = (void *) ewkt_polygon_xy( p_data, (gaiaRingPtr)R);
	}

polygon_textm(P) ::= EWKT_OPEN_BRACKET ringm(R) extra_ringsm(E) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaRingPtr)R)->Next = (gaiaRingPtr)E;
		P = (void *) ewkt_polygon_xym( p_data, (gaiaRingPtr)R);
	}

polygon_textz(P) ::= EWKT_OPEN_BRACKET ringz(R) extra_ringsz(E) EWKT_CLOSE_BRACKET.
	{  
		((gaiaRingPtr)R)->Next = (gaiaRingPtr)E;
		P = (void *) ewkt_polygon_xyz( p_data, (gaiaRingPtr)R);
	}

polygon_textzm(P) ::= EWKT_OPEN_BRACKET ringzm(R) extra_ringszm(E) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaRingPtr)R)->Next = (gaiaRingPtr)E;
		P = (void *) ewkt_polygon_xyzm( p_data, (gaiaRingPtr)R);
	}

// A valid ring must have at least 4 points
// The functions called build a gaiaRingPtr from a linked list of gaiaPointPtrs
ring(R) ::= EWKT_OPEN_BRACKET point_coordxy(A) EWKT_COMMA point_coordxy(B) EWKT_COMMA point_coordxy(C) EWKT_COMMA point_coordxy(D) extra_pointsxy(E) EWKT_CLOSE_BRACKET.
	{
		((gaiaPointPtr)A)->Next = (gaiaPointPtr)B; 
		((gaiaPointPtr)B)->Next = (gaiaPointPtr)C;
		((gaiaPointPtr)C)->Next = (gaiaPointPtr)D; 
		((gaiaPointPtr)D)->Next = (gaiaPointPtr)E;
		R = (void *) ewkt_ring_xy( p_data, (gaiaPointPtr)A);
	}

// To match more than one 2D ring:
extra_rings(R) ::=  . { R = NULL; }
extra_rings(R) ::= EWKT_COMMA ring(S) extra_rings(T).
	{
		((gaiaRingPtr)S)->Next = (gaiaRingPtr)T;
		R = S;
	}
	
ringm(R) ::= EWKT_OPEN_BRACKET point_coordxym(A) EWKT_COMMA point_coordxym(B) EWKT_COMMA point_coordxym(C) EWKT_COMMA point_coordxym(D) extra_pointsxym(E) EWKT_CLOSE_BRACKET.
	{
		((gaiaPointPtr)A)->Next = (gaiaPointPtr)B; 
		((gaiaPointPtr)B)->Next = (gaiaPointPtr)C;
		((gaiaPointPtr)C)->Next = (gaiaPointPtr)D; 
		((gaiaPointPtr)D)->Next = (gaiaPointPtr)E;
		R = (void *) ewkt_ring_xym( p_data, (gaiaPointPtr)A);
	}

// To match more than one 2D (with a measure) ring:
extra_ringsm(R) ::=  . { R = NULL; }
extra_ringsm(R) ::= EWKT_COMMA ringm(S) extra_ringsm(T).
	{
		((gaiaRingPtr)S)->Next = (gaiaRingPtr)T;
		R = S;
	}
	
ringz(R) ::= EWKT_OPEN_BRACKET point_coordxyz(A) EWKT_COMMA point_coordxyz(B) EWKT_COMMA point_coordxyz(C) EWKT_COMMA point_coordxyz(D) extra_pointsxyz(E) EWKT_CLOSE_BRACKET.
	{
		((gaiaPointPtr)A)->Next = (gaiaPointPtr)B; 
		((gaiaPointPtr)B)->Next = (gaiaPointPtr)C;
		((gaiaPointPtr)C)->Next = (gaiaPointPtr)D; 
		((gaiaPointPtr)D)->Next = (gaiaPointPtr)E;
		R = (void *) ewkt_ring_xyz( p_data, (gaiaPointPtr)A);
	}

// To match more than one 3D ring:
extra_ringsz(R) ::=  . { R = NULL; }
extra_ringsz(R) ::= EWKT_COMMA ringz(S) extra_ringsz(T).
	{
		((gaiaRingPtr)S)->Next = (gaiaRingPtr)T;
		R = S;
	}

ringzm(R) ::= EWKT_OPEN_BRACKET point_coordxyzm(A) EWKT_COMMA point_coordxyzm(B) EWKT_COMMA point_coordxyzm(C) EWKT_COMMA point_coordxyzm(D) extra_pointsxyzm(E) EWKT_CLOSE_BRACKET.
	{
		((gaiaPointPtr)A)->Next = (gaiaPointPtr)B; 
		((gaiaPointPtr)B)->Next = (gaiaPointPtr)C;
		((gaiaPointPtr)C)->Next = (gaiaPointPtr)D; 
		((gaiaPointPtr)D)->Next = (gaiaPointPtr)E;
		R = (void *) ewkt_ring_xyzm( p_data, (gaiaPointPtr)A);
	}
	
// To match more than one 3D (with a measure) ring:
extra_ringszm(R) ::=  . { R = NULL; }
extra_ringszm(R) ::= EWKT_COMMA ringzm(S) extra_ringszm(T).
	{
		((gaiaRingPtr)S)->Next = (gaiaRingPtr)T;
		R = S;
	}

// Syntax for a "multipoint" object:
// X in the following lines is a geometry collection containing a multipoint
multipoint(M) ::= EWKT_MULTIPOINT multipoint_text(X). { M = X; }
multipointm(M) ::= EWKT_MULTIPOINT_M multipoint_textm(X). { M = X; }
multipointz(M) ::= EWKT_MULTIPOINT multipoint_textz(X). { M = X; }
multipointzm(M) ::= EWKT_MULTIPOINT multipoint_textzm(X). { M = X; }

// Multipoints can contain any number of points (but at least one):
// The functions called build a geometry collection containing a multipoint
multipoint_text(M) ::= EWKT_OPEN_BRACKET point_coordxy(P) extra_pointsxy(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) ewkt_multipoint_xy( p_data, (gaiaPointPtr)P);
	}
multipoint_textm(M) ::= EWKT_OPEN_BRACKET point_coordxym(P) extra_pointsxym(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) ewkt_multipoint_xym( p_data, (gaiaPointPtr)P);
	}
multipoint_textz(M) ::= EWKT_OPEN_BRACKET point_coordxyz(P) extra_pointsxyz(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) ewkt_multipoint_xyz( p_data, (gaiaPointPtr)P);
	}
multipoint_textzm(M) ::= EWKT_OPEN_BRACKET point_coordxyzm(P) extra_pointsxyzm(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) ewkt_multipoint_xyzm( p_data, (gaiaPointPtr)P);
	}
multipoint_text(M) ::= EWKT_OPEN_BRACKET point_brkt_coordxy(P) extra_brkt_pointsxy(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) ewkt_multipoint_xy( p_data, (gaiaPointPtr)P);
	}
multipoint_textm(M) ::= EWKT_OPEN_BRACKET point_brkt_coordxym(P) extra_brkt_pointsxym(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) ewkt_multipoint_xym( p_data, (gaiaPointPtr)P);
	}
multipoint_textz(M) ::= EWKT_OPEN_BRACKET point_brkt_coordxyz(P) extra_brkt_pointsxyz(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) ewkt_multipoint_xyz( p_data, (gaiaPointPtr)P);
	}
multipoint_textzm(M) ::= EWKT_OPEN_BRACKET point_brkt_coordxyzm(P) extra_brkt_pointsxyzm(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) ewkt_multipoint_xyzm( p_data, (gaiaPointPtr)P);
	}


// Syntax for a "multilinestring" object:
// X in the following lines refers to a geometry collection containing a multilinestring
multilinestring(M) ::= EWKT_MULTILINESTRING multilinestring_text(X). { M = X; }
multilinestringm(M) ::= EWKT_MULTILINESTRING_M multilinestring_textm(X). { M = X; }
multilinestringz(M) ::= EWKT_MULTILINESTRING multilinestring_textz(X). { M = X; }
multilinestringzm(M) ::= EWKT_MULTILINESTRING multilinestring_textzm(X). { M = X; }

// Multilinestrings can contain any number of linestrings (but at least one):
// The functions called build a geometry collection containing a multilinestring
multilinestring_text(M) ::= EWKT_OPEN_BRACKET linestring_text(L) multilinestring_text2(X) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)X; 
	   M = (void *) ewkt_multilinestring_xy( p_data, (gaiaLinestringPtr)L);
	}

// Extra linestrings
multilinestring_text2(X) ::=  . { X = NULL; }
multilinestring_text2(X) ::= EWKT_COMMA linestring_text(L) multilinestring_text2(Y).
	{ ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)Y;  X = L; }

multilinestring_textm(M) ::= EWKT_OPEN_BRACKET linestring_textm(L) multilinestring_textm2(X) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)X; 
	   M = (void *) ewkt_multilinestring_xym( p_data, (gaiaLinestringPtr)L);
	}

multilinestring_textm2(X) ::=  . { X = NULL; }
multilinestring_textm2(X) ::= EWKT_COMMA linestring_textm(L) multilinestring_textm2(Y).
	{ ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)Y;  X = L; }

multilinestring_textz(M) ::= EWKT_OPEN_BRACKET linestring_textz(L) multilinestring_textz2(X) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)X; 
	   M = (void *) ewkt_multilinestring_xyz( p_data, (gaiaLinestringPtr)L);
	}

multilinestring_textz2(X) ::=  . { X = NULL; }
multilinestring_textz2(X) ::= EWKT_COMMA linestring_textz(L) multilinestring_textz2(Y).
	{ ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)Y;  X = L; }

multilinestring_textzm(M) ::= EWKT_OPEN_BRACKET linestring_textzm(L) multilinestring_textzm2(X) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)X; 
	   M = (void *) ewkt_multilinestring_xyzm( p_data, (gaiaLinestringPtr)L);
	}

multilinestring_textzm2(X) ::=  . { X = NULL; }
multilinestring_textzm2(X) ::= EWKT_COMMA linestring_textzm(L) multilinestring_textzm2(Y).
	{ ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)Y;  X = L; }


// Syntax for a "multipolygon" object
// X in the following lines refers to a geometry collection containing a multipolygon
multipolygon(M) ::= EWKT_MULTIPOLYGON multipolygon_text(X). { M = X; }
multipolygonm(M) ::= EWKT_MULTIPOLYGON_M multipolygon_textm(X). { M = X; }
multipolygonz(M) ::= EWKT_MULTIPOLYGON multipolygon_textz(X). { M = X; }
multipolygonzm(M) ::= EWKT_MULTIPOLYGON multipolygon_textzm(X). { M = X; }

// Multipolygons can contain any number of polygons (but at least one):
// The functions called build a geometry collection containing a multipolygon
multipolygon_text(M) ::= EWKT_OPEN_BRACKET polygon_text(P) multipolygon_text2(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)Q; 
	   M = (void *) ewkt_multipolygon_xy( p_data, (gaiaPolygonPtr)P);
	}

// Extra polygons
multipolygon_text2(Q) ::=  . { Q = NULL; }
multipolygon_text2(A) ::= EWKT_COMMA polygon_text(P) multipolygon_text2(B).
	{ ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)B;  A = P; }

multipolygon_textm(M) ::= EWKT_OPEN_BRACKET polygon_textm(P) multipolygon_textm2(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)Q; 
	   M = (void *) ewkt_multipolygon_xym( p_data, (gaiaPolygonPtr)P);
	}

multipolygon_textm2(Q) ::=  . { Q = NULL; }
multipolygon_textm2(A) ::= EWKT_COMMA polygon_textm(P) multipolygon_textm2(B).
	{ ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)B;  A = P; }

multipolygon_textz(M) ::= EWKT_OPEN_BRACKET polygon_textz(P) multipolygon_textz2(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)Q; 
	   M = (void *) ewkt_multipolygon_xyz( p_data, (gaiaPolygonPtr)P);
	}

multipolygon_textz2(Q) ::=  . { Q = NULL; }
multipolygon_textz2(A) ::= EWKT_COMMA polygon_textz(P) multipolygon_textz2(B).
	{ ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)B;  A = P; }

multipolygon_textzm(M) ::= EWKT_OPEN_BRACKET polygon_textzm(P) multipolygon_textzm2(Q) EWKT_CLOSE_BRACKET.
	{ 
	   ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)Q; 
	   M = (void *) ewkt_multipolygon_xyzm( p_data, (gaiaPolygonPtr)P);
	}

multipolygon_textzm2(Q) ::=  . { Q = NULL; }
multipolygon_textzm2(A) ::= EWKT_COMMA polygon_textzm(P) multipolygon_textzm2(B).
	{ ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)B;  A = P; }


// Syntax for a "geometrycollection" object:
// X in the following lines refers to a geometry collection generated based on user input
geocoll(G) ::= EWKT_GEOMETRYCOLLECTION geocoll_text(X). { G = X; }
geocollm(G) ::= EWKT_GEOMETRYCOLLECTION_M geocoll_textm(X). { G = X; }
geocollz(G) ::= EWKT_GEOMETRYCOLLECTION geocoll_textz(X). { G = X; }
geocollzm(G) ::= EWKT_GEOMETRYCOLLECTION geocoll_textzm(X). { G = X; }

// Geometry collections can contain any number of points, linestrings, or polygons (but at least one):
geocoll_text(G) ::= EWKT_OPEN_BRACKET point(P) geocoll_text2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xy( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_text(G) ::= EWKT_OPEN_BRACKET linestring(L) geocoll_text2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xy( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_text(G) ::= EWKT_OPEN_BRACKET polygon(P) geocoll_text2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xy( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_text(G) ::= EWKT_OPEN_BRACKET multipoint(P) geocoll_text2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xy( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_text(G) ::= EWKT_OPEN_BRACKET multilinestring(L) geocoll_text2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xy( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_text(G) ::= EWKT_OPEN_BRACKET multipolygon(P) geocoll_text2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xy( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_text(G) ::= EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION geocoll_text(C) geocoll_text2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xy( p_data, (gaiaGeomCollPtr)C);
	}

// Extra points, linestrings, or polygons
geocoll_text2(X) ::=  . { X = NULL; }
geocoll_text2(X) ::= EWKT_COMMA point(P) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_text2(X) ::= EWKT_COMMA linestring(L) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_text2(X) ::= EWKT_COMMA polygon(P) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_text2(X) ::= EWKT_COMMA multipoint(P) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_text2(X) ::= EWKT_COMMA multilinestring(L) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_text2(X) ::= EWKT_COMMA multipolygon(P) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_text2(X) ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION geocoll_text(C) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)Y;
		X = C;
	}


geocoll_textm(G) ::= EWKT_OPEN_BRACKET pointm(P) geocoll_textm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xym( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textm(G) ::= EWKT_OPEN_BRACKET linestringm(L) geocoll_textm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xym( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textm(G) ::= EWKT_OPEN_BRACKET polygonm(P) geocoll_textm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xym( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textm(G) ::= EWKT_OPEN_BRACKET multipointm(P) geocoll_textm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xym( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textm(G) ::= EWKT_OPEN_BRACKET multilinestringm(L) geocoll_textm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xym( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textm(G) ::= EWKT_OPEN_BRACKET multipolygonm(P) geocoll_textm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xym( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textm(G) ::= 
EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION_M geocoll_textm(C) geocoll_textm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xym( p_data, (gaiaGeomCollPtr)C);
	}

geocoll_textm2(X) ::=  . { X = NULL; }
geocoll_textm2(X) ::= EWKT_COMMA pointm(P) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textm2(X) ::= EWKT_COMMA linestringm(L) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textm2(X) ::= EWKT_COMMA polygonm(P) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textm2(X) ::= EWKT_COMMA multipointm(P) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textm2(X) ::= EWKT_COMMA multilinestringm(L) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textm2(X) ::= EWKT_COMMA multipolygonm(P) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textm2(X) ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION_M geocoll_textm(C) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)Y;
		X = C;
	}
	

geocoll_textz(G) ::= EWKT_OPEN_BRACKET pointz(P) geocoll_textz2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyz( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textz(G) ::= EWKT_OPEN_BRACKET linestringz(L) geocoll_textz2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyz( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textz(G) ::= EWKT_OPEN_BRACKET polygonz(P) geocoll_textz2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyz( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textz(G) ::= EWKT_OPEN_BRACKET multipointz(P) geocoll_textz2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyz( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textz(G) ::= EWKT_OPEN_BRACKET multilinestringz(L) geocoll_textz2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyz( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textz(G) ::= EWKT_OPEN_BRACKET multipolygonz(P) geocoll_textz2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyz( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textz(G) ::= 
EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION geocoll_textz(C) geocoll_textz2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyz( p_data, (gaiaGeomCollPtr)C);
	}


geocoll_textz2(X) ::=  . { X = NULL; }
geocoll_textz2(X) ::= EWKT_COMMA pointz(P) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textz2(X) ::= EWKT_COMMA linestringz(L) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textz2(X) ::= EWKT_COMMA polygonz(P) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textz2(X) ::= EWKT_COMMA multipointz(P) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textz2(X) ::= EWKT_COMMA multilinestringz(L) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textz2(X) ::= EWKT_COMMA multipolygonz(P) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textz2(X) ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION geocoll_textz(C) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)Y;
		X = C;
	}
	
	
geocoll_textzm(G) ::= EWKT_OPEN_BRACKET pointzm(P) geocoll_textzm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyzm( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textzm(G) ::= EWKT_OPEN_BRACKET linestringzm(L) geocoll_textzm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyzm( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textzm(G) ::= EWKT_OPEN_BRACKET polygonzm(P) geocoll_textzm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyzm( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textzm(G) ::= EWKT_OPEN_BRACKET multipointzm(P) geocoll_textzm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyzm( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textzm(G) ::= EWKT_OPEN_BRACKET multilinestringzm(L) geocoll_textzm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyzm( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textzm(G) ::= EWKT_OPEN_BRACKET multipolygonzm(P) geocoll_textzm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyzm( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textzm(G) ::= 
EWKT_OPEN_BRACKET EWKT_GEOMETRYCOLLECTION geocoll_textzm(C) geocoll_textzm2(X) EWKT_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)X;
		G = (void *) ewkt_geomColl_xyzm( p_data, (gaiaGeomCollPtr)C);
	}


geocoll_textzm2(X) ::=  . { X = NULL; }
geocoll_textzm2(X) ::= EWKT_COMMA pointzm(P) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textzm2(X) ::= EWKT_COMMA linestringzm(L) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textzm2(X) ::= EWKT_COMMA polygonzm(P) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textzm2(X) ::= EWKT_COMMA multipointzm(P) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textzm2(X) ::= EWKT_COMMA multilinestringzm(L) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textzm2(X) ::= EWKT_COMMA multipolygonzm(P) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textzm2(X) ::= EWKT_COMMA EWKT_GEOMETRYCOLLECTION geocoll_textzm(C) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)Y;
		X = C;
	}
