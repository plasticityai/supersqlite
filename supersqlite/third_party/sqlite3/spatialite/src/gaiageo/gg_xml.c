/*

 gg_xml.c -- XML Document implementation
    
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#ifdef ENABLE_LIBXML2		/* LIBXML2 enabled: supporting XML documents */

#include <zlib.h>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <spatialite_private.h>
#include <spatialite/sqlite.h>
#include <spatialite/debug.h>
#include <spatialite/gaiageo.h>
#include <spatialite/gaiaaux.h>


struct gaiaxml_namespace
{
/* a Namespace declaration */
    int type;
    xmlChar *prefix;
    xmlChar *href;
    struct gaiaxml_namespace *next;
};

struct gaiaxml_ns_list
{
/* a Namespaces list */
    struct gaiaxml_namespace *first;
    struct gaiaxml_namespace *last;
};

static int
is_valid_cache (struct splite_internal_cache *cache)
{
/* testing if the passed cache is a valid one */
    if (cache == NULL)
	return 0;
    if (cache->magic1 != SPATIALITE_CACHE_MAGIC1
	|| cache->magic2 != SPATIALITE_CACHE_MAGIC2)
	return 0;
    return 1;
}

static struct gaiaxml_namespace *
splite_create_namespace (int type, const xmlChar * prefix, const xmlChar * href)
{
/* allocating and initializing a Namespace declaration */
    int len;
    struct gaiaxml_namespace *ptr = malloc (sizeof (struct gaiaxml_namespace));
    ptr->type = type;
    if (prefix == NULL)
	ptr->prefix = NULL;
    else
      {
	  len = strlen ((const char *) prefix);
	  ptr->prefix = malloc (len + 1);
	  memcpy (ptr->prefix, prefix, len + 1);
      }
    if (href == NULL)
	ptr->href = NULL;
    else
      {
	  len = strlen ((const char *) href);
	  ptr->href = malloc (len + 1);
	  memcpy (ptr->href, href, len + 1);
      }
    ptr->next = NULL;
    return ptr;
}

static void
splite_free_namespace (struct gaiaxml_namespace *ptr)
{
/* memory cleanup - destroying a Namespace declaration */
    if (ptr == NULL)
	return;
    if (ptr->prefix != NULL)
	free (ptr->prefix);
    if (ptr->href != NULL)
	free (ptr->href);
    free (ptr);
}

static struct gaiaxml_ns_list *
splite_create_ns_list (void)
{
/* allocating and initializing a Namespaces list */
    struct gaiaxml_ns_list *ptr = malloc (sizeof (struct gaiaxml_ns_list));
    ptr->first = NULL;
    ptr->last = NULL;
    return ptr;
}

static void
splite_free_ns_list (struct gaiaxml_ns_list *ptr)
{
/* memory cleanup - destroying a Namespaces list */
    struct gaiaxml_namespace *p;
    struct gaiaxml_namespace *p_n;
    if (ptr == NULL)
	return;
    p = ptr->first;
    while (p != NULL)
      {
	  p_n = p->next;
	  splite_free_namespace (p);
	  p = p_n;
      }
    free (ptr);
}

static void
splite_add_namespace (struct gaiaxml_ns_list *list, int type,
		      const xmlChar * prefix, const xmlChar * href)
{
/* inserting a new Namespace into the list */
    struct gaiaxml_namespace *ns;
    if (list == NULL)
	return;
    ns = list->first;
    while (ns != NULL)
      {
	  /* checking if already defined */
	  int ok_type = 0;
	  int ok_prefix = 0;
	  int ok_href = 0;
	  if (ns->type == type)
	      ok_type = 1;
	  if (ns->prefix == NULL && prefix == NULL)
	      ok_prefix = 1;
	  if (ns->prefix != NULL && prefix != NULL)
	    {
		if (strcmp ((const char *) (ns->prefix), (const char *) prefix)
		    == 0)
		    ok_prefix = 1;
	    }
	  if (ns->href == NULL && href == NULL)
	      ok_href = 1;
	  if (ns->href != NULL && href != NULL)
	    {
		if (strcmp ((const char *) (ns->href), (const char *) href) ==
		    0)
		    ok_href = 1;
	    }
	  if (ok_type && ok_prefix && ok_href)
	      return;
	  ns = ns->next;
      }
/* inserting a new Namespace */
    ns = splite_create_namespace (type, prefix, href);
    if (list->first == NULL)
	list->first = ns;
    if (list->last != NULL)
	list->last->next = ns;
    list->last = ns;
}

static void
spliteSilentError (void *ctx, const char *msg, ...)
{
/* shutting up XML Errors */
    if (ctx != NULL)
	ctx = NULL;		/* suppressing stupid compiler warnings (unused args) */
    if (msg != NULL)
	ctx = NULL;		/* suppressing stupid compiler warnings (unused args) */
}

static void
spliteParsingError (void *ctx, const char *msg, ...)
{
/* appending to the current Parsing Error buffer */
    struct splite_internal_cache *cache = (struct splite_internal_cache *) ctx;
    gaiaOutBufferPtr buf;
    char out[65536];
    va_list args;

    if (ctx != NULL)
	ctx = NULL;		/* suppressing stupid compiler warnings (unused args) */
    if (!is_valid_cache (cache))
	return;
    buf = (gaiaOutBufferPtr) (cache->xmlParsingErrors);

    va_start (args, msg);
    vsnprintf (out, 65536, msg, args);
    gaiaAppendToOutBuffer (buf, out);
    va_end (args);
}

static void
spliteSchemaValidationError (void *ctx, const char *msg, ...)
{
/* appending to the current SchemaValidation Error buffer */
    struct splite_internal_cache *cache = (struct splite_internal_cache *) ctx;
    gaiaOutBufferPtr buf;
    char out[65536];
    va_list args;

    if (ctx != NULL)
	ctx = NULL;		/* suppressing stupid compiler warnings (unused args) */
    if (!is_valid_cache (cache))
	return;
    buf = (gaiaOutBufferPtr) (cache->xmlSchemaValidationErrors);

    va_start (args, msg);
    vsnprintf (out, 65536, msg, args);
    gaiaAppendToOutBuffer (buf, out);
    va_end (args);
}

static void
spliteResetXmlErrors (struct splite_internal_cache *cache)
{
/* resetting the XML Error buffers */
    gaiaOutBufferPtr buf;
    if (!is_valid_cache (cache))
	return;
    buf = (gaiaOutBufferPtr) (cache->xmlParsingErrors);
    gaiaOutBufferReset (buf);
    buf = (gaiaOutBufferPtr) (cache->xmlSchemaValidationErrors);
    gaiaOutBufferReset (buf);
}

GAIAGEO_DECLARE char *
gaiaXmlBlobGetLastParseError (const void *ptr)
{
/* get the most recent XML Parse error/warning message */
    struct splite_internal_cache *cache = (struct splite_internal_cache *) ptr;
    gaiaOutBufferPtr buf;
    if (!is_valid_cache (cache))
	return NULL;
    buf = (gaiaOutBufferPtr) (cache->xmlParsingErrors);
    return buf->Buffer;
}

GAIAGEO_DECLARE char *
gaiaXmlBlobGetLastValidateError (const void *ptr)
{
/* get the most recent XML Validate error/warning message */
    struct splite_internal_cache *cache = (struct splite_internal_cache *) ptr;
    gaiaOutBufferPtr buf;
    if (!is_valid_cache (cache))
	return NULL;
    buf = (gaiaOutBufferPtr) (cache->xmlSchemaValidationErrors);
    return buf->Buffer;
}

GAIAGEO_DECLARE char *
gaiaXmlBlobGetLastXPathError (const void *ptr)
{
/* get the most recent XML Validate error/warning message */
    struct splite_internal_cache *cache = (struct splite_internal_cache *) ptr;
    gaiaOutBufferPtr buf;
    if (!is_valid_cache (cache))
	return NULL;
    buf = (gaiaOutBufferPtr) (cache->xmlXPathErrors);
    return buf->Buffer;
}

SPATIALITE_PRIVATE void
splite_free_xml_schema_cache_item (struct splite_xmlSchema_cache_item *p)
{
/* freeing an XmlSchema Cache Item */
    if (p->schemaURI)
	free (p->schemaURI);
    if (p->parserCtxt)
	xmlSchemaFreeParserCtxt (p->parserCtxt);
    if (p->schema)
	xmlSchemaFree (p->schema);
    if (p->schemaDoc)
	xmlFreeDoc (p->schemaDoc);
    p->schemaURI = NULL;
    p->parserCtxt = NULL;
    p->schemaDoc = NULL;
    p->schema = NULL;
}

static int
splite_xmlSchemaCacheFind (struct splite_internal_cache *cache,
			   const char *schemaURI, xmlDocPtr * schema_doc,
			   xmlSchemaParserCtxtPtr * parser_ctxt,
			   xmlSchemaPtr * schema)
{
/* attempting to retrive some XmlSchema from within the Cache */
    int i;
    time_t now;
    struct splite_xmlSchema_cache_item *p;
    if (!is_valid_cache (cache))
	return 0;
    for (i = 0; i < MAX_XMLSCHEMA_CACHE; i++)
      {
	  p = &(cache->xmlSchemaCache[i]);
	  if (p->schemaURI)
	    {
		if (strcmp (schemaURI, p->schemaURI) == 0)
		  {
		      /* found a matching cache-item */
		      *schema_doc = p->schemaDoc;
		      *parser_ctxt = p->parserCtxt;
		      *schema = p->schema;
		      /* updating the timestamp */ time (&now);
		      p->timestamp = now;
		      return 1;
		  }
	    }
      }
    return 0;
}

static void
splite_xmlSchemaCacheInsert (struct splite_internal_cache *cache,
			     const char *schemaURI, xmlDocPtr schema_doc,
			     xmlSchemaParserCtxtPtr parser_ctxt,
			     xmlSchemaPtr schema)
{
/* inserting a new XmlSchema item into the Cache */
    int i;
    int len = strlen (schemaURI);
    time_t now;
    time_t oldest;
    struct splite_xmlSchema_cache_item *pSlot = NULL;
    struct splite_xmlSchema_cache_item *p;
    if (!is_valid_cache (cache))
	return;
    time (&now);
    oldest = now;
    for (i = 0; i < MAX_XMLSCHEMA_CACHE; i++)
      {
	  p = &(cache->xmlSchemaCache[i]);
	  if (p->schemaURI == NULL)
	    {
		/* found an empty slot */
		pSlot = p;
		break;
	    }
	  if (p->timestamp < oldest)
	    {
		/* saving the oldest slot */
		pSlot = p;
		oldest = p->timestamp;
	    }
      }
/* inserting into the Cache Slot */
    splite_free_xml_schema_cache_item (pSlot);
    pSlot->timestamp = now;
    pSlot->schemaURI = malloc (len + 1);
    strcpy (pSlot->schemaURI, schemaURI);
    pSlot->schemaDoc = schema_doc;
    pSlot->parserCtxt = parser_ctxt;
    pSlot->schema = schema;
}

static void
sniff_sld_payload (xmlNodePtr node, int *layers, int *point, int *line,
		   int *polygon, int *raster)
{
/* recursively sniffing a generic SLD payload type */

    while (node)
      {
	  if (node->type == XML_ELEMENT_NODE)
	    {
		const char *name = (const char *) (node->name);
		if (strcmp (name, "FeatureTypeStyle") == 0)
		    *layers += 1;
		if (strcmp (name, "CoverageStyle") == 0)
		    *layers += 1;
		if (strcmp (name, "PointSymbolizer") == 0)
		    *point += 1;
		if (strcmp (name, "LineSymbolizer") == 0)
		    *line += 1;
		if (strcmp (name, "PolygonSymbolizer") == 0)
		    *polygon += 1;
		if (strcmp (name, "RasterSymbolizer") == 0)
		    *raster += 1;
	    }
	  sniff_sld_payload (node->children, layers, point, line, polygon,
			     raster);
	  node = node->next;
      }
}

static void
sniff_payload (xmlDocPtr xml_doc, int *is_iso_metadata,
	       int *is_sld_se_vector_style, int *is_sld_se_raster_style,
	       int *is_sld_style, int *is_svg)
{
/* sniffing the payload type */
    xmlNodePtr root = xmlDocGetRootElement (xml_doc);
    *is_iso_metadata = 0;
    *is_sld_se_vector_style = 0;
    *is_sld_se_raster_style = 0;
    *is_svg = 0;
    if (root->name != NULL)
      {
	  const char *name = (const char *) (root->name);
	  if (strcmp (name, "MD_Metadata") == 0)
	      *is_iso_metadata = 1;
	  if (strcmp (name, "FeatureTypeStyle") == 0
	      || strcmp (name, "PointSymbolizer") == 0
	      || strcmp (name, "LineSymbolizer") == 0
	      || strcmp (name, "PolygonSymbolizer") == 0
	      || strcmp (name, "TextSymbolizer") == 0)
	      *is_sld_se_vector_style = 1;
	  if (strcmp (name, "RasterSymbolizer") == 0
	      || strcmp (name, "CoverageStyle") == 0)
	      *is_sld_se_raster_style = 1;
	  if (strcmp (name, "StyledLayerDescriptor") == 0)
	    {
		/* sniffing an SLD (1.0.0 ??) payload */
		int layers = 0;
		int point = 0;
		int line = 0;
		int polygon = 0;
		int raster = 0;
		xmlNodePtr node = xmlDocGetRootElement (xml_doc);
		sniff_sld_payload (node, &layers, &point, &line, &polygon,
				   &raster);
		if (layers == 1 && point == 0 && line == 0 && polygon == 0
		    && raster == 1)
		  {
		      /* raster style */
		      *is_sld_se_raster_style = 1;
		  }
		if (layers == 1 && (point > 0 || line > 0 || polygon > 0)
		    && raster == 0)
		  {
		      /* vector style */
		      *is_sld_se_vector_style = 1;
		  }
		*is_sld_style = 1;
	    }
	  if (strcmp (name, "svg") == 0)
	      *is_svg = 1;
      }
}

