
/*

 check_libxml2.c -- SpatiaLite Test Case

 Author: Sandro Furieri <a.furieri@lqt.it>

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
 
Portions created by the Initial Developer are Copyright (C) 2013
the Initial Developer. All Rights Reserved.

Contributor(s):
Brad Hards <bradh@frogmouth.net>

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "sqlite3.h"
#include "spatialite.h"

#ifdef ENABLE_LIBXML2		/* only if LIBXML2 is supported */

#define ISO_METADATA	1
#define SLD_SE_STYLE	2
#define SVG		3

static int
check_bad_xml (void *cache)
{
/* parsing a not-well-formed XML Sample */
    FILE *fl;
    int sz = 0;
    int rd;
    unsigned char *xml = NULL;
    unsigned char *p_result = NULL;
    int len;
    char *err1;
    char *err2;
    char *version = gaia_libxml2_version ();

    if (version == NULL)
      {
	  fprintf (stderr, "unable to get the library version\n");
	  return 0;
      }
    free (version);

/* loading the XMLDocument */
    fl = fopen ("books-bad.xml", "rb");
    if (!fl)
      {
	  fprintf (stderr, "cannot open \"books-bad.xml\"\n");
	  return 0;
      }
    if (fseek (fl, 0, SEEK_END) == 0)
	sz = ftell (fl);
    xml = malloc (sz);
    rewind (fl);
    rd = fread (xml, 1, sz, fl);
    if (rd != sz)
      {
	  fprintf (stderr, "read error \"books-bad.xml\"\n");
	  return 0;
      }
    fclose (fl);

/* parsing the XMLDocument */
    gaiaXmlToBlob (cache, xml, rd, 1, NULL, &p_result, &len, &err1, &err2);
    if (p_result != NULL)
      {
	  fprintf (stderr, "this is not a well-formed XML !!!\n");
	  return 0;
      }

    free (xml);
    return 1;
}

static int
check_bad_schema (void *cache)
{
/* validating by invalid Schema */
    FILE *fl;
    int sz = 0;
    int rd;
    unsigned char *xml = NULL;
    unsigned char *p_result = NULL;
    int len;
    char *err1;
    char *err2;

/* loading the XMLDocument */
    fl = fopen ("books.xml", "rb");
    if (!fl)
      {
	  fprintf (stderr, "cannot open \"books.xml\"\n");
	  return 0;
      }
    if (fseek (fl, 0, SEEK_END) == 0)
	sz = ftell (fl);
    xml = malloc (sz);
    rewind (fl);
    rd = fread (xml, 1, sz, fl);
    if (rd != sz)
      {
	  fprintf (stderr, "read error \"books.xml\"\n");
	  return 0;
      }
    fclose (fl);

/* validating the XMLDocument */
    gaiaXmlToBlob (cache, xml, rd, 1, "books-bad.xsd", &p_result, &len, &err1,
		   &err2);
    if (p_result != NULL)
      {
	  fprintf (stderr, "this is not a valid XML !!!\n");
	  return 0;
      }

    free (xml);
    return 1;
}

