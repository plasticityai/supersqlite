/* 
 vanuatuWkt.y -- Vanuatu WKT parser - LEMON config
  
 version 2.4, 2010 April 2

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
 
Portions created by the Initial Developer are Copyright (C) 2008
the Initial Developer. All Rights Reserved.

Contributor(s):
The Vanuatu Team - University of Toronto

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

/******************************************************************************
** The following code was created by Team Vanuatu of The University of Toronto.

Authors:
Ruppi Rana			ruppi.rana@gmail.com
Dev Tanna			dev.tanna@gmail.com
Elias Adum			elias.adum@gmail.com
Benton Hui			benton.hui@gmail.com
Abhayan Sundararajan		abhayan@gmail.com
Chee-Lun Michael Stephen Cho	cheelun.cho@gmail.com
Nikola Banovic			nikola.banovic@gmail.com
Yong Jian			yong.jian@utoronto.ca

Supervisor:
Greg Wilson			gvwilson@cs.toronto.ca

-------------------------------------------------------------------------------
*/

// Tokens are void pointers (so we can cast them to whatever we want)
%token_type {void *}

// Output to stderr when stack overflows
%stack_overflow {
     spatialite_e( "Giving up.  Parser stack overflow\n");
}

// Increase this number if necessary
%stack_size 1000000

// Header files to be included in vanuatuWkt.c
%include {
}

// Set the return value of gaiaParseWkt in the following pointer:
%extra_argument { struct vanuatu_data *p_data }

// Invalid syntax (ie. no rules matched)
%syntax_error {
/* 
** Sandro Furieri 2010 Apr 4
** when the LEMON parser encounters an error
** then this global variable is set 
*/
	p_data->vanuatu_parse_error = 1;
	p_data->result = NULL;
}
 
 /* This is to terminate with a new line */
 main ::= in.
 in ::= .
 in ::= in state VANUATU_NEWLINE.
 
 state ::= program.
 
 /* 
 * program is the start node. All strings matched by this CFG must be one of 
 * geo_text (text describing a 2D geometry), 
 * geo_textz (text describing a 3D geometry), 
 * geo_textm (text describing a 2D geometry with a measure)
 * geo_textzm (text describing a 3D geometry with a measure)
 */
 
program ::= geo_text.
program ::= geo_textz.
program ::= geo_textm.
program ::= geo_textzm.

// 2D geometries (no measure):
geo_text ::= point(P). { p_data->result = P; }			// P is a geometry collection containing a point
geo_text ::= linestring(L). { p_data->result = L; }		// L is a geometry collection containing a linestring
geo_text ::= polygon(P). { p_data->result = P; }		// P is a geometry collection containing a polygon
geo_text ::= multipoint(M). { p_data->result = M; }		// M is a geometry collection containing a multipoint
geo_text ::= multilinestring(M). { p_data->result = M; }	// M is a geometry collection containing a multilinestring
geo_text ::= multipolygon(M). { p_data->result = M; }		// M is a geometry collection containing a multipolygon
geo_text ::= geocoll(H). { p_data->result = H; }		// H is a geometry collection created from user input

// 3D geometries (no measure):
geo_textz ::= pointz(P). { p_data->result = P; }
geo_textz ::= linestringz(L). { p_data->result = L; }
geo_textz ::= polygonz(P). { p_data->result = P; }
geo_textz ::= multipointz(M). { p_data->result = M; }
geo_textz ::= multilinestringz(M). { p_data->result = M; }
geo_textz ::= multipolygonz(M). { p_data->result = M; }
geo_textz ::= geocollz(H). { p_data->result = H; }

// 2D geometries (with a measure):
geo_textm ::= pointm(P). { p_data->result = P; }
geo_textm ::= linestringm(L). { p_data->result = L; }
geo_textm ::= polygonm(P). { p_data->result = P; }
geo_textm ::= multipointm(M). { p_data->result = M; }
geo_textm ::= multilinestringm(M). { p_data->result = M; }
geo_textm ::= multipolygonm(M). { p_data->result = M; }
geo_textm ::= geocollm(H). { p_data->result = H; }