static void
find_iso_ids (xmlNodePtr node, const char *name, char **string, int *open_tag,
	      int *char_string, int *count)
{
/* recursively scanning the DOM tree [fileIdentifier or parentIdentifier] */
    xmlNode *cur_node = NULL;
    int open = 0;
    int cs = 0;

    for (cur_node = node; cur_node; cur_node = cur_node->next)
      {
	  if (cur_node->type == XML_ELEMENT_NODE)
	    {
		const char *xname = (const char *) (cur_node->name);
		if (*open_tag == 1)
		  {
		      if (strcmp (xname, "CharacterString") == 0)
			{
			    cs = 1;
			    *char_string = 1;
			}
		  }
		if (strcmp (xname, name) == 0)
		  {
		      if (cur_node->parent != NULL)
			{
			    if (cur_node->parent->type == XML_ELEMENT_NODE)
			      {
				  if (strcmp
				      ((const char *) (cur_node->parent->name),
				       "MD_Metadata") == 0)
				    {
					/* 
					   / only if <MD_Metadata>
					   /           <fileIdentifier>
					   /             <CharacterString> 
					 */
					open = 1;
					*open_tag = 1;
				    }
			      }
			}
		  }
	    }
	  if (cur_node->type == XML_TEXT_NODE && *open_tag == 1
	      && *char_string == 1)
	    {
		if (cur_node->content != NULL)
		  {
		      int len = strlen ((const char *) cur_node->content);
		      char *buf = malloc (len + 1);
		      strcpy (buf, (const char *) cur_node->content);
		      if (*string)
			  free (*string);
		      *string = buf;
		      *count += 1;
		  }
	    }

	  find_iso_ids (cur_node->children, name, string, open_tag, char_string,
			count);
	  if (open)
	      *open_tag = 0;
	  if (cs)
	      *char_string = 0;
      }
}

static void
find_iso_title (xmlNodePtr node, char **string, int *open_tag, int *char_string,
		int *count)
{
/* recursively scanning the DOM tree [title] */
    xmlNode *cur_node = NULL;
    xmlNode *parent;
    int open = 0;
    int cs = 0;
    int ok_parent;

    for (cur_node = node; cur_node; cur_node = cur_node->next)
      {
	  if (cur_node->type == XML_ELEMENT_NODE)
	    {
		if (*open_tag == 1)
		  {
		      if (strcmp
			  ((const char *) (cur_node->name),
			   "CharacterString") == 0)
			{
			    cs = 1;
			    *char_string = 1;
			}
		  }
		if (strcmp ((const char *) (cur_node->name), "title") == 0)
		  {
		      ok_parent = 0;
		      parent = cur_node->parent;
		      if (parent)
			{
			    if (strcmp
				((const char *) (parent->name),
				 "CI_Citation") == 0)
				ok_parent++;
			}
		      if (ok_parent == 1)
			{
			    parent = parent->parent;
			    if (strcmp
				((const char *) (parent->name),
				 "citation") == 0)
				ok_parent++;
			}
		      if (ok_parent == 2)
			{
			    parent = parent->parent;
			    if (strcmp
				((const char *) (parent->name),
				 "MD_DataIdentification") == 0)
				ok_parent++;
			}
		      if (ok_parent == 3)
			{
			    parent = parent->parent;
			    if (strcmp
				((const char *) (parent->name),
				 "identificationInfo") == 0)
				ok_parent++;
			}
		      if (ok_parent == 4)
			{
			    parent = parent->parent;
			    if (strcmp
				((const char *) (parent->name),
				 "MD_Metadata") == 0)
				ok_parent++;
			}
		      if (ok_parent == 5)
			{
			    /* 
			       / only if <MD_Metadata>
			       /           <identificationInfo>
			       /             <MD_DataIdentification>
			       /               <citation>
			       /                 <CI_Citation>
			       /                   <title> 
			     */
			    open = 1;
			    *open_tag = 1;
			}
		  }
	    }
	  if (cur_node->type == XML_TEXT_NODE && *open_tag == 1
	      && *char_string == 1)
	    {
		if (cur_node->content != NULL)
		  {
		      int len = strlen ((const char *) cur_node->content);
		      char *buf = malloc (len + 1);
		      strcpy (buf, (const char *) cur_node->content);
		      if (*string)
			  free (*string);
		      *string = buf;
		      *count += 1;
		  }
	    }

	  find_iso_title (cur_node->children, string, open_tag, char_string,
			  count);
	  if (open)
	      *open_tag = 0;
	  if (cs)
	      *char_string = 0;
      }
}

static void
find_iso_abstract (xmlNodePtr node, char **string, int *open_tag,
		   int *char_string, int *count)
{
/* recursively scanning the DOM abstract [title] */
    xmlNode *cur_node = NULL;
    xmlNode *parent;
    int open = 0;
    int cs = 0;
    int ok_parent;

    for (cur_node = node; cur_node; cur_node = cur_node->next)
      {
	  if (cur_node->type == XML_ELEMENT_NODE)
	    {
		if (*open_tag == 1)
		  {
		      if (strcmp
			  ((const char *) (cur_node->name),
			   "CharacterString") == 0)
			{
			    cs = 1;
			    *char_string = 1;
			}
		  }
		if (strcmp ((const char *) (cur_node->name), "abstract") == 0)
		  {
		      ok_parent = 0;
		      parent = cur_node->parent;
		      if (parent)
			{
			    if (strcmp
				((const char *) (parent->name),
				 "MD_DataIdentification") == 0)
				ok_parent++;
			}
		      if (ok_parent == 1)
			{
			    parent = parent->parent;
			    if (strcmp
				((const char *) (parent->name),
				 "identificationInfo") == 0)
				ok_parent++;
			}
		      if (ok_parent == 2)
			{
			    parent = parent->parent;
			    if (strcmp
				((const char *) (parent->name),
				 "MD_Metadata") == 0)
				ok_parent++;
			}
		      if (ok_parent == 3)
			{
			    /* only if <MD_Metadata>
			       /            <identificationInfo>
			       /              <MD_DataIdentification>
			       /                <abstract> 
			     */
			    open = 1;
			    *open_tag = 1;
			}
		  }
	    }
	  if (cur_node->type == XML_TEXT_NODE && *open_tag == 1
	      && *char_string == 1)
	    {
		if (cur_node->content != NULL)
		  {
		      int len = strlen ((const char *) cur_node->content);
		      char *buf = malloc (len + 1);
		      strcpy (buf, (const char *) cur_node->content);
		      if (*string)
			  free (*string);
		      *string = buf;
		      *count += 1;
		  }
	    }

	  find_iso_abstract (cur_node->children, string, open_tag, char_string,
			     count);
	  if (open)
	      *open_tag = 0;
	  if (cs)
	      *char_string = 0;
      }
}

static void
find_bbox_coord (xmlNodePtr node, const char *name, double *coord,
		 int *open_tag, int *decimal, int *count)
{
/* recursively scanning an EX_GeographicBoundingBox sub-tree */
    xmlNode *cur_node = NULL;
    int open = 0;
    int dec = 0;

    for (cur_node = node; cur_node; cur_node = cur_node->next)
      {
	  if (cur_node->type == XML_ELEMENT_NODE)
	    {
		if (*open_tag == 1)
		  {
		      if (strcmp ((const char *) (cur_node->name), "Decimal") ==
			  0)
			{
			    dec = 1;
			    *decimal = 1;
			}
		  }
		if (strcmp ((const char *) (cur_node->name), name) == 0)
		  {
		      open = 1;
		      *open_tag = 1;
		  }
	    }
	  if (cur_node->type == XML_TEXT_NODE && *open_tag == 1
	      && *decimal == 1)
	    {
		if (cur_node->content != NULL)
		  {
		      /* found a coord value */
		      double value = atof ((const char *) cur_node->content);
		      *coord = value;
		      *count += 1;
		  }
	    }

	  find_bbox_coord (cur_node->children, name, coord, open_tag, decimal,
			   count);
	  if (open)
	      *open_tag = 0;
	  if (dec)
	      *decimal = 0;
      }
}

static int
parse_bounding_box (xmlNodePtr node, double *minx, double *miny, double *maxx,
		    double *maxy)
{
/* attempting to parse an EX_GeographicBoundingBox sub-tree */
    int ok_minx = 0;
    int ok_miny = 0;
    int ok_maxx = 0;
    int ok_maxy = 0;
    int open_tag;
    int decimal;
    int count;
    double coord;

/* retrieving minx - West */
    open_tag = 0;
    decimal = 0;
    count = 0;
    find_bbox_coord (node, "westBoundLongitude", &coord, &open_tag, &decimal,
		     &count);
    if (count == 1)
      {
	  *minx = coord;
	  ok_minx = 1;
      }

/* retrieving maxx - East */
    open_tag = 0;
    decimal = 0;
    count = 0;
    find_bbox_coord (node, "eastBoundLongitude", &coord, &open_tag, &decimal,
		     &count);
    if (count == 1)
      {
	  *maxx = coord;
	  ok_maxx = 1;
      }

/* retrieving miny - South */
    open_tag = 0;
    decimal = 0;
    count = 0;
    find_bbox_coord (node, "southBoundLatitude", &coord, &open_tag, &decimal,
		     &count);
    if (count == 1)
      {
	  *miny = coord;
	  ok_miny = 1;
      }

/* retrieving maxy - North */
    open_tag = 0;
    decimal = 0;
    count = 0;
    find_bbox_coord (node, "northBoundLatitude", &coord, &open_tag, &decimal,
		     &count);
    if (count == 1)
      {
	  *maxy = coord;
	  ok_maxy = 1;
      }

    if (ok_minx && ok_miny && ok_maxx && ok_maxy)
      {
	  /* ok, valid BBOX */
	  return 1;
      }
    return 0;
}

static void
find_iso_geometry (xmlNodePtr node, gaiaGeomCollPtr * geom)
{
/* recursively scanning the DOM tree [geometry] */
    xmlNode *cur_node = NULL;
    xmlNode *parent;
    int ok_parent;

    for (cur_node = node; cur_node; cur_node = cur_node->next)
      {
	  if (cur_node->type == XML_ELEMENT_NODE)
	    {
		if (strcmp
		    ((const char *) (cur_node->name),
		     "EX_GeographicBoundingBox") == 0)
		  {
		      ok_parent = 0;
		      parent = cur_node->parent;
		      if (parent)
			{
			    if (strcmp
				((const char *) (parent->name),
				 "geographicElement") == 0)
				ok_parent++;
			}
		      if (ok_parent == 1)
			{
			    parent = parent->parent;
			    if (strcmp
				((const char *) (parent->name),
				 "EX_Extent") == 0)
				ok_parent++;
			}
		      if (ok_parent == 2)
			{
			    parent = parent->parent;
			    if (strcmp ((const char *) (parent->name), "extent")
				== 0)
				ok_parent++;
			}
		      if (ok_parent == 3)
			{
			    parent = parent->parent;
			    if (strcmp
				((const char *) (parent->name),
				 "MD_DataIdentification") == 0)
				ok_parent++;
			}
		      if (ok_parent == 4)
			{
			    parent = parent->parent;
			    if (strcmp
				((const char *) (parent->name),
				 "identificationInfo") == 0)
				ok_parent++;
			}
		      if (ok_parent == 5)
			{
			    parent = parent->parent;
			    if (strcmp
				((const char *) (parent->name),
				 "MD_Metadata") == 0)
				ok_parent++;
			}
		      if (ok_parent == 6)
			{
			    /* only if <MD_Metadata>
			       /            <identificationInfo>
			       /              <MD_DataIdentification>
			       /                <extent>
			       /                  <EX_Extent>
			       /                    <geographicElement>
			       /                      <EX_GeographicBoundingBox> 
			     */
			    double minx = 0.0;
			    double maxx = 0.0;
			    double miny = 0.0;
			    double maxy = 0.0;
			    if (parse_bounding_box
				(cur_node, &minx, &miny, &maxx, &maxy))
			      {
				  gaiaPolygonPtr pg;
				  gaiaRingPtr rng;
				  gaiaGeomCollPtr g = *geom;
				  if (g == NULL)
				    {
					g = gaiaAllocGeomColl ();
					g->Srid = 4326;
					g->DeclaredType = GAIA_MULTIPOLYGON;
				    }
				  pg = gaiaAddPolygonToGeomColl (g, 5, 0);
				  rng = pg->Exterior;
				  gaiaSetPoint (rng->Coords, 0, minx, miny);
				  gaiaSetPoint (rng->Coords, 1, maxx, miny);
				  gaiaSetPoint (rng->Coords, 2, maxx, maxy);
				  gaiaSetPoint (rng->Coords, 3, minx, maxy);
				  gaiaSetPoint (rng->Coords, 4, minx, miny);
				  *geom = g;
			      }
			}
		  }
	    }
	  find_iso_geometry (cur_node->children, geom);
      }
}

static void
retrieve_iso_identifiers (xmlDocPtr xml_doc, char **fileIdentifier,
			  char **parentIdentifier, char **title,
			  char **abstract, unsigned char **geometry,
			  short *geometry_len)
{
/*
/ attempting to retrieve the FileIdentifier, ParentIdentifier,
/ Title, Abstract and Geometry items from an ISO Metadata document
*/
    xmlNodePtr root = xmlDocGetRootElement (xml_doc);
    int open_tag;
    int char_string;
    int count;
    char *string;
    gaiaGeomCollPtr geom = NULL;

    *fileIdentifier = NULL;
    *parentIdentifier = NULL;
    *title = NULL;
    *abstract = NULL;
    *geometry = NULL;

/* attempting to retrieve the FileIdentifier item */
    open_tag = 0;
    char_string = 0;
    count = 0;
    string = NULL;
    find_iso_ids (root, "fileIdentifier", &string, &open_tag, &char_string,
		  &count);
    if (string)
      {
	  if (count == 1)
	      *fileIdentifier = string;
	  else
	      free (string);
      }

/* attempting to retrieve the ParentIdentifier item */
    open_tag = 0;
    char_string = 0;
    count = 0;
    string = NULL;
    find_iso_ids (root, "parentIdentifier", &string, &open_tag, &char_string,
		  &count);
    if (string)
      {
	  if (count == 1)
	      *parentIdentifier = string;
	  else
	      free (string);
      }

/* attempting to retrieve the Title item */
    open_tag = 0;
    char_string = 0;
    count = 0;
    string = NULL;
    find_iso_title (root, &string, &open_tag, &char_string, &count);
    if (string)
      {
	  if (count == 1)
	      *title = string;
	  else
	      free (string);
      }

/* attempting to retrieve the Abstract item */
    open_tag = 0;
    char_string = 0;
    count = 0;
    string = NULL;
    find_iso_abstract (root, &string, &open_tag, &char_string, &count);
    if (string)
      {
	  if (count == 1)
	      *abstract = string;
	  else
	      free (string);
      }

/* attempting to retrieve the Geometry item */
    open_tag = 0;
    char_string = 0;
    count = 0;
    string = NULL;
    find_iso_geometry (root, &geom);
    if (geom)
      {
	  int blob_len;
	  unsigned char *blob = NULL;
	  gaiaMbrGeometry (geom);
	  gaiaToSpatiaLiteBlobWkb (geom, &blob, &blob_len);
	  gaiaFreeGeomColl (geom);
	  *geometry = blob;
	  *geometry_len = (short) blob_len;
      }
}