static int
check_validate (void *cache, const char *path)
{
/* validating an XML Sample */
    FILE *fl;
    int sz = 0;
    int rd;
    unsigned char *xml = NULL;
    char *schema_uri = NULL;
    char *schema_uri2 = NULL;
    unsigned char *p_result = NULL;
    int len;

/* loading the XMLDocument */
    fl = fopen (path, "rb");
    if (!fl)
      {
	  fprintf (stderr, "cannot open \"%s\"\n", path);
	  return 0;
      }
    if (fseek (fl, 0, SEEK_END) == 0)
	sz = ftell (fl);
    xml = malloc (sz);
    rewind (fl);
    rd = fread (xml, 1, sz, fl);
    if (rd != sz)
      {
	  fprintf (stderr, "read error \"%s\"\n", path);
	  return 0;
      }
    fclose (fl);

/* extracting the Internal SchemaURI */
    schema_uri = gaiaXmlGetInternalSchemaURI (cache, xml, rd);
    if (schema_uri == NULL)
      {
	  fprintf (stderr, "unable to identify the Schema for \"%s\"\n", path);
	  return 0;
      }
/* validating the XMLDocument */
    gaiaXmlToBlob (cache, xml, rd, 1, schema_uri, &p_result, &len, NULL, NULL);
    if (p_result == NULL)
      {
	  fprintf (stderr, "unable to validate \"%s\"\n", path);
	  return 0;
      }
    if (!gaiaIsSchemaValidatedXmlBlob (p_result, len))
      {
	  fprintf (stderr, "validation failed: \"%s\"\n", path);
	  return 0;
      }
    schema_uri2 = gaiaXmlBlobGetSchemaURI (p_result, len);
    if (schema_uri2 == NULL)
      {
	  fprintf (stderr,
		   "unable to retrieve the ValidationSchemaURI for \"%s\"\n",
		   path);
	  return 0;
      }
    if (strcmp (schema_uri, schema_uri2) != 0)
      {
	  fprintf (stderr,
		   "%s: mismatching SchemaURI \"%s\" (expected \"%s\")\n", path,
		   schema_uri2, schema_uri);
	  return 0;
      }

    free (schema_uri);
    free (schema_uri2);
    free (p_result);
    free (xml);

    return 1;
}

