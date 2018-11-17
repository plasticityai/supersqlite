/* 
 vanuatuLexer.l -- Vanuatu WKT parser - FLEX config
  
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

%option prefix="VanuatuWkt"
%option reentrant
%option extra-type="struct vanuatu_data *"
%option nounput
%option noinput

/**
*  The main string-token matcher.
*  The lower case part is probably not needed.  We should really be converting 
*  The string to all uppercase/lowercase to make it case iNsEnSiTiVe.
*  What Flex will do is, For the input string, beginning from the front, Flex
*  will try to match with any of the defined tokens from below.  Flex will 
*  then match the string of longest length.  Suppose the string is: POINT ZM,
*  Flex would match both POINT Z and POINT ZM, but since POINT ZM is the longer
*  of the two tokens, FLEX will match POINT ZM.
*/
%%
-?[0-9]+|-?[0-9]+"."[0-9]*|"+"?[0-9]+|"+"?[0-9]+"."[0-9]*       	{ VanuatuWktget_extra(yyscanner)->vanuatu_col += (int) strlen(yytext);  VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = atof(yytext); return VANUATU_NUM; }
","					{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_COMMA; }
"("					{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_OPEN_BRACKET; }
")"					{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_CLOSE_BRACKET; }
(?i:"point")				{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_POINT; }
(?i:"point"[ \t\n]*"z")			{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_POINT_Z; }
(?i:"point"[ \t\n]*"m")			{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_POINT_M; }
(?i:"point"[ \t\n]*"zm")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_POINT_ZM; }
(?i:"linestring")			{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_LINESTRING; }
(?i:"linestring"[ \t\n]*"z")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_LINESTRING_Z; }
(?i:"linestring"[ \t\n]*"m")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_LINESTRING_M; }
(?i:"linestring"[ \t\n]*"zm")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_LINESTRING_ZM; }
(?i:"polygon")				{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_POLYGON; }
(?i:"polygon"[ \t\n]*"z")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_POLYGON_Z; }
(?i:"polygon"[ \t\n]*"m")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_POLYGON_M; }
(?i:"polygon"[ \t\n]*"zm")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_POLYGON_ZM; }
(?i:"multipoint")			{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTIPOINT; }
(?i:"multipoint"[ \t\n]*"z")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTIPOINT_Z; }
(?i:"multipoint"[ \t\n]*"m")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTIPOINT_M; }
(?i:"multipoint"[ \t\n]*"zm")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTIPOINT_ZM; }
(?i:"multilinestring")			{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTILINESTRING; }
(?i:"multilinestring"[ \t\n]*"z")	{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTILINESTRING_Z; }
(?i:"multilinestring"[ \t\n]*"m")	{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTILINESTRING_M; }
(?i:"multilinestring"[ \t\n]*"zm")	{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTILINESTRING_ZM; }	
(?i:"multipolygon")			{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTIPOLYGON; }
(?i:"multipolygon"[ \t\n]*"z")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTIPOLYGON_Z; }
(?i:"multipolygon"[ \t\n]*"m")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTIPOLYGON_M; }
(?i:"multipolygon"[ \t\n]*"zm")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_MULTIPOLYGON_ZM; }
(?i:"geometrycollection")		{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_GEOMETRYCOLLECTION; }
(?i:"geometrycollection"[ \t\n]*"z")	{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_GEOMETRYCOLLECTION_Z; }
(?i:"geometrycollection"[ \t\n]*"m")	{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_GEOMETRYCOLLECTION_M; }
(?i:"geometrycollection"[ \t\n]*"zm")	{ VanuatuWktget_extra(yyscanner)->VanuatuWktlval.dval = 0; return VANUATU_GEOMETRYCOLLECTION_ZM; }

[ \t]   { VanuatuWktget_extra(yyscanner)->vanuatu_col += (int) strlen(yytext); }               /* ignore but count white space */

\n      { VanuatuWktget_extra(yyscanner)->vanuatu_col = 0; VanuatuWktget_extra(yyscanner)->vanuatu_line++; }

.       { VanuatuWktget_extra(yyscanner)->vanuatu_col += (int) strlen(yytext); return -1; }
%%

int yywrap(yyscan_t yyscanner )
{
  return 1;
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
