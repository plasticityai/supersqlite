/* 
 kmlLexer.l -- KML parser - FLEX config
  
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

%option prefix="Kml"
%option reentrant
%option extra-type="struct kml_data *"
%option noinput
%option nounput

/**
*  The main string-token matcher.
*  The lower case part is probably not needed.  We should really be converting 
*  The string to all uppercase/lowercase to make it case iNsEnSiTiVe.
*  What Flex will do is, For the input string, beginning from the front, Flex
*  will try to match with any of the defined tokens from below.  Flex will 
*  then match the string of longest length.  Suppose the string is: POINTM,
*  Flex would match both POINT and POINTM, but since POINTM is the longer
*  of the two tokens, FLEX will match POINTM.
*/
%%
"/"				{ kml_freeString(&(Kmlget_extra(yyscanner)->KmlLval.pval)); return KML_END; }
"="				{ kml_freeString(&(Kmlget_extra(yyscanner)->KmlLval.pval)); return KML_EQ; }
"<"				{ kml_freeString(&(Kmlget_extra(yyscanner)->KmlLval.pval)); return KML_OPEN; }
">"				{ kml_freeString(&(Kmlget_extra(yyscanner)->KmlLval.pval)); return KML_CLOSE; }
[0-9,\.\+-]*			{ kml_saveString(&(Kmlget_extra(yyscanner)->KmlLval.pval), yytext); return KML_COORD; }
\"[^<>\"]*\"			{ kml_saveString(&(Kmlget_extra(yyscanner)->KmlLval.pval), yytext); return KML_VALUE; }
[a-zA-Z_][a-zA-Z_:0-9]*		{ kml_saveString(&(Kmlget_extra(yyscanner)->KmlLval.pval), yytext); return KML_KEYWORD; }

[ \t]   { kml_freeString(&(Kmlget_extra(yyscanner)->KmlLval.pval)); Kmlget_extra(yyscanner)->kml_col += (int) strlen(yytext); }               /* ignore but count white space */

\n      { kml_freeString(&(Kmlget_extra(yyscanner)->KmlLval.pval)); Kmlget_extra(yyscanner)->kml_col = 0; Kmlget_extra(yyscanner)->kml_line++; }

.       { kml_freeString(&(Kmlget_extra(yyscanner)->KmlLval.pval)); Kmlget_extra(yyscanner)->kml_col += (int) strlen(yytext); return -1; }
%%

int yywrap(yyscan_t yyscanner )
{
  return 1;
}