// 3D geometries (with a measure):
geo_textzm ::= pointzm(P). { p_data->result = P; }
geo_textzm ::= linestringzm(L). { p_data->result = L; }
geo_textzm ::= polygonzm(P). { p_data->result = P; }
geo_textzm ::= multipointzm(M). { p_data->result = M; }
geo_textzm ::= multilinestringzm(M). { p_data->result = M; }
geo_textzm ::= multipolygonzm(M). { p_data->result = M; }
geo_textzm ::= geocollzm(H). { p_data->result = H; }


// Syntax for a "point" object:
// The functions called build a geometry collection from a gaiaPointPtr
point(P) ::= VANUATU_POINT VANUATU_OPEN_BRACKET point_coordxy(Q) VANUATU_CLOSE_BRACKET. 
	{ P = vanuatu_buildGeomFromPoint( p_data, (gaiaPointPtr)Q); }
pointm(P) ::= VANUATU_POINT_M VANUATU_OPEN_BRACKET point_coordxym(Q) VANUATU_CLOSE_BRACKET. 
	{ P = vanuatu_buildGeomFromPoint( p_data, (gaiaPointPtr)Q);  }
pointz(P) ::= VANUATU_POINT_Z VANUATU_OPEN_BRACKET point_coordxyz(Q) VANUATU_CLOSE_BRACKET. 
	{ P = vanuatu_buildGeomFromPoint( p_data, (gaiaPointPtr)Q);  }
pointzm(P) ::= VANUATU_POINT_ZM VANUATU_OPEN_BRACKET point_coordxyzm(Q) VANUATU_CLOSE_BRACKET. 
	{ P = vanuatu_buildGeomFromPoint( p_data, (gaiaPointPtr)Q);  }

// Point coordinates in different dimensions: MultiPoint((pt),(pt))
// Create the point by calling the proper function in SpatiaLite :
point_brkt_coordxy(P) ::= VANUATU_OPEN_BRACKET coord(X) coord(Y) VANUATU_CLOSE_BRACKET.  
	{ P = (void *) vanuatu_point_xy( p_data, (double *)X, (double *)Y); }
point_brkt_coordxym(P) ::= VANUATU_OPEN_BRACKET coord(X) coord(Y) coord(M) VANUATU_CLOSE_BRACKET.  
	{ P = (void *) vanuatu_point_xym( p_data, (double *)X, (double *)Y, (double *)M); }
point_brkt_coordxyz(P) ::= VANUATU_OPEN_BRACKET coord(X) coord(Y) coord(Z) VANUATU_CLOSE_BRACKET.  
	{ P = (void *) vanuatu_point_xyz( p_data, (double *)X, (double *)Y, (double *)Z); }
point_brkt_coordxyzm(P) ::= VANUATU_OPEN_BRACKET coord(X) coord(Y) coord(Z) coord(M) VANUATU_CLOSE_BRACKET.  
	{ P = (void *) vanuatu_point_xyzm( p_data, (double *)X, (double *)Y, (double *)Z, (double *)M); }

// Point coordinates in different dimensions.
// Create the point by calling the proper function in SpatiaLite :
point_coordxy(P) ::= coord(X) coord(Y). 
	{ P = (void *) vanuatu_point_xy( p_data, (double *)X, (double *)Y); }
point_coordxym(P) ::= coord(X) coord(Y) coord(M). 
	{ P = (void *) vanuatu_point_xym( p_data, (double *)X, (double *)Y, (double *)M); }
point_coordxyz(P) ::= coord(X) coord(Y) coord(Z). 
	{ P = (void *) vanuatu_point_xyz( p_data, (double *)X, (double *)Y, (double *)Z); }
point_coordxyzm(P) ::= coord(X) coord(Y) coord(Z) coord(M). 
	{ P = (void *) vanuatu_point_xyzm( p_data, (double *)X, (double *)Y, (double *)Z, (double *)M); }

// All coordinates are assumed to be doubles (guaranteed by the flex tokenizer).
coord(A) ::= VANUATU_NUM(B). { A = B; } 