static int
check_extended (void *cache, const char *path, int mode)
{
/* validating an XML Sample */
    FILE *fl;
    int sz = 0;
    int rd;
    unsigned char *xml = NULL;
    int iso;
    int style;
    int svg;
    unsigned char *p_result = NULL;
    int len;
    char *file_id = NULL;
    char *parent_id = NULL;
    char *title = NULL;
    char *abstract = NULL;
    unsigned char *geom_blob;
    int geom_size;
    gaiaGeomCollPtr geom;

/* loading the XMLDocument */
    fl = fopen (path, "rb");
    if (!fl)
      {
	  fprintf (stderr, "cannot open \"%s\"\n", path);
	  return 0;
      }
    if (fseek (fl, 0, SEEK_END) == 0)
	sz = ftell (fl);
    xml = malloc (sz);
    rewind (fl);
    rd = fread (xml, 1, sz, fl);
    if (rd != sz)
      {
	  fprintf (stderr, "read error \"%s\"\n", path);
	  return 0;
      }
    fclose (fl);

/* simple parsing without validation */
    gaiaXmlToBlob (cache, xml, rd, 1, NULL, &p_result, &len, NULL, NULL);
    if (p_result == NULL)
      {
	  fprintf (stderr, "unable to parse \"%s\"\n", path);
	  return 0;
      }

/* checking the payload type */
    iso = gaiaIsIsoMetadataXmlBlob (p_result, len);
    style = gaiaIsSldSeRasterStyleXmlBlob (p_result, len);
    style = gaiaIsSldSeVectorStyleXmlBlob (p_result, len);
    svg = gaiaIsSvgXmlBlob (p_result, len);
    if (mode == ISO_METADATA && iso && !style && !svg)
	;
    else if (mode == SLD_SE_STYLE && !iso && style && !svg)
	;
    else if (mode == SVG && !iso && !style && svg)
	;
    else
      {
	  fprintf (stderr, "mismatching type: \"%s\" iso=%d style=%d svg=%d\n",
		   path, iso, style, svg);
	  return 0;
      }

/* testing ISO Metadata attributes */
    file_id = gaiaXmlBlobGetFileId (p_result, len);
    parent_id = gaiaXmlBlobGetParentId (p_result, len);
    title = gaiaXmlBlobGetTitle (p_result, len);
    abstract = gaiaXmlBlobGetAbstract (p_result, len);
    gaiaXmlBlobGetGeometry (p_result, len, &geom_blob, &geom_size);
    if (mode == ISO_METADATA)
      {
	  /* verifying ISO Metadata attributes */
	  if (file_id == NULL)
	    {
		fprintf (stderr, "unexpected NULL FileIdentifier in \"%s\"\n",
			 path);
		return 0;
	    }
	  if (strcmp (file_id, "029097fd-2ef2-487c-a5ca-6ec7a3dbac53") != 0)
	    {
		fprintf (stderr, "unexpected FileIdentifier in \"%s\" [%s]\n",
			 path, file_id);
		return 0;
	    }
	  if (parent_id == NULL)
	    {
		fprintf (stderr, "unexpected NULL ParentIdentifier in \"%s\"\n",
			 path);
		return 0;
	    }
	  if (strcmp (parent_id, "024027fd-3ef2-487c-a8ca-6ec8a3dfac57") != 0)
	    {
		fprintf (stderr, "unexpected ParentIdentifier in \"%s\" [%s]\n",
			 path, parent_id);
		return 0;
	    }
	  if (title == NULL)
	    {
		fprintf (stderr, "unexpected NULL Title in \"%s\"\n", path);
		return 0;
	    }
	  if (strcmp (title, "Image2000 Product 1 (nl2) Multispectral") != 0)
	    {
		fprintf (stderr, "unexpected Title in \"%s\" [%s]\n", path,
			 title);
		return 0;
	    }
	  if (abstract == NULL)
	    {
		fprintf (stderr, "unexpected NULL Abstract in \"%s\"\n", path);
		return 0;
	    }
	  if (strcmp
	      (abstract,
	       "IMAGE2000 product 1 individual orthorectified scenes.") != 0)
	    {
		fprintf (stderr, "unexpected Abstract in \"%s\" [%s]\n", path,
			 abstract);
		return 0;
	    }
	  if (geom_blob == NULL)
	    {
		fprintf (stderr, "unexpected NULL Geometry in \"%s\"\n", path);
		return 0;
	    }
	  geom = gaiaFromSpatiaLiteBlobWkb (geom_blob, geom_size);
	  if (geom == NULL)
	    {
		fprintf (stderr, "unexpected invalid Geometry in \"%s\"\n",
			 path);
		return 0;
	    }
	  if (geom->Srid != 4326)
	    {
		fprintf (stderr, "unexpected Geometry SRID in \"%s\" [%d]\n",
			 path, geom->Srid);
		return 0;
	    }
	  if (geom->DeclaredType != GAIA_MULTIPOLYGON)
	    {
		fprintf (stderr, "unexpected Geometry Type in \"%s\" [%d]\n",
			 path, geom->DeclaredType);
		return 0;
	    }
	  if (geom->MinX != 3.93000000)
	    {
		fprintf (stderr, "unexpected Geometry MinX in \"%s\" [%1.8f]\n",
			 path, geom->MinX);
		return 0;
	    }
	  if (geom->MinY != 52.10000000)
	    {
		fprintf (stderr, "unexpected Geometry MinY in \"%s\" [%1.8f]\n",
			 path, geom->MinY);
		return 0;
	    }
	  if (geom->MaxX != 7.57000000)
	    {
		fprintf (stderr, "unexpected Geometry MaxX in \"%s\" [%1.8f]\n",
			 path, geom->MaxX);
		return 0;
	    }
	  if (geom->MaxY != 54.10000000)
	    {
		fprintf (stderr, "unexpected Geometry MaxY in \"%s\" [%1.8f]\n",
			 path, geom->MaxY);
		return 0;
	    }
	  gaiaFreeGeomColl (geom);
      }
    else
      {
	  /* not ISO Metadata */
	  if (strcmp (path, "stazioni_se.xml") == 0)
	    {
		if (strcmp (title, "Railway Stations - blue star") != 0)
		  {
		      fprintf (stderr, "unexpected Title in \"%s\"\n", path);
		      return 0;
		  }
		if (strcmp (abstract, "a simple SE Point Symbolizer") != 0)
		  {
		      fprintf (stderr, "unexpected Abstract in \"%s\"\n", path);
		      return 0;
		  }
	    }
	  else
	    {
		if (title != NULL)
		  {
		      fprintf (stderr, "unexpected Title in \"%s\"\n", path);
		      return 0;
		  }
		if (abstract != NULL)
		  {
		      fprintf (stderr, "unexpected Abstract in \"%s\"\n", path);
		      return 0;
		  }
	    }
      }

    free (p_result);
    free (xml);
    if (file_id)
	free (file_id);
    if (parent_id)
	free (parent_id);
    if (title)
	free (title);
    if (abstract)
	free (abstract);
    if (geom_blob)
	free (geom_blob);

    return 1;
}

