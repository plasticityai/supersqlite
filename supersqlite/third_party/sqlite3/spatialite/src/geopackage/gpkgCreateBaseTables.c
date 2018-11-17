/*

    GeoPackage extensions for SpatiaLite / SQLite
 
Version: MPL 1.1/GPL 2.0/LGPL 2.1

The contents of this file are subject to the Mozilla Public License Version
1.1 (the "License"); you may not use this file except in compliance with
the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/
 
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is GeoPackage Extensions

The Initial Developer of the Original Code is Brad Hards (bradh@frogmouth.net)
 
Portions created by the Initial Developer are Copyright (C) 2012-2015
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


#include "spatialite/geopackage.h"
#include "config.h"
#include "geopackage_internal.h"

#define GAIA_UNUSED() if (argc || argv) argc = argc;

#ifdef ENABLE_GEOPACKAGE

GEOPACKAGE_PRIVATE void
fnct_gpkgCreateBaseTables (sqlite3_context * context, int argc
			   __attribute__ ((unused)), sqlite3_value ** argv)
{
/* SQL function:
/ gpkgCreateBaseTables()
/
/ Create base tables for an "empty" GeoPackage
/ returns nothing on success, raises exception on error
/
*/
    char *sql_stmt = NULL;
    sqlite3 *sqlite = NULL;
    char *errMsg = NULL;
    int ret = 0;
    int i = 0;

    const char *tableSchemas[] = {
	"PRAGMA application_id = 1196437808",

	/* GeoPackage specification Table 18 */
	"CREATE TABLE gpkg_spatial_ref_sys (\n"
	    "srs_name TEXT NOT NULL,\n"
	    "srs_id INTEGER NOT NULL PRIMARY KEY,\n"
	    "organization TEXT NOT NULL,\n"
	    "organization_coordsys_id INTEGER NOT NULL,\n"
	    "definition TEXT NOT NULL,\n" "description TEXT\n" ")",

	/* GeoPackage Section 1.1.2.1.2 */
	"INSERT INTO gpkg_spatial_ref_sys (srs_name, srs_id, organization, organization_coordsys_id, definition) VALUES ('Undefined Cartesian', -1, 'NONE', -1, 'Undefined')",
	"INSERT INTO gpkg_spatial_ref_sys (srs_name, srs_id, organization, organization_coordsys_id, definition) VALUES ('Undefined Geographic', 0, 'NONE', 0, 'Undefined')",
	"INSERT INTO gpkg_spatial_ref_sys (srs_name, srs_id, organization, organization_coordsys_id, definition) VALUES ('WGS84', 4326, 'epsg', 4326, 'GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]')",

	/* GeoPackage specification Table 4 / Table 21 */
	/* Note that NULL is distinct on UNIQUE columns, so identifier definition isn't quite that far out-there */
	"CREATE TABLE gpkg_contents (\n"
	    "table_name TEXT NOT NULL PRIMARY KEY,\n"
	    "data_type TEXT NOT NULL,\n"
	    "identifier TEXT UNIQUE,\n"
	    "description TEXT DEFAULT '',\n"
	    "last_change DATETIME NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ',CURRENT_TIMESTAMP)),\n"
	    "min_x DOUBLE,\n"
	    "min_y DOUBLE,\n"
	    "max_x DOUBLE,\n"
	    "max_y DOUBLE,\n"
	    "srs_id INTEGER,\n"
	    "CONSTRAINT fk_gc_r_srid FOREIGN KEY (srs_id) REFERENCES gpkg_spatial_ref_sys(srs_id))",

	/* GeoPackage specification Table 6 / Table 22 */
	"CREATE TABLE gpkg_geometry_columns (\n"
	    "table_name TEXT NOT NULL,\n"
	    "column_name TEXT NOT NULL,\n"
	    "geometry_type_name TEXT NOT NULL,\n"
	    "srs_id INTEGER NOT NULL,\n"
	    "z TINYINT NOT NULL,\n"
	    "m TINYINT NOT NULL,\n"
	    "CONSTRAINT pk_geom_cols PRIMARY KEY (table_name, column_name),\n"
	    "CONSTRAINT uk_gc_table_name UNIQUE (table_name),\n"
	    "CONSTRAINT fk_gc_tn FOREIGN KEY (table_name) REFERENCES gpkg_contents(table_name),\n"
	    "CONSTRAINT fk_gc_srs FOREIGN KEY (srs_id) REFERENCES gpkg_spatial_ref_sys (srs_id))",

	/* GeoPackage specification Table 8 / Table 26 */
	"CREATE TABLE gpkg_tile_matrix_set (\n"
	    "table_name TEXT NOT NULL PRIMARY KEY,\n"
	    "srs_id INTEGER NOT NULL,\n"
	    "min_x DOUBLE NOT NULL,\n"
	    "min_y DOUBLE NOT NULL,\n"
	    "max_x DOUBLE NOT NULL,\n"
	    "max_y DOUBLE NOT NULL,\n"
	    "CONSTRAINT fk_gtms_table_name FOREIGN KEY (table_name) REFERENCES gpkg_contents(table_name),\n"
	    "CONSTRAINT fk_gtms_srs FOREIGN KEY (srs_id) REFERENCES gpkg_spatial_ref_sys (srs_id))",

	/* Geopackage specification Table 9 / Table 27 */
	/* TODO: figure out if the defaults are required - https://github.com/opengis/geopackage/issues/67 */
	"CREATE TABLE gpkg_tile_matrix (\n"
	    "table_name TEXT NOT NULL,\n"
	    "zoom_level INTEGER NOT NULL,\n"
	    "matrix_width INTEGER NOT NULL,\n"
	    "matrix_height INTEGER NOT NULL,\n"
	    "tile_width INTEGER NOT NULL,\n"
	    "tile_height INTEGER NOT NULL,\n"
	    "pixel_x_size DOUBLE NOT NULL,\n"
	    "pixel_y_size DOUBLE NOT NULL,\n"
	    "CONSTRAINT pk_ttm PRIMARY KEY (table_name, zoom_level),\n"
	    "CONSTRAINT fk_tmm_table_name FOREIGN KEY (table_name) REFERENCES gpkg_contents(table_name))",

	/* GeoPackage specification Table 11 / Table 31 */
	"CREATE TABLE gpkg_data_columns (\n"
	    "table_name TEXT NOT NULL,\n"
	    "column_name TEXT NOT NULL,\n"
	    "name TEXT,\n"
	    "title TEXT,\n"
	    "description TEXT,\n"
	    "mime_type TEXT,\n"
	    "constraint_name TEXT,\n"
	    "CONSTRAINT pk_gdc PRIMARY KEY (table_name, column_name),\n"
	    "CONSTRAINT fk_gdc_tn FOREIGN KEY (table_name) REFERENCES gpkg_contents(table_name))",

	/* GeoPackage specification Table 12 / Table 32 */
	"CREATE TABLE gpkg_data_column_constraints (\n"
	    "constraint_name TEXT NOT NULL,\n"
	    "constraint_type TEXT NOT NULL,\n"
	    "value TEXT,\n"
	    "min NUMERIC,\n"
	    "minIsInclusive BOOLEAN,\n"
	    "max NUMERIC,\n"
	    "maxIsInclusive BOOLEAN,\n"
	    "description TEXT,\n"
	    "CONSTRAINT gdcc_ntv UNIQUE (constraint_name, constraint_type, value))",

	/* GeoPackage specification Table 14 / Table 33 */
	"CREATE TABLE gpkg_metadata (\n"
	    "id INTEGER CONSTRAINT m_pk PRIMARY KEY ASC NOT NULL UNIQUE,\n"
	    "md_scope TEXT NOT NULL DEFAULT 'dataset',\n"
	    "md_standard_uri TEXT NOT NULL,\n"
	    "mime_type TEXT NOT NULL DEFAULT 'text/xml',\n"
	    "metadata TEXT NOT NULL)",

	/* GeoPackage specification Table 16 / Table 34 */
	"CREATE TABLE gpkg_metadata_reference (\n"
	    "reference_scope TEXT NOT NULL,\n"
	    "table_name TEXT,\n"
	    "column_name TEXT,\n"
	    "row_id_value INTEGER,\n"
	    "timestamp DATETIME NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ',CURRENT_TIMESTAMP)),\n"
	    "md_file_id INTEGER NOT NULL,\n"
	    "md_parent_id INTEGER,\n"
	    "CONSTRAINT crmr_mfi_fk FOREIGN KEY (md_file_id) REFERENCES gpkg_metadata(id),\n"
	    "CONSTRAINT crmr_mpi_fk FOREIGN KEY (md_parent_id) REFERENCES gpkg_metadata(id))",

	/* GeoPackage specification Table 17 / Table 36 */
	"CREATE TABLE gpkg_extensions (\n"
	    "table_name TEXT,\n"
	    "column_name TEXT,\n"
	    "extension_name TEXT NOT NULL,\n"
	    "definition TEXT NOT NULL,\n"
	    "scope TEXT NOT NULL,\n"
	    "CONSTRAINT ge_tce UNIQUE (table_name, column_name, extension_name))",

	/* Next 10 constraints are from GeoPackage specification Table 37 */
	"CREATE TRIGGER 'gpkg_tile_matrix_zoom_level_insert'\n"
	    "BEFORE INSERT ON 'gpkg_tile_matrix'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table ''gpkg_tile_matrix'' violates constraint: zoom_level cannot be less than 0')\n"
	    "WHERE (NEW.zoom_level < 0);\n" "END",

	"CREATE TRIGGER 'gpkg_tile_matrix_zoom_level_update'\n"
	    "BEFORE UPDATE of zoom_level ON 'gpkg_tile_matrix'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table ''gpkg_tile_matrix'' violates constraint: zoom_level cannot be less than 0')\n"
	    "WHERE (NEW.zoom_level < 0);\n" "END",

	"CREATE TRIGGER 'gpkg_tile_matrix_matrix_width_insert'\n"
	    "BEFORE INSERT ON 'gpkg_tile_matrix'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table ''gpkg_tile_matrix'' violates constraint: matrix_width cannot be less than 1')\n"
	    "WHERE (NEW.matrix_width < 1);\n" "END",

	"CREATE TRIGGER 'gpkg_tile_matrix_matrix_width_update'\n"
	    "BEFORE UPDATE OF matrix_width ON 'gpkg_tile_matrix'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table ''gpkg_tile_matrix'' violates constraint: matrix_width cannot be less than 1')\n"
	    "WHERE (NEW.matrix_width < 1);\n" "END",

	"CREATE TRIGGER 'gpkg_tile_matrix_matrix_height_insert'\n"
	    "BEFORE INSERT ON 'gpkg_tile_matrix'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table ''gpkg_tile_matrix'' violates constraint: matrix_height cannot be less than 1')\n"
	    "WHERE (NEW.matrix_height < 1);\n" "END",

	"CREATE TRIGGER 'gpkg_tile_matrix_matrix_height_update'\n"
	    "BEFORE UPDATE OF matrix_height ON 'gpkg_tile_matrix'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table ''gpkg_tile_matrix'' violates constraint: matrix_height cannot be less than 1')\n"
	    "WHERE (NEW.matrix_height < 1);\n" "END",

	"CREATE TRIGGER 'gpkg_tile_matrix_pixel_x_size_insert'\n"
	    "BEFORE INSERT ON 'gpkg_tile_matrix'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table ''gpkg_tile_matrix'' violates constraint: pixel_x_size must be greater than 0')\n"
	    "WHERE NOT (NEW.pixel_x_size > 0);\n" "END",

	"CREATE TRIGGER 'gpkg_tile_matrix_pixel_x_size_update'\n"
	    "BEFORE UPDATE OF pixel_x_size ON 'gpkg_tile_matrix'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table ''gpkg_tile_matrix'' violates constraint: pixel_x_size must be greater than 0')\n"
	    "WHERE NOT (NEW.pixel_x_size > 0);\n" "END",

	"CREATE TRIGGER 'gpkg_tile_matrix_pixel_y_size_insert'\n"
	    "BEFORE INSERT ON 'gpkg_tile_matrix'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table ''gpkg_tile_matrix'' violates constraint: pixel_y_size must be greater than 0')\n"
	    "WHERE NOT (NEW.pixel_y_size > 0);\n" "END",

	"CREATE TRIGGER 'gpkg_tile_matrix_pixel_y_size_update'\n"
	    "BEFORE UPDATE OF pixel_y_size ON 'gpkg_tile_matrix'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table ''gpkg_tile_matrix'' violates constraint: pixel_y_size must be greater than 0')\n"
	    "WHERE NOT (NEW.pixel_y_size > 0);\n" "END",

	/* Next two constraints are from GeoPackage specification Table 38 */
	/* Note the change from catalogue to catalog, per https://github.com/opengis/geopackage/issues/69 */
	"CREATE TRIGGER 'gpkg_metadata_md_scope_insert'\n"
	    "BEFORE INSERT ON 'gpkg_metadata'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table gpkg_metadata violates constraint: md_scope must be one of undefined | fieldSession | collectionSession | series | dataset | featureType | feature | attributeType | attribute | tile | model | catalog | schema | taxonomy | software | service | collectionHardware | nonGeographicDataset | dimensionGroup')\n"
	    "WHERE NOT(NEW.md_scope IN ('undefined','fieldSession','collectionSession','series','dataset', 'featureType','feature','attributeType','attribute','tile','model', 'catalog','schema','taxonomy','software','service', 'collectionHardware','nonGeographicDataset','dimensionGroup'));\n"
	    "END",

	"CREATE TRIGGER 'gpkg_metadata_md_scope_update'\n"
	    "BEFORE UPDATE OF 'md_scope' ON 'gpkg_metadata'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table gpkg_metadata violates constraint: md_scope must be one of undefined | fieldSession | collectionSession | series | dataset | featureType | feature | attributeType | attribute | tile | model | catalog | schema | taxonomy | software | service | collectionHardware | nonGeographicDataset | dimensionGroup')\n"
	    "WHERE NOT(NEW.md_scope IN ('undefined','fieldSession','collectionSession','series','dataset', 'featureType','feature','attributeType','attribute','tile','model', 'catalog','schema','taxonomy','software','service', 'collectionHardware','nonGeographicDataset','dimensionGroup'));\n"
	    "END",

	/* The following eight constraints are from GeoPackage specification Table 39 */
	"CREATE TRIGGER 'gpkg_metadata_reference_reference_scope_insert'\n"
	    "BEFORE INSERT ON 'gpkg_metadata_reference'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table gpkg_metadata_reference violates constraint: reference_scope must be one of \"geopackage\", \"table\", \"column\", \"row\", \"row/col\"')\n"
	    "WHERE NOT NEW.reference_scope IN ('geopackage','table','column','row','row/col');\n"
	    "END",

	"CREATE TRIGGER 'gpkg_metadata_reference_reference_scope_update'\n"
	    "BEFORE UPDATE OF 'reference_scope' ON 'gpkg_metadata_reference'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table gpkg_metadata_reference violates constraint: reference_scope must be one of \"geopackage\", \"table\", \"column\", \"row\", \"row/col\"')\n"
	    "WHERE NOT NEW.reference_scope IN ('geopackage','table','column','row','row/col');\n"
	    "END",

	"CREATE TRIGGER 'gpkg_metadata_reference_column_name_insert'\n"
	    "BEFORE INSERT ON 'gpkg_metadata_reference'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table gpkg_metadata_reference violates constraint: column name must be NULL when reference_scope is \"geopackage\", \"table\" or \"row\"')\n"
	    "WHERE (NEW.reference_scope IN ('geopackage','table','row') AND NEW.column_name IS NOT NULL);\n"
	    "SELECT RAISE(ABORT, 'insert on table gpkg_metadata_reference violates constraint: column name must be defined for the specified table when reference_scope is \"column\" or \"row/col\"')\n"
	    "WHERE (NEW.reference_scope IN ('column','row/col') AND NOT NEW.table_name IN (SELECT name FROM SQLITE_MASTER WHERE type = 'table' AND name = NEW.table_name AND sql LIKE ('%' || NEW.column_name || '%')));\n"
	    "END",

	"CREATE TRIGGER 'gpkg_metadata_reference_column_name_update'\n"
	    "BEFORE UPDATE OF column_name ON 'gpkg_metadata_reference'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table gpkg_metadata_reference violates constraint: column name must be NULL when reference_scope is \"geopackage\", \"table\" or \"row\"')\n"
	    "WHERE (NEW.reference_scope IN ('geopackage','table','row') AND NEW.column_nameIS NOT NULL);\n"
	    "SELECT RAISE(ABORT, 'update on table gpkg_metadata_reference violates constraint: column name must be defined for the specified table when reference_scope is \"column\" or \"row/col\"')\n"
	    "WHERE (NEW.reference_scope IN ('column','row/col') AND NOT NEW.table_name IN (SELECT name FROM SQLITE_MASTER WHERE type = 'table' AND name = NEW.table_name AND sql LIKE ('%' || NEW.column_name || '%')));\n"
	    "END",

	"CREATE TRIGGER 'gpkg_metadata_reference_row_id_value_insert'\n"
	    "BEFORE INSERT ON 'gpkg_metadata_reference'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table gpkg_metadata_reference violates constraint: row_id_value must be NULL when reference_scope is \"geopackage\", \"table\" or \"column\"')\n"
	    "WHERE NEW.reference_scope IN ('geopackage','table','column') AND NEW.row_id_value IS NOT NULL;\n"
	    "SELECT RAISE(ABORT, 'insert on table gpkg_metadata_reference violates constraint: row_id_value must exist in specified table when reference_scope is \"row\" or \"row/col\"')\n"
	    "WHERE NEW.reference_scope IN ('row','row/col') AND NOT EXISTS (SELECT rowid FROM (SELECT NEW.table_name AS table_name) WHERE rowid = NEW.row_id_value);\n"
	    "END",

	"CREATE TRIGGER 'gpkg_metadata_reference_row_id_value_update'\n"
	    "BEFORE UPDATE OF 'row_id_value' ON 'gpkg_metadata_reference'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table gpkg_metadata_reference violates constraint: row_id_value must be NULL when reference_scope is \"geopackage\", \"table\" or \"column\"')\n"
	    "WHERE NEW.reference_scope IN ('geopackage','table','column') AND NEW.row_id_value IS NOT NULL;\n"
	    "SELECT RAISE(ABORT, 'update on table gpkg_metadata_reference violates constraint: row_id_value must exist in specified table when reference_scope is \"row\" or \"row/col\"')\n"
	    "WHERE NEW.reference_scope IN ('row','row/col') AND NOT EXISTS (SELECT rowid FROM (SELECT NEW.table_name AS table_name) WHERE rowid = NEW.row_id_value);\n"
	    "END",

	"CREATE TRIGGER 'gpkg_metadata_reference_timestamp_insert'\n"
	    "BEFORE INSERT ON 'gpkg_metadata_reference'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table gpkg_metadata_reference violates constraint: timestamp must be a valid time in ISO 8601 \"yyyy-mm-ddThh-mm-ss.cccZ\" form')\n"
	    "WHERE NOT (NEW.timestamp GLOB '[1-2][0-9][0-9][0-9]-[0-1][0-9]-[1-3][0-9]T[0-2][0-9]:[0-5][0-9]:[0-5][0-9].[0-9][0-9][0-9]Z' AND strftime('%s',NEW.timestamp) NOT NULL);\n"
	    "END",

	"CREATE TRIGGER 'gpkg_metadata_reference_timestamp_update'\n"
	    "BEFORE UPDATE OF 'timestamp' ON 'gpkg_metadata_reference'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table gpkg_metadata_reference violates constraint: timestamp must be a valid time in ISO 8601 \"yyyy-mm-ddThh-mm-ss.cccZ\" form')\n"
	    "WHERE NOT (NEW.timestamp GLOB '[1-2][0-9][0-9][0-9]-[0-1][0-9]-[1-3][0-9]T[0-2][0-9]:[0-5][0-9]:[0-5][0-9].[0-9][0-9][0-9]Z' AND strftime('%s',NEW.timestamp) NOT NULL);\n"
	    "END",

	/* the following four constraints probably should be in the GeoPackage spec, but aren't */
	"CREATE TRIGGER 'gpkg_geometry_columns_z_insert'\n"
	    "BEFORE INSERT ON 'gpkg_geometry_columns'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table gpkg_geometry_columns violates constraint: z must be one of 0, 1 or 2')\n"
	    "WHERE NOT(NEW.z IN (0, 1, 2));\n" "END",

	"CREATE TRIGGER 'gpkg_geometry_columns_z_update'\n"
	    "BEFORE UPDATE OF 'z' ON 'gpkg_geometry_columns'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table gpkg_geometry_columns violates constraint: z must be one of 0, 1 or 2')\n"
	    "WHERE NOT NEW.z IN (0, 1, 2);\n" "END",

	"CREATE TRIGGER 'gpkg_geometry_columns_m_insert'\n"
	    "BEFORE INSERT ON 'gpkg_geometry_columns'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'insert on table gpkg_geometry_columns violates constraint: m must be one of 0, 1 or 2')\n"
	    "WHERE NOT(NEW.m IN (0, 1, 2));\n" "END",

	"CREATE TRIGGER 'gpkg_geometry_columns_m_update'\n"
	    "BEFORE UPDATE OF 'm' ON 'gpkg_geometry_columns'\n"
	    "FOR EACH ROW BEGIN\n"
	    "SELECT RAISE(ABORT, 'update on table gpkg_geometry_columns violates constraint: m must be one of 0, 1 or 2')\n"
	    "WHERE NOT NEW.m IN (0, 1, 2);\n" "END",

	NULL
    };

    GAIA_UNUSED ();		/* LCOV_EXCL_LINE */

    for (i = 0; tableSchemas[i] != NULL; ++i)
      {
	  sql_stmt = sqlite3_mprintf ("%s", tableSchemas[i]);
	  sqlite = sqlite3_context_db_handle (context);
	  ret = sqlite3_exec (sqlite, sql_stmt, NULL, NULL, &errMsg);
	  sqlite3_free (sql_stmt);
	  if (ret != SQLITE_OK)
	    {
		sqlite3_result_error (context, errMsg, -1);
		sqlite3_free (errMsg);
		return;
	    }
      }
}
#endif