static void
find_sld_name (xmlNodePtr node, char **string)
{
/* recursively scanning the DOM tree [name] */
    while (node)
      {
	  if (node->type == XML_ELEMENT_NODE)
	    {
		const char *name = (const char *) (node->name);
		if (strcmp (name, "Name") == 0)
		  {
		      xmlNodePtr child = node->children;
		      if (child)
			{
			    if (child->type == XML_TEXT_NODE)
			      {
				  int len;
				  const char *value =
				      (const char *) (child->content);
				  len = strlen (value);
				  if (*string != NULL)
				      free (*string);
				  *string = malloc (len + 1);
				  strcpy (*string, value);
			      }
			}
		  }
	    }
	  node = node->next;
      }
}

static void
find_sld_title (xmlNodePtr node, char **string)
{
/* recursively scanning the DOM tree [title] */
    while (node)
      {
	  if (node->type == XML_ELEMENT_NODE)
	    {
		const char *name = (const char *) (node->name);
		if (strcmp (name, "Title") == 0)
		  {
		      xmlNodePtr child = node->children;
		      if (child)
			{
			    if (child->type == XML_TEXT_NODE)
			      {
				  int len;
				  const char *value =
				      (const char *) (child->content);
				  len = strlen (value);
				  if (*string != NULL)
				      free (*string);
				  *string = malloc (len + 1);
				  strcpy (*string, value);
			      }
			}
		  }
		if (strcmp (name, "Description") == 0)
		    find_sld_title (node->children, string);
	    }
	  node = node->next;
      }
}

static void
find_sld_abstract (xmlNodePtr node, char **string)
{
/* recursively scanning the DOM tree [abstract] */
    while (node)
      {
	  if (node->type == XML_ELEMENT_NODE)
	    {
		const char *name = (const char *) (node->name);
		if (strcmp (name, "Abstract") == 0)
		  {
		      xmlNodePtr child = node->children;
		      if (child)
			{
			    if (child->type == XML_TEXT_NODE)
			      {
				  int len;
				  const char *value =
				      (const char *) (child->content);
				  len = strlen (value);
				  if (*string != NULL)
				      free (*string);
				  *string = malloc (len + 1);
				  strcpy (*string, value);
			      }
			}
		  }
		if (strcmp (name, "Description") == 0)
		    find_sld_abstract (node->children, string);
	    }
	  node = node->next;
      }
}

static void
retrieve_sld_identifiers (xmlDocPtr xml_doc, char **name, char **title,
			  char **abstract)
{
/*
/ attempting to retrieve the Name, Title and Abstract items 
/ from an SLD Style document
*/
    xmlNodePtr root = xmlDocGetRootElement (xml_doc);
    char *string;
    const char *xname = (const char *) (root->name);

    *name = NULL;
    *title = NULL;
    *abstract = NULL;

    if (xname != NULL)
      {
	  if (strcmp (xname, "StyledLayerDescriptor") != 0)
	      return;
      }

/* attempting to retrieve the Name item */
    string = NULL;
    find_sld_name (root->children, &string);
    if (string)
	*name = string;

/* attempting to retrieve the Title item */
    string = NULL;
    find_sld_title (root->children, &string);
    if (string)
	*title = string;

/* attempting to retrieve the Abstract item */
    string = NULL;
    find_sld_abstract (root->children, &string);
    if (string)
	*abstract = string;
}

static void
find_sld_se_name (xmlNodePtr node, char **string, int *style, int *rule)
{
/* recursively scanning the DOM tree [name] */
    int is_style = 0;
    int is_rule = 0;
    while (node)
      {
	  if (node->type == XML_ELEMENT_NODE)
	    {
		const char *name = (const char *) (node->name);
		if (strcmp (name, "FeatureTypeStyle") == 0
		    || strcmp (name, "CoverageStyle") == 0)
		  {
		      is_style = 1;
		      *style = 1;
		  }
		if (strcmp (name, "Rule") == 0)
		  {
		      is_rule = 1;
		      *rule = 1;
		  }
		if (strcmp (name, "Name") == 0)
		  {
		      if (*style == 1 && *rule == 0)
			{
			    xmlNodePtr child = node->children;
			    if (child)
			      {
				  if (child->type == XML_TEXT_NODE)
				    {
					int len;
					const char *value =
					    (const char *) (child->content);
					len = strlen (value);
					if (*string != NULL)
					    free (*string);
					*string = malloc (len + 1);
					strcpy (*string, value);
				    }
			      }
			}
		  }
	    }

	  find_sld_se_name (node->children, string, style, rule);
	  if (is_style)
	      *style = 0;
	  if (is_rule)
	      *rule = 0;
	  node = node->next;
      }
}

static void
find_sld_se_title (xmlNodePtr node, char **string, int *style, int *rule)
{
/* recursively scanning the DOM tree [title] */
    int is_style = 0;
    int is_rule = 0;
    while (node)
      {
	  if (node->type == XML_ELEMENT_NODE)
	    {
		const char *name = (const char *) (node->name);
		if (strcmp (name, "FeatureTypeStyle") == 0
		    || strcmp (name, "CoverageStyle") == 0)
		  {
		      is_style = 1;
		      *style = 1;
		  }
		if (strcmp (name, "Rule") == 0)
		  {
		      is_rule = 1;
		      *rule = 1;
		  }
		if (strcmp (name, "Title") == 0)
		  {
		      if (*style == 1 && *rule == 0)
			{
			    xmlNodePtr child = node->children;
			    if (child)
			      {
				  if (child->type == XML_TEXT_NODE)
				    {
					int len;
					const char *value =
					    (const char *) (child->content);
					len = strlen (value);
					if (*string != NULL)
					    free (*string);
					*string = malloc (len + 1);
					strcpy (*string, value);
				    }
			      }
			}
		  }
	    }

	  find_sld_se_title (node->children, string, style, rule);
	  if (is_style)
	      *style = 0;
	  if (is_rule)
	      *rule = 0;
	  node = node->next;
      }
}

static void
find_sld_se_abstract (xmlNodePtr node, char **string, int *style, int *rule)
{
/* recursively scanning the DOM tree [abstract] */
    int is_style = 0;
    int is_rule = 0;
    while (node)
      {
	  if (node->type == XML_ELEMENT_NODE)
	    {
		const char *name = (const char *) (node->name);
		if (strcmp (name, "FeatureTypeStyle") == 0
		    || strcmp (name, "CoverageStyle") == 0)
		  {
		      is_style = 1;
		      *style = 1;
		  }
		if (strcmp (name, "Rule") == 0)
		  {
		      is_rule = 1;
		      *rule = 1;
		  }
		if (strcmp (name, "Abstract") == 0)
		  {
		      if (*style == 1 && *rule == 0)
			{
			    xmlNodePtr child = node->children;
			    if (child)
			      {
				  if (child->type == XML_TEXT_NODE)
				    {
					int len;
					const char *value =
					    (const char *) (child->content);
					len = strlen (value);
					if (*string != NULL)
					    free (*string);
					*string = malloc (len + 1);
					strcpy (*string, value);
				    }
			      }
			}
		  }
	    }

	  find_sld_se_abstract (node->children, string, style, rule);
	  if (is_style)
	      *style = 0;
	  if (is_rule)
	      *rule = 0;
	  node = node->next;
      }
}

static void
retrieve_sld_se_identifiers (xmlDocPtr xml_doc, char **name, char **title,
			     char **abstract)
{
/*
/ attempting to retrieve the Name, Title and Abstract items 
/ from an SLD/SE Style document
*/
    xmlNodePtr root = xmlDocGetRootElement (xml_doc);
    int style;
    int rule;
    char *string;
    const char *xname = (const char *) (root->name);

    *name = NULL;
    *title = NULL;
    *abstract = NULL;

/* attempting to retrieve the Name item */
    style = 0;
    rule = 0;
    string = NULL;
    if (xname != NULL)
      {
	  if (strcmp (xname, "PointSymbolizer") == 0
	      || strcmp (xname, "LineSymbolizer") == 0
	      || strcmp (xname, "PolygonSymbolizer") == 0
	      || strcmp (xname, "TextSymbolizer") == 0
	      || strcmp (xname, "RasterSymbolizer") == 0)
	      style = 1;
      }
    find_sld_se_name (root, &string, &style, &rule);
    if (string)
	*name = string;

/* attempting to retrieve the Title item */
    style = 0;
    rule = 0;
    string = NULL;
    if (xname != NULL)
      {
	  if (strcmp (xname, "PointSymbolizer") == 0
	      || strcmp (xname, "LineSymbolizer") == 0
	      || strcmp (xname, "PolygonSymbolizer") == 0
	      || strcmp (xname, "TextSymbolizer") == 0
	      || strcmp (xname, "RasterSymbolizer") == 0)
	      style = 1;
      }
    find_sld_se_title (root, &string, &style, &rule);
    if (string)
	*title = string;

/* attempting to retrieve the Abstract item */
    style = 0;
    rule = 0;
    string = NULL;
    if (xname != NULL)
      {
	  if (strcmp (xname, "PointSymbolizer") == 0
	      || strcmp (xname, "LineSymbolizer") == 0
	      || strcmp (xname, "PolygonSymbolizer") == 0
	      || strcmp (xname, "TextSymbolizer") == 0
	      || strcmp (xname, "RasterSymbolizer") == 0)
	      style = 1;
      }
    find_sld_se_abstract (root, &string, &style, &rule);
    if (string)
	*abstract = string;
}