// Rules to match an infinite number of points: MultiPoint((pt), (pt))
// Also links the generated gaiaPointPtrs together
extra_brkt_pointsxy(A) ::=  . { A = NULL; }
extra_brkt_pointsxy(A) ::= VANUATU_COMMA point_brkt_coordxy(P) extra_brkt_pointsxy(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_brkt_pointsxym(A) ::=  . { A = NULL; }
extra_brkt_pointsxym(A) ::= VANUATU_COMMA point_brkt_coordxym(P) extra_brkt_pointsxym(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_brkt_pointsxyz(A) ::=  .  { A = NULL; }
extra_brkt_pointsxyz(A) ::= VANUATU_COMMA point_brkt_coordxyz(P) extra_brkt_pointsxyz(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_brkt_pointsxyzm(A) ::=  .  { A = NULL; }
extra_brkt_pointsxyzm(A) ::= VANUATU_COMMA point_brkt_coordxyzm(P) extra_brkt_pointsxyzm(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }


// Rules to match an infinite number of points:
// Also links the generated gaiaPointPtrs together
extra_pointsxy(A) ::=  . { A = NULL; }
extra_pointsxy(A) ::= VANUATU_COMMA point_coordxy(P) extra_pointsxy(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_pointsxym(A) ::=  . { A = NULL; }
extra_pointsxym(A) ::= VANUATU_COMMA point_coordxym(P) extra_pointsxym(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_pointsxyz(A) ::=  .  { A = NULL; }
extra_pointsxyz(A) ::= VANUATU_COMMA point_coordxyz(P) extra_pointsxyz(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }

extra_pointsxyzm(A) ::=  .  { A = NULL; }
extra_pointsxyzm(A) ::= VANUATU_COMMA point_coordxyzm(P) extra_pointsxyzm(B).
	{ ((gaiaPointPtr)P)->Next = (gaiaPointPtr)B;  A = P; }


// Syntax for a "linestring" object:
// The functions called build a geometry collection from a gaiaLinestringPtr
linestring(L) ::= VANUATU_LINESTRING linestring_text(X). 
	{ L = vanuatu_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); }
linestringm(L) ::= VANUATU_LINESTRING_M linestring_textm(X).
	{ L = vanuatu_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); }
linestringz(L) ::= VANUATU_LINESTRING_Z linestring_textz(X).
	{ L = vanuatu_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); }
linestringzm(L) ::= VANUATU_LINESTRING_ZM linestring_textzm(X).
	{ L = vanuatu_buildGeomFromLinestring( p_data, (gaiaLinestringPtr)X); }

// A valid linestring must have at least two vertices:
// The functions called build a gaiaLinestring from a linked list of points
linestring_text(L) ::= VANUATU_OPEN_BRACKET point_coordxy(P) VANUATU_COMMA point_coordxy(Q) extra_pointsxy(R) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)Q)->Next = (gaiaPointPtr)R; 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q;
	   L = (void *) vanuatu_linestring_xy( p_data, (gaiaPointPtr)P);
	}

linestring_textm(L) ::= VANUATU_OPEN_BRACKET point_coordxym(P) VANUATU_COMMA point_coordxym(Q) extra_pointsxym(R) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)Q)->Next = (gaiaPointPtr)R; 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q;
	   L = (void *) vanuatu_linestring_xym( p_data, (gaiaPointPtr)P);
	}

linestring_textz(L) ::= VANUATU_OPEN_BRACKET point_coordxyz(P) VANUATU_COMMA point_coordxyz(Q) extra_pointsxyz(R) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)Q)->Next = (gaiaPointPtr)R; 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q;
	   L = (void *) vanuatu_linestring_xyz( p_data, (gaiaPointPtr)P);
	}

linestring_textzm(L) ::= VANUATU_OPEN_BRACKET point_coordxyzm(P) VANUATU_COMMA point_coordxyzm(Q) extra_pointsxyzm(R) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)Q)->Next = (gaiaPointPtr)R; 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q;
	   L = (void *) vanuatu_linestring_xyzm( p_data, (gaiaPointPtr)P);
	}


// Syntax for a "polygon" object:
// The functions called build a geometry collection from a gaiaPolygonPtr
polygon(P) ::= VANUATU_POLYGON polygon_text(X).
	{ P = vanuatu_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); }
polygonm(P) ::= VANUATU_POLYGON_M polygon_textm(X).
	{ P = vanuatu_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); }
polygonz(P) ::= VANUATU_POLYGON_Z polygon_textz(X).
	{ P = vanuatu_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); }
polygonzm(P) ::= VANUATU_POLYGON_ZM polygon_textzm(X).
	{ P = vanuatu_buildGeomFromPolygon( p_data, (gaiaPolygonPtr)X); }