static int
check_parse (void *cache, const char *path)
{
/* parsing an XML Sample */
    FILE *fl;
    int sz = 0;
    int rd;
    unsigned char *xml = NULL;
    int compressed_sz;
    int uncompressed_sz;
    int doc_sz;
    int formatted_sz;
    int formatted_txt_sz;
    unsigned char *p_result = NULL;
    unsigned char *out;
    char *txt;

/* loading the XMLDocument */
    fl = fopen (path, "rb");
    if (!fl)
      {
	  fprintf (stderr, "cannot open \"%s\"\n", path);
	  return 0;
      }
    if (fseek (fl, 0, SEEK_END) == 0)
	sz = ftell (fl);
    xml = malloc (sz);
    rewind (fl);
    rd = fread (xml, 1, sz, fl);
    if (rd != sz)
      {
	  fprintf (stderr, "read error \"%s\"\n", path);
	  return 0;
      }
    fclose (fl);

/* parsing the XMLDocument (no validation / compressed) */
    gaiaXmlToBlob (cache, xml, rd, 1, NULL, &p_result, &compressed_sz, NULL,
		   NULL);
    if (p_result == NULL)
      {
	  fprintf (stderr, "unable to parse(1) \"%s\"\n", path);
	  return 0;
      }
    doc_sz = gaiaXmlBlobGetDocumentSize (p_result, compressed_sz);
    gaiaXmlFromBlob (p_result, compressed_sz, 4, &out, &formatted_sz);
    if (out == NULL)
      {
	  fprintf (stderr, "unable to format(1) \"%s\"\n", path);
	  return 0;
      }
    free (out);
    txt = gaiaXmlTextFromBlob (p_result, compressed_sz, 2);
    if (txt == NULL)
      {
	  fprintf (stderr, "unable to format-text(1) \"%s\"\n", path);
	  return 0;
      }
    formatted_txt_sz = strlen (txt);
    free (txt);
    free (p_result);

/* parsing the XMLDocument (no validation / not compressed) */
    gaiaXmlToBlob (cache, xml, rd, 0, NULL, &p_result, &uncompressed_sz, NULL,
		   NULL);
    if (p_result == NULL)
      {
	  fprintf (stderr, "unable to parse(2) \"%s\"\n", path);
	  return 0;
      }
    free (p_result);

    if (strcmp (path, "books.xml") == 0)
      {
	  if (compressed_sz != 432)
	    {
		fprintf (stderr,
			 "books.xml: unexpected compressed size %d (expected 432)\n",
			 compressed_sz);
		return 0;
	    }
	  if (uncompressed_sz != 780)
	    {
		fprintf (stderr,
			 "books.xml: unexpected compressed size %d (expected 780)\n",
			 uncompressed_sz);
		return 0;
	    }
	  if (doc_sz != 741)
	    {
		fprintf (stderr,
			 "books.xml: unexpected document size %d (expected 741)\n",
			 doc_sz);
		return 0;
	    }
	  if (formatted_sz != 864)
	    {
		fprintf (stderr,
			 "books.xml: unexpected formatted size %d (expected 864)\n",
			 formatted_sz);
		return 0;
	    }
	  if (formatted_txt_sz != 803)
	    {
		fprintf (stderr,
			 "books.xml: unexpected formatted-text size %d (expected 803)\n",
			 formatted_txt_sz);
		return 0;
	    }
      }
    if (strcmp (path, "opera.xml") == 0)
      {
	  if (compressed_sz != 425)
	    {
		fprintf (stderr,
			 "opera.xml: unexpected compressed size %d (expected 425)\n",
			 compressed_sz);
		return 0;
	    }
	  if (uncompressed_sz != 951)
	    {
		fprintf (stderr,
			 "opera.xml: unexpected compressed size %d (expected 951)\n",
			 uncompressed_sz);
		return 0;
	    }
	  if (doc_sz != 912)
	    {
		fprintf (stderr,
			 "opera.xml: unexpected document size %d (expected 912)\n",
			 doc_sz);
		return 0;
	    }
	  if (formatted_sz != 970)
	    {
		fprintf (stderr,
			 "opera.xml: unexpected formatted size %d (expected 970)\n",
			 formatted_sz);
		return 0;
	    }
	  if (formatted_txt_sz != 909)
	    {
		fprintf (stderr,
			 "opera.xml: unexpected formatted-text size %d (expected 909)\n",
			 formatted_txt_sz);
		return 0;
	    }
      }
    if (strcmp (path, "movies.xml") == 0)
      {
	  if (compressed_sz != 577)
	    {
		fprintf (stderr,
			 "movies.xml: unexpected compressed size %d (expected 577)\n",
			 compressed_sz);
		return 0;
	    }
	  if (uncompressed_sz != 1809)
	    {
		fprintf (stderr,
			 "movies.xml: unexpected compressed size %d (expected 1809)\n",
			 uncompressed_sz);
		return 0;
	    }
	  if (doc_sz != 1770)
	    {
		fprintf (stderr,
			 "movies.xml: unexpected document size %d (expected 1770)\n",
			 doc_sz);
		return 0;
	    }
	  if (formatted_sz != 945)
	    {
		fprintf (stderr,
			 "movies.xml: unexpected formatted size %d (expected 945)\n",
			 formatted_sz);
		return 0;
	    }
	  if (formatted_txt_sz != 884)
	    {
		fprintf (stderr,
			 "movies.xml: unexpected formatted-text size %d (expected 884)\n",
			 formatted_txt_sz);
		return 0;
	    }
      }
    free (xml);

    return 1;
}