GAIAGEO_DECLARE void
gaiaXmlToBlob (const void *p_cache, const unsigned char *xml, int xml_len,
	       int compressed, const char *schemaURI, unsigned char **result,
	       int *size, char **parsing_errors,
	       char **schema_validation_errors)
{
/* attempting to build an XmlBLOB buffer */
    xmlDocPtr xml_doc;
    xmlDocPtr schema_doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr parser_ctxt;
    xmlSchemaValidCtxtPtr valid_ctxt;
    int is_iso_metadata = 0;
    int is_sld_se_vector_style = 0;
    int is_sld_se_raster_style = 0;
    int is_sld_style = 0;
    int is_svg = 0;
    int len;
    int zip_len;
    short uri_len = 0;
    short fileid_len = 0;
    short parentid_len = 0;
    short name_len = 0;
    short title_len = 0;
    short abstract_len = 0;
    short geometry_len = 0;
    char *fileIdentifier = NULL;
    char *parentIdentifier = NULL;
    char *name = NULL;
    char *title = NULL;
    char *abstract = NULL;
    unsigned char *geometry = NULL;
    uLong crc;
    Bytef *zip_buf;
    unsigned char *buf;
    unsigned char *ptr;
    unsigned char flags = 0x00;
    int endian_arch = gaiaEndianArch ();
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    gaiaOutBufferPtr parsingBuf = NULL;
    gaiaOutBufferPtr schemaValidationBuf = NULL;
    xmlGenericErrorFunc silentError = NULL;
    xmlGenericErrorFunc parsingError = NULL;
    xmlGenericErrorFunc schemaError = NULL;
    if (is_valid_cache (cache))
      {
	  parsingBuf = (gaiaOutBufferPtr) (cache->xmlParsingErrors);
	  schemaValidationBuf =
	      (gaiaOutBufferPtr) (cache->xmlSchemaValidationErrors);
	  parsingError = (xmlGenericErrorFunc) spliteParsingError;
	  schemaError = (xmlGenericErrorFunc) spliteSchemaValidationError;
	  spliteResetXmlErrors (cache);
      }

    *result = NULL;
    *size = 0;
    if (parsing_errors)
	*parsing_errors = NULL;
    if (schema_validation_errors)
	*schema_validation_errors = NULL;
    if (xml == NULL)
	return;

    xmlSetGenericErrorFunc (NULL, silentError);

    if (schemaURI != NULL)
      {
	  if (splite_xmlSchemaCacheFind
	      (cache, schemaURI, &schema_doc, &parser_ctxt, &schema))
	      ;
	  else
	    {
		/* preparing the Schema */
		xmlSetGenericErrorFunc (cache, schemaError);
		schema_doc = xmlReadFile ((const char *) schemaURI, NULL, 0);
		if (schema_doc == NULL)
		  {
		      spatialite_e ("unable to load the Schema\n");
		      if (schema_validation_errors)
			  *schema_validation_errors =
			      schemaValidationBuf->Buffer;
		      xmlSetGenericErrorFunc ((void *) stderr, NULL);
		      return;
		  }
		parser_ctxt = xmlSchemaNewDocParserCtxt (schema_doc);
		if (parser_ctxt == NULL)
		  {
		      spatialite_e ("unable to prepare the Schema Context\n");
		      xmlFreeDoc (schema_doc);
		      if (schema_validation_errors)
			  *schema_validation_errors =
			      schemaValidationBuf->Buffer;
		      xmlSetGenericErrorFunc ((void *) stderr, NULL);
		      return;
		  }
		schema = xmlSchemaParse (parser_ctxt);
		if (schema == NULL)
		  {
		      spatialite_e ("invalid Schema\n");
		      xmlFreeDoc (schema_doc);
		      if (schema_validation_errors)
			  *schema_validation_errors =
			      schemaValidationBuf->Buffer;
		      xmlSetGenericErrorFunc ((void *) stderr, NULL);
		      return;
		  }
		splite_xmlSchemaCacheInsert (cache, schemaURI, schema_doc,
					     parser_ctxt, schema);
	    }
      }

/* testing if the XMLDocument is well-formed */
    xmlSetGenericErrorFunc (cache, parsingError);
    xml_doc =
	xmlReadMemory ((const char *) xml, xml_len, "noname.xml", NULL, 0);
    if (xml_doc == NULL)
      {
	  /* parsing error; not a well-formed XML */
	  spatialite_e ("XML parsing error\n");
	  if (parsing_errors && parsingBuf)
	      *parsing_errors = parsingBuf->Buffer;
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return;
      }
    if (parsing_errors && parsingBuf)
	*parsing_errors = parsingBuf->Buffer;

    if (schemaURI != NULL)
      {
	  /* Schema validation */
	  xmlSetGenericErrorFunc (cache, schemaError);
	  valid_ctxt = xmlSchemaNewValidCtxt (schema);
	  if (valid_ctxt == NULL)
	    {
		spatialite_e ("unable to prepare a validation context\n");
		xmlFreeDoc (xml_doc);
		if (schema_validation_errors && schemaValidationBuf)
		    *schema_validation_errors = schemaValidationBuf->Buffer;
		xmlSetGenericErrorFunc ((void *) stderr, NULL);
		return;
	    }
	  if (xmlSchemaValidateDoc (valid_ctxt, xml_doc) != 0)
	    {
		spatialite_e ("Schema validation failed\n");
		xmlSchemaFreeValidCtxt (valid_ctxt);
		xmlFreeDoc (xml_doc);
		if (schema_validation_errors && schemaValidationBuf)
		    *schema_validation_errors = schemaValidationBuf->Buffer;
		xmlSetGenericErrorFunc ((void *) stderr, NULL);
		return;
	    }
	  xmlSchemaFreeValidCtxt (valid_ctxt);
      }

/* testing for special cases: ISO Metadata, SLD/SE Styles and SVG */
    sniff_payload (xml_doc, &is_iso_metadata, &is_sld_se_vector_style,
		   &is_sld_se_raster_style, &is_sld_style, &is_svg);
    if (is_iso_metadata)
	retrieve_iso_identifiers (xml_doc, &fileIdentifier,
				  &parentIdentifier, &title, &abstract,
				  &geometry, &geometry_len);
    if (is_sld_style)
	retrieve_sld_identifiers (xml_doc, &name, &title, &abstract);
    else if (is_sld_se_vector_style || is_sld_se_raster_style)
	retrieve_sld_se_identifiers (xml_doc, &name, &title, &abstract);
    xmlFreeDoc (xml_doc);

    if (compressed)
      {
	  /* compressing the XML payload */
	  uLong zLen = compressBound (xml_len);
	  zip_buf = malloc (zLen);
	  if (compress (zip_buf, &zLen, (const Bytef *) xml, (uLong) xml_len) !=
	      Z_OK)
	    {
		/* compression error */
		spatialite_e ("XmlBLOB DEFLATE compress error\n");
		free (zip_buf);
		xmlSetGenericErrorFunc ((void *) stderr, NULL);
		return;
	    }
	  zip_len = (int) zLen;
      }
    else
	zip_len = xml_len;

/* reporting errors */
    if (parsing_errors && parsingBuf)
	*parsing_errors = parsingBuf->Buffer;
    if (schema_validation_errors && schemaValidationBuf)
	*schema_validation_errors = schemaValidationBuf->Buffer;

/* computing the XmlBLOB size */
    len = 39;			/* fixed header-footer size */
    if (schemaURI)
	uri_len = strlen ((const char *) schemaURI);
    if (fileIdentifier)
	fileid_len = strlen ((const char *) fileIdentifier);
    if (parentIdentifier)
	parentid_len = strlen ((const char *) parentIdentifier);
    if (name)
	name_len = strlen ((const char *) name);
    if (title)
	title_len = strlen ((const char *) title);
    if (abstract)
	abstract_len = strlen ((const char *) abstract);
    len += zip_len;
    len += uri_len;
    len += fileid_len;
    len += parentid_len;
    len += name_len;
    len += title_len;
    len += abstract_len;
    len += geometry_len;
    buf = malloc (len);
    *buf = GAIA_XML_START;	/* START signature */
    flags |= GAIA_XML_LITTLE_ENDIAN;
    if (compressed)
	flags |= GAIA_XML_COMPRESSED;
    if (schemaURI != NULL)
	flags |= GAIA_XML_VALIDATED;
    if (is_iso_metadata)
	flags |= GAIA_XML_ISO_METADATA;
    if (is_sld_se_vector_style)
	flags |= GAIA_XML_SLD_SE_VECTOR_STYLE;
    if (is_sld_se_raster_style)
	flags |= GAIA_XML_SLD_SE_RASTER_STYLE;
    if (is_sld_style)
	flags |= GAIA_XML_SLD_STYLE;
    if (is_svg)
	flags |= GAIA_XML_SVG;
    *(buf + 1) = flags;		/* XmlBLOB flags */
    *(buf + 2) = GAIA_XML_HEADER;	/* HEADER signature */
    gaiaExport32 (buf + 3, xml_len, 1, endian_arch);	/* the uncompressed XMLDocument size */
    gaiaExport32 (buf + 7, zip_len, 1, endian_arch);	/* the compressed XMLDocument size */
    gaiaExport16 (buf + 11, uri_len, 1, endian_arch);	/* the SchemaURI length in bytes */
    *(buf + 13) = GAIA_XML_SCHEMA;	/* SCHEMA signature */
    ptr = buf + 14;
    if (schemaURI)
      {
	  /* the SchemaURI */
	  memcpy (ptr, schemaURI, uri_len);
	  ptr += uri_len;
      }
    gaiaExport16 (ptr, fileid_len, 1, endian_arch);	/* the FileIdentifier length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_FILEID;	/* FileIdentifier signature */
    ptr++;
    if (fileIdentifier)
      {
	  /* the FileIdentifier */
	  memcpy (ptr, fileIdentifier, fileid_len);
	  free (fileIdentifier);
	  ptr += fileid_len;
      }
    gaiaExport16 (ptr, parentid_len, 1, endian_arch);	/* the ParentIdentifier length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_PARENTID;	/* ParentIdentifier signature */
    ptr++;
    if (parentIdentifier)
      {
	  /* the ParentIdentifier */
	  memcpy (ptr, parentIdentifier, parentid_len);
	  free (parentIdentifier);
	  ptr += parentid_len;
      }
    gaiaExport16 (ptr, name_len, 1, endian_arch);	/* the Name length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_NAME;	/* Title signature */
    ptr++;
    if (name)
      {
	  /* the Name */
	  memcpy (ptr, name, name_len);
	  free (name);
	  ptr += name_len;
      }
    gaiaExport16 (ptr, title_len, 1, endian_arch);	/* the Title length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_TITLE;	/* Title signature */
    ptr++;
    if (title)
      {
	  /* the Title */
	  memcpy (ptr, title, title_len);
	  free (title);
	  ptr += title_len;
      }
    gaiaExport16 (ptr, abstract_len, 1, endian_arch);	/* the Abstract length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_ABSTRACT;	/* Abstract signature */
    ptr++;
    if (abstract)
      {
	  /* the Abstract */
	  memcpy (ptr, abstract, abstract_len);
	  free (abstract);
	  ptr += abstract_len;
      }
    gaiaExport16 (ptr, geometry_len, 1, endian_arch);	/* the Geometry length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_GEOMETRY;	/* Geometry signature */
    ptr++;
    if (geometry)
      {
	  /* the Geometry */
	  memcpy (ptr, geometry, geometry_len);
	  free (geometry);
	  ptr += geometry_len;
      }
    *ptr = GAIA_XML_PAYLOAD;	/* PAYLOAD signature */
    ptr++;
    if (compressed)
      {
	  /* the compressed XML payload */
	  memcpy (ptr, zip_buf, zip_len);
	  free (zip_buf);
	  ptr += zip_len;
      }
    else
      {
	  /* the uncompressed XML payload */
	  memcpy (ptr, xml, xml_len);
	  ptr += xml_len;
      }
    *ptr = GAIA_XML_CRC32;	/* CRC32 signature */
    ptr++;
/* computing the CRC32 */
    crc = crc32 (0L, buf, ptr - buf);
    gaiaExportU32 (ptr, crc, 1, endian_arch);	/* the CRC32 */
    ptr += 4;
    *ptr = GAIA_XML_END;	/* END signature */

    *result = buf;
    *size = len;
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
}

GAIAGEO_DECLARE void
gaiaXmlBlobCompression (const unsigned char *blob,
			int in_size, int compressed,
			unsigned char **result, int *out_size)
{
/* Return another XmlBLOB buffer compressed / uncompressed */
    int in_compressed = 0;
    int little_endian = 0;
    unsigned char flag;
    int in_xml_len;
    int in_zip_len;
    short uri_len;
    short fileid_len;
    short parentid_len;
    short name_len;
    short title_len;
    short abstract_len;
    short geometry_len;
    int out_xml_len;
    int out_zip_len;
    uLong crc;
    Bytef *zip_buf;
    int len;
    char *schemaURI;
    char *fileIdentifier;
    char *parentIdentifier;
    char *name;
    char *title;
    char *abstract;
    unsigned char *geometry;
    int is_iso_metadata = 0;
    int is_sld_se_vector_style = 0;
    int is_sld_se_raster_style = 0;
    int is_sld_style = 0;
    int is_svg = 0;
    unsigned char *xml;
    unsigned char *buf;
    unsigned char *ptr;
    unsigned char flags;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();

    *result = NULL;
    *out_size = 0;
/* validity check */
    if (!gaiaIsValidXmlBlob (blob, in_size))
	return;			/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    if ((flag & GAIA_XML_COMPRESSED) == GAIA_XML_COMPRESSED)
	in_compressed = 1;
    if ((flag & GAIA_XML_ISO_METADATA) == GAIA_XML_ISO_METADATA)
	is_iso_metadata = 1;
    if ((flag & GAIA_XML_SLD_SE_VECTOR_STYLE) == GAIA_XML_SLD_SE_VECTOR_STYLE)
	is_sld_se_vector_style = 1;
    if ((flag & GAIA_XML_SLD_SE_RASTER_STYLE) == GAIA_XML_SLD_SE_RASTER_STYLE)
	is_sld_se_raster_style = 1;
    if ((flag & GAIA_XML_SLD_STYLE) == GAIA_XML_SLD_STYLE)
	is_sld_style = 1;
    if ((flag & GAIA_XML_SVG) == GAIA_XML_SVG)
	is_svg = 1;
    in_xml_len = gaiaImport32 (blob + 3, little_endian, endian_arch);
    in_zip_len = gaiaImport32 (blob + 7, little_endian, endian_arch);
    uri_len = gaiaImport16 (blob + 11, little_endian, endian_arch);
    ptr = (unsigned char *) blob + 14;
    if (uri_len)
      {
	  schemaURI = (char *) ptr;
	  ptr += uri_len;
      }
    else
      {
	  schemaURI = NULL;
      }
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3;
    if (fileid_len)
      {
	  fileIdentifier = (char *) ptr;
	  ptr += fileid_len;
      }
    else
      {
	  fileIdentifier = NULL;
      }
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3;
    if (parentid_len)
      {
	  parentIdentifier = (char *) ptr;
	  ptr += parentid_len;
      }
    else
      {
	  parentIdentifier = NULL;
      }
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3;
	  if (name_len)
	    {
		name = (char *) ptr;
		ptr += name_len;
	    }
	  else
	    {
		name = NULL;
	    }
      }
    else
      {
	  name_len = 0;
	  name = NULL;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3;
    if (title_len)
      {
	  title = (char *) ptr;
	  ptr += title_len;
      }
    else
      {
	  title = NULL;
      }
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3;
    if (abstract_len)
      {
	  abstract = (char *) ptr;
	  ptr += abstract_len;
      }
    else
      {
	  abstract = NULL;
      }
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3;
    if (geometry_len)
      {
	  geometry = (unsigned char *) ptr;
	  ptr += geometry_len;
      }
    else
      {
	  geometry = NULL;
      }
    ptr++;

    if (in_compressed == compressed)
      {
	  /* unchanged compression */
	  out_xml_len = in_xml_len;
	  out_zip_len = in_zip_len;
	  zip_buf = (unsigned char *) ptr;
      }
    else if (compressed)
      {
	  /* compressing the XML payload */
	  uLong zLen;
	  out_xml_len = in_xml_len;
	  zLen = compressBound (out_xml_len);
	  xml = (unsigned char *) ptr;
	  zip_buf = malloc (zLen);
	  if (compress
	      (zip_buf, &zLen, (const Bytef *) xml,
	       (uLong) out_xml_len) != Z_OK)
	    {
		/* compression error */
		spatialite_e ("XmlBLOB DEFLATE compress error\n");
		free (zip_buf);
		return;
	    }
	  out_zip_len = (int) zLen;
      }
    else
      {
	  /* unzipping the XML payload */
	  uLong refLen = in_xml_len;
	  const Bytef *in = ptr;
	  xml = malloc (in_xml_len + 1);
	  if (uncompress (xml, &refLen, in, in_zip_len) != Z_OK)
	    {
		/* uncompress error */
		spatialite_e ("XmlBLOB DEFLATE uncompress error\n");
		free (xml);
		return;
	    }
	  *(xml + in_xml_len) = '\0';
	  out_xml_len = in_xml_len;
	  out_zip_len = out_xml_len;
      }

/* computing the XmlBLOB size */
    len = 39;			/* fixed header-footer size */
    len += out_zip_len;
    len += uri_len;
    len += fileid_len;
    len += parentid_len;
    len += name_len;
    len += title_len;
    len += abstract_len;
    len += geometry_len;
    buf = malloc (len);
    *buf = GAIA_XML_START;	/* START signature */
    flags = 0x00;
    flags |= GAIA_XML_LITTLE_ENDIAN;
    if (compressed)
	flags |= GAIA_XML_COMPRESSED;
    if (schemaURI != NULL)
	flags |= GAIA_XML_VALIDATED;
    if (is_iso_metadata)
	flags |= GAIA_XML_ISO_METADATA;
    if (is_sld_se_vector_style)
	flags |= GAIA_XML_SLD_SE_VECTOR_STYLE;
    if (is_sld_se_raster_style)
	flags |= GAIA_XML_SLD_SE_RASTER_STYLE;
    if (is_sld_style)
	flags |= GAIA_XML_SLD_STYLE;
    if (is_svg)
	flags |= GAIA_XML_SVG;
    *(buf + 1) = flags;		/* XmlBLOB flags */
    *(buf + 2) = GAIA_XML_HEADER;	/* HEADER signature */
    gaiaExport32 (buf + 3, out_xml_len, 1, endian_arch);	/* the uncompressed XMLDocument size */
    gaiaExport32 (buf + 7, out_zip_len, 1, endian_arch);	/* the compressed XMLDocument size */
    gaiaExport16 (buf + 11, uri_len, 1, endian_arch);	/* the SchemaURI length in bytes */
    *(buf + 13) = GAIA_XML_SCHEMA;	/* SCHEMA signature */
    ptr = buf + 14;
    if (schemaURI)
      {
	  /* the SchemaURI */
	  memcpy (ptr, schemaURI, uri_len);
	  ptr += uri_len;
      }
    gaiaExport16 (ptr, fileid_len, 1, endian_arch);	/* the FileIdentifier length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_FILEID;	/* FileIdentifier signature */
    ptr++;
    if (fileIdentifier)
      {
	  /* the FileIdentifier */
	  memcpy (ptr, fileIdentifier, fileid_len);
	  ptr += fileid_len;
      }
    gaiaExport16 (ptr, parentid_len, 1, endian_arch);	/* the ParentIdentifier length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_PARENTID;	/* ParentIdentifier signature */
    ptr++;
    if (parentIdentifier)
      {
	  /* the ParentIdentifier */
	  memcpy (ptr, parentIdentifier, parentid_len);
	  ptr += parentid_len;
      }
    gaiaExport16 (ptr, name_len, 1, endian_arch);	/* the Name length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_NAME;	/* Name signature */
    ptr++;
    if (name)
      {
	  /* the Name */
	  memcpy (ptr, name, name_len);
	  ptr += name_len;
      }
    gaiaExport16 (ptr, title_len, 1, endian_arch);	/* the Title length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_TITLE;	/* Title signature */
    ptr++;
    if (title)
      {
	  /* the Title */
	  memcpy (ptr, title, title_len);
	  ptr += title_len;
      }
    gaiaExport16 (ptr, abstract_len, 1, endian_arch);	/* the Abstract length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_ABSTRACT;	/* Abstract signature */
    ptr++;
    if (abstract)
      {
	  /* the Abstract */
	  memcpy (ptr, abstract, abstract_len);
	  ptr += abstract_len;
      }
    gaiaExport16 (ptr, geometry_len, 1, endian_arch);	/* the Geometry length in bytes */
    ptr += 2;
    *ptr = GAIA_XML_GEOMETRY;	/* Geometry signature */
    ptr++;
    if (geometry)
      {
	  /* the Geometry */
	  memcpy (ptr, geometry, geometry_len);
	  ptr += geometry_len;
      }

    *ptr = GAIA_XML_PAYLOAD;	/* PAYLOAD signature */
    ptr++;
    if (in_compressed == compressed)
      {
	  /* the unchanged XML payload */
	  memcpy (ptr, zip_buf, out_zip_len);
	  ptr += out_zip_len;
      }
    else if (compressed)
      {
	  /* the compressed XML payload */
	  memcpy (ptr, zip_buf, out_zip_len);
	  free (zip_buf);
	  ptr += out_zip_len;
      }
    else
      {
	  /* the uncompressed XML payload */
	  memcpy (ptr, xml, out_xml_len);
	  free (xml);
	  ptr += out_xml_len;
      }
    *ptr = GAIA_XML_CRC32;	/* CRC32 signature */
    ptr++;
/* computing the CRC32 */
    crc = crc32 (0L, buf, ptr - buf);
    gaiaExportU32 (ptr, crc, 1, endian_arch);	/* the CRC32 */
    ptr += 4;
    *ptr = GAIA_XML_END;	/* END signature */

    *result = buf;
    *out_size = len;
}