// A valid polygon must have at least one ring:
// The functions called build a gaiaPolygonPtr from a linked list of gaiaRingPtrs
polygon_text(P) ::= VANUATU_OPEN_BRACKET ring(R) extra_rings(E) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaRingPtr)R)->Next = (gaiaRingPtr)E;
		P = (void *) vanuatu_polygon_xy( p_data, (gaiaRingPtr)R);
	}

polygon_textm(P) ::= VANUATU_OPEN_BRACKET ringm(R) extra_ringsm(E) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaRingPtr)R)->Next = (gaiaRingPtr)E;
		P = (void *) vanuatu_polygon_xym( p_data, (gaiaRingPtr)R);
	}

polygon_textz(P) ::= VANUATU_OPEN_BRACKET ringz(R) extra_ringsz(E) VANUATU_CLOSE_BRACKET.
	{  
		((gaiaRingPtr)R)->Next = (gaiaRingPtr)E;
		P = (void *) vanuatu_polygon_xyz( p_data, (gaiaRingPtr)R);
	}

polygon_textzm(P) ::= VANUATU_OPEN_BRACKET ringzm(R) extra_ringszm(E) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaRingPtr)R)->Next = (gaiaRingPtr)E;
		P = (void *) vanuatu_polygon_xyzm( p_data, (gaiaRingPtr)R);
	}

// A valid ring must have at least 4 points
// The functions called build a gaiaRingPtr from a linked list of gaiaPointPtrs
ring(R) ::= VANUATU_OPEN_BRACKET point_coordxy(A) VANUATU_COMMA point_coordxy(B) VANUATU_COMMA point_coordxy(C) VANUATU_COMMA point_coordxy(D) extra_pointsxy(E) VANUATU_CLOSE_BRACKET.
	{
		((gaiaPointPtr)A)->Next = (gaiaPointPtr)B; 
		((gaiaPointPtr)B)->Next = (gaiaPointPtr)C;
		((gaiaPointPtr)C)->Next = (gaiaPointPtr)D; 
		((gaiaPointPtr)D)->Next = (gaiaPointPtr)E;
		R = (void *) vanuatu_ring_xy( p_data, (gaiaPointPtr)A);
	}

// To match more than one 2D ring:
extra_rings(R) ::=  . { R = NULL; }
extra_rings(R) ::= VANUATU_COMMA ring(S) extra_rings(T).
	{
		((gaiaRingPtr)S)->Next = (gaiaRingPtr)T;
		R = S;
	}
	
ringm(R) ::= VANUATU_OPEN_BRACKET point_coordxym(A) VANUATU_COMMA point_coordxym(B) VANUATU_COMMA point_coordxym(C) VANUATU_COMMA point_coordxym(D) extra_pointsxym(E) VANUATU_CLOSE_BRACKET.
	{
		((gaiaPointPtr)A)->Next = (gaiaPointPtr)B; 
		((gaiaPointPtr)B)->Next = (gaiaPointPtr)C;
		((gaiaPointPtr)C)->Next = (gaiaPointPtr)D; 
		((gaiaPointPtr)D)->Next = (gaiaPointPtr)E;
		R = (void *) vanuatu_ring_xym( p_data, (gaiaPointPtr)A);
	}

// To match more than one 2D (with a measure) ring:
extra_ringsm(R) ::=  . { R = NULL; }
extra_ringsm(R) ::= VANUATU_COMMA ringm(S) extra_ringsm(T).
	{
		((gaiaRingPtr)S)->Next = (gaiaRingPtr)T;
		R = S;
	}
	
ringz(R) ::= VANUATU_OPEN_BRACKET point_coordxyz(A) VANUATU_COMMA point_coordxyz(B) VANUATU_COMMA point_coordxyz(C) VANUATU_COMMA point_coordxyz(D) extra_pointsxyz(E) VANUATU_CLOSE_BRACKET.
	{
		((gaiaPointPtr)A)->Next = (gaiaPointPtr)B; 
		((gaiaPointPtr)B)->Next = (gaiaPointPtr)C;
		((gaiaPointPtr)C)->Next = (gaiaPointPtr)D; 
		((gaiaPointPtr)D)->Next = (gaiaPointPtr)E;
		R = (void *) vanuatu_ring_xyz( p_data, (gaiaPointPtr)A);
	}

// To match more than one 3D ring:
extra_ringsz(R) ::=  . { R = NULL; }
extra_ringsz(R) ::= VANUATU_COMMA ringz(S) extra_ringsz(T).
	{
		((gaiaRingPtr)S)->Next = (gaiaRingPtr)T;
		R = S;
	}