#endif

int
main (int argc, char *argv[])
{
    int ret;
    sqlite3 *handle;
    void *cache = spatialite_alloc_connection ();

    if (argc > 1 || argv[0] == NULL)
	argc = 1;		/* silencing stupid compiler warnings */

    ret =
	sqlite3_open_v2 (":memory:", &handle,
			 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "cannot open in-memory db: %s\n",
		   sqlite3_errmsg (handle));
	  sqlite3_close (handle);
	  return -1;
      }

    spatialite_init_ex (handle, cache, 0);

#ifdef ENABLE_LIBXML2		/* only if LIBXML2 is supported */

    if (!check_parse (cache, "books.xml"))
      {
	  fprintf (stderr, "unable to parse \"books.xml\"\n");
	  return -2;
      }
    if (!check_parse (cache, "opera.xml"))
      {
	  fprintf (stderr, "unable to parse \"opera.xml\"\n");
	  return -3;
      }
    if (!check_parse (cache, "movies.xml"))
      {
	  fprintf (stderr, "unable to parse \"movies.xml\"\n");
	  return -4;
      }

    if (!check_validate (cache, "books.xml"))
      {
	  fprintf (stderr, "unable to validate \"books.xml\"\n");
	  return -5;
      }
    if (!check_validate (cache, "opera.xml"))
      {
	  fprintf (stderr, "unable to validate \"opera.xml\"\n");
	  return -6;
      }
    if (!check_validate (cache, "movies.xml"))
      {
	  fprintf (stderr, "unable to validate \"movies.xml\"\n");
	  return -7;
      }
    if (!check_extended (cache, "inspire-data-example.xml", ISO_METADATA))
      {
	  fprintf (stderr, "unable to parse \"inspire-data-example.xml\"\n");
	  return -8;
      }
    if (!check_extended (cache, "stazioni_se.xml", SLD_SE_STYLE))
      {
	  fprintf (stderr, "unable to parse \"stazioni_se.xml\"\n");
	  return -9;
      }
    if (!check_extended (cache, "thunderstorm_mild.svg", SVG))
      {
	  fprintf (stderr, "unable to parse \"thunderstorm_mild.svg\"\n");
	  return -10;
      }

    if (!check_bad_xml (cache))
      {
	  fprintf (stderr, "unable to test not well-formed XML\n");
	  return -11;
      }
    if (!check_bad_schema (cache))
      {
	  fprintf (stderr, "unable to test invalid Schema\n");
	  return -12;
      }

#endif

    ret = sqlite3_close (handle);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "sqlite3_close() error: %s\n",
		   sqlite3_errmsg (handle));
	  return -10;
      }

    spatialite_cleanup_ex (cache);

    spatialite_shutdown ();

    return 0;
}