static int
is_valid_legacy_xml_blob (const unsigned char *blob, int blob_size)
{
/* Checks if a BLOB actually is a valid LEGACY XmlBLOB buffer */
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    short title_len;
    short abstract_len;
    short geometry_len;
    uLong crc;
    uLong refCrc;
    int endian_arch = gaiaEndianArch ();

/* validity check */
    if (blob_size < 36)
	return 0;		/* cannot be an XmlBLOB */
    if (*blob != GAIA_XML_START)
	return 0;		/* failed to recognize START signature */
    if (*(blob + (blob_size - 1)) != GAIA_XML_END)
	return 0;		/* failed to recognize END signature */
    if (*(blob + (blob_size - 6)) != GAIA_XML_CRC32)
	return 0;		/* failed to recognize CRC32 signature */
    if (*(blob + 2) != GAIA_XML_LEGACY_HEADER)
	return 0;		/* failed to recognize HEADER signature */
    if (*(blob + 13) != GAIA_XML_SCHEMA)
	return 0;		/* failed to recognize SCHEMA signature */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_SCHEMA)
	return 0;
    ptr++;
    ptr += uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_FILEID)
	return 0;
    ptr++;
    ptr += fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_PARENTID)
	return 0;
    ptr++;
    ptr += parentid_len;
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_TITLE)
	return 0;
    ptr++;
    ptr += title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_ABSTRACT)
	return 0;
    ptr++;
    ptr += abstract_len;
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_GEOMETRY)
	return 0;
    ptr++;
    ptr += geometry_len;
    if (*ptr != GAIA_XML_PAYLOAD)
	return 0;

/* verifying the CRC32 */
    crc = crc32 (0L, blob, blob_size - 5);
    refCrc = gaiaImportU32 (blob + blob_size - 5, little_endian, endian_arch);
    if (crc != refCrc)
	return 0;

    return 1;
}

GAIAGEO_DECLARE int
gaiaIsValidXmlBlob (const unsigned char *blob, int blob_size)
{
/* Checks if a BLOB actually is a valid XmlBLOB buffer */
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    short name_len;
    short title_len;
    short abstract_len;
    short geometry_len;
    uLong crc;
    uLong refCrc;
    int endian_arch = gaiaEndianArch ();

    if (blob_size > 3)
      {
	  /* legacy format */
	  if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	      return is_valid_legacy_xml_blob (blob, blob_size);
      }

/* validity check */
    if (blob_size < 39)
	return 0;		/* cannot be an XmlBLOB */
    if (*blob != GAIA_XML_START)
	return 0;		/* failed to recognize START signature */
    if (*(blob + (blob_size - 1)) != GAIA_XML_END)
	return 0;		/* failed to recognize END signature */
    if (*(blob + (blob_size - 6)) != GAIA_XML_CRC32)
	return 0;		/* failed to recognize CRC32 signature */
    if (*(blob + 2) != GAIA_XML_HEADER)
	return 0;		/* failed to recognize HEADER signature */
    if (*(blob + 13) != GAIA_XML_SCHEMA)
	return 0;		/* failed to recognize SCHEMA signature */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_SCHEMA)
	return 0;
    ptr++;
    ptr += uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_FILEID)
	return 0;
    ptr++;
    ptr += fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_PARENTID)
	return 0;
    ptr++;
    ptr += parentid_len;
    name_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_NAME)
	return 0;
    ptr++;
    ptr += name_len;
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_TITLE)
	return 0;
    ptr++;
    ptr += title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_ABSTRACT)
	return 0;
    ptr++;
    ptr += abstract_len;
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 2;
    if (*ptr != GAIA_XML_GEOMETRY)
	return 0;
    ptr++;
    ptr += geometry_len;
    if (*ptr != GAIA_XML_PAYLOAD)
	return 0;

/* verifying the CRC32 */
    crc = crc32 (0L, blob, blob_size - 5);
    refCrc = gaiaImportU32 (blob + blob_size - 5, little_endian, endian_arch);
    if (crc != refCrc)
	return 0;

    return 1;
}

static void
find_xml_namespaces (xmlNode * node, struct gaiaxml_ns_list *list)
{
/* recursively identifying all XML Namespaces from DOM-nodes */
    struct _xmlAttr *attr;
    while (node)
      {
	  xmlNs *ns = node->ns;
	  if (ns != NULL)
	      splite_add_namespace (list, ns->type, ns->prefix, ns->href);
	  attr = node->properties;
	  while (attr != NULL)
	    {
		ns = attr->ns;
		if (ns != NULL)
		    splite_add_namespace (list, ns->type, ns->prefix, ns->href);
		attr = attr->next;
	    }
	  find_xml_namespaces (node->children, list);
	  node = node->next;
      }
}

static void
xml_out (gaiaOutBufferPtr buf, const xmlChar * str)
{
/* clean XML output */
    const xmlChar *p = str;
    while (*p != '\0')
      {
	  if (*p == '>')
	      gaiaAppendToOutBuffer (buf, "&gt;");
	  else if (*p == '<')
	      gaiaAppendToOutBuffer (buf, "&lt;");
	  else if (*p == '&')
	      gaiaAppendToOutBuffer (buf, "&amp;");
	  else if (*p == '"')
	      gaiaAppendToOutBuffer (buf, "&quot;");
	  else if (*p == '\'')
	      gaiaAppendToOutBuffer (buf, "&apos;");
	  else
	    {
		char xx[2];
		xx[0] = *p;
		xx[1] = '\0';
		gaiaAppendToOutBuffer (buf, xx);
	    }
	  p++;
      }
}

static void
format_xml (xmlNode * root, xmlNode * node, struct gaiaxml_ns_list *list,
	    gaiaOutBufferPtr buf, int indent, int *level)
{
/* recursively printing the XML-DOM nodes */
    struct _xmlAttr *attr;
    xmlNode *child;
    xmlNs *ns;
    const xmlChar *namespace;
    char *indenting = NULL;
    const char no = '\0';
    const char *pre;
    int tab;
    int width;
    int has_children;
    int has_text;
    if (!indent)
	pre = &no;
    else
      {
	  if (indent <= 8)
	      tab = indent;
	  else
	      tab = 8;
	  width = tab * *level;
	  indenting = malloc (width + 2);
	  *indenting = '\n';
	  memset (indenting + 1, ' ', width);
	  *(indenting + width + 1) = '\0';
	  pre = indenting;
      }

    while (node)
      {
	  if (node->type == XML_COMMENT_NODE)
	    {
		/* comment node */
		if (*pre != '\0')
		    gaiaAppendToOutBuffer (buf, "<!--");
		else
		    gaiaAppendToOutBuffer (buf, "\n<!--");
		xml_out (buf, node->content);
		gaiaAppendToOutBuffer (buf, "-->");
	    }
	  if (node->type == XML_ELEMENT_NODE)
	    {
		if (*pre != '\0')
		    gaiaAppendToOutBuffer (buf, pre);
		gaiaAppendToOutBuffer (buf, "<");
		ns = node->ns;
		namespace = NULL;
		if (ns != NULL)
		    namespace = ns->prefix;
		if (namespace)
		  {
		      xml_out (buf, namespace);
		      gaiaAppendToOutBuffer (buf, ":");
		  }
		xml_out (buf, node->name);
		if (node == root)
		  {
		      /* Namespaces */
		      struct gaiaxml_namespace *p_ns = list->first;
		      while (p_ns != NULL)
			{
			    if (p_ns->prefix == NULL)
				gaiaAppendToOutBuffer (buf, " xmlns=\"");
			    else
			      {
				  gaiaAppendToOutBuffer (buf, " xmlns:");
				  xml_out (buf, p_ns->prefix);
				  gaiaAppendToOutBuffer (buf, "=\"");
			      }
			    xml_out (buf, p_ns->href);
			    gaiaAppendToOutBuffer (buf, "\"");
			    p_ns = p_ns->next;
			}
		  }
		attr = node->properties;
		while (attr != NULL)
		  {
		      /* attributes */
		      if (attr->type == XML_ATTRIBUTE_NODE)
			{
			    xmlNode *text = attr->children;
			    gaiaAppendToOutBuffer (buf, " ");
			    ns = attr->ns;
			    namespace = NULL;
			    if (ns != NULL)
				namespace = ns->prefix;
			    if (namespace)
			      {
				  xml_out (buf, namespace);
				  gaiaAppendToOutBuffer (buf, ":");
			      }
			    xml_out (buf, attr->name);
			    gaiaAppendToOutBuffer (buf, "=\"");
			    if (text != NULL)
			      {
				  if (text->type == XML_TEXT_NODE)
				      xml_out (buf, text->content);
			      }
			    gaiaAppendToOutBuffer (buf, "\"");
			}
		      attr = attr->next;
		  }
		has_children = 0;
		has_text = 0;
		child = node->children;
		while (child)
		  {
		      if (child->type == XML_ELEMENT_NODE
			  || child->type == XML_COMMENT_NODE)
			  has_children = 1;
		      if (child->type == XML_TEXT_NODE)
			  has_text++;
		      child = child->next;
		  }
		if (has_children)
		    has_text = 0;

		if (!has_text && !has_children)
		    gaiaAppendToOutBuffer (buf, " />");

		if (has_text)
		  {
		      child = node->children;
		      if (child->type == XML_TEXT_NODE)
			{
			    /* text node */
			    gaiaAppendToOutBuffer (buf, ">");
			    xml_out (buf, child->content);
			    gaiaAppendToOutBuffer (buf, "</");
			    ns = node->ns;
			    namespace = NULL;
			    if (ns != NULL)
				namespace = ns->prefix;
			    if (namespace)
			      {
				  xml_out (buf, namespace);
				  gaiaAppendToOutBuffer (buf, ":");
			      }
			    xml_out (buf, node->name);
			    gaiaAppendToOutBuffer (buf, ">");
			}
		  }
		if (has_children)
		  {
		      /* recursively expanding all children */
		      gaiaAppendToOutBuffer (buf, ">");
		      *level += 1;
		      format_xml (root, node->children, list, buf, indent,
				  level);
		      *level -= 1;
		      if (*pre != '\0')
			  gaiaAppendToOutBuffer (buf, pre);
		      gaiaAppendToOutBuffer (buf, "</");
		      ns = node->ns;
		      namespace = NULL;
		      if (ns != NULL)
			  namespace = ns->prefix;
		      if (namespace)
			{
			    xml_out (buf, namespace);
			    gaiaAppendToOutBuffer (buf, ":");
			}
		      xml_out (buf, node->name);
		      gaiaAppendToOutBuffer (buf, ">");
		  }
	    }
	  node = node->next;
      }
    if (indenting)
	free (indenting);
}