ringzm(R) ::= VANUATU_OPEN_BRACKET point_coordxyzm(A) VANUATU_COMMA point_coordxyzm(B) VANUATU_COMMA point_coordxyzm(C) VANUATU_COMMA point_coordxyzm(D) extra_pointsxyzm(E) VANUATU_CLOSE_BRACKET.
	{
		((gaiaPointPtr)A)->Next = (gaiaPointPtr)B; 
		((gaiaPointPtr)B)->Next = (gaiaPointPtr)C;
		((gaiaPointPtr)C)->Next = (gaiaPointPtr)D; 
		((gaiaPointPtr)D)->Next = (gaiaPointPtr)E;
		R = (void *) vanuatu_ring_xyzm( p_data, (gaiaPointPtr)A);
	}
	
// To match more than one 3D (with a measure) ring:
extra_ringszm(R) ::=  . { R = NULL; }
extra_ringszm(R) ::= VANUATU_COMMA ringzm(S) extra_ringszm(T).
	{
		((gaiaRingPtr)S)->Next = (gaiaRingPtr)T;
		R = S;
	}

// Syntax for a "multipoint" object:
// X in the following lines is a geometry collection containing a multipoint
multipoint(M) ::= VANUATU_MULTIPOINT multipoint_text(X). { M = X; }
multipointm(M) ::= VANUATU_MULTIPOINT_M multipoint_textm(X). { M = X; }
multipointz(M) ::= VANUATU_MULTIPOINT_Z multipoint_textz(X). { M = X; }
multipointzm(M) ::= VANUATU_MULTIPOINT_ZM multipoint_textzm(X). { M = X; }

// Multipoints can contain any number of points (but at least one):
// The functions called build a geometry collection containing a multipoint
multipoint_text(M) ::= VANUATU_OPEN_BRACKET point_coordxy(P) extra_pointsxy(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) vanuatu_multipoint_xy( p_data, (gaiaPointPtr)P);
	}
multipoint_textm(M) ::= VANUATU_OPEN_BRACKET point_coordxym(P) extra_pointsxym(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) vanuatu_multipoint_xym( p_data, (gaiaPointPtr)P);
	}
multipoint_textz(M) ::= VANUATU_OPEN_BRACKET point_coordxyz(P) extra_pointsxyz(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) vanuatu_multipoint_xyz( p_data, (gaiaPointPtr)P);
	}
multipoint_textzm(M) ::= VANUATU_OPEN_BRACKET point_coordxyzm(P) extra_pointsxyzm(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) vanuatu_multipoint_xyzm( p_data, (gaiaPointPtr)P);
	}
multipoint_text(M) ::= VANUATU_OPEN_BRACKET point_brkt_coordxy(P) extra_brkt_pointsxy(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) vanuatu_multipoint_xy( p_data, (gaiaPointPtr)P);
	}
multipoint_textm(M) ::= VANUATU_OPEN_BRACKET point_brkt_coordxym(P) extra_brkt_pointsxym(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) vanuatu_multipoint_xym( p_data, (gaiaPointPtr)P);
	}
multipoint_textz(M) ::= VANUATU_OPEN_BRACKET point_brkt_coordxyz(P) extra_brkt_pointsxyz(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) vanuatu_multipoint_xyz( p_data, (gaiaPointPtr)P);
	}
multipoint_textzm(M) ::= VANUATU_OPEN_BRACKET point_brkt_coordxyzm(P) extra_brkt_pointsxyzm(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPointPtr)P)->Next = (gaiaPointPtr)Q; 
	   M = (void *) vanuatu_multipoint_xyzm( p_data, (gaiaPointPtr)P);
	}


// Syntax for a "multilinestring" object:
// X in the following lines refers to a geometry collection containing a multilinestring
multilinestring(M) ::= VANUATU_MULTILINESTRING multilinestring_text(X). { M = X; }
multilinestringm(M) ::= VANUATU_MULTILINESTRING_M multilinestring_textm(X). { M = X; }
multilinestringz(M) ::= VANUATU_MULTILINESTRING_Z multilinestring_textz(X). { M = X; }
multilinestringzm(M) ::= VANUATU_MULTILINESTRING_ZM multilinestring_textzm(X). { M = X; }

