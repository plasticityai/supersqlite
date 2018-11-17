/* 
 kml.y -- KML parser - LEMON config
  
 version 2.4, 2011 June 14

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

// Header files to be included in kml.c
%include {
}

// Set the return value of gaiaParseKML in the following pointer:
%extra_argument { struct kml_data *p_data }

// Invalid syntax (ie. no rules matched)
%syntax_error {
/* 
** when the LEMON parser encounters an error
** then this global variable is set 
*/
	p_data->kml_parse_error = 1;
	p_data->result = NULL;
}
 
 /* This is to terminate with a new line */
 main ::= in.
 in ::= .
 in ::= in state KML_NEWLINE.
 
 state ::= program.
 
 /* 
 * program is the start node. 
 */
 
program ::= kml_tree.

// KML node:
kml_tree ::= node(N). { p_data->result = N; }			// N is a KML node
kml_tree ::= node_chain(C). { p_data->result = C; }		// C is a chain of KML nodes


// syntax for a KML node object:
node(N) ::= open_tag(K) KML_END KML_CLOSE.
	{ N = kml_createSelfClosedNode( p_data, (void *)K, NULL); }
node(N) ::= open_tag(K) attr(A) KML_END KML_CLOSE.
	{ N = kml_createSelfClosedNode( p_data, (void *)K, (void *)A); }
node(N) ::= open_tag(K) attributes(A) KML_END KML_CLOSE.
	{ N = kml_createSelfClosedNode( p_data, (void *)K, (void *)A); }
node(N) ::= open_tag(K) KML_CLOSE.
	{ N = kml_createNode( p_data, (void *)K, NULL, NULL); }
node(N) ::= open_tag(K) attr(A) KML_CLOSE.
	{ N = kml_createNode( p_data, (void *)K, (void *)A, NULL); }
node(N) ::= open_tag(K) attributes(A) KML_CLOSE.
	{ N = kml_createNode( p_data, (void *)K, (void *)A, NULL); }
node(N) ::= open_tag(K) KML_CLOSE coord(C).
	{ N = kml_createNode( p_data, (void *)K, NULL, (void *)C); }
node(N) ::= open_tag(K) KML_CLOSE coord_chain(C).
	{ N = kml_createNode( p_data, (void *)K, NULL, (void *)C); }
node(N) ::= open_tag(K) attr(A) KML_CLOSE coord(C).
	{ N = kml_createNode( p_data, (void *)K, (void *)A, (void *)C); }
node(N) ::= open_tag(K) attr(A) KML_CLOSE coord_chain(C).
	{ N = kml_createNode( p_data, (void *)K, (void *)A, (void *)C); }
node(N) ::= open_tag(K) attributes(A) KML_CLOSE coord(C).
	{ N = kml_createNode( p_data, (void *)K, (void *)A, (void *)C); }
node(N) ::= open_tag(K) attributes(A) KML_CLOSE coord_chain(C).
	{ N = kml_createNode( p_data, (void *)K, (void *)A, (void *)C); }
node(N) ::= close_tag(K).
	{ N = kml_closingNode( p_data, (void *)K); }
	

// syntax for a KML tag object:
open_tag(T) ::= KML_OPEN keyword(K). { T = K; }
close_tag(T) ::= KML_OPEN KML_END keyword(K) KML_CLOSE. { T = K; }
	

// Keyword.
keyword(A) ::= KML_KEYWORD(B). { A = B; }


// Rules to match an infinite number of KML nodes:
// Also links the generated kmlNodePtrs together
extra_nodes(A) ::=  . { A = NULL; }
extra_nodes(A) ::= node(P) extra_nodes(B).
	{ ((kmlNodePtr)P)->Next = (kmlNodePtr)B;  A = P; }
	

// a chain can contain any number of KML Nodes (but at least two):
node_chain(C) ::= node(A) node(B) extra_nodes(Q).
	{ 
	   ((kmlNodePtr)B)->Next = (kmlNodePtr)Q; 
	   ((kmlNodePtr)A)->Next = (kmlNodePtr)B;
	   C = A;
	}

	
// syntax for a KML attribute:
attr(A) ::= KML_KEYWORD(K) KML_EQ KML_VALUE(V).
	{ A = kml_attribute( p_data, (void *)K, (void *)V); }


// Rules to match an infinite number of KML attributes:
// Also links the generated kmlAttrPtrs together
extra_attr(A) ::=  . { A = NULL; }
extra_attr(A) ::= attr(P) extra_attr(B).
	{ ((kmlAttrPtr)P)->Next = (kmlAttrPtr)B;  A = P; }
	

// a chain can contain any number of KML Attributes (but at least two):
attributes(C) ::= attr(A) attr(B) extra_attr(Q).
	{ 
	   ((kmlAttrPtr)B)->Next = (kmlAttrPtr)Q; 
	   ((kmlAttrPtr)A)->Next = (kmlAttrPtr)B;
	   C = A;
	}

	
// syntax for a KML coordinate:
coord(C) ::= KML_COORD(V).
	{ C = kml_coord( p_data, (void *)V); }


// Rules to match an infinite number of KML coords:
// Also links the generated kmlCoordPtrs together
extra_coord(A) ::=  . { A = NULL; }
extra_coord(A) ::= coord(P) extra_coord(B).
	{ ((kmlCoordPtr)P)->Next = (kmlCoordPtr)B;  A = P; }
	

// a chain can contain any number of KML Coordinates (but at least two):
coord_chain(C) ::= coord(A) coord(B) extra_coord(Q).
	{ 
	   ((kmlCoordPtr)B)->Next = (kmlCoordPtr)Q; 
	   ((kmlCoordPtr)A)->Next = (kmlCoordPtr)B;
	   C = A;
	}
	
	