static int
gaiaXmlFormat (xmlDocPtr xml_doc, xmlChar ** out, int *out_len,
	       const xmlChar * encoding, int indent)
{
/* reformatting an XML Document - properly indenting */
    int level = 0;
    int ret;
    gaiaOutBuffer buf;
    const xmlChar *version = xml_doc->version;
    xmlNode *root = xmlDocGetRootElement (xml_doc);
    struct gaiaxml_ns_list *list = splite_create_ns_list ();

    gaiaOutBufferInitialize (&buf);
    if (encoding != NULL)
      {
	  gaiaAppendToOutBuffer (&buf, "<?xml version=\"");
	  gaiaAppendToOutBuffer (&buf, (const char *) version);
	  gaiaAppendToOutBuffer (&buf, "\" encoding=\"");
	  gaiaAppendToOutBuffer (&buf, (const char *) encoding);
	  gaiaAppendToOutBuffer (&buf, "\"?>");
      }
    else
      {
	  gaiaAppendToOutBuffer (&buf, "<?xml version=\"");
	  gaiaAppendToOutBuffer (&buf, (const char *) version);
	  gaiaAppendToOutBuffer (&buf, "\"?>");
      }

    find_xml_namespaces (root, list);
    format_xml (root, root, list, &buf, indent, &level);
    splite_free_ns_list (list);

    if (buf.Error == 0 && buf.Buffer != NULL)
      {
	  xmlChar *output;
	  /* terminating the last line */
	  gaiaAppendToOutBuffer (&buf, "\n");
	  output = malloc (buf.WriteOffset + 1);
	  memcpy (output, buf.Buffer, buf.WriteOffset);
	  /* NULL-terminated string */
	  *(output + buf.WriteOffset) = '\0';
	  *out = output;
	  *out_len = buf.WriteOffset + 1;
	  ret = 1;
      }
    else
      {
	  *out = NULL;
	  *out_len = 0;
	  ret = 0;
      }
    gaiaOutBufferReset (&buf);
    return ret;
}

GAIAGEO_DECLARE char *
gaiaXmlTextFromBlob (const unsigned char *blob, int blob_size, int indent)
{
/* attempting to extract an XMLDocument from within an XmlBLOB buffer */
    int compressed = 0;
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    int xml_len;
    int zip_len;
    short uri_len;
    short fileid_len;
    short parentid_len;
    short name_len = 0;
    short title_len;
    short abstract_len;
    short geometry_len;
    unsigned char *xml;
    xmlDocPtr xml_doc;
    xmlChar *out;
    int out_len;
    xmlChar *encoding = NULL;
    void *cvt;
    char *utf8;
    int err;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();
    xmlGenericErrorFunc silentError = (xmlGenericErrorFunc) spliteSilentError;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return NULL;		/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    if ((flag & GAIA_XML_COMPRESSED) == GAIA_XML_COMPRESSED)
	compressed = 1;
    xml_len = gaiaImport32 (blob + 3, little_endian, endian_arch);
    zip_len = gaiaImport32 (blob + 7, little_endian, endian_arch);
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3 + name_len;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + abstract_len;
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + geometry_len;
    ptr++;

    if (compressed)
      {
	  /* unzipping the XML payload */
	  uLong refLen = xml_len;
	  const Bytef *in = ptr;
	  xml = malloc (xml_len + 1);
	  if (uncompress (xml, &refLen, in, zip_len) != Z_OK)
	    {
		/* uncompress error */
		spatialite_e ("XmlBLOB DEFLATE uncompress error\n");
		free (xml);
		return NULL;
	    }
	  *(xml + xml_len) = '\0';
      }
    else
      {
	  /* just copying the uncompressed XML payload */
	  xml = malloc (xml_len + 1);
	  memcpy (xml, ptr, xml_len);
	  *(xml + xml_len) = '\0';
      }
/* retrieving the XMLDocument encoding */
    xmlSetGenericErrorFunc (NULL, silentError);
    xml_doc =
	xmlReadMemory ((const char *) xml, xml_len, "noname.xml", NULL, 0);
    if (xml_doc == NULL)
      {
	  /* parsing error; not a well-formed XML */
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return NULL;
      }
    if (xml_doc->encoding)
      {
	  /* using the internal character enconding */
	  int enclen = (int) strlen ((const char *) xml_doc->encoding);
	  encoding = malloc (enclen + 1);
	  strcpy ((char *) encoding, (const char *) (xml_doc->encoding));
      }
    else
      {
	  /* no declared encoding: defaulting to UTF-8 */
	  encoding = malloc (6);
	  strcpy ((char *) encoding, "UTF-8");
      }

    if (indent < 0)
      {
	  /* just returning the XMLDocument "as is" */
	  xmlFreeDoc (xml_doc);
	  cvt = gaiaCreateUTF8Converter ((const char *) encoding);
	  free (encoding);
	  if (cvt == NULL)
	    {
		xmlSetGenericErrorFunc ((void *) stderr, NULL);
		return NULL;
	    }
	  utf8 = gaiaConvertToUTF8 (cvt, (const char *) xml, xml_len, &err);
	  free (xml);
	  gaiaFreeUTF8Converter (cvt);
	  if (utf8 && !err)
	    {
		xmlSetGenericErrorFunc ((void *) stderr, NULL);
		return utf8;
	    }
	  if (utf8)
	      free (utf8);
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return NULL;
      }

/* properly indenting the XMLDocument */
    gaiaXmlFormat (xml_doc, &out, &out_len, encoding, indent);
    free (xml);
    xmlFreeDoc (xml_doc);
    free (encoding);
    if (out)
      {
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return (char *) out;
      }
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
    return NULL;
}

GAIAGEO_DECLARE void
gaiaXmlFromBlob (const unsigned char *blob, int blob_size, int indent,
		 unsigned char **result, int *res_size)
{
/* attempting to extract an XMLDocument from within an XmlBLOB buffer */
    int compressed = 0;
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    int xml_len;
    int zip_len;
    short uri_len;
    short fileid_len;
    short parentid_len;
    short name_len;
    short title_len;
    short abstract_len;
    short geometry_len;
    unsigned char *xml;
    xmlDocPtr xml_doc;
    xmlChar *out;
    int out_len;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();
    xmlGenericErrorFunc silentError = (xmlGenericErrorFunc) spliteSilentError;
    *result = NULL;
    *res_size = 0;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return;			/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    if ((flag & GAIA_XML_COMPRESSED) == GAIA_XML_COMPRESSED)
	compressed = 1;
    xml_len = gaiaImport32 (blob + 3, little_endian, endian_arch);
    zip_len = gaiaImport32 (blob + 7, little_endian, endian_arch);
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3 + name_len;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + abstract_len;
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + geometry_len;
    ptr++;

    if (compressed)
      {
	  /* unzipping the XML payload */
	  uLong refLen = xml_len;
	  const Bytef *in = ptr;
	  xml = malloc (xml_len + 1);
	  if (uncompress (xml, &refLen, in, zip_len) != Z_OK)
	    {
		/* uncompress error */
		spatialite_e ("XmlBLOB DEFLATE uncompress error\n");
		free (xml);
		return;
	    }
	  *(xml + xml_len) = '\0';
      }
    else
      {
	  /* just copying the uncompressed XML payload */
	  xml = malloc (xml_len + 1);
	  memcpy (xml, ptr, xml_len);
	  *(xml + xml_len) = '\0';
      }
    if (indent < 0)
      {
	  /* just returning the XMLDocument "as is" */
	  *result = xml;
	  *res_size = xml_len;
	  return;
      }

/* properly indenting the XMLDocument */
    xmlSetGenericErrorFunc (NULL, silentError);
    xml_doc =
	xmlReadMemory ((const char *) xml, xml_len, "noname.xml", NULL, 0);
    if (xml_doc == NULL)
      {
	  /* parsing error; not a well-formed XML */
	  *result = xml;
	  *res_size = xml_len;
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return;
      }
    gaiaXmlFormat (xml_doc, &out, &out_len, xml_doc->encoding, indent);
    free (xml);
    xmlFreeDoc (xml_doc);
    *result = out;
    *res_size = out_len;
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
}

GAIAGEO_DECLARE int
gaiaXmlLoad (const void *p_cache, const char *path_or_url,
	     unsigned char **result, int *size, char **parsing_errors)
{
/* attempting to load an external XML Document into a BLOB buffer */
    unsigned char *out;
    int len;
    xmlDocPtr xml_doc;
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    gaiaOutBufferPtr parsingBuf = NULL;
    xmlGenericErrorFunc parsingError = NULL;
    if (is_valid_cache (cache))
      {
	  parsingBuf = (gaiaOutBufferPtr) (cache->xmlParsingErrors);
	  parsingError = (xmlGenericErrorFunc) spliteParsingError;
	  spliteResetXmlErrors (cache);
      }

    *result = NULL;
    *size = 0;
    if (parsing_errors)
	*parsing_errors = NULL;
    if (path_or_url == NULL)
	return 0;

/* testing if the XMLDocument is well-formed */
    xmlSetGenericErrorFunc (cache, parsingError);
    xml_doc = xmlReadFile (path_or_url, NULL, 0);
    if (xml_doc == NULL)
      {
	  /* parsing error; not a well-formed XML */
	  spatialite_e ("XML parsing error\n");
	  if (parsing_errors && parsingBuf)
	      *parsing_errors = parsingBuf->Buffer;
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return 0;
      }
    if (parsing_errors && parsingBuf)
	*parsing_errors = parsingBuf->Buffer;

/* exporting the XML Document into a BLOB */
    xmlDocDumpFormatMemory (xml_doc, &out, &len, 0);
    xmlFreeDoc (xml_doc);
    *result = out;
    *size = len;
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
    if (out == NULL)
	return 0;
    return 1;
}

GAIAGEO_DECLARE int
gaiaXmlStore (const unsigned char *blob, int size, const char *path, int indent)
{
/* attempting to store an XmlBLOB Document into an external file */
    FILE *fl;
    int wr;
    unsigned char *result = NULL;
    int res_size;
    gaiaXmlFromBlob (blob, size, indent, &result, &res_size);
    if (result == NULL)
	return 0;

/* exporting the XML Document into an external file */
    fl = fopen (path, "wb");
    if (fl == NULL)
      {
	  spatialite_e ("Unable to open \"%s\"\n", path);
	  return 0;
      }
    wr = fwrite (result, 1, res_size, fl);
    if (wr != res_size)
      {
	  spatialite_e
	      ("I/O error: written %d bytes into \"%s\", expected %d\n", wr,
	       path, res_size);
	  fclose (fl);
	  return 0;
      }
    fclose (fl);
    return 1;
}

GAIAGEO_DECLARE int
gaiaIsCompressedXmlBlob (const unsigned char *blob, int blob_size)
{
/* Checks if a valid XmlBLOB buffer is compressed or not */
    int compressed = 0;
    unsigned char flag;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return -1;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_COMPRESSED) == GAIA_XML_COMPRESSED)
	compressed = 1;
    return compressed;
}

GAIAGEO_DECLARE int
gaiaIsSchemaValidatedXmlBlob (const unsigned char *blob, int blob_size)
{
/* Checks if a valid XmlBLOB buffer has successfully passed a formal Schema validation or not */
    int validated = 0;
    unsigned char flag;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return -1;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_VALIDATED) == GAIA_XML_VALIDATED)
	validated = 1;
    return validated;
}

GAIAGEO_DECLARE int
gaiaIsIsoMetadataXmlBlob (const unsigned char *blob, int blob_size)
{
/* Checks if a valid XmlBLOB buffer does actually contains an ISO Metadata or not */
    int iso_metadata = 0;
    unsigned char flag;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return -1;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_ISO_METADATA) == GAIA_XML_ISO_METADATA)
	iso_metadata = 1;
    return iso_metadata;
}

GAIAGEO_DECLARE int
gaiaIsSldSeVectorStyleXmlBlob (const unsigned char *blob, int blob_size)
{
/* Checks if a valid XmlBLOB buffer does actually contains an SLD/SE Vector Style or not */
    int sld_se_style = 0;
    unsigned char flag;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return -1;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_SLD_SE_VECTOR_STYLE) == GAIA_XML_SLD_SE_VECTOR_STYLE)
	sld_se_style = 1;
    return sld_se_style;
}

GAIAGEO_DECLARE int
gaiaIsSldSeRasterStyleXmlBlob (const unsigned char *blob, int blob_size)
{
/* Checks if a valid XmlBLOB buffer does actually contains an SLD/SE Raster Style or not */
    int sld_se_style = 0;
    unsigned char flag;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return -1;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_SLD_SE_RASTER_STYLE) == GAIA_XML_SLD_SE_RASTER_STYLE)
	sld_se_style = 1;
    return sld_se_style;
}

GAIAGEO_DECLARE int
gaiaIsSldStyleXmlBlob (const unsigned char *blob, int blob_size)
{
/* Checks if a valid XmlBLOB buffer does actually contains an SLD Style or not */
    int sld_style = 0;
    unsigned char flag;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return -1;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_SLD_STYLE) == GAIA_XML_SLD_STYLE)
	sld_style = 1;
    return sld_style;
}

GAIAGEO_DECLARE int
gaiaIsSvgXmlBlob (const unsigned char *blob, int blob_size)
{
/* Checks if a valid XmlBLOB buffer does actually contains an SLD/SE Style or not */
    int svg = 0;
    unsigned char flag;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return -1;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_SVG) == GAIA_XML_SVG)
	svg = 1;
    return svg;
}

GAIAGEO_DECLARE int
gaiaXmlBlobGetDocumentSize (const unsigned char *blob, int blob_size)
{
/* Return the XMLDocument size (in bytes) from a valid XmlBLOB buffer */
    int little_endian = 0;
    unsigned char flag;
    int xml_len;
    int endian_arch = gaiaEndianArch ();

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return -1;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    xml_len = gaiaImport32 (blob + 3, little_endian, endian_arch);
    return xml_len;
}

GAIAGEO_DECLARE char *
gaiaXmlBlobGetSchemaURI (const unsigned char *blob, int blob_size)
{
/* Return the SchemaURI from a valid XmlBLOB buffer */
    int little_endian = 0;
    unsigned char flag;
    short uri_len;
    char *uri;
    int endian_arch = gaiaEndianArch ();

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return NULL;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    uri_len = gaiaImport16 (blob + 11, little_endian, endian_arch);
    if (!uri_len)
	return NULL;

    uri = malloc (uri_len + 1);
    memcpy (uri, blob + 14, uri_len);
    *(uri + uri_len) = '\0';
    return uri;
}