// Multilinestrings can contain any number of linestrings (but at least one):
// The functions called build a geometry collection containing a multilinestring
multilinestring_text(M) ::= VANUATU_OPEN_BRACKET linestring_text(L) multilinestring_text2(X) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)X; 
	   M = (void *) vanuatu_multilinestring_xy( p_data, (gaiaLinestringPtr)L);
	}

// Extra linestrings
multilinestring_text2(X) ::=  . { X = NULL; }
multilinestring_text2(X) ::= VANUATU_COMMA linestring_text(L) multilinestring_text2(Y).
	{ ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)Y;  X = L; }

multilinestring_textm(M) ::= VANUATU_OPEN_BRACKET linestring_textm(L) multilinestring_textm2(X) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)X; 
	   M = (void *) vanuatu_multilinestring_xym( p_data, (gaiaLinestringPtr)L);
	}

multilinestring_textm2(X) ::=  . { X = NULL; }
multilinestring_textm2(X) ::= VANUATU_COMMA linestring_textm(L) multilinestring_textm2(Y).
	{ ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)Y;  X = L; }

multilinestring_textz(M) ::= VANUATU_OPEN_BRACKET linestring_textz(L) multilinestring_textz2(X) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)X; 
	   M = (void *) vanuatu_multilinestring_xyz( p_data, (gaiaLinestringPtr)L);
	}

multilinestring_textz2(X) ::=  . { X = NULL; }
multilinestring_textz2(X) ::= VANUATU_COMMA linestring_textz(L) multilinestring_textz2(Y).
	{ ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)Y;  X = L; }

multilinestring_textzm(M) ::= VANUATU_OPEN_BRACKET linestring_textzm(L) multilinestring_textzm2(X) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)X; 
	   M = (void *) vanuatu_multilinestring_xyzm( p_data, (gaiaLinestringPtr)L);
	}

multilinestring_textzm2(X) ::=  . { X = NULL; }
multilinestring_textzm2(X) ::= VANUATU_COMMA linestring_textzm(L) multilinestring_textzm2(Y).
	{ ((gaiaLinestringPtr)L)->Next = (gaiaLinestringPtr)Y;  X = L; }


// Syntax for a "multipolygon" object
// X in the following lines refers to a geometry collection containing a multipolygon
multipolygon(M) ::= VANUATU_MULTIPOLYGON multipolygon_text(X). { M = X; }
multipolygonm(M) ::= VANUATU_MULTIPOLYGON_M multipolygon_textm(X). { M = X; }
multipolygonz(M) ::= VANUATU_MULTIPOLYGON_Z multipolygon_textz(X). { M = X; }
multipolygonzm(M) ::= VANUATU_MULTIPOLYGON_ZM multipolygon_textzm(X). { M = X; }

// Multipolygons can contain any number of polygons (but at least one):
// The functions called build a geometry collection containing a multipolygon
multipolygon_text(M) ::= VANUATU_OPEN_BRACKET polygon_text(P) multipolygon_text2(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)Q; 
	   M = (void *) vanuatu_multipolygon_xy( p_data, (gaiaPolygonPtr)P);
	}

// Extra polygons
multipolygon_text2(Q) ::=  . { Q = NULL; }
multipolygon_text2(A) ::= VANUATU_COMMA polygon_text(P) multipolygon_text2(B).
	{ ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)B;  A = P; }

multipolygon_textm(M) ::= VANUATU_OPEN_BRACKET polygon_textm(P) multipolygon_textm2(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)Q; 
	   M = (void *) vanuatu_multipolygon_xym( p_data, (gaiaPolygonPtr)P);
	}

multipolygon_textm2(Q) ::=  . { Q = NULL; }
multipolygon_textm2(A) ::= VANUATU_COMMA polygon_textm(P) multipolygon_textm2(B).
	{ ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)B;  A = P; }

multipolygon_textz(M) ::= VANUATU_OPEN_BRACKET polygon_textz(P) multipolygon_textz2(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)Q; 
	   M = (void *) vanuatu_multipolygon_xyz( p_data, (gaiaPolygonPtr)P);
	}

multipolygon_textz2(Q) ::=  . { Q = NULL; }
multipolygon_textz2(A) ::= VANUATU_COMMA polygon_textz(P) multipolygon_textz2(B).
	{ ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)B;  A = P; }

