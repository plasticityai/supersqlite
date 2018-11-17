/* 
 dxf_private.h -- DXF Import Private API
  
 version 4.3, 2015 June 29

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
 
Portions created by the Initial Developer are Copyright (C) 2008-2015
the Initial Developer. All Rights Reserved.

Contributor(s):

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

/*
 
CREDITS:

inital development of the DXF module has been funded by:
Regione Toscana - Settore Sistema Informativo Territoriale ed Ambientale

*/

/**
 \file spatialite_private.h

 SpatiaLite private header file
 */
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef _WIN32
#ifdef DLL_EXPORT
#define DXF_PRIVATE
#else
#define DXF_PRIVATE
#endif
#else
#define DXF_PRIVATE __attribute__ ((visibility("hidden")))
#endif
#endif

#ifndef _DXF_PRIVATE_H
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define _DXF_PRIVATE_H
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct dxf_out_layer
    {
	double minx;
	double miny;
	double maxx;
	double maxy;
	char *layer_name;
	struct dxf_out_layer *next;
    } gaiaDxfExportLayer;
    typedef gaiaDxfExportLayer *gaiaDxfExportLayerPtr;

    typedef struct dxf_out
    {
	double minx;
	double miny;
	double maxx;
	double maxy;
	gaiaDxfExportLayer *first;
	gaiaDxfExportLayer *last;
    } gaiaDxfExport;
    typedef gaiaDxfExport *gaiaDxfExportPtr;

    DXF_PRIVATE int
	create_text_stmt (sqlite3 * handle, const char *name,
			  sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_point_stmt (sqlite3 * handle, const char *name,
			   sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_line_stmt (sqlite3 * handle, const char *name,
			  sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_polyg_stmt (sqlite3 * handle, const char *name,
			   sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_hatch_boundary_stmt (sqlite3 * handle, const char *name,
				    sqlite3_stmt ** xstmt);
    DXF_PRIVATE int
	create_hatch_pattern_stmt (sqlite3 * handle, const char *name,
				   sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_insert_stmt (sqlite3 * handle, const char *name,
			    sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_extra_stmt (sqlite3 * handle, const char *extra_name,
			   sqlite3_stmt ** xstmt);
    DXF_PRIVATE char *create_extra_attr_table_name (const char *name);

    DXF_PRIVATE int
	check_text_table (sqlite3 * handle, const char *name, int srid,
			  int is3D);

    DXF_PRIVATE int
	check_point_table (sqlite3 * handle, const char *name, int srid,
			   int is3D);

    DXF_PRIVATE int
	check_line_table (sqlite3 * handle, const char *name, int srid,
			  int is3D);

    DXF_PRIVATE int
	check_polyg_table (sqlite3 * handle, const char *name, int srid,
			   int is3D);

    DXF_PRIVATE int
	check_hatch_tables (sqlite3 * handle, const char *name, int srid);

    DXF_PRIVATE int check_insert_table (sqlite3 * handle, const char *name);

    DXF_PRIVATE int check_extra_attr_table (sqlite3 * handle, const char *name);

    DXF_PRIVATE int
	create_block_text_stmt (sqlite3 * handle, const char *name,
				sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	import_mixed (sqlite3 * handle, gaiaDxfParserPtr dxf, int append);

    DXF_PRIVATE int
	import_by_layer (sqlite3 * handle, gaiaDxfParserPtr dxf, int append);

    DXF_PRIVATE int
	create_instext_table (sqlite3 * handle, const char *name,
			      const char *block, int is3d,
			      sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_inspoint_table (sqlite3 * handle, const char *name,
			       const char *block, int is3d,
			       sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_insline_table (sqlite3 * handle, const char *name,
			      const char *block, int is3d,
			      sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_inspolyg_table (sqlite3 * handle, const char *name,
			       const char *block, int is3d,
			       sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_inshatch_table (sqlite3 * handle, const char *name,
			       const char *block, sqlite3_stmt ** xstmt);

    DXF_PRIVATE int
	create_insert_extra_attr_table (sqlite3 * handle, const char *name,
					char *extra_name,
					sqlite3_stmt ** xstmt_ext);

    DXF_PRIVATE int check_unclosed_polyg (gaiaDxfPolylinePtr pg, int is3d);

    DXF_PRIVATE int check_unclosed_hole (gaiaDxfHolePtr hole, int is3d);

#ifdef __cplusplus
}
#endif

#endif				/* _DXF_PRIVATE_H */