GAIAGEO_DECLARE char *
gaiaXmlGetInternalSchemaURI (const void *p_cache, const unsigned char *xml,
			     int xml_len)
{
/* Return the internally defined SchemaURI from a valid XmlDocument */
    xmlDocPtr xml_doc;
    char *uri = NULL;
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;
    xmlGenericErrorFunc silentError = (xmlGenericErrorFunc) spliteSilentError;

/* retrieving the XMLDocument internal SchemaURI (if any) */
    xmlSetGenericErrorFunc (NULL, silentError);
    xml_doc =
	xmlReadMemory ((const char *) xml, xml_len, "noname.xml", NULL, 0);
    if (xml_doc == NULL)
      {
	  /* parsing error; not a well-formed XML */
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return NULL;
      }

    if (vxpath_eval_expr
	(p_cache, xml_doc, "/*/@xsi:schemaLocation", &xpathCtx, &xpathObj))
      {
	  /* attempting first to extract xsi:schemaLocation */
	  xmlNodeSetPtr nodeset = xpathObj->nodesetval;
	  xmlNodePtr node;
	  int num_nodes = (nodeset) ? nodeset->nodeNr : 0;
	  if (num_nodes == 1)
	    {
		node = nodeset->nodeTab[0];
		if (node->type == XML_ATTRIBUTE_NODE)
		  {
		      if (node->children != NULL)
			{
			    if (node->children->content != NULL)
			      {
				  const char *str =
				      (const char *) (node->children->content);
				  const char *ptr = str;
				  int i;
				  int len = strlen (str);
				  for (i = len - 1; i >= 0; i--)
				    {
					if (*(str + i) == ' ')
					  {
					      /* last occurrence of SPACE [namespace/schema separator] */
					      ptr = str + i + 1;
					      break;
					  }
				    }
				  len = strlen (ptr);
				  uri = malloc (len + 1);
				  strcpy (uri, ptr);
			      }
			}
		  }
	    }
	  if (uri != NULL)
	      xmlXPathFreeContext (xpathCtx);
	  xmlXPathFreeObject (xpathObj);
      }
    if (uri == NULL)
      {
	  /* checking for xsi:noNamespaceSchemaLocation */
	  if (vxpath_eval_expr
	      (p_cache, xml_doc, "/*/@xsi:noNamespaceSchemaLocation", &xpathCtx,
	       &xpathObj))
	    {
		xmlNodeSetPtr nodeset = xpathObj->nodesetval;
		xmlNodePtr node;
		int num_nodes = (nodeset) ? nodeset->nodeNr : 0;
		if (num_nodes == 1)
		  {
		      node = nodeset->nodeTab[0];
		      if (node->type == XML_ATTRIBUTE_NODE)
			{
			    if (node->children != NULL)
			      {
				  if (node->children->content != NULL)
				    {
					int len =
					    strlen ((const char *)
						    node->children->content);
					uri = malloc (len + 1);
					strcpy (uri,
						(const char *) node->
						children->content);
				    }
			      }
			}
		  }
		xmlXPathFreeContext (xpathCtx);
		xmlXPathFreeObject (xpathObj);
	    }
      }

    xmlFreeDoc (xml_doc);
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
    return uri;
}

GAIAGEO_DECLARE char *
gaiaXmlBlobGetFileId (const unsigned char *blob, int blob_size)
{
/* Return the FileIdentifier from a valid XmlBLOB buffer */
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    char *file_identifier;
    int endian_arch = gaiaEndianArch ();

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return NULL;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    if (!fileid_len)
	return NULL;
    ptr += 3;

    file_identifier = malloc (fileid_len + 1);
    memcpy (file_identifier, ptr, fileid_len);
    *(file_identifier + fileid_len) = '\0';
    return file_identifier;
}

GAIAGEO_DECLARE char *
gaiaXmlBlobGetParentId (const unsigned char *blob, int blob_size)
{
/* Return the ParentIdentifier from a valid XmlBLOB buffer */
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    char *parent_identifier;
    int endian_arch = gaiaEndianArch ();

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return NULL;		/* cannot be an XmlBLOB */
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    if (!parentid_len)
	return NULL;
    ptr += 3;

    parent_identifier = malloc (parentid_len + 1);
    memcpy (parent_identifier, ptr, parentid_len);
    *(parent_identifier + parentid_len) = '\0';
    return parent_identifier;
}

static xmlNodePtr
find_iso_node (xmlNodePtr root, const char *name)
{
/* scanning the Root node [fileIdentifier or parentIdentifier] */
    xmlNodePtr nodeId = NULL;
    xmlNodePtr node;

    for (node = root->children; node; node = node->next)
      {
	  if (node->type == XML_ELEMENT_NODE)
	    {
		const char *xname = (const char *) (node->name);
		if (strcmp (xname, name) == 0)
		  {
		      nodeId = node;
		      break;
		  }
	    }
      }
    if (nodeId == NULL)
	return NULL;
    for (node = nodeId->children; node; node = node->next)
      {
	  if (node->type == XML_ELEMENT_NODE)
	    {
		const char *xname = (const char *) (node->name);
		if (strcmp (xname, "CharacterString") == 0)
		    return node;
	    }
      }
    return NULL;
}

static int
setIsoId (xmlDocPtr xml_doc, const char *node_name, const char *identifier,
	  unsigned char **out_blob, int *out_len)
{
/* attempting to change an ISO Id */
    xmlNodePtr node;
    xmlNodePtr new_node;
    xmlNodePtr text;
    xmlNodePtr old_node;
    xmlChar *buf;
    int len;
    *out_blob = NULL;
    *out_len = 0;
    node = find_iso_node (xmlDocGetRootElement (xml_doc), node_name);
    if (node == NULL)
	return 0;
/* replacing the existing XML Node */
    new_node = xmlNewNode (node->ns, node->name);
    text = xmlNewText ((xmlChar *) identifier);
    xmlAddChild (new_node, text);
    old_node = xmlReplaceNode (node, new_node);
    xmlFreeNode (old_node);
    xmlDocDumpFormatMemory (xml_doc, &buf, &len, 0);
    if (buf == NULL)
	return 0;
    *out_blob = buf;
    *out_len = len;
    return 1;
}

GAIAGEO_DECLARE int
gaiaXmlBlobSetFileId (const void *p_cache, const unsigned char *blob,
		      int blob_size, const char *identifier,
		      unsigned char **new_blob, int *new_size)
{
/* Return a new XmlBLOB buffer by replacing the FileId value */
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    int compressed = 0;
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    int xml_len;
    int zip_len;
    int name_len;
    short title_len;
    short abstract_len;
    short geometry_len;
    char *schemaURI = NULL;
    unsigned char *xml;
    xmlDocPtr xml_doc;
    unsigned char *out_blob;
    int out_len;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();
    xmlGenericErrorFunc silentError = (xmlGenericErrorFunc) spliteSilentError;

    *new_blob = NULL;
    *new_size = 0;
/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return 0;		/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_ISO_METADATA) == GAIA_XML_ISO_METADATA)
	;
    else
	return 0;		/* not an ISO Metadata XmlBLOB */
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    if ((flag & GAIA_XML_COMPRESSED) == GAIA_XML_COMPRESSED)
	compressed = 1;
    xml_len = gaiaImport32 (blob + 3, little_endian, endian_arch);
    zip_len = gaiaImport32 (blob + 7, little_endian, endian_arch);
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    if (uri_len > 0)
      {
	  schemaURI = malloc (uri_len + 1);
	  memcpy (schemaURI, blob + 14, uri_len);
	  *(schemaURI + uri_len) = '\0';
      }
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3 + name_len;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + abstract_len;
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + geometry_len;
    ptr++;

    if (compressed)
      {
	  /* unzipping the XML payload */
	  uLong refLen = xml_len;
	  const Bytef *in = ptr;
	  xml = malloc (xml_len + 1);
	  if (uncompress (xml, &refLen, in, zip_len) != Z_OK)
	    {
		/* uncompress error */
		spatialite_e ("XmlBLOB DEFLATE uncompress error\n");
		free (xml);
		return 0;
	    }
	  *(xml + xml_len) = '\0';
      }
    else
      {
	  /* just copying the uncompressed XML payload */
	  xml = malloc (xml_len + 1);
	  memcpy (xml, ptr, xml_len);
	  *(xml + xml_len) = '\0';
      }
/* loading the XMLDocument */
    xmlSetGenericErrorFunc (NULL, silentError);
    xml_doc =
	xmlReadMemory ((const char *) xml, xml_len, "noname.xml", NULL, 0);
    if (xml_doc == NULL)
      {
	  /* parsing error; not a well-formed XML */
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return 0;
      }
/* replacing the FileId value */
    setIsoId (xml_doc, "fileIdentifier", identifier, &out_blob, &out_len);
    free (xml);
    xmlFreeDoc (xml_doc);
    if (out_blob)
      {
	  gaiaXmlToBlob (cache, out_blob, out_len, compressed, schemaURI,
			 new_blob, new_size, NULL, NULL);
#ifdef __MINGW32__
/* MinGW is unable to statically link libxml2 if xmlFree() is declared */
	  free (out_blob);
#else
	  xmlFree (out_blob);
#endif
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return 1;
      }
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
    return 0;
}

GAIAGEO_DECLARE int
gaiaXmlBlobSetParentId (const void *p_cache, const unsigned char *blob,
			int blob_size, const char *identifier,
			unsigned char **new_blob, int *new_size)
{
/* Return a new XmlBLOB buffer by replacing the ParentId value */
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    int compressed = 0;
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    int xml_len;
    int zip_len;
    short name_len;
    short title_len;
    short abstract_len;
    short geometry_len;
    char *schemaURI = NULL;
    unsigned char *xml;
    xmlDocPtr xml_doc;
    unsigned char *out_blob;
    int out_len;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();
    xmlGenericErrorFunc silentError = (xmlGenericErrorFunc) spliteSilentError;

    *new_blob = NULL;
    *new_size = 0;
/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return 0;		/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_ISO_METADATA) == GAIA_XML_ISO_METADATA)
	;
    else
	return 0;		/* not an ISO Metadata XmlBLOB */
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    if ((flag & GAIA_XML_COMPRESSED) == GAIA_XML_COMPRESSED)
	compressed = 1;
    xml_len = gaiaImport32 (blob + 3, little_endian, endian_arch);
    zip_len = gaiaImport32 (blob + 7, little_endian, endian_arch);
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    if (uri_len > 0)
      {
	  schemaURI = malloc (uri_len + 1);
	  memcpy (schemaURI, blob + 14, uri_len);
	  *(schemaURI + uri_len) = '\0';
      }
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3 + name_len;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + abstract_len;
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + geometry_len;
    ptr++;

    if (compressed)
      {
	  /* unzipping the XML payload */
	  uLong refLen = xml_len;
	  const Bytef *in = ptr;
	  xml = malloc (xml_len + 1);
	  if (uncompress (xml, &refLen, in, zip_len) != Z_OK)
	    {
		/* uncompress error */
		spatialite_e ("XmlBLOB DEFLATE uncompress error\n");
		free (xml);
		return 0;
	    }
	  *(xml + xml_len) = '\0';
      }
    else
      {
	  /* just copying the uncompressed XML payload */
	  xml = malloc (xml_len + 1);
	  memcpy (xml, ptr, xml_len);
	  *(xml + xml_len) = '\0';
      }
/* loading the XMLDocument */
    xmlSetGenericErrorFunc (NULL, silentError);
    xml_doc =
	xmlReadMemory ((const char *) xml, xml_len, "noname.xml", NULL, 0);
    if (xml_doc == NULL)
      {
	  /* parsing error; not a well-formed XML */
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return 0;
      }
/* replacing the ParentId value */
    setIsoId (xml_doc, "parentIdentifier", identifier, &out_blob, &out_len);
    free (xml);
    xmlFreeDoc (xml_doc);
    if (out_blob)
      {
	  gaiaXmlToBlob (cache, out_blob, out_len, compressed, schemaURI,
			 new_blob, new_size, NULL, NULL);
#ifdef __MINGW32__
/* MinGW is unable to statically link libxml2 if xmlFree() is declared */
	  free (out_blob);
#else
	  xmlFree (out_blob);
#endif
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return 1;
      }
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
    return 0;
}

static xmlNodePtr
find_iso_sibling (xmlNodePtr root, const char *name)
{
/* scanning the Root node [previous sibling] */
    xmlNodePtr node;

    for (node = root->children; node; node = node->next)
      {
	  if (node->type == XML_ELEMENT_NODE)
	    {
		const char *xname = (const char *) (node->name);
		if (strcmp (xname, name) == 0)
		    return node;
	    }
      }
    return NULL;
}

static int
addIsoId (xmlDocPtr xml_doc, const char *node_name, const char *identifier,
	  const char *ns_id, const char *uri_id, const char *ns_charstr,
	  const char *uri_charstr, unsigned char **out_blob, int *out_len)
{
/* attempting to insert a new ISO Id */
    xmlNsPtr ns_id_ptr = NULL;
    xmlNsPtr ns_charstr_ptr = NULL;
    xmlNodePtr id_node;
    xmlNodePtr charstr_node;
    xmlNodePtr text;
    xmlNodePtr root = xmlDocGetRootElement (xml_doc);
    xmlNodePtr before = NULL;
    xmlChar *buf;
    int len;
    *out_blob = NULL;
    *out_len = 0;
    if (find_iso_node (root, node_name))
	return 0;
/* retrieving the ID NameSpace */
    if (uri_id != NULL)
	ns_id_ptr = xmlSearchNsByHref (xml_doc, root, (xmlChar *) uri_id);
    if (ns_id_ptr == NULL)
	ns_id_ptr = xmlSearchNs (xml_doc, root, (xmlChar *) ns_id);
/* inserting the "ID" XML Node */
    id_node = xmlNewNode (ns_id_ptr, (xmlChar *) node_name);
    if (strcmp (node_name, "parentIdentifier") == 0)
      {
	  /* attempting to identify the previous sibling */
	  before = find_iso_sibling (root, "characterSet");
	  if (before == NULL)
	      before = find_iso_sibling (root, "language");
	  if (before == NULL)
	      before = find_iso_sibling (root, "fileIdentifier");
      }
    if (before)
	xmlAddNextSibling (before, id_node);
    else
      {
	  before = root->children;
	  if (before)
	      xmlAddPrevSibling (before, id_node);
	  else
	      xmlAddChild (root, id_node);
      }
    if (ns_id_ptr == NULL && ns_id != NULL && uri_id != NULL)
      {
	  ns_id_ptr = xmlNewNs (root, (xmlChar *) uri_id, (xmlChar *) ns_id);
	  xmlSetNs (id_node, ns_id_ptr);
      }
/* retrieving the CharacterString NameSpace */
    if (uri_charstr != NULL)
	ns_charstr_ptr =
	    xmlSearchNsByHref (xml_doc, root, (xmlChar *) uri_charstr);
    if (ns_charstr_ptr == NULL)
	ns_charstr_ptr = xmlSearchNs (xml_doc, root, (xmlChar *) ns_charstr);
/* inserting the "CharacterString" XML Node */
    charstr_node = xmlNewNode (ns_charstr_ptr, (xmlChar *) "CharacterString");
    xmlAddChild (id_node, charstr_node);
    if (ns_charstr_ptr == NULL && ns_charstr != NULL && uri_charstr != NULL)
      {
	  ns_charstr_ptr =
	      xmlNewNs (root, (xmlChar *) uri_charstr, (xmlChar *) ns_charstr);
	  xmlSetNs (charstr_node, ns_charstr_ptr);
      }
    text = xmlNewText ((xmlChar *) identifier);
    xmlAddChild (charstr_node, text);
    xmlDocDumpFormatMemory (xml_doc, &buf, &len, 0);
    if (buf == NULL)
	return 0;
    *out_blob = buf;
    *out_len = len;
    return 1;
}