multipolygon_textzm(M) ::= VANUATU_OPEN_BRACKET polygon_textzm(P) multipolygon_textzm2(Q) VANUATU_CLOSE_BRACKET.
	{ 
	   ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)Q; 
	   M = (void *) vanuatu_multipolygon_xyzm( p_data, (gaiaPolygonPtr)P);
	}

multipolygon_textzm2(Q) ::=  . { Q = NULL; }
multipolygon_textzm2(A) ::= VANUATU_COMMA polygon_textzm(P) multipolygon_textzm2(B).
	{ ((gaiaPolygonPtr)P)->Next = (gaiaPolygonPtr)B;  A = P; }


// Syntax for a "geometrycollection" object:
// X in the following lines refers to a geometry collection generated based on user input
geocoll(G) ::= VANUATU_GEOMETRYCOLLECTION geocoll_text(X). { G = X; }
geocollm(G) ::= VANUATU_GEOMETRYCOLLECTION_M geocoll_textm(X). { G = X; }
geocollz(G) ::= VANUATU_GEOMETRYCOLLECTION_Z geocoll_textz(X). { G = X; }
geocollzm(G) ::= VANUATU_GEOMETRYCOLLECTION_ZM geocoll_textzm(X). { G = X; }

// Geometry collections can contain any number of points, linestrings, or polygons (but at least one):
geocoll_text(G) ::= VANUATU_OPEN_BRACKET point(P) geocoll_text2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xy( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_text(G) ::= VANUATU_OPEN_BRACKET linestring(L) geocoll_text2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xy( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_text(G) ::= VANUATU_OPEN_BRACKET polygon(P) geocoll_text2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xy( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_text(G) ::= VANUATU_OPEN_BRACKET multipoint(P) geocoll_text2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xy( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_text(G) ::= VANUATU_OPEN_BRACKET multilinestring(L) geocoll_text2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xy( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_text(G) ::= VANUATU_OPEN_BRACKET multipolygon(P) geocoll_text2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xy( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_text(G) ::= VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION geocoll_text(C) geocoll_text2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xy( p_data, (gaiaGeomCollPtr)C);
	}

// Extra points, linestrings, or polygons
geocoll_text2(X) ::=  . { X = NULL; }
geocoll_text2(X) ::= VANUATU_COMMA point(P) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_text2(X) ::= VANUATU_COMMA linestring(L) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_text2(X) ::= VANUATU_COMMA polygon(P) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_text2(X) ::= VANUATU_COMMA multipoint(P) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_text2(X) ::= VANUATU_COMMA multilinestring(L) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_text2(X) ::= VANUATU_COMMA multipolygon(P) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_text2(X) ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION geocoll_text(C) geocoll_text2(Y).
	{
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)Y;
		X = C;
	}


geocoll_textm(G) ::= VANUATU_OPEN_BRACKET pointm(P) geocoll_textm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xym( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textm(G) ::= VANUATU_OPEN_BRACKET linestringm(L) geocoll_textm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xym( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textm(G) ::= VANUATU_OPEN_BRACKET polygonm(P) geocoll_textm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xym( p_data, (gaiaGeomCollPtr)P);
	}
geocoll_textm(G) ::= VANUATU_OPEN_BRACKET multipointm(P) geocoll_textm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xym( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textm(G) ::= VANUATU_OPEN_BRACKET multilinestringm(L) geocoll_textm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xym( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textm(G) ::= VANUATU_OPEN_BRACKET multipolygonm(P) geocoll_textm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xym( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textm(G) ::= 
VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION_M geocoll_textm(C) geocoll_textm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xym( p_data, (gaiaGeomCollPtr)C);
	}

geocoll_textm2(X) ::=  . { X = NULL; }
geocoll_textm2(X) ::= VANUATU_COMMA pointm(P) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textm2(X) ::= VANUATU_COMMA linestringm(L) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textm2(X) ::= VANUATU_COMMA polygonm(P) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textm2(X) ::= VANUATU_COMMA multipointm(P) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textm2(X) ::= VANUATU_COMMA multilinestringm(L) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textm2(X) ::= VANUATU_COMMA multipolygonm(P) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textm2(X) ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION_M geocoll_textm(C) geocoll_textm2(Y).
	{
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)Y;
		X = C;
	}
	

geocoll_textz(G) ::= VANUATU_OPEN_BRACKET pointz(P) geocoll_textz2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyz( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textz(G) ::= VANUATU_OPEN_BRACKET linestringz(L) geocoll_textz2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyz( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textz(G) ::= VANUATU_OPEN_BRACKET polygonz(P) geocoll_textz2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyz( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textz(G) ::= VANUATU_OPEN_BRACKET multipointz(P) geocoll_textz2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyz( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textz(G) ::= VANUATU_OPEN_BRACKET multilinestringz(L) geocoll_textz2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyz( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textz(G) ::= VANUATU_OPEN_BRACKET multipolygonz(P) geocoll_textz2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyz( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textz(G) ::= 
VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION_Z geocoll_textz(C) geocoll_textz2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyz( p_data, (gaiaGeomCollPtr)C);
	}

geocoll_textz2(X) ::=  . { X = NULL; }
geocoll_textz2(X) ::= VANUATU_COMMA pointz(P) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textz2(X) ::= VANUATU_COMMA linestringz(L) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textz2(X) ::= VANUATU_COMMA polygonz(P) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textz2(X) ::= VANUATU_COMMA multipointz(P) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textz2(X) ::= VANUATU_COMMA multilinestringz(L) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textz2(X) ::= VANUATU_COMMA multipolygonz(P) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textz2(X) ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION_Z geocoll_textz(C) geocoll_textz2(Y).
	{
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)Y;
		X = C;
	}
	
	
geocoll_textzm(G) ::= VANUATU_OPEN_BRACKET pointzm(P) geocoll_textzm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyzm( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textzm(G) ::= VANUATU_OPEN_BRACKET linestringzm(L) geocoll_textzm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyzm( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textzm(G) ::= VANUATU_OPEN_BRACKET polygonzm(P) geocoll_textzm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyzm( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textzm(G) ::= VANUATU_OPEN_BRACKET multipointzm(P) geocoll_textzm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyzm( p_data, (gaiaGeomCollPtr)P);
	}
	
geocoll_textzm(G) ::= VANUATU_OPEN_BRACKET multilinestringzm(L) geocoll_textzm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyzm( p_data, (gaiaGeomCollPtr)L);
	}
	
geocoll_textzm(G) ::= VANUATU_OPEN_BRACKET multipolygonzm(P) geocoll_textzm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyzm( p_data, (gaiaGeomCollPtr)P);
	}

geocoll_textzm(G) ::= 
VANUATU_OPEN_BRACKET VANUATU_GEOMETRYCOLLECTION_ZM geocoll_textzm(C) geocoll_textzm2(X) VANUATU_CLOSE_BRACKET.
	{ 
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)X;
		G = (void *) vanuatu_geomColl_xyzm( p_data, (gaiaGeomCollPtr)C);
	}

geocoll_textzm2(X) ::=  . { X = NULL; }
geocoll_textzm2(X) ::= VANUATU_COMMA pointzm(P) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textzm2(X) ::= VANUATU_COMMA linestringzm(L) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textzm2(X) ::= VANUATU_COMMA polygonzm(P) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textzm2(X) ::= VANUATU_COMMA multipointzm(P) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}
	
geocoll_textzm2(X) ::= VANUATU_COMMA multilinestringzm(L) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)L)->Next = (gaiaGeomCollPtr)Y;
		X = L;
	}
	
geocoll_textzm2(X) ::= VANUATU_COMMA multipolygonzm(P) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)P)->Next = (gaiaGeomCollPtr)Y;
		X = P;
	}

geocoll_textzm2(X) ::= VANUATU_COMMA VANUATU_GEOMETRYCOLLECTION_ZM geocoll_textzm(C) geocoll_textzm2(Y).
	{
		((gaiaGeomCollPtr)C)->Next = (gaiaGeomCollPtr)Y;
		X = C;
	}


/******************************************************************************
** This is the end of the code that was created by Team Vanuatu 
** of The University of Toronto.

Authors:
Ruppi Rana			ruppi.rana@gmail.com
Dev Tanna			dev.tanna@gmail.com
Elias Adum			elias.adum@gmail.com
Benton Hui			benton.hui@gmail.com
Abhayan Sundararajan		abhayan@gmail.com
Chee-Lun Michael Stephen Cho	cheelun.cho@gmail.com
Nikola Banovic			nikola.banovic@gmail.com
Yong Jian			yong.jian@utoronto.ca

Supervisor:
Greg Wilson			gvwilson@cs.toronto.ca

-------------------------------------------------------------------------------
*/