GAIAGEO_DECLARE int
gaiaXmlBlobAddFileId (const void *p_cache, const unsigned char *blob,
		      int blob_size, const char *identifier, const char *ns_id,
		      const char *uri_id, const char *ns_charstr,
		      const char *uri_charstr, unsigned char **new_blob,
		      int *new_size)
{
/* Return a new XmlBLOB buffer by inserting a FileId value */
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    int compressed = 0;
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    int xml_len;
    int zip_len;
    short name_len;
    short title_len;
    short abstract_len;
    short geometry_len;
    char *schemaURI = NULL;
    unsigned char *xml;
    xmlDocPtr xml_doc;
    unsigned char *out_blob;
    int out_len;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();
    xmlGenericErrorFunc silentError = (xmlGenericErrorFunc) spliteSilentError;

    *new_blob = NULL;
    *new_size = 0;
/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return 0;		/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_ISO_METADATA) == GAIA_XML_ISO_METADATA)
	;
    else
	return 0;		/* not an ISO Metadata XmlBLOB */
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    if ((flag & GAIA_XML_COMPRESSED) == GAIA_XML_COMPRESSED)
	compressed = 1;
    xml_len = gaiaImport32 (blob + 3, little_endian, endian_arch);
    zip_len = gaiaImport32 (blob + 7, little_endian, endian_arch);
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    if (uri_len > 0)
      {
	  schemaURI = malloc (uri_len + 1);
	  memcpy (schemaURI, blob + 14, uri_len);
	  *(schemaURI + uri_len) = '\0';
      }
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3 + name_len;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + abstract_len;
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + geometry_len;
    ptr++;

    if (compressed)
      {
	  /* unzipping the XML payload */
	  uLong refLen = xml_len;
	  const Bytef *in = ptr;
	  xml = malloc (xml_len + 1);
	  if (uncompress (xml, &refLen, in, zip_len) != Z_OK)
	    {
		/* uncompress error */
		spatialite_e ("XmlBLOB DEFLATE uncompress error\n");
		free (xml);
		return 0;
	    }
	  *(xml + xml_len) = '\0';
      }
    else
      {
	  /* just copying the uncompressed XML payload */
	  xml = malloc (xml_len + 1);
	  memcpy (xml, ptr, xml_len);
	  *(xml + xml_len) = '\0';
      }
/* loading the XMLDocument */
    xmlSetGenericErrorFunc (NULL, silentError);
    xml_doc =
	xmlReadMemory ((const char *) xml, xml_len, "noname.xml", NULL, 0);
    if (xml_doc == NULL)
      {
	  /* parsing error; not a well-formed XML */
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return 0;
      }
/* inserting the FileId value */
    addIsoId (xml_doc, "fileIdentifier", identifier, ns_id, uri_id, ns_charstr,
	      uri_charstr, &out_blob, &out_len);
    free (xml);
    xmlFreeDoc (xml_doc);
    if (out_blob)
      {
	  gaiaXmlToBlob (cache, out_blob, out_len, compressed, schemaURI,
			 new_blob, new_size, NULL, NULL);
#ifdef __MINGW32__
/* MinGW is unable to statically link libxml2 if xmlFree() is declared */
	  free (out_blob);
#else
	  xmlFree (out_blob);
#endif
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return 1;
      }
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
    return 0;
}

GAIAGEO_DECLARE int
gaiaXmlBlobAddParentId (const void *p_cache, const unsigned char *blob,
			int blob_size, const char *identifier,
			const char *ns_id, const char *uri_id,
			const char *ns_charstr, const char *uri_charstr,
			unsigned char **new_blob, int *new_size)
{
/* Return a new XmlBLOB buffer by inserting a ParentId value */
    struct splite_internal_cache *cache =
	(struct splite_internal_cache *) p_cache;
    int compressed = 0;
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    int xml_len;
    int zip_len;
    short name_len;
    short title_len;
    short abstract_len;
    short geometry_len;
    char *schemaURI = NULL;
    unsigned char *xml;
    xmlDocPtr xml_doc;
    unsigned char *out_blob;
    int out_len;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();
    xmlGenericErrorFunc silentError = (xmlGenericErrorFunc) spliteSilentError;

    *new_blob = NULL;
    *new_size = 0;
/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return 0;		/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_ISO_METADATA) == GAIA_XML_ISO_METADATA)
	;
    else
	return 0;		/* not an ISO Metadata XmlBLOB */
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    if ((flag & GAIA_XML_COMPRESSED) == GAIA_XML_COMPRESSED)
	compressed = 1;
    xml_len = gaiaImport32 (blob + 3, little_endian, endian_arch);
    zip_len = gaiaImport32 (blob + 7, little_endian, endian_arch);
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    if (uri_len > 0)
      {
	  schemaURI = malloc (uri_len + 1);
	  memcpy (schemaURI, blob + 14, uri_len);
	  *(schemaURI + uri_len) = '\0';
      }
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3 + name_len;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + abstract_len;
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + geometry_len;
    ptr++;

    if (compressed)
      {
	  /* unzipping the XML payload */
	  uLong refLen = xml_len;
	  const Bytef *in = ptr;
	  xml = malloc (xml_len + 1);
	  if (uncompress (xml, &refLen, in, zip_len) != Z_OK)
	    {
		/* uncompress error */
		spatialite_e ("XmlBLOB DEFLATE uncompress error\n");
		free (xml);
		return 0;
	    }
	  *(xml + xml_len) = '\0';
      }
    else
      {
	  /* just copying the uncompressed XML payload */
	  xml = malloc (xml_len + 1);
	  memcpy (xml, ptr, xml_len);
	  *(xml + xml_len) = '\0';
      }
/* loading the XMLDocument */
    xmlSetGenericErrorFunc (NULL, silentError);
    xml_doc =
	xmlReadMemory ((const char *) xml, xml_len, "noname.xml", NULL, 0);
    if (xml_doc == NULL)
      {
	  /* parsing error; not a well-formed XML */
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return 0;
      }
/* inserting the ParentId value */
    addIsoId (xml_doc, "parentIdentifier", identifier, ns_id, uri_id,
	      ns_charstr, uri_charstr, &out_blob, &out_len);
    free (xml);
    xmlFreeDoc (xml_doc);
    if (out_blob)
      {
	  gaiaXmlToBlob (cache, out_blob, out_len, compressed, schemaURI,
			 new_blob, new_size, NULL, NULL);
#ifdef __MINGW32__
/* MinGW is unable to statically link libxml2 if xmlFree() is declared */
	  free (out_blob);
#else
	  xmlFree (out_blob);
#endif
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return 1;
      }
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
    return 0;
}

GAIAGEO_DECLARE char *
gaiaXmlBlobGetName (const unsigned char *blob, int blob_size)
{
/* Return the Name from a valid XmlBLOB buffer */
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    short name_len;
    char *name;
    int endian_arch = gaiaEndianArch ();

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return NULL;		/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	return NULL;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    name_len = gaiaImport16 (ptr, little_endian, endian_arch);
    if (!name_len)
	return NULL;
    ptr += 3;

    name = malloc (name_len + 1);
    memcpy (name, ptr, name_len);
    *(name + name_len) = '\0';
    return name;
}

GAIAGEO_DECLARE char *
gaiaXmlBlobGetTitle (const unsigned char *blob, int blob_size)
{
/* Return the Title from a valid XmlBLOB buffer */
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    short name_len;
    short title_len;
    char *title;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return NULL;		/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3 + name_len;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    if (!title_len)
	return NULL;
    ptr += 3;

    title = malloc (title_len + 1);
    memcpy (title, ptr, title_len);
    *(title + title_len) = '\0';
    return title;
}

GAIAGEO_DECLARE char *
gaiaXmlBlobGetAbstract (const unsigned char *blob, int blob_size)
{
/* Return the Abstract from a valid XmlBLOB buffer */
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    short name_len;
    short title_len;
    short abstract_len;
    char *abstract;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return NULL;		/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3 + name_len;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    if (!abstract_len)
	return NULL;
    ptr += 3;

    abstract = malloc (abstract_len + 1);
    memcpy (abstract, ptr, abstract_len);
    *(abstract + abstract_len) = '\0';
    return abstract;
}

GAIAGEO_DECLARE void
gaiaXmlBlobGetGeometry (const unsigned char *blob, int blob_size,
			unsigned char **blob_geom, int *geom_size)
{
/* Return the Geometry from a valid XmlBLOB buffer */
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    short uri_len;
    short fileid_len;
    short parentid_len;
    short name_len;
    short title_len;
    short abstract_len;
    short geometry_len;
    unsigned char *geometry;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();

    *blob_geom = NULL;
    *geom_size = 0;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return;			/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3 + name_len;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + abstract_len;
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    if (!geometry_len)
	return;
    ptr += 3;

    geometry = malloc (geometry_len);
    memcpy (geometry, ptr, geometry_len);
    *blob_geom = geometry;
    *geom_size = geometry_len;
}

GAIAGEO_DECLARE char *
gaiaXmlBlobGetEncoding (const unsigned char *blob, int blob_size)
{
/* Return the Charset Encoding from a valid XmlBLOB buffer */
    int compressed = 0;
    int little_endian = 0;
    unsigned char flag;
    const unsigned char *ptr;
    int xml_len;
    int zip_len;
    short uri_len;
    short fileid_len;
    short parentid_len;
    short name_len;
    short title_len;
    short abstract_len;
    short geometry_len;
    unsigned char *xml;
    xmlDocPtr xml_doc;
    char *encoding = NULL;
    int legacy_blob = 0;
    int endian_arch = gaiaEndianArch ();
    xmlGenericErrorFunc silentError = (xmlGenericErrorFunc) spliteSilentError;

/* validity check */
    if (!gaiaIsValidXmlBlob (blob, blob_size))
	return NULL;		/* cannot be an XmlBLOB */
    if (*(blob + 2) == GAIA_XML_LEGACY_HEADER)
	legacy_blob = 1;
    flag = *(blob + 1);
    if ((flag & GAIA_XML_LITTLE_ENDIAN) == GAIA_XML_LITTLE_ENDIAN)
	little_endian = 1;
    if ((flag & GAIA_XML_COMPRESSED) == GAIA_XML_COMPRESSED)
	compressed = 1;
    xml_len = gaiaImport32 (blob + 3, little_endian, endian_arch);
    zip_len = gaiaImport32 (blob + 7, little_endian, endian_arch);
    ptr = blob + 11;
    uri_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + uri_len;
    fileid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + fileid_len;
    parentid_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + parentid_len;
    if (!legacy_blob)
      {
	  name_len = gaiaImport16 (ptr, little_endian, endian_arch);
	  ptr += 3 + name_len;
      }
    title_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + title_len;
    abstract_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + abstract_len;
    geometry_len = gaiaImport16 (ptr, little_endian, endian_arch);
    ptr += 3 + geometry_len;
    ptr++;
    if (compressed)
      {
	  /* unzipping the XML payload */
	  uLong refLen = xml_len;
	  const Bytef *in = ptr;
	  xml = malloc (xml_len + 1);
	  if (uncompress (xml, &refLen, in, zip_len) != Z_OK)
	    {
		/* uncompress error */
		spatialite_e ("XmlBLOB DEFLATE uncompress error\n");
		free (xml);
		return NULL;
	    }
	  *(xml + xml_len) = '\0';
      }
    else
      {
	  /* just copying the uncompressed XML payload */
	  xml = malloc (xml_len + 1);
	  memcpy (xml, ptr, xml_len);
	  *(xml + xml_len) = '\0';
      }
/* retrieving the XMLDocument encoding */
    xmlSetGenericErrorFunc (NULL, silentError);
    xml_doc =
	xmlReadMemory ((const char *) xml, xml_len, "noname.xml", NULL, 0);
    if (xml_doc == NULL)
      {
	  /* parsing error; not a well-formed XML */
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return NULL;
      }
    free (xml);
    if (xml_doc->encoding)
      {
	  /* using the internal character enconding */
	  int enclen = strlen ((const char *) xml_doc->encoding);
	  encoding = malloc (enclen + 1);
	  strcpy (encoding, (const char *) xml_doc->encoding);
	  xmlFreeDoc (xml_doc);
	  xmlSetGenericErrorFunc ((void *) stderr, NULL);
	  return encoding;
      }
    xmlFreeDoc (xml_doc);
    xmlSetGenericErrorFunc ((void *) stderr, NULL);
    return NULL;
}

GAIAGEO_DECLARE char *
gaia_libxml2_version (void)
{
/* return the current LIBXML2 version */
    int len;
    char *version;
    const char *ver = LIBXML_DOTTED_VERSION;
    len = strlen (ver);
    version = malloc (len + 1);
    strcpy (version, ver);
    return version;
}

#endif /* end LIBXML2: supporting XML documents */
