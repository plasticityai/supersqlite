/*

 gg_shape.c -- Gaia shapefile handling
  
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

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <errno.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include "config-msvc.h"
#else
#include "config.h"
#endif

#if OMIT_ICONV == 0		/* if ICONV is disabled no SHP support is available */

#if defined(__MINGW32__) || defined(_WIN32)
#define LIBICONV_STATIC
#include <iconv.h>
#define LIBCHARSET_STATIC
#ifdef _MSC_VER
/* <localcharset.h> isn't supported on OSGeo4W */
/* applying a tricky workaround to fix this issue */
extern const char *locale_charset (void);
#else /* sane Windows - not OSGeo4W */
#include <localcharset.h>
#endif /* end localcharset */
#else /* not MINGW32 */
#if defined(__APPLE__) || defined(__ANDROID__)
#include <iconv.h>
#include <localcharset.h>
#else /* neither Mac OsX nor Android */
#include <iconv.h>
#include <langinfo.h>
#endif
#endif

#include <spatialite/sqlite.h>

#include <spatialite/gaiageo.h>
#include <spatialite/debug.h>

#ifdef _WIN32
#define atoll	_atoi64
#endif /* not WIN32 */

/* 64 bit integer: portable format for printf() */
#if defined(_WIN32) && !defined(__MINGW32__)
#define FRMT64 "%I64d"
#else
#define FRMT64 "%lld"
#endif

#define SHAPEFILE_NO_DATA 1e-38

#ifdef _WIN32
#define strcasecmp	_stricmp
#endif /* not WIN32 */

struct auxdbf_fld
{
/* auxiliary DBF field struct */
    char *name;
    struct auxdbf_fld *next;
};

struct auxdbf_list
{
/* auxiliary DBF struct */
    struct auxdbf_fld *first;
    struct auxdbf_fld *last;
};

GAIAGEO_DECLARE void
gaiaFreeValue (gaiaValuePtr p)
{
/* frees all memory allocations for this DBF Field value */
    if (!p)
	return;
    if (p->TxtValue)
	free (p->TxtValue);
    free (p);
}

GAIAGEO_DECLARE void
gaiaSetNullValue (gaiaDbfFieldPtr field)
{
/* assignes a NULL value to some DBF field */
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_NULL_VALUE;
    field->Value->TxtValue = NULL;
}

GAIAGEO_DECLARE void
gaiaSetIntValue (gaiaDbfFieldPtr field, sqlite3_int64 value)
{
/* assignes an INTEGER value to some DBF field */
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_INT_VALUE;
    field->Value->TxtValue = NULL;
    field->Value->IntValue = value;
}

GAIAGEO_DECLARE void
gaiaSetDoubleValue (gaiaDbfFieldPtr field, double value)
{
/* assignes a DOUBLE value to some DBF field */
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_DOUBLE_VALUE;
    field->Value->TxtValue = NULL;
    field->Value->DblValue = value;
}

GAIAGEO_DECLARE void
gaiaSetStrValue (gaiaDbfFieldPtr field, char *str)
{
/* assignes a STRING value to some DBF field */
    int len = strlen (str);
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_TEXT_VALUE;
    field->Value->TxtValue = malloc (len + 1);
    strcpy (field->Value->TxtValue, str);
}

GAIAGEO_DECLARE gaiaDbfFieldPtr
gaiaAllocDbfField (char *name, unsigned char type,
		   int offset, unsigned char length, unsigned char decimals)
{
/* allocates and initializes a DBF Field definition */
    gaiaDbfFieldPtr p = malloc (sizeof (gaiaDbfField));
    int len = strlen (name);
    p->Name = malloc (len + 1);
    strcpy (p->Name, name);
    p->Type = type;
    p->Offset = offset;
    p->Length = length;
    p->Decimals = decimals;
    p->Value = NULL;
    p->Next = NULL;
    return p;
}

GAIAGEO_DECLARE void
gaiaFreeDbfField (gaiaDbfFieldPtr p)
{
/* frees all memory allocations for this DBF Field definition */
    if (!p)
	return;
    if (p->Name)
	free (p->Name);
    if (p->Value)
	gaiaFreeValue (p->Value);
    free (p);
}

GAIAGEO_DECLARE gaiaDbfFieldPtr
gaiaCloneDbfField (gaiaDbfFieldPtr org)
{
/* creating a new DBF LIST copied from the original one */
    gaiaDbfFieldPtr p = malloc (sizeof (gaiaDbfField));
    int len = strlen (org->Name);
    p->Name = malloc (len + 1);
    strcpy (p->Name, org->Name);
    p->Type = org->Type;
    p->Offset = org->Offset;
    p->Length = org->Length;
    p->Decimals = org->Decimals;
    p->Value = gaiaCloneValue (org->Value);
    p->Next = NULL;
    return p;
}

GAIAGEO_DECLARE gaiaDbfListPtr
gaiaAllocDbfList ()
{
/* allocates and initializes the DBF Fields list */
    gaiaDbfListPtr list = malloc (sizeof (gaiaDbfList));
    list->RowId = 0;
    list->Geometry = NULL;
    list->First = NULL;
    list->Last = NULL;
    return list;
}

GAIAGEO_DECLARE void
gaiaFreeDbfList (gaiaDbfListPtr list)
{
/* frees all memory allocations related to DBF Fields list */
    gaiaDbfFieldPtr p;
    gaiaDbfFieldPtr pn;
    if (!list)
	return;
    p = list->First;
    while (p)
      {
	  pn = p->Next;
	  gaiaFreeDbfField (p);
	  p = pn;
      }
    if (list->Geometry)
	gaiaFreeGeomColl (list->Geometry);
    free (list);
}

GAIAGEO_DECLARE int
gaiaIsValidDbfList (gaiaDbfListPtr list)
{
/* checks if the DBF fields list contains any invalid data type */
    gaiaDbfFieldPtr p;
    if (!list)
	return 0;
    p = list->First;
    while (p)
      {
	  if (p->Type == 'N' || p->Type == 'C' || p->Type == 'L'
	      || p->Type == 'D' || p->Type == 'F')
	      ;
	  else
	      return 0;
	  p = p->Next;
      }
    return 1;
}

GAIAGEO_DECLARE gaiaDbfFieldPtr
gaiaAddDbfField (gaiaDbfListPtr list, char *name, unsigned char type,
		 int offset, unsigned char length, unsigned char decimals)
{
/* inserts a Field in the DBF Fields list */
    gaiaDbfFieldPtr p;
    if (!list)
	return NULL;
    p = gaiaAllocDbfField (name, type, offset, length, decimals);
    if (!(list->First))
	list->First = p;
    if (list->Last)
	list->Last->Next = p;
    list->Last = p;
    return p;
}

GAIAGEO_DECLARE void
gaiaResetDbfEntity (gaiaDbfListPtr list)
{
/* resets data values */
    gaiaDbfFieldPtr p;
    if (!list)
	return;
    p = list->First;
    while (p)
      {
	  if (p->Value)
	      gaiaFreeValue (p->Value);
	  p->Value = NULL;
	  p = p->Next;
      }
    if (list->Geometry)
	gaiaFreeGeomColl (list->Geometry);
    list->Geometry = NULL;
}

GAIAGEO_DECLARE gaiaValuePtr
gaiaCloneValue (gaiaValuePtr org)
{
/* creating a new VARIANT value copied from the original one */
    gaiaValuePtr value;
    int len;
    value = malloc (sizeof (gaiaValue));
    value->Type = GAIA_NULL_VALUE;
    value->TxtValue = NULL;
    switch (org->Type)
      {
      case GAIA_INT_VALUE:
	  value->Type = GAIA_INT_VALUE;
	  value->IntValue = org->IntValue;
	  break;
      case GAIA_DOUBLE_VALUE:
	  value->Type = GAIA_DOUBLE_VALUE;
	  value->DblValue = org->DblValue;
	  break;
      case GAIA_TEXT_VALUE:
	  value->Type = GAIA_TEXT_VALUE;
	  len = strlen (org->TxtValue);
	  value->TxtValue = malloc (len + 1);
	  strcpy (value->TxtValue, org->TxtValue);
      };
    return value;
}

GAIAGEO_DECLARE gaiaDbfListPtr
gaiaCloneDbfEntity (gaiaDbfListPtr org)
{
/* creating a new DBF LIST copied from the original one */
    gaiaDbfFieldPtr p;
    gaiaDbfFieldPtr newFld;
    gaiaDbfListPtr entity = gaiaAllocDbfList ();
    entity->RowId = org->RowId;
    if (org->Geometry)
	entity->Geometry = gaiaCloneGeomColl (org->Geometry);
    p = org->First;
    while (p)
      {
	  newFld =
	      gaiaAddDbfField (entity, p->Name, p->Type, p->Offset, p->Length,
			       p->Decimals);
	  if (p->Value)
	      newFld->Value = gaiaCloneValue (p->Value);
	  p = p->Next;
      }
    return entity;
}

GAIAGEO_DECLARE gaiaShapefilePtr
gaiaAllocShapefile ()
{
/* allocates and initializes the Shapefile object */
    gaiaShapefilePtr shp = malloc (sizeof (gaiaShapefile));
    shp->endian_arch = 1;
    shp->Path = NULL;
    shp->Shape = -1;
    shp->EffectiveType = GAIA_UNKNOWN;
    shp->EffectiveDims = GAIA_XY;
    shp->flShp = NULL;
    shp->flShx = NULL;
    shp->flDbf = NULL;
    shp->Dbf = NULL;
    shp->BufShp = NULL;
    shp->ShpBfsz = 0;
    shp->BufDbf = NULL;
    shp->DbfHdsz = 0;
    shp->DbfReclen = 0;
    shp->DbfSize = 0;
    shp->DbfRecno = 0;
    shp->ShpSize = 0;
    shp->ShxSize = 0;
    shp->MinX = DBL_MAX;
    shp->MinY = DBL_MAX;
    shp->MaxX = -DBL_MAX;
    shp->MaxY = -DBL_MAX;
    shp->Valid = 0;
    shp->IconvObj = NULL;
    shp->LastError = NULL;
    return shp;
}

GAIAGEO_DECLARE void
gaiaFreeShapefile (gaiaShapefilePtr shp)
{
/* frees all memory allocations related to the Shapefile object */
    if (shp->Path)
	free (shp->Path);
    if (shp->flShp)
	fclose (shp->flShp);
    if (shp->flShx)
	fclose (shp->flShx);
    if (shp->flDbf)
	fclose (shp->flDbf);
    if (shp->Dbf)
	gaiaFreeDbfList (shp->Dbf);
    if (shp->BufShp)
	free (shp->BufShp);
    if (shp->BufDbf)
	free (shp->BufDbf);
    if (shp->IconvObj)
	iconv_close ((iconv_t) shp->IconvObj);
    if (shp->LastError)
	free (shp->LastError);
    free (shp);
}

GAIAGEO_DECLARE void
gaiaOpenShpRead (gaiaShapefilePtr shp, const char *path, const char *charFrom,
		 const char *charTo)
{
/* trying to open the shapefile and initial checkings */
    FILE *fl_shx = NULL;
    FILE *fl_shp = NULL;
    FILE *fl_dbf = NULL;
    char xpath[1024];
    int rd;
    unsigned char buf_shx[256];
    unsigned char *buf_shp = NULL;
    int buf_size = 1024;
    int shape;
    unsigned char bf[1024];
    int dbf_size;
    int dbf_reclen = 0;
    int off_dbf;
    int ind;
    char field_name[2048];
    char *sys_err;
    char errMsg[1024];
    iconv_t iconv_ret;
    char utf8buf[2048];
#if !defined(__MINGW32__) && defined(_WIN32)
    const char *pBuf;
#else /* not WIN32 */
    char *pBuf;
#endif
    size_t len;
    size_t utf8len;
    char *pUtf8buf;
    int endian_arch = gaiaEndianArch ();
    gaiaDbfListPtr dbf_list = NULL;
    if (charFrom && charTo)
      {
	  iconv_ret = iconv_open (charTo, charFrom);
	  if (iconv_ret == (iconv_t) (-1))
	    {
		sprintf (errMsg, "conversion from '%s' to '%s' not available\n",
			 charFrom, charTo);
		goto unsupported_conversion;
	    }
	  shp->IconvObj = iconv_ret;
      }
    else
      {
	  sprintf (errMsg, "a NULL charset-name was passed\n");
	  goto unsupported_conversion;
      }
    if (shp->flShp != NULL || shp->flShx != NULL || shp->flDbf != NULL)
      {
	  sprintf (errMsg,
		   "attempting to reopen an already opened Shapefile\n");
	  goto unsupported_conversion;
      }
    sprintf (xpath, "%s.shx", path);
    fl_shx = fopen (xpath, "rb");
    if (!fl_shx)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for reading: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.shp", path);
    fl_shp = fopen (xpath, "rb");
    if (!fl_shp)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for reading: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.dbf", path);
    fl_dbf = fopen (xpath, "rb");
    if (!fl_dbf)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for reading: %s", xpath,
		   sys_err);
	  goto no_file;
      }
/* reading SHX file header */
    rd = fread (buf_shx, sizeof (unsigned char), 100, fl_shx);
    if (rd != 100)
	goto error;
    if (gaiaImport32 (buf_shx + 0, GAIA_BIG_ENDIAN, endian_arch) != 9994)	/* checks the SHX magic number */
	goto error;
/* reading SHP file header */
    buf_shp = malloc (sizeof (unsigned char) * buf_size);
    rd = fread (buf_shp, sizeof (unsigned char), 100, fl_shp);
    if (rd != 100)
	goto error;
    if (gaiaImport32 (buf_shp + 0, GAIA_BIG_ENDIAN, endian_arch) != 9994)	/* checks the SHP magic number */
	goto error;
    shape = gaiaImport32 (buf_shp + 32, GAIA_LITTLE_ENDIAN, endian_arch);
    if (shape == GAIA_SHP_POINT || shape == GAIA_SHP_POINTZ
	|| shape == GAIA_SHP_POINTM || shape == GAIA_SHP_POLYLINE
	|| shape == GAIA_SHP_POLYLINEZ || shape == GAIA_SHP_POLYLINEM
	|| shape == GAIA_SHP_POLYGON || shape == GAIA_SHP_POLYGONZ
	|| shape == GAIA_SHP_POLYGONM || shape == GAIA_SHP_MULTIPOINT
	|| shape == GAIA_SHP_MULTIPOINTZ || shape == GAIA_SHP_MULTIPOINTM)
	;
    else
	goto unsupported;
/* reading DBF file header */
    rd = fread (bf, sizeof (unsigned char), 32, fl_dbf);
    if (rd != 32)
	goto error;
    switch (*bf)
      {
	  /* checks the DBF magic number */
      case 0x03:
      case 0x83:
	  break;
      case 0x02:
      case 0xF8:
	  sprintf (errMsg, "'%s'\ninvalid magic number %02x [FoxBASE format]",
		   path, *bf);
	  goto dbf_bad_magic;
      case 0xF5:
	  sprintf (errMsg,
		   "'%s'\ninvalid magic number %02x [FoxPro 2.x (or earlier) format]",
		   path, *bf);
	  goto dbf_bad_magic;
      case 0x30:
      case 0x31:
      case 0x32:
	  sprintf (errMsg,
		   "'%s'\ninvalid magic number %02x [Visual FoxPro format]",
		   path, *bf);
	  goto dbf_bad_magic;
      case 0x43:
      case 0x63:
      case 0xBB:
      case 0xCB:
	  sprintf (errMsg, "'%s'\ninvalid magic number %02x [dBASE IV format]",
		   path, *bf);
	  goto dbf_bad_magic;
      default:
	  sprintf (errMsg, "'%s'\ninvalid magic number %02x [unknown format]",
		   path, *bf);
	  goto dbf_bad_magic;
      };
    dbf_size = gaiaImport16 (bf + 8, GAIA_LITTLE_ENDIAN, endian_arch);
    dbf_reclen = gaiaImport16 (bf + 10, GAIA_LITTLE_ENDIAN, endian_arch);
    dbf_size--;
    off_dbf = 0;
    dbf_list = gaiaAllocDbfList ();
    for (ind = 32; ind < dbf_size; ind += 32)
      {
	  /* fetches DBF fields definitions */
	  rd = fread (bf, sizeof (unsigned char), 32, fl_dbf);
	  if (rd != 32)
	      goto error;
	  if (*(bf + 11) == 'M')
	    {
		/* skipping any MEMO field */
		memcpy (field_name, bf, 11);
		field_name[11] = '\0';
		off_dbf += *(bf + 16);
		spatialite_e
		    ("WARNING: column \"%s\" is of the MEMO type and will be ignored\n",
		     field_name);
		continue;
	    }
	  memcpy (field_name, bf, 11);
	  field_name[11] = '\0';
	  len = strlen ((char *) field_name);
	  utf8len = 2048;
	  pBuf = (char *) field_name;
	  pUtf8buf = utf8buf;
	  if (iconv
	      ((iconv_t) (shp->IconvObj), &pBuf, &len, &pUtf8buf,
	       &utf8len) == (size_t) (-1))
	      goto conversion_error;
	  memcpy (field_name, utf8buf, 2048 - utf8len);
	  field_name[2048 - utf8len] = '\0';
	  gaiaAddDbfField (dbf_list, field_name, *(bf + 11), off_dbf,
			   *(bf + 16), *(bf + 17));
	  off_dbf += *(bf + 16);
      }
    if (!gaiaIsValidDbfList (dbf_list))
      {
	  /* invalid DBF */
	  goto illegal_dbf;
      }
    len = strlen (path);
    shp->Path = malloc (len + 1);
    strcpy (shp->Path, path);
    shp->ReadOnly = 1;
    shp->Shape = shape;
    switch (shape)
      {
	  /* setting up a prudential geometry type */
      case GAIA_SHP_POINT:
      case GAIA_SHP_POINTZ:
      case GAIA_SHP_POINTM:
	  shp->EffectiveType = GAIA_POINT;
	  break;
      case GAIA_SHP_POLYLINE:
      case GAIA_SHP_POLYLINEZ:
      case GAIA_SHP_POLYLINEM:
	  shp->EffectiveType = GAIA_MULTILINESTRING;
	  break;
      case GAIA_SHP_POLYGON:
      case GAIA_SHP_POLYGONZ:
      case GAIA_SHP_POLYGONM:
	  shp->EffectiveType = GAIA_MULTIPOLYGON;
	  break;
      case GAIA_SHP_MULTIPOINT:
      case GAIA_SHP_MULTIPOINTZ:
      case GAIA_SHP_MULTIPOINTM:
	  shp->EffectiveType = GAIA_MULTIPOINT;
	  break;
      }
    switch (shape)
      {
	  /* setting up a prudential dimension model */
      case GAIA_SHP_POINTZ:
      case GAIA_SHP_POLYLINEZ:
      case GAIA_SHP_POLYGONZ:
      case GAIA_SHP_MULTIPOINTZ:
	  shp->EffectiveDims = GAIA_XY_Z_M;
	  break;
      case GAIA_SHP_POINTM:
      case GAIA_SHP_POLYLINEM:
      case GAIA_SHP_POLYGONM:
      case GAIA_SHP_MULTIPOINTM:
	  shp->EffectiveDims = GAIA_XY_M;
	  break;
      default:
	  shp->EffectiveDims = GAIA_XY;
	  break;
      }
    shp->flShp = fl_shp;
    shp->flShx = fl_shx;
    shp->flDbf = fl_dbf;
    shp->Dbf = dbf_list;
/* saving the SHP buffer */
    shp->BufShp = buf_shp;
    shp->ShpBfsz = buf_size;
/* allocating DBF buffer */
    shp->BufDbf = malloc (sizeof (unsigned char) * dbf_reclen);
    shp->DbfHdsz = dbf_size + 1;
    shp->DbfReclen = dbf_reclen;
    shp->Valid = 1;
    shp->endian_arch = endian_arch;
    return;
  unsupported_conversion:
/* illegal charset */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    return;
  no_file:
/* one of shapefile's files can't be accessed */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    if (fl_shx)
	fclose (fl_shx);
    if (fl_shp)
	fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
  dbf_bad_magic:
/* the DBF has an invalid magin number */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (buf_shp)
	free (buf_shp);
    fclose (fl_shx);
    fclose (fl_shp);
    fclose (fl_dbf);
    return;
  error:
/* the shapefile is invalid or corrupted */
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s' is corrupted / has invalid format", path);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (buf_shp)
	free (buf_shp);
    fclose (fl_shx);
    fclose (fl_shp);
    fclose (fl_dbf);
    return;
  unsupported:
/* the shapefile has an unrecognized shape type */
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s' shape=%d is not supported", path, shape);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (buf_shp)
	free (buf_shp);
    fclose (fl_shx);
    fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
  illegal_dbf:
/* the DBF-file contains unsupported data types */
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s.dbf' contains unsupported data types", path);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (buf_shp)
	free (buf_shp);
    fclose (fl_shx);
    fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
  conversion_error:
/* libiconv error */
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s.dbf' field name: invalid character sequence", path);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (buf_shp)
	free (buf_shp);
    fclose (fl_shx);
    fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
}

static struct auxdbf_list *
alloc_auxdbf (gaiaDbfListPtr dbf_list)
{
/* allocating the auxiliary DBF struct */
    int len;
    gaiaDbfFieldPtr fld;
    struct auxdbf_fld *fld_ex;
    struct auxdbf_list *auxdbf = malloc (sizeof (struct auxdbf_list));
    auxdbf->first = NULL;
    auxdbf->last = NULL;
    fld = dbf_list->First;
    while (fld)
      {
	  fld_ex = malloc (sizeof (struct auxdbf_fld));
	  len = strlen (fld->Name);
	  fld_ex->name = malloc (len + 1);
	  strcpy (fld_ex->name, fld->Name);
	  fld_ex->next = NULL;
	  if (auxdbf->first == NULL)
	      auxdbf->first = fld_ex;
	  if (auxdbf->last != NULL)
	      auxdbf->last->next = fld_ex;
	  auxdbf->last = fld_ex;
	  fld = fld->Next;
      }
    return auxdbf;
}

static void
free_auxdbf (struct auxdbf_list *auxdbf)
{
/* freeing an auxiliary DBF struct */
    struct auxdbf_fld *n_fld;
    struct auxdbf_fld *fld = auxdbf->first;
    while (fld != NULL)
      {
	  n_fld = fld->next;
	  if (fld->name != NULL)
	      free (fld->name);
	  free (fld);
	  fld = n_fld;
      }
    free (auxdbf);
}

static void
truncate_long_name (struct auxdbf_list *list, gaiaDbfFieldPtr xfld)
{
/* attempting to create a unique short name <= 10 bytes */
    char suffix;
    char buf[16];
    struct auxdbf_fld *fld;
    struct auxdbf_fld *base = NULL;
    memcpy (buf, xfld->Name, 9);
    buf[10] = '\0';

    fld = list->first;
    while (fld)
      {
	  /* identifying the base aux Field */
	  if (strcmp (xfld->Name, fld->name) == 0)
	    {
		base = fld;
		break;
	    }
	  fld = fld->next;
      }

    suffix = '0';
    while (1)
      {
	  /* attempting to find a numeric suffix ensuring uniqueness */
	  int ok = 1;
	  buf[9] = suffix;
	  fld = list->first;
	  while (fld)
	    {
		if (base != fld)
		  {
		      if (strcasecmp (buf, fld->name) == 0)
			{
			    /* invalid: already defined */
			    ok = 0;
			    break;
			}
		  }
		fld = fld->next;
	    }
	  if (ok)
	    {
		strcpy (xfld->Name, buf);
		if (base != NULL)
		    strcpy (base->name, buf);
		return;
	    }
	  if (suffix == '9')
	      break;
	  else
	      suffix++;
      }

    suffix = 'A';
    while (1)
      {
	  /* attempting to find a letter suffix ensuring uniqueness */
	  int ok = 1;
	  buf[9] = suffix;
	  fld = list->first;
	  while (fld)
	    {
		if (base != fld)
		  {
		      if (strcasecmp (buf, fld->name) == 0)
			{
			    /* invalid: already defined */
			    ok = 0;
			    break;
			}
		  }
		fld = fld->next;
	    }
	  if (ok)
	    {
		strcpy (xfld->Name, buf);
		if (base != NULL)
		    strcpy (base->name, buf);
		return;
	    }
	  if (suffix == 'Z')
	      break;
	  else
	      suffix++;
      }
}

GAIAGEO_DECLARE void
gaiaOpenShpWrite (gaiaShapefilePtr shp, const char *path, int shape,
		  gaiaDbfListPtr dbf_list, const char *charFrom,
		  const char *charTo)
{
/* trying to create the shapefile */
    FILE *fl_shx = NULL;
    FILE *fl_shp = NULL;
    FILE *fl_dbf = NULL;
    char xpath[1024];
    unsigned char *buf_shp = NULL;
    int buf_size = 1024;
    unsigned char *dbf_buf = NULL;
    gaiaDbfFieldPtr fld;
    char *sys_err;
    char errMsg[1024];
    short dbf_reclen = 0;
    int shp_size = 0;
    int shx_size = 0;
    unsigned short dbf_size = 0;
    iconv_t iconv_ret;
    int endian_arch = gaiaEndianArch ();
    char buf[2048];
    char utf8buf[2048];
#if !defined(__MINGW32__) && defined(_WIN32)
    const char *pBuf;
#else /* not WIN32 */
    char *pBuf;
#endif
    size_t len;
    size_t utf8len;
    char *pUtf8buf;
    int defaultId = 1;
    struct auxdbf_list *auxdbf = NULL;
    if (charFrom && charTo)
      {
	  iconv_ret = iconv_open (charTo, charFrom);
	  if (iconv_ret == (iconv_t) (-1))
	    {
		sprintf (errMsg, "conversion from '%s' to '%s' not available\n",
			 charFrom, charTo);
		goto unsupported_conversion;
	    }
	  shp->IconvObj = iconv_ret;
      }
    else
      {
	  sprintf (errMsg, "a NULL charset-name was passed\n");
	  goto unsupported_conversion;
      }
    if (shp->flShp != NULL || shp->flShx != NULL || shp->flDbf != NULL)
      {
	  sprintf (errMsg,
		   "attempting to reopen an already opened Shapefile\n");
	  goto unsupported_conversion;
      }
    buf_shp = malloc (buf_size);
/* trying to open shapefile files */
    sprintf (xpath, "%s.shx", path);
    fl_shx = fopen (xpath, "wb");
    if (!fl_shx)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for writing: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.shp", path);
    fl_shp = fopen (xpath, "wb");
    if (!fl_shp)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for writing: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.dbf", path);
    fl_dbf = fopen (xpath, "wb");
    if (!fl_dbf)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for writing: %s", xpath,
		   sys_err);
	  goto no_file;
      }
/* allocating DBF buffer */
    dbf_reclen = 1;		/* an extra byte is needed because in DBF rows first byte is a marker for deletion */
    fld = dbf_list->First;
    while (fld)
      {
	  /* computing the DBF record length */
	  dbf_reclen += fld->Length;
	  fld = fld->Next;
      }
    dbf_buf = malloc (dbf_reclen);
/* writing an empty SHP file header */
    memset (buf_shp, 0, 100);
    fwrite (buf_shp, 1, 100, fl_shp);
    shp_size = 50;		/* note: shapefile [SHP and SHX] counts sizes in WORDS of 16 bits, not in bytes of 8 bits !!!! */
/* writing an empty SHX file header */
    memset (buf_shp, 0, 100);
    fwrite (buf_shp, 1, 100, fl_shx);
    shx_size = 50;
/* writing the DBF file header */
    memset (buf_shp, '\0', 32);
    fwrite (buf_shp, 1, 32, fl_dbf);
    dbf_size = 32;		/* note: DBF counts sizes in bytes */
    auxdbf = alloc_auxdbf (dbf_list);
    fld = dbf_list->First;
    while (fld)
      {
	  /* exporting DBF Fields specifications */
	  memset (buf_shp, 0, 32);
	  if (strlen (fld->Name) > 10)
	    {
		/* long name: attempting to safely truncate */
		truncate_long_name (auxdbf, fld);
	    }
	  strcpy (buf, fld->Name);
	  len = strlen (buf);
	  utf8len = 2048;
	  pBuf = buf;
	  pUtf8buf = utf8buf;
	  if (iconv
	      ((iconv_t) (shp->IconvObj), &pBuf, &len, &pUtf8buf,
	       &utf8len) == (size_t) (-1))
	      sprintf (buf, "FLD#%d", defaultId++);
	  else
	    {
		memcpy (buf, utf8buf, 2048 - utf8len);
		buf[2048 - utf8len] = '\0';
		if (strlen (buf) > 10)
		    sprintf (buf, "FLD#%d", defaultId++);
	    }
	  memcpy (buf_shp, buf, strlen (buf));
	  *(buf_shp + 11) = fld->Type;
	  *(buf_shp + 16) = fld->Length;
	  *(buf_shp + 17) = fld->Decimals;
	  fwrite (buf_shp, 1, 32, fl_dbf);
	  dbf_size += 32;
	  fld = fld->Next;
      }
    free_auxdbf (auxdbf);
    fwrite ("\r", 1, 1, fl_dbf);	/* this one is a special DBF delimiter that closes file header */
    dbf_size++;
/* setting up the SHP struct */
    len = strlen (path);
    shp->Path = malloc (len + 1);
    strcpy (shp->Path, path);
    shp->ReadOnly = 0;
    switch (shape)
      {
	  /* setting up SHAPE and dimensions */
      case GAIA_POINT:
	  shp->Shape = GAIA_SHP_POINT;
	  shp->EffectiveType = GAIA_POINT;
	  shp->EffectiveDims = GAIA_XY;
	  break;
      case GAIA_POINTZ:
	  shp->Shape = GAIA_SHP_POINTZ;
	  shp->EffectiveType = GAIA_POINT;
	  shp->EffectiveDims = GAIA_XY_Z;
	  break;
      case GAIA_POINTM:
	  shp->Shape = GAIA_SHP_POINTM;
	  shp->EffectiveType = GAIA_POINT;
	  shp->EffectiveDims = GAIA_XY_M;
	  break;
      case GAIA_POINTZM:
	  shp->Shape = GAIA_SHP_POINTZ;
	  shp->EffectiveType = GAIA_POINT;
	  shp->EffectiveDims = GAIA_XY_Z_M;
	  break;
      case GAIA_MULTIPOINT:
	  shp->Shape = GAIA_SHP_MULTIPOINT;
	  shp->EffectiveType = GAIA_MULTIPOINT;
	  shp->EffectiveDims = GAIA_XY;
	  break;
      case GAIA_MULTIPOINTZ:
	  shp->Shape = GAIA_SHP_MULTIPOINTZ;
	  shp->EffectiveType = GAIA_MULTIPOINT;
	  shp->EffectiveDims = GAIA_XY_Z;
	  break;
      case GAIA_MULTIPOINTM:
	  shp->Shape = GAIA_SHP_MULTIPOINTM;
	  shp->EffectiveType = GAIA_MULTIPOINT;
	  shp->EffectiveDims = GAIA_XY_M;
	  break;
      case GAIA_MULTIPOINTZM:
	  shp->Shape = GAIA_SHP_MULTIPOINTZ;
	  shp->EffectiveType = GAIA_MULTIPOINT;
	  shp->EffectiveDims = GAIA_XY_Z_M;
	  break;
      case GAIA_LINESTRING:
	  shp->Shape = GAIA_SHP_POLYLINE;
	  shp->EffectiveType = GAIA_LINESTRING;
	  shp->EffectiveDims = GAIA_XY;
	  break;
      case GAIA_LINESTRINGZ:
	  shp->Shape = GAIA_SHP_POLYLINEZ;
	  shp->EffectiveType = GAIA_LINESTRING;
	  shp->EffectiveDims = GAIA_XY_Z;
	  break;
      case GAIA_LINESTRINGM:
	  shp->Shape = GAIA_SHP_POLYLINEM;
	  shp->EffectiveType = GAIA_LINESTRING;
	  shp->EffectiveDims = GAIA_XY_M;
	  break;
      case GAIA_LINESTRINGZM:
	  shp->Shape = GAIA_SHP_POLYLINEZ;
	  shp->EffectiveType = GAIA_LINESTRING;
	  shp->EffectiveDims = GAIA_XY_Z_M;
	  break;
      case GAIA_MULTILINESTRING:
	  shp->Shape = GAIA_SHP_POLYLINE;
	  shp->EffectiveType = GAIA_MULTILINESTRING;
	  shp->EffectiveDims = GAIA_XY;
	  break;
      case GAIA_MULTILINESTRINGZ:
	  shp->Shape = GAIA_SHP_POLYLINEZ;
	  shp->EffectiveType = GAIA_MULTILINESTRING;
	  shp->EffectiveDims = GAIA_XY_Z;
	  break;
      case GAIA_MULTILINESTRINGM:
	  shp->Shape = GAIA_SHP_POLYLINEM;
	  shp->EffectiveType = GAIA_MULTILINESTRING;
	  shp->EffectiveDims = GAIA_XY_M;
	  break;
      case GAIA_MULTILINESTRINGZM:
	  shp->Shape = GAIA_SHP_POLYLINEZ;
	  shp->EffectiveType = GAIA_MULTILINESTRING;
	  shp->EffectiveDims = GAIA_XY_Z_M;
	  break;
      case GAIA_POLYGON:
	  shp->Shape = GAIA_SHP_POLYGON;
	  shp->EffectiveType = GAIA_POLYGON;
	  shp->EffectiveDims = GAIA_XY;
	  break;
      case GAIA_POLYGONZ:
	  shp->Shape = GAIA_SHP_POLYGONZ;
	  shp->EffectiveType = GAIA_POLYGON;
	  shp->EffectiveDims = GAIA_XY_Z;
	  break;
      case GAIA_POLYGONM:
	  shp->Shape = GAIA_SHP_POLYGONM;
	  shp->EffectiveType = GAIA_POLYGON;
	  shp->EffectiveDims = GAIA_XY_M;
	  break;
      case GAIA_POLYGONZM:
	  shp->Shape = GAIA_SHP_POLYGONZ;
	  shp->EffectiveType = GAIA_POLYGON;
	  shp->EffectiveDims = GAIA_XY_Z_M;
	  break;
      case GAIA_MULTIPOLYGON:
	  shp->Shape = GAIA_SHP_POLYGON;
	  shp->EffectiveType = GAIA_MULTIPOLYGON;
	  shp->EffectiveDims = GAIA_XY;
	  break;
      case GAIA_MULTIPOLYGONZ:
	  shp->Shape = GAIA_SHP_POLYGONZ;
	  shp->EffectiveType = GAIA_MULTIPOLYGON;
	  shp->EffectiveDims = GAIA_XY_Z;
	  break;
      case GAIA_MULTIPOLYGONM:
	  shp->Shape = GAIA_SHP_POLYGONM;
	  shp->EffectiveType = GAIA_MULTIPOLYGON;
	  shp->EffectiveDims = GAIA_XY_M;
	  break;
      case GAIA_MULTIPOLYGONZM:
	  shp->Shape = GAIA_SHP_POLYGONZ;
	  shp->EffectiveType = GAIA_MULTIPOLYGON;
	  shp->EffectiveDims = GAIA_XY_Z_M;
	  break;
      };
    shp->flShp = fl_shp;
    shp->flShx = fl_shx;
    shp->flDbf = fl_dbf;
    shp->Dbf = dbf_list;
    shp->BufShp = buf_shp;
    shp->ShpBfsz = buf_size;
    shp->BufDbf = dbf_buf;
    shp->DbfHdsz = dbf_size + 1;
    shp->DbfReclen = dbf_reclen;
    shp->DbfSize = dbf_size;
    shp->DbfRecno = 0;
    shp->ShpSize = shp_size;
    shp->ShxSize = shx_size;
    shp->MinX = DBL_MAX;
    shp->MinY = DBL_MAX;
    shp->MaxX = -DBL_MAX;
    shp->MaxY = -DBL_MAX;
    shp->Valid = 1;
    shp->endian_arch = endian_arch;
    return;
  unsupported_conversion:
/* illegal charset */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    return;
  no_file:
/* one of shapefile's files can't be created/opened */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    if (buf_shp)
	free (buf_shp);
    if (fl_shx)
	fclose (fl_shx);
    if (fl_shp)
	fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
}

static double
to_sqlite_julian_date (int year, int month, int day, double *julian)
{
/* trying to convert an 'YYYY-MM-DD' date into a JulianDate [double] */
    int Y;
    int M;
    int D;
    int A;
    int B;
    int X1;
    int X2;
    if (year < 1900 || year > 2400)
	return 0;
    if (month < 1 || month > 12)
	return 0;
    if (day < 1)
	return 0;
    switch (month)
      {
      case 2:
	  if ((year / 4) == 0)
	    {
		if (day > 29)
		    return 0;
	    }
	  else
	    {
		if (day > 28)
		    return 0;
	    }
	  break;
      case 4:
      case 6:
      case 9:
      case 11:
	  if (day > 30)
	      return 0;
	  break;
      default:
	  if (day > 31)
	      return 0;
      };
/* computing the Julian date */
    Y = year;
    M = month;
    D = day;
    if (M <= 2)
      {
	  Y--;
	  M += 12;
      }
    A = Y / 100;
    B = 2 - A + (A / 4);
    X1 = 36525 * (Y + 4716) / 100;
    X2 = 306001 * (M + 1) / 10000;
    *julian = (double) (X1 + X2 + D + B - 1524.5);
    return 1;
}

static int
parseDbfField (unsigned char *buf_dbf, void *iconv_obj, gaiaDbfFieldPtr pFld,
	       int text_dates)
{
/* parsing a generic DBF field */
    unsigned char buf[512];
    char utf8buf[2048];
#if !defined(__MINGW32__) && defined(_WIN32)
    const char *pBuf;
#else /* not WIN32 */
    char *pBuf;
#endif
    size_t len;
    size_t utf8len;
    char *pUtf8buf;
    int i;
    memcpy (buf, buf_dbf + pFld->Offset + 1, pFld->Length);
    buf[pFld->Length] = '\0';
    if (*buf == '\0')
	gaiaSetNullValue (pFld);
    else
      {
	  if (pFld->Type == 'N')
	    {
		/* NUMERIC value */
		if (pFld->Decimals > 0 || pFld->Length > 18)
		    gaiaSetDoubleValue (pFld, atof ((char *) buf));
		else
		    gaiaSetIntValue (pFld, atoll ((char *) buf));
	    }
	  else if (pFld->Type == 'M')
	    {
		/* MEMO value - assumed to always be NULL */
		gaiaSetNullValue (pFld);
	    }
	  else if (pFld->Type == 'F')
	    {
		/* FLOAT value */
		gaiaSetDoubleValue (pFld, atof ((char *) buf));
	    }
	  else if (pFld->Type == 'D')
	    {
		/* DATE value */
		if (text_dates)
		  {
		      /* assuming to be plain text */
		      gaiaSetStrValue (pFld, (char *) buf);
		  }
		else
		  {
		      if (strlen ((char *) buf) != 8)
			  gaiaSetNullValue (pFld);
		      else
			{
			    /* converting into a Julian Date */
			    double julian;
			    char date[5];
			    int year = 0;
			    int month = 0;
			    int day = 0;
			    date[0] = buf[0];
			    date[1] = buf[1];
			    date[2] = buf[2];
			    date[3] = buf[3];
			    date[4] = '\0';
			    year = atoi (date);
			    date[0] = buf[4];
			    date[1] = buf[5];
			    date[2] = '\0';
			    month = atoi (date);
			    date[0] = buf[6];
			    date[1] = buf[7];
			    date[2] = '\0';
			    day = atoi (date);
			    if (to_sqlite_julian_date
				(year, month, day, &julian))
				gaiaSetDoubleValue (pFld, julian);
			    else
				gaiaSetNullValue (pFld);
			}
		  }
	    }
	  else if (pFld->Type == 'L')
	    {
		/* LOGICAL [aka Boolean] value */
		if (*buf == '1' || *buf == 't' || *buf == 'T'
		    || *buf == 'Y' || *buf == 'y')
		    gaiaSetIntValue (pFld, 1);
		else
		    gaiaSetIntValue (pFld, 0);
	    }
	  else
	    {
		/* CHARACTER [aka String, Text] value */

/* Sandro 2013-01-07
/ fixing an issue reported by Filip Arlet <filip.arlet@gmail.com>
		for (i = strlen ((char *) buf) - 1; i > 1; i--)
*/
		for (i = strlen ((char *) buf) - 1; i >= 0; i--)
		  {
		      /* cleaning up trailing spaces */
		      if (buf[i] == ' ')
			  buf[i] = '\0';
		      else
			  break;
		  }
		len = strlen ((char *) buf);
		utf8len = 2048;
		pBuf = (char *) buf;
		pUtf8buf = utf8buf;
		if (iconv
		    ((iconv_t) (iconv_obj), &pBuf, &len, &pUtf8buf,
		     &utf8len) == (size_t) (-1))
		    return 0;
		memcpy (buf, utf8buf, 2048 - utf8len);
		buf[2048 - utf8len] = '\0';
		gaiaSetStrValue (pFld, (char *) buf);
	    }
      }
    return 1;
}

struct shp_ring_item
{
/* a RING item [to be reassembled into a (Multi)Polygon] */
    gaiaRingPtr Ring;
    int IsExterior;
    gaiaRingPtr Mother;
    struct shp_ring_item *Next;
};

struct shp_ring_collection
{
/* a collection of RING items */
    struct shp_ring_item *First;
    struct shp_ring_item *Last;
};

static void
shp_free_rings (struct shp_ring_collection *ringsColl)
{
/* memory cleanup: rings collection */
    struct shp_ring_item *p;
    struct shp_ring_item *pN;
    p = ringsColl->First;
    while (p)
      {
	  pN = p->Next;
	  if (p->Ring)
	      gaiaFreeRing (p->Ring);
	  free (p);
	  p = pN;
      }
}

static void
shp_add_ring (struct shp_ring_collection *ringsColl, gaiaRingPtr ring)
{
/* inserting a ring into the rings collection */
    struct shp_ring_item *p = malloc (sizeof (struct shp_ring_item));
    p->Ring = ring;
    gaiaMbrRing (ring);
    gaiaClockwise (ring);
/* accordingly to SHP rules interior/exterior depends on direction */
    p->IsExterior = ring->Clockwise;
    p->Mother = NULL;
    p->Next = NULL;
/* updating the linked list */
    if (ringsColl->First == NULL)
	ringsColl->First = p;
    if (ringsColl->Last != NULL)
	ringsColl->Last->Next = p;
    ringsColl->Last = p;
}

static int
shp_check_rings (gaiaRingPtr exterior, gaiaRingPtr candidate)
{
/* 
/ speditively checks if the candidate could be an interior Ring
/ contained into the exterior Ring
*/
    double z;
    double m;
    double x0;
    double y0;
    double x1;
    double y1;
    int mid;
    int ret0;
    int ret1;
    if (candidate->DimensionModel == GAIA_XY_Z)
      {
	  gaiaGetPointXYZ (candidate->Coords, 0, &x0, &y0, &z);
      }
    else if (candidate->DimensionModel == GAIA_XY_M)
      {
	  gaiaGetPointXYM (candidate->Coords, 0, &x0, &y0, &m);
      }
    else if (candidate->DimensionModel == GAIA_XY_Z_M)
      {
	  gaiaGetPointXYZM (candidate->Coords, 0, &x0, &y0, &z, &m);
      }
    else
      {
	  gaiaGetPoint (candidate->Coords, 0, &x0, &y0);
      }
    mid = candidate->Points / 2;
    if (candidate->DimensionModel == GAIA_XY_Z)
      {
	  gaiaGetPointXYZ (candidate->Coords, mid, &x1, &y1, &z);
      }
    else if (candidate->DimensionModel == GAIA_XY_M)
      {
	  gaiaGetPointXYM (candidate->Coords, mid, &x1, &y1, &m);
      }
    else if (candidate->DimensionModel == GAIA_XY_Z_M)
      {
	  gaiaGetPointXYZM (candidate->Coords, mid, &x1, &y1, &z, &m);
      }
    else
      {
	  gaiaGetPoint (candidate->Coords, mid, &x1, &y1);
      }

/* testing if the first point falls on the exterior ring surface */
    ret0 = gaiaIsPointOnRingSurface (exterior, x0, y0);
/* testing if the second point falls on the exterior ring surface */
    ret1 = gaiaIsPointOnRingSurface (exterior, x1, y1);
    if (ret0 || ret1)
	return 1;
    return 0;
}

static int
shp_mbr_contains (gaiaRingPtr r1, gaiaRingPtr r2)
{
/* checks if the first Ring contains the second one - MBR based */
    int ok_1 = 0;
    int ok_2 = 0;
    int ok_3 = 0;
    int ok_4 = 0;
    if (r2->MinX >= r1->MinX && r2->MinX <= r1->MaxX)
	ok_1 = 1;
    if (r2->MaxX >= r1->MinX && r2->MaxX <= r1->MaxX)
	ok_2 = 1;
    if (r2->MinY >= r1->MinY && r2->MinY <= r1->MaxY)
	ok_3 = 1;
    if (r2->MaxY >= r1->MinY && r2->MaxY <= r1->MaxY)
	ok_4 = 1;
    if (ok_1 && ok_2 && ok_3 && ok_4)
	return 1;
    return 0;
}

static void
shp_arrange_rings (struct shp_ring_collection *ringsColl)
{
/* 
/ arranging Rings so to associate any interior ring
/ to the containing exterior ring
*/
    struct shp_ring_item *pInt;
    struct shp_ring_item *pExt;
    pExt = ringsColl->First;
    while (pExt != NULL)
      {
	  /* looping on Exterior Rings */
	  if (pExt->IsExterior)
	    {
		pInt = ringsColl->First;
		while (pInt != NULL)
		  {
		      /* looping on Interior Rings */
		      if (pInt->IsExterior == 0 && pInt->Mother == NULL
			  && shp_mbr_contains (pExt->Ring, pInt->Ring))
			{
			    /* ok, matches */
			    if (shp_check_rings (pExt->Ring, pInt->Ring))
				pInt->Mother = pExt->Ring;
			}
		      pInt = pInt->Next;
		  }
	    }
	  pExt = pExt->Next;
      }
    pExt = ringsColl->First;
    while (pExt != NULL)
      {
	  if (pExt->IsExterior == 0 && pExt->Mother == NULL)
	    {
		/* orphan ring: promoting to Exterior */
		pExt->IsExterior = 1;
	    }
	  pExt = pExt->Next;
      }
}

static void
shp_build_area (struct shp_ring_collection *ringsColl, gaiaGeomCollPtr geom)
{
/* building the final (Multi)Polygon Geometry */
    gaiaPolygonPtr polyg;
    struct shp_ring_item *pExt;
    struct shp_ring_item *pInt;
    pExt = ringsColl->First;
    while (pExt != NULL)
      {
	  if (pExt->IsExterior)
	    {
		/* creating a new Polygon */
		polyg = gaiaInsertPolygonInGeomColl (geom, pExt->Ring);
		pInt = ringsColl->First;
		while (pInt != NULL)
		  {
		      if (pExt->Ring == pInt->Mother)
			{
			    /* adding an interior ring to current POLYGON */
			    gaiaAddRingToPolyg (polyg, pInt->Ring);
			    /* releasing Ring ownership */
			    pInt->Ring = NULL;
			}
		      pInt = pInt->Next;
		  }
		/* releasing Ring ownership */
		pExt->Ring = NULL;
	    }
	  pExt = pExt->Next;
      }
}

GAIAGEO_DECLARE int
gaiaReadShpEntity (gaiaShapefilePtr shp, int current_row, int srid)
{
    return gaiaReadShpEntity_ex (shp, current_row, srid, 0);
}

GAIAGEO_DECLARE int
gaiaReadShpEntity_ex (gaiaShapefilePtr shp, int current_row, int srid,
		      int text_dates)
{
/* trying to read an entity from shapefile */
    unsigned char buf[512];
    int len;
    int rd;
    int skpos;
    int offset;
    int off_shp;
    int sz;
    int shape;
    double x;
    double y;
    double z;
    double m;
    int points;
    int n;
    int n1;
    int base;
    int baseZ;
    int baseM;
    int start;
    int end;
    int iv;
    int ind;
    int max_size;
    int min_size;
    int hasM;
    char errMsg[1024];
    gaiaGeomCollPtr geom = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaRingPtr ring = NULL;
    gaiaDbfFieldPtr pFld;
    struct shp_ring_collection ringsColl;
/* initializing the RING collection */
    ringsColl.First = NULL;
    ringsColl.Last = NULL;
/* positioning and reading the SHX file */
    offset = 100 + (current_row * 8);	/* 100 bytes for the header + current row displacement; each SHX row = 8 bytes */
    skpos = fseek (shp->flShx, offset, SEEK_SET);
    if (skpos != 0)
	goto eof;
    rd = fread (buf, sizeof (unsigned char), 8, shp->flShx);
    if (rd != 8)
	goto eof;
    off_shp = gaiaImport32 (buf, GAIA_BIG_ENDIAN, shp->endian_arch);
/* positioning and reading the DBF file */
    offset = shp->DbfHdsz + (current_row * shp->DbfReclen);
    skpos = fseek (shp->flDbf, offset, SEEK_SET);
    if (skpos != 0)
	goto error;
    rd = fread (shp->BufDbf, sizeof (unsigned char), shp->DbfReclen,
		shp->flDbf);
    if (rd != shp->DbfReclen)
	goto error;
/* positioning and reading corresponding SHP entity - geometry */
    offset = off_shp * 2;
    skpos = fseek (shp->flShp, offset, SEEK_SET);
    if (skpos != 0)
	goto error;
    rd = fread (buf, sizeof (unsigned char), 12, shp->flShp);
    if (rd != 12)
	goto error;
    sz = gaiaImport32 (buf + 4, GAIA_BIG_ENDIAN, shp->endian_arch);
    shape = gaiaImport32 (buf + 8, GAIA_LITTLE_ENDIAN, shp->endian_arch);
    if (shape == GAIA_SHP_NULL)
      {
	  /* handling a NULL shape */
	  goto null_shape;
      }
    else if (shape != shp->Shape)
	goto error;
    if ((sz * 2) > shp->ShpBfsz)
      {
	  /* current buffer is too small; we need to allocate a bigger buffer */
	  free (shp->BufShp);
	  shp->ShpBfsz = sz * 2;
	  shp->BufShp = malloc (sizeof (unsigned char) * shp->ShpBfsz);
      }
    if (shape == GAIA_SHP_POINT)
      {
	  /* shape point */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 16, shp->flShp);
	  if (rd != 16)
	      goto error;
	  x = gaiaImport64 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  y = gaiaImport64 (shp->BufShp + 8, GAIA_LITTLE_ENDIAN,
			    shp->endian_arch);
	  if (shp->EffectiveDims == GAIA_XY_Z)
	    {
		geom = gaiaAllocGeomCollXYZ ();
		gaiaAddPointToGeomCollXYZ (geom, x, y, 0.0);
	    }
	  else if (shp->EffectiveDims == GAIA_XY_M)
	    {
		geom = gaiaAllocGeomCollXYM ();
		gaiaAddPointToGeomCollXYM (geom, x, y, 0.0);
	    }
	  else if (shp->EffectiveDims == GAIA_XY_Z_M)
	    {
		geom = gaiaAllocGeomCollXYZM ();
		gaiaAddPointToGeomCollXYZM (geom, x, y, 0.0, 0.0);
	    }
	  else
	    {
		geom = gaiaAllocGeomColl ();
		gaiaAddPointToGeomColl (geom, x, y);
	    }
	  geom->DeclaredType = GAIA_POINT;
	  geom->Srid = srid;
      }
    if (shape == GAIA_SHP_POINTZ)
      {
	  /* shape point Z */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	    {
		/* required by some buggish SHP (e.g. the GDAL/OGR ones) */
		if (rd != 24)
		    goto error;
	    }
	  x = gaiaImport64 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  y = gaiaImport64 (shp->BufShp + 8, GAIA_LITTLE_ENDIAN,
			    shp->endian_arch);
	  z = gaiaImport64 (shp->BufShp + 16, GAIA_LITTLE_ENDIAN,
			    shp->endian_arch);
	  if (rd == 24)
	      m = 0.0;
	  else
	      m = gaiaImport64 (shp->BufShp + 24, GAIA_LITTLE_ENDIAN,
				shp->endian_arch);
	  if (shp->EffectiveDims == GAIA_XY_Z)
	    {
		geom = gaiaAllocGeomCollXYZ ();
		gaiaAddPointToGeomCollXYZ (geom, x, y, z);
	    }
	  else if (shp->EffectiveDims == GAIA_XY_M)
	    {
		geom = gaiaAllocGeomCollXYM ();
		gaiaAddPointToGeomCollXYM (geom, x, y, m);
	    }
	  else if (shp->EffectiveDims == GAIA_XY_Z_M)
	    {
		geom = gaiaAllocGeomCollXYZM ();
		gaiaAddPointToGeomCollXYZM (geom, x, y, z, m);
	    }
	  else
	    {
		geom = gaiaAllocGeomColl ();
		gaiaAddPointToGeomColl (geom, x, y);
	    }
	  geom->DeclaredType = GAIA_POINT;
	  geom->Srid = srid;
      }
    if (shape == GAIA_SHP_POINTM)
      {
	  /* shape point M */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 24, shp->flShp);
	  if (rd != 24)
	      goto error;
	  x = gaiaImport64 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  y = gaiaImport64 (shp->BufShp + 8, GAIA_LITTLE_ENDIAN,
			    shp->endian_arch);
	  m = gaiaImport64 (shp->BufShp + 16, GAIA_LITTLE_ENDIAN,
			    shp->endian_arch);
	  if (shp->EffectiveDims == GAIA_XY_Z)
	    {
		geom = gaiaAllocGeomCollXYZ ();
		gaiaAddPointToGeomCollXYZ (geom, x, y, 0.0);
	    }
	  else if (shp->EffectiveDims == GAIA_XY_M)
	    {
		geom = gaiaAllocGeomCollXYM ();
		gaiaAddPointToGeomCollXYM (geom, x, y, m);
	    }
	  else if (shp->EffectiveDims == GAIA_XY_Z_M)
	    {
		geom = gaiaAllocGeomCollXYZM ();
		gaiaAddPointToGeomCollXYZM (geom, x, y, 0.0, m);
	    }
	  else
	    {
		geom = gaiaAllocGeomColl ();
		gaiaAddPointToGeomColl (geom, x, y);
	    }
	  geom->DeclaredType = GAIA_POINT;
	  geom->Srid = srid;
      }
    if (shape == GAIA_SHP_POLYLINE)
      {
	  /* shape polyline */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
			     shp->endian_arch);
	  base = 8 + (n * 4);
	  start = 0;
	  for (ind = 0; ind < n; ind++)
	    {
		if (ind < (n - 1))
		    end =
			gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    end = n1;
		points = end - start;
		if (shp->EffectiveDims == GAIA_XY_Z)
		    line = gaiaAllocLinestringXYZ (points);
		else if (shp->EffectiveDims == GAIA_XY_M)
		    line = gaiaAllocLinestringXYM (points);
		else if (shp->EffectiveDims == GAIA_XY_Z_M)
		    line = gaiaAllocLinestringXYZM (points);
		else
		    line = gaiaAllocLinestring (points);
		points = 0;
		for (iv = start; iv < end; iv++)
		  {
		      x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      y = gaiaImport64 (shp->BufShp + base + (iv * 16) +
					8, GAIA_LITTLE_ENDIAN,
					shp->endian_arch);
		      if (shp->EffectiveDims == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (line->Coords, points, x, y, 0.0);
			}
		      else if (shp->EffectiveDims == GAIA_XY_M)
			{
			    gaiaSetPointXYM (line->Coords, points, x, y, 0.0);
			}
		      else if (shp->EffectiveDims == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (line->Coords, points, x, y,
					      0.0, 0.0);
			}
		      else
			{
			    gaiaSetPoint (line->Coords, points, x, y);
			}
		      start++;
		      points++;
		  }
		if (!geom)
		  {
		      if (shp->EffectiveDims == GAIA_XY_Z)
			  geom = gaiaAllocGeomCollXYZ ();
		      else if (shp->EffectiveDims == GAIA_XY_M)
			  geom = gaiaAllocGeomCollXYM ();
		      else if (shp->EffectiveDims == GAIA_XY_Z_M)
			  geom = gaiaAllocGeomCollXYZM ();
		      else
			  geom = gaiaAllocGeomColl ();
		      if (shp->EffectiveType == GAIA_LINESTRING)
			  geom->DeclaredType = GAIA_LINESTRING;
		      else
			  geom->DeclaredType = GAIA_MULTILINESTRING;
		      geom->Srid = srid;
		  }
		gaiaInsertLinestringInGeomColl (geom, line);
	    }
      }
    if (shape == GAIA_SHP_POLYLINEZ)
      {
	  /* shape polyline Z */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
			     shp->endian_arch);
	  hasM = 0;
	  max_size = 38 + (2 * n) + (n1 * 16);	/* size [in 16 bits words !!!] ZM */
	  min_size = 30 + (2 * n) + (n1 * 12);	/* size [in 16 bits words !!!] Z-only */
	  if (sz < min_size)
	      goto error;
	  if (sz == max_size)
	      hasM = 1;
	  base = 8 + (n * 4);
	  baseZ = base + (n1 * 16) + 16;
	  baseM = baseZ + (n1 * 8) + 16;
	  start = 0;
	  for (ind = 0; ind < n; ind++)
	    {
		if (ind < (n - 1))
		    end =
			gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    end = n1;
		points = end - start;
		if (shp->EffectiveDims == GAIA_XY_Z)
		    line = gaiaAllocLinestringXYZ (points);
		else if (shp->EffectiveDims == GAIA_XY_M)
		    line = gaiaAllocLinestringXYM (points);
		else if (shp->EffectiveDims == GAIA_XY_Z_M)
		    line = gaiaAllocLinestringXYZM (points);
		else
		    line = gaiaAllocLinestring (points);
		points = 0;
		for (iv = start; iv < end; iv++)
		  {
		      x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      y = gaiaImport64 (shp->BufShp + base + (iv * 16) +
					8, GAIA_LITTLE_ENDIAN,
					shp->endian_arch);
		      z = gaiaImport64 (shp->BufShp + baseZ + (iv * 8),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      if (hasM)
			  m = gaiaImport64 (shp->BufShp + baseM +
					    (iv * 8), GAIA_LITTLE_ENDIAN,
					    shp->endian_arch);
		      else
			  m = 0.0;
		      if (m < SHAPEFILE_NO_DATA)
			  m = 0.0;
		      if (shp->EffectiveDims == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (line->Coords, points, x, y, z);
			}
		      else if (shp->EffectiveDims == GAIA_XY_M)
			{
			    gaiaSetPointXYM (line->Coords, points, x, y, m);
			}
		      else if (shp->EffectiveDims == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (line->Coords, points, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (line->Coords, points, x, y);
			}
		      start++;
		      points++;
		  }
		if (!geom)
		  {
		      if (shp->EffectiveDims == GAIA_XY_Z)
			  geom = gaiaAllocGeomCollXYZ ();
		      else if (shp->EffectiveDims == GAIA_XY_M)
			  geom = gaiaAllocGeomCollXYM ();
		      else if (shp->EffectiveDims == GAIA_XY_Z_M)
			  geom = gaiaAllocGeomCollXYZM ();
		      else
			  geom = gaiaAllocGeomColl ();
		      if (shp->EffectiveType == GAIA_LINESTRING)
			  geom->DeclaredType = GAIA_LINESTRING;
		      else
			  geom->DeclaredType = GAIA_MULTILINESTRING;
		      geom->Srid = srid;
		  }
		gaiaInsertLinestringInGeomColl (geom, line);
	    }
      }
    if (shape == GAIA_SHP_POLYLINEM)
      {
	  /* shape polyline M */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
			     shp->endian_arch);
	  hasM = 0;
	  max_size = 30 + (2 * n) + (n1 * 12);	/* size [in 16 bits words !!!] M */
	  min_size = 22 + (2 * n) + (n1 * 8);	/* size [in 16 bits words !!!] no-M */
	  if (sz < min_size)
	      goto error;
	  if (sz == max_size)
	      hasM = 1;
	  base = 8 + (n * 4);
	  baseM = base + (n1 * 16) + 16;
	  start = 0;
	  for (ind = 0; ind < n; ind++)
	    {
		if (ind < (n - 1))
		    end =
			gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    end = n1;
		points = end - start;
		if (shp->EffectiveDims == GAIA_XY_Z)
		    line = gaiaAllocLinestringXYZ (points);
		else if (shp->EffectiveDims == GAIA_XY_M)
		    line = gaiaAllocLinestringXYM (points);
		else if (shp->EffectiveDims == GAIA_XY_Z_M)
		    line = gaiaAllocLinestringXYZM (points);
		else
		    line = gaiaAllocLinestring (points);
		points = 0;
		for (iv = start; iv < end; iv++)
		  {
		      x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      y = gaiaImport64 (shp->BufShp + base + (iv * 16) +
					8, GAIA_LITTLE_ENDIAN,
					shp->endian_arch);
		      if (hasM)
			  m = gaiaImport64 (shp->BufShp + baseM +
					    (iv * 8), GAIA_LITTLE_ENDIAN,
					    shp->endian_arch);
		      else
			  m = 0.0;
		      if (m < SHAPEFILE_NO_DATA)
			  m = 0.0;
		      if (shp->EffectiveDims == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (line->Coords, points, x, y, 0.0);
			}
		      else if (shp->EffectiveDims == GAIA_XY_M)
			{
			    gaiaSetPointXYM (line->Coords, points, x, y, m);
			}
		      else if (shp->EffectiveDims == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (line->Coords, points, x, y,
					      0.0, m);
			}
		      else
			{
			    gaiaSetPoint (line->Coords, points, x, y);
			}
		      start++;
		      points++;
		  }
		if (!geom)
		  {
		      if (shp->EffectiveDims == GAIA_XY_Z)
			  geom = gaiaAllocGeomCollXYZ ();
		      else if (shp->EffectiveDims == GAIA_XY_M)
			  geom = gaiaAllocGeomCollXYM ();
		      else if (shp->EffectiveDims == GAIA_XY_Z_M)
			  geom = gaiaAllocGeomCollXYZM ();
		      else
			  geom = gaiaAllocGeomColl ();
		      if (shp->EffectiveType == GAIA_LINESTRING)
			  geom->DeclaredType = GAIA_LINESTRING;
		      else
			  geom->DeclaredType = GAIA_MULTILINESTRING;
		      geom->Srid = srid;
		  }
		gaiaInsertLinestringInGeomColl (geom, line);
	    }
      }
    if (shape == GAIA_SHP_POLYGON)
      {
	  /* shape polygon */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
			     shp->endian_arch);
	  base = 8 + (n * 4);
	  start = 0;
	  for (ind = 0; ind < n; ind++)
	    {
		if (ind < (n - 1))
		    end =
			gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    end = n1;
		points = end - start;
		if (shp->EffectiveDims == GAIA_XY_Z)
		    ring = gaiaAllocRingXYZ (points);
		else if (shp->EffectiveDims == GAIA_XY_M)
		    ring = gaiaAllocRingXYM (points);
		else if (shp->EffectiveDims == GAIA_XY_Z_M)
		    ring = gaiaAllocRingXYZM (points);
		else
		    ring = gaiaAllocRing (points);
		points = 0;
		for (iv = start; iv < end; iv++)
		  {
		      x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      y = gaiaImport64 (shp->BufShp + base + (iv * 16) +
					8, GAIA_LITTLE_ENDIAN,
					shp->endian_arch);
		      if (shp->EffectiveDims == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, points, x, y, 0.0);
			}
		      else if (shp->EffectiveDims == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, points, x, y, 0.0);
			}
		      else if (shp->EffectiveDims == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, points, x, y,
					      0.0, 0.0);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, points, x, y);
			}
		      start++;
		      points++;
		  }
		shp_add_ring (&ringsColl, ring);
	    }
	  shp_arrange_rings (&ringsColl);
	  /* allocating the final geometry */
	  if (shp->EffectiveDims == GAIA_XY_Z)
	      geom = gaiaAllocGeomCollXYZ ();
	  else if (shp->EffectiveDims == GAIA_XY_M)
	      geom = gaiaAllocGeomCollXYM ();
	  else if (shp->EffectiveDims == GAIA_XY_Z_M)
	      geom = gaiaAllocGeomCollXYZM ();
	  else
	      geom = gaiaAllocGeomColl ();
	  if (shp->EffectiveType == GAIA_POLYGON)
	      geom->DeclaredType = GAIA_POLYGON;
	  else
	      geom->DeclaredType = GAIA_MULTIPOLYGON;
	  geom->Srid = srid;
	  shp_build_area (&ringsColl, geom);
      }
    if (shape == GAIA_SHP_POLYGONZ)
      {
	  /* shape polygon Z */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
			     shp->endian_arch);
	  hasM = 0;
	  max_size = 38 + (2 * n) + (n1 * 16);	/* size [in 16 bits words !!!] ZM */
	  min_size = 30 + (2 * n) + (n1 * 12);	/* size [in 16 bits words !!!] Z-only */
	  if (sz < min_size)
	      goto error;
	  if (sz == max_size)
	      hasM = 1;
	  base = 8 + (n * 4);
	  baseZ = base + (n1 * 16) + 16;
	  baseM = baseZ + (n1 * 8) + 16;
	  start = 0;
	  for (ind = 0; ind < n; ind++)
	    {
		if (ind < (n - 1))
		    end =
			gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    end = n1;
		points = end - start;
		if (shp->EffectiveDims == GAIA_XY_Z)
		    ring = gaiaAllocRingXYZ (points);
		else if (shp->EffectiveDims == GAIA_XY_M)
		    ring = gaiaAllocRingXYM (points);
		else if (shp->EffectiveDims == GAIA_XY_Z_M)
		    ring = gaiaAllocRingXYZM (points);
		else
		    ring = gaiaAllocRing (points);
		points = 0;
		for (iv = start; iv < end; iv++)
		  {
		      x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      y = gaiaImport64 (shp->BufShp + base + (iv * 16) +
					8, GAIA_LITTLE_ENDIAN,
					shp->endian_arch);
		      z = gaiaImport64 (shp->BufShp + baseZ + (iv * 8),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      if (hasM)
			  m = gaiaImport64 (shp->BufShp + baseM +
					    (iv * 8), GAIA_LITTLE_ENDIAN,
					    shp->endian_arch);
		      else
			  m = 0.0;
		      if (m < SHAPEFILE_NO_DATA)
			  m = 0.0;
		      if (shp->EffectiveDims == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, points, x, y, z);
			}
		      else if (shp->EffectiveDims == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, points, x, y, m);
			}
		      else if (shp->EffectiveDims == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, points, x, y, z, m);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, points, x, y);
			}
		      start++;
		      points++;
		  }
		shp_add_ring (&ringsColl, ring);
	    }
	  shp_arrange_rings (&ringsColl);
	  /* allocating the final geometry */
	  if (shp->EffectiveDims == GAIA_XY_Z)
	      geom = gaiaAllocGeomCollXYZ ();
	  else if (shp->EffectiveDims == GAIA_XY_M)
	      geom = gaiaAllocGeomCollXYM ();
	  else if (shp->EffectiveDims == GAIA_XY_Z_M)
	      geom = gaiaAllocGeomCollXYZM ();
	  else
	      geom = gaiaAllocGeomColl ();
	  if (shp->EffectiveType == GAIA_POLYGON)
	      geom->DeclaredType = GAIA_POLYGON;
	  else
	      geom->DeclaredType = GAIA_MULTIPOLYGON;
	  geom->Srid = srid;
	  shp_build_area (&ringsColl, geom);
      }
    if (shape == GAIA_SHP_POLYGONM)
      {
	  /* shape polygon M */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
			     shp->endian_arch);
	  hasM = 0;
	  max_size = 30 + (2 * n) + (n1 * 12);	/* size [in 16 bits words !!!] M */
	  min_size = 22 + (2 * n) + (n1 * 8);	/* size [in 16 bits words !!!] no-M */
	  if (sz < min_size)
	      goto error;
	  if (sz == max_size)
	      hasM = 1;
	  base = 8 + (n * 4);
	  baseM = base + (n1 * 16) + 16;
	  start = 0;
	  for (ind = 0; ind < n; ind++)
	    {
		if (ind < (n - 1))
		    end =
			gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    end = n1;
		points = end - start;
		if (shp->EffectiveDims == GAIA_XY_Z)
		    ring = gaiaAllocRingXYZ (points);
		else if (shp->EffectiveDims == GAIA_XY_M)
		    ring = gaiaAllocRingXYM (points);
		else if (shp->EffectiveDims == GAIA_XY_Z_M)
		    ring = gaiaAllocRingXYZM (points);
		else
		    ring = gaiaAllocRing (points);
		points = 0;
		for (iv = start; iv < end; iv++)
		  {
		      x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      y = gaiaImport64 (shp->BufShp + base + (iv * 16) +
					8, GAIA_LITTLE_ENDIAN,
					shp->endian_arch);
		      if (hasM)
			  m = gaiaImport64 (shp->BufShp + baseM +
					    (iv * 8), GAIA_LITTLE_ENDIAN,
					    shp->endian_arch);
		      m = 0.0;
		      if (m < SHAPEFILE_NO_DATA)
			  m = 0.0;
		      if (shp->EffectiveDims == GAIA_XY_Z)
			{
			    gaiaSetPointXYZ (ring->Coords, points, x, y, 0.0);
			}
		      else if (shp->EffectiveDims == GAIA_XY_M)
			{
			    gaiaSetPointXYM (ring->Coords, points, x, y, m);
			}
		      else if (shp->EffectiveDims == GAIA_XY_Z_M)
			{
			    gaiaSetPointXYZM (ring->Coords, points, x, y,
					      0.0, m);
			}
		      else
			{
			    gaiaSetPoint (ring->Coords, points, x, y);
			}
		      start++;
		      points++;
		  }
		shp_add_ring (&ringsColl, ring);
	    }
	  shp_arrange_rings (&ringsColl);
	  /* allocating the final geometry */
	  if (shp->EffectiveDims == GAIA_XY_Z)
	      geom = gaiaAllocGeomCollXYZ ();
	  else if (shp->EffectiveDims == GAIA_XY_M)
	      geom = gaiaAllocGeomCollXYM ();
	  else if (shp->EffectiveDims == GAIA_XY_Z_M)
	      geom = gaiaAllocGeomCollXYZM ();
	  else
	      geom = gaiaAllocGeomColl ();
	  if (shp->EffectiveType == GAIA_POLYGON)
	      geom->DeclaredType = GAIA_POLYGON;
	  else
	      geom->DeclaredType = GAIA_MULTIPOLYGON;
	  geom->Srid = srid;
	  shp_build_area (&ringsColl, geom);
      }
    if (shape == GAIA_SHP_MULTIPOINT)
      {
	  /* shape multipoint */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  if (shp->EffectiveDims == GAIA_XY_Z)
	      geom = gaiaAllocGeomCollXYZ ();
	  else if (shp->EffectiveDims == GAIA_XY_M)
	      geom = gaiaAllocGeomCollXYM ();
	  else if (shp->EffectiveDims == GAIA_XY_Z_M)
	      geom = gaiaAllocGeomCollXYZM ();
	  else
	      geom = gaiaAllocGeomColl ();
	  geom->DeclaredType = GAIA_MULTIPOINT;
	  geom->Srid = srid;
	  for (iv = 0; iv < n; iv++)
	    {
		x = gaiaImport64 (shp->BufShp + 4 + (iv * 16),
				  GAIA_LITTLE_ENDIAN, shp->endian_arch);
		y = gaiaImport64 (shp->BufShp + 4 + (iv * 16) + 8,
				  GAIA_LITTLE_ENDIAN, shp->endian_arch);
		if (shp->EffectiveDims == GAIA_XY_Z)
		    gaiaAddPointToGeomCollXYZ (geom, x, y, 0.0);
		else if (shp->EffectiveDims == GAIA_XY_M)
		    gaiaAddPointToGeomCollXYM (geom, x, y, 0.0);
		else if (shp->EffectiveDims == GAIA_XY_Z_M)
		    gaiaAddPointToGeomCollXYZM (geom, x, y, 0.0, 0.0);
		else
		    gaiaAddPointToGeomColl (geom, x, y);
	    }
      }
    if (shape == GAIA_SHP_MULTIPOINTZ)
      {
	  /* shape multipoint Z */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  hasM = 0;
	  max_size = 36 + (n * 16);	/* size [in 16 bits words !!!] ZM */
	  min_size = 28 + (n * 12);	/* size [in 16 bits words !!!] Z-only */
	  if (sz < min_size)
	      goto error;
	  if (sz == max_size)
	      hasM = 1;
	  baseZ = 4 + (n * 16) + 16;
	  baseM = baseZ + (n * 8) + 16;
	  if (shp->EffectiveDims == GAIA_XY_Z)
	      geom = gaiaAllocGeomCollXYZ ();
	  else if (shp->EffectiveDims == GAIA_XY_M)
	      geom = gaiaAllocGeomCollXYM ();
	  else if (shp->EffectiveDims == GAIA_XY_Z_M)
	      geom = gaiaAllocGeomCollXYZM ();
	  else
	      geom = gaiaAllocGeomColl ();
	  geom->DeclaredType = GAIA_MULTIPOINT;
	  geom->Srid = srid;
	  for (iv = 0; iv < n; iv++)
	    {
		x = gaiaImport64 (shp->BufShp + 4 + (iv * 16),
				  GAIA_LITTLE_ENDIAN, shp->endian_arch);
		y = gaiaImport64 (shp->BufShp + 4 + (iv * 16) + 8,
				  GAIA_LITTLE_ENDIAN, shp->endian_arch);
		z = gaiaImport64 (shp->BufShp + baseZ + (iv * 8),
				  GAIA_LITTLE_ENDIAN, shp->endian_arch);
		if (hasM)
		    m = gaiaImport64 (shp->BufShp + baseM + (iv * 8),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    m = 0.0;
		if (m < SHAPEFILE_NO_DATA)
		    m = 0.0;
		if (shp->EffectiveDims == GAIA_XY_Z)
		    gaiaAddPointToGeomCollXYZ (geom, x, y, z);
		else if (shp->EffectiveDims == GAIA_XY_M)
		    gaiaAddPointToGeomCollXYM (geom, x, y, m);
		else if (shp->EffectiveDims == GAIA_XY_Z_M)
		    gaiaAddPointToGeomCollXYZM (geom, x, y, z, m);
		else
		    gaiaAddPointToGeomColl (geom, x, y);
	    }
      }
    if (shape == GAIA_SHP_MULTIPOINTM)
      {
	  /* shape multipoint M */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  hasM = 0;
	  max_size = 28 + (n * 12);	/* size [in 16 bits words !!!] M */
	  min_size = 20 + (n * 8);	/* size [in 16 bits words !!!] no-M */
	  if (sz < min_size)
	      goto error;
	  if (sz == max_size)
	      hasM = 1;
	  baseM = 4 + (n * 16) + 16;
	  if (shp->EffectiveDims == GAIA_XY_Z)
	      geom = gaiaAllocGeomCollXYZ ();
	  else if (shp->EffectiveDims == GAIA_XY_M)
	      geom = gaiaAllocGeomCollXYM ();
	  else if (shp->EffectiveDims == GAIA_XY_Z_M)
	      geom = gaiaAllocGeomCollXYZM ();
	  else
	      geom = gaiaAllocGeomColl ();
	  geom->DeclaredType = GAIA_MULTIPOINT;
	  geom->Srid = srid;
	  for (iv = 0; iv < n; iv++)
	    {
		x = gaiaImport64 (shp->BufShp + 4 + (iv * 16),
				  GAIA_LITTLE_ENDIAN, shp->endian_arch);
		y = gaiaImport64 (shp->BufShp + 4 + (iv * 16) + 8,
				  GAIA_LITTLE_ENDIAN, shp->endian_arch);
		if (hasM)
		    m = gaiaImport64 (shp->BufShp + baseM + (iv * 8),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    m = 0.0;
		if (m < SHAPEFILE_NO_DATA)
		    m = 0.0;
		if (shp->EffectiveDims == GAIA_XY_Z)
		    gaiaAddPointToGeomCollXYZ (geom, x, y, 0.0);
		else if (shp->EffectiveDims == GAIA_XY_M)
		    gaiaAddPointToGeomCollXYM (geom, x, y, m);
		else if (shp->EffectiveDims == GAIA_XY_Z_M)
		    gaiaAddPointToGeomCollXYZM (geom, x, y, 0.0, m);
		else
		    gaiaAddPointToGeomColl (geom, x, y);
	    }
      }
/* setting up the current SHP ENTITY */
  null_shape:
    gaiaResetDbfEntity (shp->Dbf);
    shp->Dbf->RowId = current_row;
    shp->Dbf->Geometry = geom;
/* fetching the DBF values */
    pFld = shp->Dbf->First;
    while (pFld)
      {
	  if (!parseDbfField (shp->BufDbf, shp->IconvObj, pFld, text_dates))
	      goto conversion_error;
	  pFld = pFld->Next;
      }
    if (shp->LastError)
	free (shp->LastError);
    shp->LastError = NULL;
    shp_free_rings (&ringsColl);
    return 1;
  eof:
    if (shp->LastError)
	free (shp->LastError);
    shp->LastError = NULL;
    shp_free_rings (&ringsColl);
    return 0;
  error:
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s' is corrupted / has invalid format", shp->Path);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    shp_free_rings (&ringsColl);
    return 0;
  conversion_error:
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "Invalid character sequence");
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    shp_free_rings (&ringsColl);
    return 0;
}

static void
gaiaSaneClockwise (gaiaPolygonPtr polyg)
{
/*
/ when exporting POLYGONs to SHAPEFILE, we must guarantee that:
/ - all EXTERIOR RING must be clockwise
/ - all INTERIOR RING must be anti-clockwise
/
/ this function checks for the above conditions,
/ and if needed inverts the rings
*/
    int ib;
    int iv;
    int iv2;
    double x;
    double y;
    double z;
    double m;
    gaiaRingPtr new_ring;
    gaiaRingPtr ring = polyg->Exterior;
    gaiaClockwise (ring);
    if (!(ring->Clockwise))
      {
	  /* exterior ring needs inversion */
	  if (ring->DimensionModel == GAIA_XY_Z)
	      new_ring = gaiaAllocRingXYZ (ring->Points);
	  else if (ring->DimensionModel == GAIA_XY_M)
	      new_ring = gaiaAllocRingXYM (ring->Points);
	  else if (ring->DimensionModel == GAIA_XY_Z_M)
	      new_ring = gaiaAllocRingXYZM (ring->Points);
	  else
	      new_ring = gaiaAllocRing (ring->Points);
	  iv2 = 0;
	  for (iv = ring->Points - 1; iv >= 0; iv--)
	    {
		if (ring->DimensionModel == GAIA_XY_Z)
		  {
		      gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
		      gaiaSetPointXYZ (new_ring->Coords, iv2, x, y, z);
		  }
		else if (ring->DimensionModel == GAIA_XY_M)
		  {
		      gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
		      gaiaSetPointXYM (new_ring->Coords, iv2, x, y, m);
		  }
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		  {
		      gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
		      gaiaSetPointXYZM (new_ring->Coords, iv2, x, y, z, m);
		  }
		else
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      gaiaSetPoint (new_ring->Coords, iv2, x, y);
		  }
		iv2++;
	    }
	  polyg->Exterior = new_ring;
	  gaiaFreeRing (ring);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  gaiaClockwise (ring);
	  if (ring->Clockwise)
	    {
		/* interior ring needs inversion */
		if (ring->DimensionModel == GAIA_XY_Z)
		    new_ring = gaiaAllocRingXYZ (ring->Points);
		else if (ring->DimensionModel == GAIA_XY_M)
		    new_ring = gaiaAllocRingXYM (ring->Points);
		else if (ring->DimensionModel == GAIA_XY_Z_M)
		    new_ring = gaiaAllocRingXYZM (ring->Points);
		else
		    new_ring = gaiaAllocRing (ring->Points);
		iv2 = 0;
		for (iv = ring->Points - 1; iv >= 0; iv--)
		  {
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (ring->Coords, iv, &x, &y, &z);
			    gaiaSetPointXYZ (new_ring->Coords, iv2, x, y, z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (ring->Coords, iv, &x, &y, &m);
			    gaiaSetPointXYM (new_ring->Coords, iv2, x, y, m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (ring->Coords, iv, &x, &y, &z, &m);
			    gaiaSetPointXYZM (new_ring->Coords, iv2, x, y,
					      z, m);
			}
		      else
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			    gaiaSetPoint (new_ring->Coords, iv2, x, y);
			}
		      iv2++;
		  }
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      if (ring->DimensionModel == GAIA_XY_Z)
			{
			    gaiaGetPointXYZ (new_ring->Coords, iv, &x, &y, &z);
			    gaiaSetPointXYZ (ring->Coords, iv, x, y, z);
			}
		      else if (ring->DimensionModel == GAIA_XY_M)
			{
			    gaiaGetPointXYM (new_ring->Coords, iv, &x, &y, &m);
			    gaiaSetPointXYM (ring->Coords, iv, x, y, m);
			}
		      else if (ring->DimensionModel == GAIA_XY_Z_M)
			{
			    gaiaGetPointXYZM (new_ring->Coords, iv, &x, &y,
					      &z, &m);
			    gaiaSetPointXYZM (ring->Coords, iv, x, y, z, m);
			}
		      else
			{
			    gaiaGetPoint (new_ring->Coords, iv, &x, &y);
			    gaiaSetPoint (ring->Coords, iv, x, y);
			}
		  }
		gaiaFreeRing (new_ring);
	    }
      }
}

GAIAGEO_DECLARE int
gaiaWriteShpEntity (gaiaShapefilePtr shp, gaiaDbfListPtr entity)
{
/* trying to write an entity into shapefile */
    char dummy[128];
    char fmt[16];
    int endian_arch = shp->endian_arch;
    gaiaDbfFieldPtr fld;
    int iv;
    int tot_ln;
    int tot_v;
    int tot_pts;
    int this_size;
    int ix;
    double x;
    double y;
    double z;
    double m;
    int hasM;
    double minZ;
    double maxZ;
    double minM;
    double maxM;
#if !defined(__MINGW32__) && defined(_WIN32)
    const char *pBuf;
#else /* not WIN32 */
    char *pBuf;
#endif
    size_t len;
    size_t utf8len;
    char *dynbuf;
    char *pUtf8buf;
    char utf8buf[2048];
/* writing the DBF record */
    memset (shp->BufDbf, '\0', shp->DbfReclen);
    *(shp->BufDbf) = ' ';	/* in DBF first byte of each row marks for validity or deletion */
    fld = entity->First;
    while (fld)
      {
	  /* transferring field values */
	  switch (fld->Type)
	    {
	    case 'L':
		if (!(fld->Value))
		    *(shp->BufDbf + fld->Offset) = '?';
		else if (fld->Value->Type != GAIA_INT_VALUE)
		    *(shp->BufDbf + fld->Offset + 1) = '?';
		else
		  {
		      if (fld->Value->IntValue == 0)
			  *(shp->BufDbf + fld->Offset + 1) = 'N';
		      else
			  *(shp->BufDbf + fld->Offset + 1) = 'Y';
		  }
		break;
	    case 'D':
		memset (shp->BufDbf + fld->Offset + 1, '0', 8);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_TEXT_VALUE)
			{
			    if (strlen (fld->Value->TxtValue) == 8)
				memcpy (shp->BufDbf + fld->Offset + 1,
					fld->Value->TxtValue, 8);
			}
		  }
		break;
	    case 'C':
		memset (shp->BufDbf + fld->Offset + 1, ' ', fld->Length);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_TEXT_VALUE)
			{
			    len = strlen (fld->Value->TxtValue);
			    dynbuf = malloc (len + 1);
			    strcpy (dynbuf, fld->Value->TxtValue);
			    if (len > 512)
			      {
				  dynbuf[512] = '\0';
				  len = strlen (dynbuf);
			      }
			    utf8len = 2048;
			    pBuf = dynbuf;
			    pUtf8buf = utf8buf;
			    if (iconv
				((iconv_t) (shp->IconvObj), &pBuf, &len,
				 &pUtf8buf, &utf8len) == (size_t) (-1))
			      {
				  free (dynbuf);
				  goto conversion_error;
			      }
			    memcpy (dynbuf, utf8buf, 2048 - utf8len);
			    dynbuf[2048 - utf8len] = '\0';
			    if (strlen (dynbuf) < fld->Length)
				memcpy (shp->BufDbf + fld->Offset + 1, dynbuf,
					strlen (dynbuf));
			    else
				memcpy (shp->BufDbf + fld->Offset + 1, dynbuf,
					fld->Length);
			    free (dynbuf);
			}
		  }
		break;
	    case 'N':
		memset (shp->BufDbf + fld->Offset + 1, '\0', fld->Length);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_INT_VALUE)
			{
			    sprintf (dummy, FRMT64, fld->Value->IntValue);
			    if (strlen (dummy) <= fld->Length)
				memcpy (shp->BufDbf + fld->Offset + 1,
					dummy, strlen (dummy));
			}
		      if (fld->Value->Type == GAIA_DOUBLE_VALUE)
			{
			    sprintf (fmt, "%%1.%df", fld->Decimals);
			    sprintf (dummy, fmt, fld->Value->DblValue);
			    if (strlen (dummy) <= fld->Length)
				memcpy (shp->BufDbf + fld->Offset + 1,
					dummy, strlen (dummy));
			}
		  }
		break;
	    };
	  fld = fld->Next;
      }
    if (!(entity->Geometry))
      {
	  /* exporting a NULL Shape */
	  gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
	  gaiaExport32 (shp->BufShp + 4, 2, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
	  fwrite (shp->BufShp, 1, 8, shp->flShx);
	  (shp->ShxSize) += 4;	/* updating current SHX file position [in 16 bits words !!!] */
	  gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
	  gaiaExport32 (shp->BufShp + 4, 2, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
	  gaiaExport32 (shp->BufShp + 8, GAIA_SHP_NULL, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = NULL */
	  fwrite (shp->BufShp, 1, 12, shp->flShp);
	  (shp->ShpSize) += 6;	/* updating current SHP file position [in 16 bits words !!!] */
      }
    else
      {
	  /* updates the shapefile main MBR-BBOX */
	  gaiaMbrGeometry (entity->Geometry);
	  if (entity->Geometry->MinX < shp->MinX)
	      shp->MinX = entity->Geometry->MinX;
	  if (entity->Geometry->MaxX > shp->MaxX)
	      shp->MaxX = entity->Geometry->MaxX;
	  if (entity->Geometry->MinY < shp->MinY)
	      shp->MinY = entity->Geometry->MinY;
	  if (entity->Geometry->MaxY > shp->MaxY)
	      shp->MaxY = entity->Geometry->MaxY;
	  if (shp->Shape == GAIA_SHP_POINT)
	    {
		/* this one is expected to be a POINT */
		gaiaPointPtr pt = entity->Geometry->FirstPoint;
		if (!pt)
		  {
		      strcpy (dummy,
			      "a POINT is expected, but there is no POINT in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		/* inserting POINT entity into SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, 10, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;	/* updating current SHX file position [in 16 bits words !!!] */
		/* inserting POINT into SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, 10, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_POINT, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POINT */
		gaiaExport64 (shp->BufShp + 12, pt->X, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports X coordinate */
		gaiaExport64 (shp->BufShp + 20, pt->Y, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports Y coordinate */
		fwrite (shp->BufShp, 1, 28, shp->flShp);
		(shp->ShpSize) += 14;	/* updating current SHP file position [in 16 bits words !!!] */
	    }
	  if (shp->Shape == GAIA_SHP_POINTZ)
	    {
		/* this one is expected to be a POINT Z */
		gaiaPointPtr pt = entity->Geometry->FirstPoint;
		if (!pt)
		  {
		      strcpy (dummy,
			      "a POINT is expected, but there is no POINT in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		/* inserting POINT Z entity into SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, 18, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;	/* updating current SHX file position [in 16 bits words !!!] */
		/* inserting POINT into SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, 18, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_POINTZ, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POINT Z */
		gaiaExport64 (shp->BufShp + 12, pt->X, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports X coordinate */
		gaiaExport64 (shp->BufShp + 20, pt->Y, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports Y coordinate */
		gaiaExport64 (shp->BufShp + 28, pt->Z, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports Z coordinate */
		gaiaExport64 (shp->BufShp + 36, pt->M, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports M coordinate */
		fwrite (shp->BufShp, 1, 44, shp->flShp);
		(shp->ShpSize) += 22;	/* updating current SHP file position [in 16 bits words !!!] */
	    }
	  if (shp->Shape == GAIA_SHP_POINTM)
	    {
		/* this one is expected to be a POINT M */
		gaiaPointPtr pt = entity->Geometry->FirstPoint;
		if (!pt)
		  {
		      strcpy (dummy,
			      "a POINT is expected, but there is no POINT in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		/* inserting POINT entity into SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, 14, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;	/* updating current SHX file position [in 16 bits words !!!] */
		/* inserting POINT into SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, 14, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_POINTM, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POINT M */
		gaiaExport64 (shp->BufShp + 12, pt->X, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports X coordinate */
		gaiaExport64 (shp->BufShp + 20, pt->Y, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports Y coordinate */
		gaiaExport64 (shp->BufShp + 28, pt->Y, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports M coordinate */
		fwrite (shp->BufShp, 1, 36, shp->flShp);
		(shp->ShpSize) += 18;	/* updating current SHP file position [in 16 bits words !!!] */
	    }
	  if (shp->Shape == GAIA_SHP_POLYLINE)
	    {
		/* this one is expected to be a LINESTRING / MULTILINESTRING */
		gaiaLinestringPtr line;
		tot_ln = 0;
		tot_v = 0;
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* computes # lines and total # points */
		      tot_v += line->Points;
		      tot_ln++;
		      line = line->Next;
		  }
		if (!tot_ln)
		  {
		      strcpy (dummy,
			      "a LINESTRING is expected, but there is no LINESTRING in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		this_size = 22 + (2 * tot_ln) + (tot_v * 8);	/* size [in 16 bits words !!!] for this SHP entity */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting LINESTRING or MULTILINESTRING in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting LINESTRING or MULTILINESTRING in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_POLYLINE, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POLYLINE */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_ln, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # lines in this polyline */
		gaiaExport32 (shp->BufShp + 48, tot_v, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		tot_v = 0;	/* resets points counter */
		ix = 52;	/* sets current buffer offset */
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* exports start point index for each line */
		      gaiaExport32 (shp->BufShp + ix, tot_v,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      tot_v += line->Points;
		      ix += 4;
		      line = line->Next;
		  }
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* exports points for each line */
		      for (iv = 0; iv < line->Points; iv++)
			{
			    /* exports a POINT [x,y] */
			    if (line->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (line->Coords, iv, &x, &y,
						   &z);
			      }
			    else if (line->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (line->Coords, iv, &x, &y,
						   &m);
			      }
			    else if (line->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (line->Coords, iv, &x,
						    &y, &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (line->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (shp->BufShp + ix, x,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			    gaiaExport64 (shp->BufShp + ix, y,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      line = line->Next;
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);	/* updating current SHP file position [in 16 bits words !!!] */
	    }
	  if (shp->Shape == GAIA_SHP_POLYLINEZ)
	    {
		/* this one is expected to be a LINESTRING / MULTILINESTRING Z */
		gaiaLinestringPtr line;
		gaiaZRangeGeometry (entity->Geometry, &minZ, &maxZ);
		gaiaMRangeGeometry (entity->Geometry, &minM, &maxM);
		tot_ln = 0;
		tot_v = 0;
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* computes # lines and total # points */
		      tot_v += line->Points;
		      tot_ln++;
		      line = line->Next;
		  }
		if (!tot_ln)
		  {
		      strcpy (dummy,
			      "a LINESTRING is expected, but there is no LINESTRING in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		hasM = 0;
		if (shp->EffectiveDims == GAIA_XY_M
		    || shp->EffectiveDims == GAIA_XY_Z_M)
		    hasM = 1;
		if (hasM)
		    this_size = 38 + (2 * tot_ln) + (tot_v * 16);	/* size [in 16 bits words !!!] ZM */
		else
		    this_size = 30 + (2 * tot_ln) + (tot_v * 12);	/* size [in 16 bits words !!!] Z-only */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting LINESTRING or MULTILINESTRING in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting LINESTRING or MULTILINESTRING in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_POLYLINEZ, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POLYLINE Z */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_ln, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # lines in this polyline */
		gaiaExport32 (shp->BufShp + 48, tot_v, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		tot_v = 0;	/* resets points counter */
		ix = 52;	/* sets current buffer offset */
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* exports start point index for each line */
		      gaiaExport32 (shp->BufShp + ix, tot_v,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      tot_v += line->Points;
		      ix += 4;
		      line = line->Next;
		  }
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* exports points for each line */
		      for (iv = 0; iv < line->Points; iv++)
			{
			    /* exports a POINT [x,y] */
			    if (line->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (line->Coords, iv, &x, &y,
						   &z);
			      }
			    else if (line->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (line->Coords, iv, &x, &y,
						   &m);
			      }
			    else if (line->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (line->Coords, iv, &x,
						    &y, &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (line->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (shp->BufShp + ix, x,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			    gaiaExport64 (shp->BufShp + ix, y,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      line = line->Next;
		  }
		/* exporting the Z-range [min/max] */
		gaiaExport64 (shp->BufShp + ix, minZ, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		gaiaExport64 (shp->BufShp + ix, maxZ, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* exports Z-values for each line */
		      for (iv = 0; iv < line->Points; iv++)
			{
			    /* exports Z-value */
			    z = 0.0;
			    if (line->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (line->Coords, iv, &x, &y,
						   &z);
			      }
			    else if (line->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (line->Coords, iv, &x, &y,
						   &m);
			      }
			    else if (line->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (line->Coords, iv, &x,
						    &y, &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (line->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (shp->BufShp + ix, z,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      line = line->Next;
		  }
		if (hasM)
		  {
		      /* exporting the M-range [min/max] */
		      gaiaExport64 (shp->BufShp + ix, minM,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      gaiaExport64 (shp->BufShp + ix, maxM,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      line = entity->Geometry->FirstLinestring;
		      while (line)
			{
			    /* exports M-values for each line */
			    for (iv = 0; iv < line->Points; iv++)
			      {
				  /* exports M-value */
				  m = 0.0;
				  if (line->DimensionModel == GAIA_XY_Z)
				    {
					gaiaGetPointXYZ (line->Coords, iv,
							 &x, &y, &z);
				    }
				  else if (line->DimensionModel == GAIA_XY_M)
				    {
					gaiaGetPointXYM (line->Coords, iv,
							 &x, &y, &m);
				    }
				  else if (line->DimensionModel == GAIA_XY_Z_M)
				    {
					gaiaGetPointXYZM (line->Coords, iv,
							  &x, &y, &z, &m);
				    }
				  else
				    {
					gaiaGetPoint (line->Coords, iv, &x, &y);
				    }
				  gaiaExport64 (shp->BufShp + ix, m,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
			      }
			    line = line->Next;
			}
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);	/* updating current SHP file position [in 16 bits words !!!] */
	    }
	  if (shp->Shape == GAIA_SHP_POLYLINEM)
	    {
		/* this one is expected to be a LINESTRING / MULTILINESTRING M */
		gaiaLinestringPtr line;
		gaiaMRangeGeometry (entity->Geometry, &minM, &maxM);
		tot_ln = 0;
		tot_v = 0;
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* computes # lines and total # points */
		      tot_v += line->Points;
		      tot_ln++;
		      line = line->Next;
		  }
		if (!tot_ln)
		  {
		      strcpy (dummy,
			      "a LINESTRING is expected, but there is no LINESTRING in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		this_size = 30 + (2 * tot_ln) + (tot_v * 12);	/* size [in 16 bits words !!!] for this SHP entity */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting LINESTRING or MULTILINESTRING in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting LINESTRING or MULTILINESTRING in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_POLYLINEM, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POLYLINE M */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_ln, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # lines in this polyline */
		gaiaExport32 (shp->BufShp + 48, tot_v, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		tot_v = 0;	/* resets points counter */
		ix = 52;	/* sets current buffer offset */
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* exports start point index for each line */
		      gaiaExport32 (shp->BufShp + ix, tot_v,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      tot_v += line->Points;
		      ix += 4;
		      line = line->Next;
		  }
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* exports points for each line */
		      for (iv = 0; iv < line->Points; iv++)
			{
			    /* exports a POINT [x,y] */
			    if (line->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (line->Coords, iv, &x, &y,
						   &z);
			      }
			    else if (line->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (line->Coords, iv, &x, &y,
						   &m);
			      }
			    else if (line->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (line->Coords, iv, &x,
						    &y, &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (line->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (shp->BufShp + ix, x,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			    gaiaExport64 (shp->BufShp + ix, y,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      line = line->Next;
		  }
		/* exporting the M-range [min/max] */
		gaiaExport64 (shp->BufShp + ix, minM, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		gaiaExport64 (shp->BufShp + ix, maxM, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* exports M-values for each line */
		      for (iv = 0; iv < line->Points; iv++)
			{
			    /* exports M-value */
			    m = 0.0;
			    if (line->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (line->Coords, iv, &x, &y,
						   &z);
			      }
			    else if (line->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (line->Coords, iv, &x, &y,
						   &m);
			      }
			    else if (line->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (line->Coords, iv, &x,
						    &y, &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (line->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (shp->BufShp + ix, m,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      line = line->Next;
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);	/* updating current SHP file position [in 16 bits words !!!] */
	    }
	  if (shp->Shape == GAIA_SHP_POLYGON)
	    {
		/* this one is expected to be a POLYGON or a MULTIPOLYGON */
		gaiaPolygonPtr polyg;
		gaiaRingPtr ring;
		int ib;
		tot_ln = 0;
		tot_v = 0;
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* computes # rings and total # points */
		      gaiaSaneClockwise (polyg);	/* we must assure that exterior ring is clockwise, and interior rings are anti-clockwise */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      tot_v += ring->Points;
		      tot_ln++;
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    tot_v += ring->Points;
			    tot_ln++;
			}
		      polyg = polyg->Next;
		  }
		if (!tot_ln)
		  {
		      strcpy (dummy,
			      "a POLYGON is expected, but there is no POLYGON in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		this_size = 22 + (2 * tot_ln) + (tot_v * 8);	/* size [in 16 bits words !!!] for this SHP entity */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting POLYGON or MULTIPOLYGON in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting POLYGON or MULTIPOLYGON in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_POLYGON, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POLYGON */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_ln, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # rings in this polygon */
		gaiaExport32 (shp->BufShp + 48, tot_v, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		tot_v = 0;	/* resets points counter */
		ix = 52;	/* sets current buffer offset */
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* exports start point index for each line */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      gaiaExport32 (shp->BufShp + ix, tot_v,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      tot_v += ring->Points;
		      ix += 4;
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    gaiaExport32 (shp->BufShp + ix, tot_v,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    tot_v += ring->Points;
			    ix += 4;
			}
		      polyg = polyg->Next;
		  }
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* exports points for each ring */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      for (iv = 0; iv < ring->Points; iv++)
			{
			    /* exports a POINT [x,y] - exterior ring */
			    if (ring->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (ring->Coords, iv, &x, &y,
						   &z);
			      }
			    else if (ring->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (ring->Coords, iv, &x, &y,
						   &m);
			      }
			    else if (ring->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (ring->Coords, iv, &x,
						    &y, &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (ring->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (shp->BufShp + ix, x,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			    gaiaExport64 (shp->BufShp + ix, y,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    for (iv = 0; iv < ring->Points; iv++)
			      {
				  /* exports a POINT [x,y] - interior ring */
				  if (ring->DimensionModel == GAIA_XY_Z)
				    {
					gaiaGetPointXYZ (ring->Coords, iv,
							 &x, &y, &z);
				    }
				  else if (ring->DimensionModel == GAIA_XY_M)
				    {
					gaiaGetPointXYM (ring->Coords, iv,
							 &x, &y, &m);
				    }
				  else if (ring->DimensionModel == GAIA_XY_Z_M)
				    {
					gaiaGetPointXYZM (ring->Coords, iv,
							  &x, &y, &z, &m);
				    }
				  else
				    {
					gaiaGetPoint (ring->Coords, iv, &x, &y);
				    }
				  gaiaExport64 (shp->BufShp + ix, x,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
				  gaiaExport64 (shp->BufShp + ix, y,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
			      }
			}
		      polyg = polyg->Next;
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);
	    }
	  if (shp->Shape == GAIA_SHP_POLYGONZ)
	    {
		/* this one is expected to be a POLYGON or a MULTIPOLYGON Z */
		gaiaPolygonPtr polyg;
		gaiaRingPtr ring;
		int ib;
		gaiaZRangeGeometry (entity->Geometry, &minZ, &maxZ);
		gaiaMRangeGeometry (entity->Geometry, &minM, &maxM);
		tot_ln = 0;
		tot_v = 0;
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* computes # rings and total # points */
		      gaiaSaneClockwise (polyg);	/* we must assure that exterior ring is clockwise, and interior rings are anti-clockwise */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      tot_v += ring->Points;
		      tot_ln++;
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    tot_v += ring->Points;
			    tot_ln++;
			}
		      polyg = polyg->Next;
		  }
		if (!tot_ln)
		  {
		      strcpy (dummy,
			      "a POLYGON is expected, but there is no POLYGON in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		hasM = 0;
		if (shp->EffectiveDims == GAIA_XY_M
		    || shp->EffectiveDims == GAIA_XY_Z_M)
		    hasM = 1;
		if (hasM)
		    this_size = 38 + (2 * tot_ln) + (tot_v * 16);	/* size [in 16 bits words !!!] ZM */
		else
		    this_size = 30 + (2 * tot_ln) + (tot_v * 12);	/* size [in 16 bits words !!!] Z-only */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting POLYGON or MULTIPOLYGON in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting POLYGON or MULTIPOLYGON in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_POLYGONZ, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POLYGON Z */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_ln, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # rings in this polygon */
		gaiaExport32 (shp->BufShp + 48, tot_v, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		tot_v = 0;	/* resets points counter */
		ix = 52;	/* sets current buffer offset */
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* exports start point index for each line */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      gaiaExport32 (shp->BufShp + ix, tot_v,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      tot_v += ring->Points;
		      ix += 4;
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    gaiaExport32 (shp->BufShp + ix, tot_v,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    tot_v += ring->Points;
			    ix += 4;
			}
		      polyg = polyg->Next;
		  }
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* exports points for each ring */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      for (iv = 0; iv < ring->Points; iv++)
			{
			    /* exports a POINT [x,y] - exterior ring */
			    if (ring->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (ring->Coords, iv, &x, &y,
						   &z);
			      }
			    else if (ring->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (ring->Coords, iv, &x, &y,
						   &m);
			      }
			    else if (ring->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (ring->Coords, iv, &x,
						    &y, &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (ring->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (shp->BufShp + ix, x,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			    gaiaExport64 (shp->BufShp + ix, y,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    for (iv = 0; iv < ring->Points; iv++)
			      {
				  /* exports a POINT [x,y] - interior ring */
				  if (ring->DimensionModel == GAIA_XY_Z)
				    {
					gaiaGetPointXYZ (ring->Coords, iv,
							 &x, &y, &z);
				    }
				  else if (ring->DimensionModel == GAIA_XY_M)
				    {
					gaiaGetPointXYM (ring->Coords, iv,
							 &x, &y, &m);
				    }
				  else if (ring->DimensionModel == GAIA_XY_Z_M)
				    {
					gaiaGetPointXYZM (ring->Coords, iv,
							  &x, &y, &z, &m);
				    }
				  else
				    {
					gaiaGetPoint (ring->Coords, iv, &x, &y);
				    }
				  gaiaExport64 (shp->BufShp + ix, x,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
				  gaiaExport64 (shp->BufShp + ix, y,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
			      }
			}
		      polyg = polyg->Next;
		  }
		/* exporting the Z-range [min/max] */
		gaiaExport64 (shp->BufShp + ix, minZ, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		gaiaExport64 (shp->BufShp + ix, maxZ, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* exports Z-values for each ring */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      for (iv = 0; iv < ring->Points; iv++)
			{
			    /* exports Z-values - exterior ring */
			    z = 0.0;
			    if (ring->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (ring->Coords, iv, &x, &y,
						   &z);
			      }
			    else if (ring->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (ring->Coords, iv, &x, &y,
						   &m);
			      }
			    else if (ring->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (ring->Coords, iv, &x,
						    &y, &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (ring->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (shp->BufShp + ix, z,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    for (iv = 0; iv < ring->Points; iv++)
			      {
				  /* exports Z-values - interior ring */
				  z = 0.0;
				  if (ring->DimensionModel == GAIA_XY_Z)
				    {
					gaiaGetPointXYZ (ring->Coords, iv,
							 &x, &y, &z);
				    }
				  else if (ring->DimensionModel == GAIA_XY_M)
				    {
					gaiaGetPointXYM (ring->Coords, iv,
							 &x, &y, &m);
				    }
				  else if (ring->DimensionModel == GAIA_XY_Z_M)
				    {
					gaiaGetPointXYZM (ring->Coords, iv,
							  &x, &y, &z, &m);
				    }
				  else
				    {
					gaiaGetPoint (ring->Coords, iv, &x, &y);
				    }
				  gaiaExport64 (shp->BufShp + ix, z,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
			      }
			}
		      polyg = polyg->Next;
		  }
		if (hasM)
		  {
		      /* exporting the M-range [min/max] */
		      gaiaExport64 (shp->BufShp + ix, minM,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      gaiaExport64 (shp->BufShp + ix, maxM,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      polyg = entity->Geometry->FirstPolygon;
		      while (polyg)
			{
			    /* exports M-values for each ring */
			    ring = polyg->Exterior;	/* this one is the exterior ring */
			    for (iv = 0; iv < ring->Points; iv++)
			      {
				  /* exports M-values - exterior ring */
				  m = 0.0;
				  if (ring->DimensionModel == GAIA_XY_Z)
				    {
					gaiaGetPointXYZ (ring->Coords, iv,
							 &x, &y, &z);
				    }
				  else if (ring->DimensionModel == GAIA_XY_M)
				    {
					gaiaGetPointXYM (ring->Coords, iv,
							 &x, &y, &m);
				    }
				  else if (ring->DimensionModel == GAIA_XY_Z_M)
				    {
					gaiaGetPointXYZM (ring->Coords, iv,
							  &x, &y, &z, &m);
				    }
				  else
				    {
					gaiaGetPoint (ring->Coords, iv, &x, &y);
				    }
				  gaiaExport64 (shp->BufShp + ix, m,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
			      }
			    for (ib = 0; ib < polyg->NumInteriors; ib++)
			      {
				  /* that ones are the interior rings */
				  ring = polyg->Interiors + ib;
				  for (iv = 0; iv < ring->Points; iv++)
				    {
					/* exports M-values - interior ring */
					m = 0.0;
					if (ring->DimensionModel == GAIA_XY_Z)
					  {
					      gaiaGetPointXYZ (ring->Coords,
							       iv, &x, &y, &z);
					  }
					else if (ring->DimensionModel ==
						 GAIA_XY_M)
					  {
					      gaiaGetPointXYM (ring->Coords,
							       iv, &x, &y, &m);
					  }
					else if (ring->DimensionModel ==
						 GAIA_XY_Z_M)
					  {
					      gaiaGetPointXYZM (ring->Coords,
								iv, &x, &y, &z,
								&m);
					  }
					else
					  {
					      gaiaGetPoint (ring->Coords,
							    iv, &x, &y);
					  }
					gaiaExport64 (shp->BufShp + ix, m,
						      GAIA_LITTLE_ENDIAN,
						      endian_arch);
					ix += 8;
				    }
			      }
			    polyg = polyg->Next;
			}
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);
	    }
	  if (shp->Shape == GAIA_SHP_POLYGONM)
	    {
		/* this one is expected to be a POLYGON or a MULTIPOLYGON M */
		gaiaPolygonPtr polyg;
		gaiaRingPtr ring;
		int ib;
		gaiaMRangeGeometry (entity->Geometry, &minM, &maxM);
		tot_ln = 0;
		tot_v = 0;
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* computes # rings and total # points */
		      gaiaSaneClockwise (polyg);	/* we must assure that exterior ring is clockwise, and interior rings are anti-clockwise */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      tot_v += ring->Points;
		      tot_ln++;
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    tot_v += ring->Points;
			    tot_ln++;
			}
		      polyg = polyg->Next;
		  }
		if (!tot_ln)
		  {
		      strcpy (dummy,
			      "a POLYGON is expected, but there is no POLYGON in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		this_size = 30 + (2 * tot_ln) + (tot_v * 12);	/* size [in 16 bits words !!!] for this SHP entity */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting POLYGON or MULTIPOLYGON in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting POLYGON or MULTIPOLYGON in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_POLYGONM, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POLYGON M */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_ln, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # rings in this polygon */
		gaiaExport32 (shp->BufShp + 48, tot_v, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		tot_v = 0;	/* resets points counter */
		ix = 52;	/* sets current buffer offset */
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* exports start point index for each line */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      gaiaExport32 (shp->BufShp + ix, tot_v,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      tot_v += ring->Points;
		      ix += 4;
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    gaiaExport32 (shp->BufShp + ix, tot_v,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    tot_v += ring->Points;
			    ix += 4;
			}
		      polyg = polyg->Next;
		  }
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* exports points for each ring */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      for (iv = 0; iv < ring->Points; iv++)
			{
			    /* exports a POINT [x,y] - exterior ring */
			    if (ring->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (ring->Coords, iv, &x, &y,
						   &z);
			      }
			    else if (ring->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (ring->Coords, iv, &x, &y,
						   &m);
			      }
			    else if (ring->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (ring->Coords, iv, &x,
						    &y, &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (ring->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (shp->BufShp + ix, x,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			    gaiaExport64 (shp->BufShp + ix, y,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    for (iv = 0; iv < ring->Points; iv++)
			      {
				  /* exports a POINT [x,y] - interior ring */
				  if (ring->DimensionModel == GAIA_XY_Z)
				    {
					gaiaGetPointXYZ (ring->Coords, iv,
							 &x, &y, &z);
				    }
				  else if (ring->DimensionModel == GAIA_XY_M)
				    {
					gaiaGetPointXYM (ring->Coords, iv,
							 &x, &y, &m);
				    }
				  else if (ring->DimensionModel == GAIA_XY_Z_M)
				    {
					gaiaGetPointXYZM (ring->Coords, iv,
							  &x, &y, &z, &m);
				    }
				  else
				    {
					gaiaGetPoint (ring->Coords, iv, &x, &y);
				    }
				  gaiaExport64 (shp->BufShp + ix, x,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
				  gaiaExport64 (shp->BufShp + ix, y,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
			      }
			}
		      polyg = polyg->Next;
		  }
		/* exporting the M-range [min/max] */
		gaiaExport64 (shp->BufShp + ix, minM, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		gaiaExport64 (shp->BufShp + ix, maxM, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* exports M-values for each ring */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      for (iv = 0; iv < ring->Points; iv++)
			{
			    /* exports M-values - exterior ring */
			    m = 0.0;
			    if (ring->DimensionModel == GAIA_XY_Z)
			      {
				  gaiaGetPointXYZ (ring->Coords, iv, &x, &y,
						   &z);
			      }
			    else if (ring->DimensionModel == GAIA_XY_M)
			      {
				  gaiaGetPointXYM (ring->Coords, iv, &x, &y,
						   &m);
			      }
			    else if (ring->DimensionModel == GAIA_XY_Z_M)
			      {
				  gaiaGetPointXYZM (ring->Coords, iv, &x,
						    &y, &z, &m);
			      }
			    else
			      {
				  gaiaGetPoint (ring->Coords, iv, &x, &y);
			      }
			    gaiaExport64 (shp->BufShp + ix, m,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    for (iv = 0; iv < ring->Points; iv++)
			      {
				  /* exports M-values - interior ring */
				  m = 0.0;
				  if (ring->DimensionModel == GAIA_XY_Z)
				    {
					gaiaGetPointXYZ (ring->Coords, iv,
							 &x, &y, &z);
				    }
				  else if (ring->DimensionModel == GAIA_XY_M)
				    {
					gaiaGetPointXYM (ring->Coords, iv,
							 &x, &y, &m);
				    }
				  else if (ring->DimensionModel == GAIA_XY_Z_M)
				    {
					gaiaGetPointXYZM (ring->Coords, iv,
							  &x, &y, &z, &m);
				    }
				  else
				    {
					gaiaGetPoint (ring->Coords, iv, &x, &y);
				    }
				  gaiaExport64 (shp->BufShp + ix, m,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
			      }
			}
		      polyg = polyg->Next;
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);
	    }
	  if (shp->Shape == GAIA_SHP_MULTIPOINT)
	    {
		/* this one is expected to be a MULTIPOINT */
		gaiaPointPtr pt;
		tot_pts = 0;
		pt = entity->Geometry->FirstPoint;
		while (pt)
		  {
		      /* computes # points */
		      tot_pts++;
		      pt = pt->Next;
		  }
		if (!tot_pts)
		  {
		      strcpy (dummy,
			      "a MULTIPOINT is expected, but there is no POINT/MULTIPOINT in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		this_size = 20 + (tot_pts * 8);	/* size [in 16 bits words !!!] for this SHP entity */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting MULTIPOINT in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting MULTIPOINT in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_MULTIPOINT, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = MULTIPOINT */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_pts, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		ix = 48;	/* sets current buffer offset */
		pt = entity->Geometry->FirstPoint;
		while (pt)
		  {
		      /* exports each point */
		      gaiaExport64 (shp->BufShp + ix, pt->X,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      gaiaExport64 (shp->BufShp + ix, pt->Y,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      pt = pt->Next;
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);	/* updating current SHP file position [in 16 bits words !!!] */
	    }
	  if (shp->Shape == GAIA_SHP_MULTIPOINTZ)
	    {
		/* this one is expected to be a MULTIPOINT Z */
		gaiaPointPtr pt;
		gaiaZRangeGeometry (entity->Geometry, &minZ, &maxZ);
		gaiaMRangeGeometry (entity->Geometry, &minM, &maxM);
		tot_pts = 0;
		pt = entity->Geometry->FirstPoint;
		while (pt)
		  {
		      /* computes # points */
		      tot_pts++;
		      pt = pt->Next;
		  }
		if (!tot_pts)
		  {
		      strcpy (dummy,
			      "a MULTIPOINT is expected, but there is no POINT/MULTIPOINT in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		hasM = 0;
		if (shp->EffectiveDims == GAIA_XY_M
		    || shp->EffectiveDims == GAIA_XY_Z_M)
		    hasM = 1;
		if (hasM)
		    this_size = 36 + (tot_pts * 16);	/* size [in 16 bits words !!!] ZM */
		else
		    this_size = 28 + (tot_pts * 12);	/* size [in 16 bits words !!!] Z-only */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting MULTIPOINT in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting MULTIPOINT in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_MULTIPOINTZ, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = MULTIPOINT Z */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_pts, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		ix = 48;	/* sets current buffer offset */
		pt = entity->Geometry->FirstPoint;
		while (pt)
		  {
		      /* exports each point */
		      gaiaExport64 (shp->BufShp + ix, pt->X,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      gaiaExport64 (shp->BufShp + ix, pt->Y,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      pt = pt->Next;
		  }
		/* exporting the Z-range [min/max] */
		gaiaExport64 (shp->BufShp + ix, minZ, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		gaiaExport64 (shp->BufShp + ix, maxZ, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		pt = entity->Geometry->FirstPoint;
		while (pt)
		  {
		      /* exports Z-values */
		      gaiaExport64 (shp->BufShp + ix, pt->Z,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      pt = pt->Next;
		  }
		if (hasM)
		  {
		      /* exporting the M-range [min/max] */
		      gaiaExport64 (shp->BufShp + ix, minM,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      gaiaExport64 (shp->BufShp + ix, maxM,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      pt = entity->Geometry->FirstPoint;
		      while (pt)
			{
			    /* exports M-values */
			    gaiaExport64 (shp->BufShp + ix, pt->M,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			    pt = pt->Next;
			}
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);	/* updating current SHP file position [in 16 bits words !!!] */
	    }
	  if (shp->Shape == GAIA_SHP_MULTIPOINTM)
	    {
		/* this one is expected to be a MULTIPOINT M */
		gaiaPointPtr pt;
		gaiaMRangeGeometry (entity->Geometry, &minM, &maxM);
		tot_pts = 0;
		pt = entity->Geometry->FirstPoint;
		while (pt)
		  {
		      /* computes # points */
		      tot_pts++;
		      pt = pt->Next;
		  }
		if (!tot_pts)
		  {
		      strcpy (dummy,
			      "a MULTIPOINT is expected, but there is no POINT/MULTIPOINT in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		this_size = 28 + (tot_pts * 12);	/* size [in 16 bits words !!!] for this SHP entity */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting MULTIPOINT in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting MULTIPOINT in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno + 1, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, GAIA_SHP_MULTIPOINTM, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = MULTIPOINT M */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_pts, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		ix = 48;	/* sets current buffer offset */
		pt = entity->Geometry->FirstPoint;
		while (pt)
		  {
		      /* exports each point */
		      gaiaExport64 (shp->BufShp + ix, pt->X,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      gaiaExport64 (shp->BufShp + ix, pt->Y,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      pt = pt->Next;
		  }
		/* exporting the M-range [min/max] */
		gaiaExport64 (shp->BufShp + ix, minM, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		gaiaExport64 (shp->BufShp + ix, maxM, GAIA_LITTLE_ENDIAN,
			      endian_arch);
		ix += 8;
		pt = entity->Geometry->FirstPoint;
		while (pt)
		  {
		      /* exports M-values */
		      gaiaExport64 (shp->BufShp + ix, pt->M,
				    GAIA_LITTLE_ENDIAN, endian_arch);
		      ix += 8;
		      pt = pt->Next;
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);	/* updating current SHP file position [in 16 bits words !!!] */
	    }
      }
/* inserting entity in DBF file */
    fwrite (shp->BufDbf, 1, shp->DbfReclen, shp->flDbf);
    (shp->DbfRecno)++;
    return 1;
  conversion_error:
    if (shp->LastError)
	free (shp->LastError);
    sprintf (dummy, "Invalid character sequence");
    len = strlen (dummy);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, dummy);
    return 0;
}

GAIAGEO_DECLARE void
gaiaFlushShpHeaders (gaiaShapefilePtr shp)
{
/* updates the various file headers */
    FILE *fl_shp = shp->flShp;
    FILE *fl_shx = shp->flShx;
    FILE *fl_dbf = shp->flDbf;
    int shp_size = shp->ShpSize;
    int shx_size = shp->ShxSize;
    int dbf_size = shp->DbfSize;
    int dbf_reclen = shp->DbfReclen;
    int dbf_recno = shp->DbfRecno;
    int endian_arch = shp->endian_arch;
    double minx = shp->MinX;
    double miny = shp->MinY;
    double maxx = shp->MaxX;
    double maxy = shp->MaxY;
    unsigned char *buf_shp = shp->BufShp;
/* writing the SHP file header */
    fseek (fl_shp, 0, SEEK_SET);	/* repositioning at SHP file start */
    gaiaExport32 (buf_shp, 9994, GAIA_BIG_ENDIAN, endian_arch);	/* SHP magic number */
    gaiaExport32 (buf_shp + 4, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 8, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 12, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 16, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 20, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 24, shp_size, GAIA_BIG_ENDIAN, endian_arch);	/* SHP file size - measured in 16 bits words !!! */
    gaiaExport32 (buf_shp + 28, 1000, GAIA_LITTLE_ENDIAN, endian_arch);	/* version */
    gaiaExport32 (buf_shp + 32, shp->Shape, GAIA_LITTLE_ENDIAN, endian_arch);	/* ESRI shape */
    gaiaExport64 (buf_shp + 36, minx, GAIA_LITTLE_ENDIAN, endian_arch);	/* the MBR/BBOX for the whole shapefile */
    gaiaExport64 (buf_shp + 44, miny, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 52, maxx, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 60, maxy, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 68, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 76, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 84, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 92, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    fwrite (buf_shp, 1, 100, fl_shp);
/* writing the SHX file header */
    fseek (fl_shx, 0, SEEK_SET);	/* repositioning at SHX file start */
    gaiaExport32 (buf_shp, 9994, GAIA_BIG_ENDIAN, endian_arch);	/* SHP magic number */
    gaiaExport32 (buf_shp + 4, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 8, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 12, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 16, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 20, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 24, shx_size, GAIA_BIG_ENDIAN, endian_arch);	/* SHXfile size - measured in 16 bits words !!! */
    gaiaExport32 (buf_shp + 28, 1000, GAIA_LITTLE_ENDIAN, endian_arch);	/* version */
    gaiaExport32 (buf_shp + 32, shp->Shape, GAIA_LITTLE_ENDIAN, endian_arch);	/* ESRI shape */
    gaiaExport64 (buf_shp + 36, minx, GAIA_LITTLE_ENDIAN, endian_arch);	/* the MBR for the whole shapefile */
    gaiaExport64 (buf_shp + 44, miny, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 52, maxx, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 60, maxy, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 68, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 76, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 84, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 92, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    fwrite (buf_shp, 1, 100, fl_shx);
/* writing the DBF file header */
    *buf_shp = 0x1a;		/* DBF - this is theEOF marker */
    fwrite (buf_shp, 1, 1, fl_dbf);
    fseek (fl_dbf, 0, SEEK_SET);	/* repositioning at DBF file start */
    memset (buf_shp, '\0', 32);
    *buf_shp = 0x03;		/* DBF magic number */
    *(buf_shp + 1) = 1;		/* this is supposed to be the last update date [Year, Month, Day], but we ignore it at all */
    *(buf_shp + 2) = 1;
    *(buf_shp + 3) = 1;
    gaiaExport32 (buf_shp + 4, dbf_recno, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # records in this DBF */
    gaiaExport16 (buf_shp + 8, (short) dbf_size, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the file header size */
    gaiaExport16 (buf_shp + 10, (short) dbf_reclen, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the record length */
    fwrite (buf_shp, 1, 32, fl_dbf);
}

GAIAGEO_DECLARE void
gaiaShpAnalyze (gaiaShapefilePtr shp)
{
/* analyzing the SHP content, in order to detect if there are LINESTRINGS or MULTILINESTRINGS 
/ the same check is needed in order to detect if there are POLYGONS or MULTIPOLYGONS 
 */
    unsigned char buf[512];
    int rd;
    int skpos;
    int offset;
    int off_shp;
    int sz;
    int shape;
    int points;
    int n;
    int n1;
    int base;
    int start;
    int end;
    int iv;
    int ind;
    double x;
    double y;
    int polygons;
    int ZM_size;
    int multi = 0;
    int hasM = 0;
    int current_row = 0;

    gaiaRingPtr ring = NULL;
    while (1)
      {
	  /* positioning and reading the SHX file */
	  offset = 100 + (current_row * 8);	/* 100 bytes for the header + current row displacement; each SHX row = 8 bytes */
	  skpos = fseek (shp->flShx, offset, SEEK_SET);
	  if (skpos != 0)
	      goto exit;
	  rd = fread (buf, sizeof (unsigned char), 8, shp->flShx);
	  if (rd != 8)
	      goto exit;
	  off_shp = gaiaImport32 (buf, GAIA_BIG_ENDIAN, shp->endian_arch);
	  /* positioning and reading corresponding SHP entity - geometry */
	  offset = off_shp * 2;
	  skpos = fseek (shp->flShp, offset, SEEK_SET);
	  if (skpos != 0)
	      goto exit;
	  rd = fread (buf, sizeof (unsigned char), 12, shp->flShp);
	  if (rd != 12)
	      goto exit;
	  sz = gaiaImport32 (buf + 4, GAIA_BIG_ENDIAN, shp->endian_arch);
	  shape = gaiaImport32 (buf + 8, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  if ((sz * 2) > shp->ShpBfsz)
	    {
		/* current buffer is too small; we need to allocate a bigger buffer */
		free (shp->BufShp);
		shp->ShpBfsz = sz * 2;
		shp->BufShp = malloc (sizeof (unsigned char) * shp->ShpBfsz);
	    }
	  if (shape == GAIA_SHP_POLYLINE || shape == GAIA_SHP_POLYLINEZ
	      || shape == GAIA_SHP_POLYLINEM)
	    {
		/* shape polyline */
		rd = fread (shp->BufShp, sizeof (unsigned char), 32,
			    shp->flShp);
		if (rd != 32)
		    goto exit;
		rd = fread (shp->BufShp, sizeof (unsigned char),
			    (sz * 2) - 36, shp->flShp);
		if (rd != (sz * 2) - 36)
		    goto exit;
		n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN,
				  shp->endian_arch);
		n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
				   shp->endian_arch);
		if (n > 1)
		    multi++;
		if (shape == GAIA_SHP_POLYLINEZ)
		  {
		      ZM_size = 38 + (2 * n) + (n1 * 16);	/* size [in 16 bits words !!!] ZM */
		      if (sz == ZM_size)
			  hasM = 1;
		  }
	    }
	  if (shape == GAIA_SHP_POLYGON || shape == GAIA_SHP_POLYGONZ
	      || shape == GAIA_SHP_POLYGONM)
	    {
		/* shape polygon */
		struct shp_ring_item *pExt;
		struct shp_ring_collection ringsColl;
		/* initializing the RING collection */
		ringsColl.First = NULL;
		ringsColl.Last = NULL;

		rd = fread (shp->BufShp, sizeof (unsigned char), 32,
			    shp->flShp);
		if (rd != 32)
		    goto exit;
		rd = fread (shp->BufShp, sizeof (unsigned char),
			    (sz * 2) - 36, shp->flShp);
		if (rd != (sz * 2) - 36)
		    goto exit;
		n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN,
				  shp->endian_arch);
		n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
				   shp->endian_arch);
		base = 8 + (n * 4);
		start = 0;
		for (ind = 0; ind < n; ind++)
		  {
		      if (ind < (n - 1))
			  end =
			      gaiaImport32 (shp->BufShp + 8 +
					    ((ind + 1) * 4),
					    GAIA_LITTLE_ENDIAN,
					    shp->endian_arch);
		      else
			  end = n1;
		      points = end - start;
		      ring = gaiaAllocRing (points);
		      points = 0;
		      for (iv = start; iv < end; iv++)
			{
			    x = gaiaImport64 (shp->BufShp + base +
					      (iv * 16), GAIA_LITTLE_ENDIAN,
					      shp->endian_arch);
			    y = gaiaImport64 (shp->BufShp + base +
					      (iv * 16) + 8,
					      GAIA_LITTLE_ENDIAN,
					      shp->endian_arch);
			    gaiaSetPoint (ring->Coords, points, x, y);
			    start++;
			    points++;
			}
		      shp_add_ring (&ringsColl, ring);
		      ring = NULL;
		  }
		shp_arrange_rings (&ringsColl);
		pExt = ringsColl.First;
		polygons = 0;
		while (pExt != NULL)
		  {
		      if (pExt->IsExterior)
			  polygons++;
		      pExt = pExt->Next;
		  }
		shp_free_rings (&ringsColl);

		if (polygons > 1)
		    multi++;
		if (shape == GAIA_SHP_POLYGONZ)
		  {
		      ZM_size = 38 + (2 * n) + (n1 * 16);	/* size [in 16 bits words !!!] ZM */
		      if (sz == ZM_size)
			  hasM = 1;
		  }
	    }
	  if (shape == GAIA_SHP_MULTIPOINTZ)
	    {
		/* shape multipoint Z */
		rd = fread (shp->BufShp, sizeof (unsigned char), 32,
			    shp->flShp);
		if (rd != 32)
		    goto exit;
		rd = fread (shp->BufShp, sizeof (unsigned char),
			    (sz * 2) - 36, shp->flShp);
		if (rd != (sz * 2) - 36)
		    goto exit;
		n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN,
				  shp->endian_arch);
		ZM_size = 38 + (n * 16);	/* size [in 16 bits words !!!] ZM */
		if (sz == ZM_size)
		    hasM = 1;
	    }
	  current_row++;
      }
  exit:
    if (ring)
	gaiaFreeRing (ring);
    if (shp->LastError)
	free (shp->LastError);
    shp->LastError = NULL;
/* setting the EffectiveType, as determined by this analysis */
    if (shp->Shape == GAIA_SHP_POLYLINE || shp->Shape == GAIA_SHP_POLYLINEZ
	|| shp->Shape == GAIA_SHP_POLYLINEM)
      {
	  /* SHAPE polyline */
	  if (multi)
	      shp->EffectiveType = GAIA_MULTILINESTRING;
	  else
	      shp->EffectiveType = GAIA_LINESTRING;
      }
    if (shp->Shape == GAIA_SHP_POLYGON || shp->Shape == GAIA_SHP_POLYGONZ
	|| shp->Shape == GAIA_SHP_POLYGONM)
      {
	  /* SHAPE polygon */
	  if (multi)
	      shp->EffectiveType = GAIA_MULTIPOLYGON;
	  else
	      shp->EffectiveType = GAIA_POLYGON;
      }
    if (shp->Shape == GAIA_SHP_POLYLINEZ || shp->Shape == GAIA_SHP_POLYGONZ
	|| shp->Shape == GAIA_SHP_MULTIPOINTZ)
      {
	  if (hasM)
	      shp->EffectiveDims = GAIA_XY_Z_M;
	  else
	      shp->EffectiveDims = GAIA_XY_Z;
      }
}

GAIAGEO_DECLARE gaiaDbfPtr
gaiaAllocDbf ()
{
/* allocates and initializes the DBF object */
    gaiaDbfPtr dbf = malloc (sizeof (gaiaDbf));
    dbf->endian_arch = 1;
    dbf->Path = NULL;
    dbf->flDbf = NULL;
    dbf->Dbf = NULL;
    dbf->BufDbf = NULL;
    dbf->DbfHdsz = 0;
    dbf->DbfReclen = 0;
    dbf->DbfSize = 0;
    dbf->DbfRecno = 0;
    dbf->Valid = 0;
    dbf->IconvObj = NULL;
    dbf->LastError = NULL;
    return dbf;
}

GAIAGEO_DECLARE void
gaiaFreeDbf (gaiaDbfPtr dbf)
{
/* frees all memory allocations related to the DBF object */
    if (dbf->Path)
	free (dbf->Path);
    if (dbf->flDbf)
	fclose (dbf->flDbf);
    if (dbf->Dbf)
	gaiaFreeDbfList (dbf->Dbf);
    if (dbf->BufDbf)
	free (dbf->BufDbf);
    if (dbf->IconvObj)
	iconv_close ((iconv_t) dbf->IconvObj);
    if (dbf->LastError)
	free (dbf->LastError);
    free (dbf);
}

GAIAGEO_DECLARE void
gaiaOpenDbfRead (gaiaDbfPtr dbf, const char *path, const char *charFrom,
		 const char *charTo)
{
/* trying to open the DBF and initial checkings */
    FILE *fl_dbf = NULL;
    int rd;
    unsigned char bf[1024];
    int dbf_size;
    int dbf_reclen = 0;
    int off_dbf;
    int ind;
    char field_name[2048];
    char *sys_err;
    char errMsg[1024];
    iconv_t iconv_ret;
    char utf8buf[2048];
#if !defined(__MINGW32__) && defined(_WIN32)
    const char *pBuf;
#else /* not WIN32 */
    char *pBuf;
#endif
    size_t len;
    size_t utf8len;
    char *pUtf8buf;
    int endian_arch = gaiaEndianArch ();
    gaiaDbfListPtr dbf_list = NULL;
    if (charFrom && charTo)
      {
	  iconv_ret = iconv_open (charTo, charFrom);
	  if (iconv_ret == (iconv_t) (-1))
	    {
		sprintf (errMsg,
			 "conversion from '%s' to '%s' not available\n",
			 charFrom, charTo);
		goto unsupported_conversion;
	    }
	  dbf->IconvObj = iconv_ret;
      }
    else
      {
	  sprintf (errMsg, "a NULL charset-name was passed\n");
	  goto unsupported_conversion;
      }
    if (dbf->flDbf != NULL)
      {
	  sprintf (errMsg, "attempting to reopen an already opened DBF\n");
	  goto unsupported_conversion;
      }
    fl_dbf = fopen (path, "rb");
    if (!fl_dbf)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for reading: %s", path,
		   sys_err);
	  goto no_file;
      }
/* reading DBF file header */
    rd = fread (bf, sizeof (unsigned char), 32, fl_dbf);
    if (rd != 32)
	goto error;
    switch (*bf)
      {
	  /* checks the DBF magic number */
      case 0x03:
      case 0x83:
	  break;
      case 0x02:
      case 0xF8:
	  sprintf (errMsg, "'%s'\ninvalid magic number %02x [FoxBASE format]",
		   path, *bf);
	  goto dbf_bad_magic;
      case 0xF5:
	  sprintf (errMsg,
		   "'%s'\ninvalid magic number %02x [FoxPro 2.x (or earlier) format]",
		   path, *bf);
	  goto dbf_bad_magic;
      case 0x30:
      case 0x31:
      case 0x32:
	  sprintf (errMsg,
		   "'%s'\ninvalid magic number %02x [Visual FoxPro format]",
		   path, *bf);
	  goto dbf_bad_magic;
      case 0x43:
      case 0x63:
      case 0xBB:
      case 0xCB:
	  sprintf (errMsg, "'%s'\ninvalid magic number %02x [dBASE IV format]",
		   path, *bf);
	  goto dbf_bad_magic;
      default:
	  sprintf (errMsg, "'%s'\ninvalid magic number %02x [unknown format]",
		   path, *bf);
	  goto dbf_bad_magic;
      };
    dbf_size = gaiaImport16 (bf + 8, GAIA_LITTLE_ENDIAN, endian_arch);
    dbf_reclen = gaiaImport16 (bf + 10, GAIA_LITTLE_ENDIAN, endian_arch);
    dbf_size--;
    off_dbf = 0;
    dbf_list = gaiaAllocDbfList ();
    for (ind = 32; ind < dbf_size; ind += 32)
      {
	  /* fetches DBF fields definitions */
	  rd = fread (bf, sizeof (unsigned char), 32, fl_dbf);
	  if (rd != 32)
	      goto error;
	  if (*(bf + 11) == 'M')
	    {
		/* skipping any MEMO field */
		memcpy (field_name, bf, 11);
		field_name[11] = '\0';
		off_dbf += *(bf + 16);
		spatialite_e
		    ("WARNING: column \"%s\" is of the MEMO type and will be ignored\n",
		     field_name);
		continue;
	    }
	  memcpy (field_name, bf, 11);
	  field_name[11] = '\0';
	  len = strlen ((char *) field_name);
	  utf8len = 2048;
	  pBuf = (char *) field_name;
	  pUtf8buf = utf8buf;
	  if (iconv
	      ((iconv_t) (dbf->IconvObj), &pBuf, &len, &pUtf8buf,
	       &utf8len) == (size_t) (-1))
	      goto conversion_error;
	  memcpy (field_name, utf8buf, 2048 - utf8len);
	  field_name[2048 - utf8len] = '\0';
	  gaiaAddDbfField (dbf_list, field_name, *(bf + 11), off_dbf,
			   *(bf + 16), *(bf + 17));
	  off_dbf += *(bf + 16);
      }
    if (!gaiaIsValidDbfList (dbf_list))
      {
	  /* invalid DBF */
	  goto illegal_dbf;
      }
    len = strlen (path);
    dbf->Path = malloc (len + 1);
    strcpy (dbf->Path, path);
    dbf->flDbf = fl_dbf;
    dbf->Dbf = dbf_list;
/* allocating DBF buffer */
    dbf->BufDbf = malloc (sizeof (unsigned char) * dbf_reclen);
    dbf->DbfHdsz = dbf_size + 1;
    dbf->DbfReclen = dbf_reclen;
    dbf->Valid = 1;
    dbf->endian_arch = endian_arch;
    return;
  unsupported_conversion:
/* illegal charset */
    if (dbf->LastError)
	free (dbf->LastError);
    len = strlen (errMsg);
    dbf->LastError = malloc (len + 1);
    strcpy (dbf->LastError, errMsg);
    return;
  no_file:
/* the DBF file can't be accessed */
    if (dbf->LastError)
	free (dbf->LastError);
    len = strlen (errMsg);
    dbf->LastError = malloc (len + 1);
    strcpy (dbf->LastError, errMsg);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
  error:
/* the DBF is invalid or corrupted */
    if (dbf->LastError)
	free (dbf->LastError);
    sprintf (errMsg, "'%s' is corrupted / has invalid format", path);
    len = strlen (errMsg);
    dbf->LastError = malloc (len + 1);
    strcpy (dbf->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    fclose (fl_dbf);
    return;
  dbf_bad_magic:
/* the DBF has an invalid magic number */
    if (dbf->LastError)
	free (dbf->LastError);
    len = strlen (errMsg);
    dbf->LastError = malloc (len + 1);
    strcpy (dbf->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    fclose (fl_dbf);
    return;
  illegal_dbf:
/* the DBF-file contains unsupported data types */
    if (dbf->LastError)
	free (dbf->LastError);
    sprintf (errMsg, "'%s' contains unsupported data types", path);
    len = strlen (errMsg);
    dbf->LastError = malloc (len + 1);
    strcpy (dbf->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
  conversion_error:
/* libiconv error */
    if (dbf->LastError)
	free (dbf->LastError);
    sprintf (errMsg, "'%s' field name: invalid character sequence", path);
    len = strlen (errMsg);
    dbf->LastError = malloc (len + 1);
    strcpy (dbf->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
}

GAIAGEO_DECLARE void
gaiaOpenDbfWrite (gaiaDbfPtr dbf, const char *path, const char *charFrom,
		  const char *charTo)
{
/* trying to create the DBF file */
    FILE *fl_dbf = NULL;
    unsigned char bf[1024];
    unsigned char *dbf_buf = NULL;
    gaiaDbfFieldPtr fld;
    char *sys_err;
    char errMsg[1024];
    short dbf_reclen = 0;
    unsigned short dbf_size = 0;
    iconv_t iconv_ret;
    char buf[2048];
    char utf8buf[2048];
#if !defined(__MINGW32__) && defined(_WIN32)
    const char *pBuf;
#else /* not WIN32 */
    char *pBuf;
#endif
    size_t len;
    size_t utf8len;
    char *pUtf8buf;
    int defaultId = 1;
    struct auxdbf_list *auxdbf = NULL;
    if (charFrom && charTo)
      {
	  iconv_ret = iconv_open (charTo, charFrom);
	  if (iconv_ret == (iconv_t) (-1))
	    {
		sprintf (errMsg, "conversion from '%s' to '%s' not available\n",
			 charFrom, charTo);
		goto unsupported_conversion;
	    }
	  dbf->IconvObj = iconv_ret;
      }
    else
      {
	  sprintf (errMsg, "a NULL charset-name was passed\n");
	  goto unsupported_conversion;
      }
    if (dbf->flDbf != NULL)
      {
	  sprintf (errMsg, "attempting to reopen an already opened DBF file\n");
	  goto unsupported_conversion;
      }
/* trying to open the DBF file */
    fl_dbf = fopen (path, "wb");
    if (!fl_dbf)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for writing: %s", path,
		   sys_err);
	  goto no_file;
      }
/* allocating DBF buffer */
    dbf_reclen = 1;		/* an extra byte is needed because in DBF rows first byte is a marker for deletion */
    fld = dbf->Dbf->First;
    while (fld)
      {
	  /* computing the DBF record length */
	  dbf_reclen += fld->Length;
	  fld = fld->Next;
      }
    dbf_buf = malloc (dbf_reclen);
/* writing the DBF file header */
    memset (bf, '\0', 32);
    fwrite (bf, 1, 32, fl_dbf);
    dbf_size = 32;		/* note: DBF counts sizes in bytes */
    auxdbf = alloc_auxdbf (dbf->Dbf);
    fld = dbf->Dbf->First;
    while (fld)
      {
	  /* exporting DBF Fields specifications */
	  memset (bf, 0, 32);
	  if (strlen (fld->Name) > 10)
	    {
		/* long name: attempting to safely truncate */
		truncate_long_name (auxdbf, fld);
	    }
	  strcpy (buf, fld->Name);
	  len = strlen (buf);
	  utf8len = 2048;
	  pBuf = buf;
	  pUtf8buf = utf8buf;
	  if (iconv
	      ((iconv_t) (dbf->IconvObj), &pBuf, &len, &pUtf8buf,
	       &utf8len) == (size_t) (-1))
	      sprintf (buf, "FLD#%d", defaultId++);
	  else
	    {
		memcpy (buf, utf8buf, 2048 - utf8len);
		buf[2048 - utf8len] = '\0';
		if (strlen (buf) > 10)
		    sprintf (buf, "FLD#%d", defaultId++);
	    }
	  memcpy (bf, buf, strlen (buf));
	  *(bf + 11) = fld->Type;
	  *(bf + 16) = fld->Length;
	  *(bf + 17) = fld->Decimals;
	  fwrite (bf, 1, 32, fl_dbf);
	  dbf_size += 32;
	  fld = fld->Next;
      }
    free_auxdbf (auxdbf);
    fwrite ("\r", 1, 1, fl_dbf);	/* this one is a special DBF delimiter that closes file header */
    dbf_size++;
    dbf->Valid = 1;
    dbf->flDbf = fl_dbf;
    dbf->BufDbf = dbf_buf;
    dbf->DbfHdsz = dbf_size + 1;
    dbf->DbfReclen = dbf_reclen;
    dbf->DbfSize = dbf_size;
    dbf->DbfRecno = 0;
    return;
  unsupported_conversion:
/* illegal charset */
    if (dbf->LastError)
	free (dbf->LastError);
    len = strlen (errMsg);
    dbf->LastError = malloc (len + 1);
    strcpy (dbf->LastError, errMsg);
    return;
  no_file:
/* the DBF file can't be created/opened */
    if (dbf->LastError)
	free (dbf->LastError);
    len = strlen (errMsg);
    dbf->LastError = malloc (len + 1);
    strcpy (dbf->LastError, errMsg);
    if (dbf_buf)
	free (dbf_buf);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
}

GAIAGEO_DECLARE int
gaiaWriteDbfEntity (gaiaDbfPtr dbf, gaiaDbfListPtr entity)
{
/* trying to write an entity into some DBF file */
    char dummy[128];
    char fmt[16];
    gaiaDbfFieldPtr fld;
#if !defined(__MINGW32__) && defined(_WIN32)
    const char *pBuf;
#else /* not WIN32 */
    char *pBuf;
#endif
    size_t len;
    size_t utf8len;
    char *pUtf8buf;
    char *dynbuf;
    char utf8buf[2048];
/* writing the DBF record */
    memset (dbf->BufDbf, '\0', dbf->DbfReclen);
    *(dbf->BufDbf) = ' ';	/* in DBF first byte of each row marks for validity or deletion */
    fld = entity->First;
    while (fld)
      {
	  /* transferring field values */
	  switch (fld->Type)
	    {
	    case 'L':
		if (!(fld->Value))
		    *(dbf->BufDbf + fld->Offset) = '?';
		else if (fld->Value->Type != GAIA_INT_VALUE)
		    *(dbf->BufDbf + fld->Offset + 1) = '?';
		else
		  {
		      if (fld->Value->IntValue == 0)
			  *(dbf->BufDbf + fld->Offset + 1) = 'N';
		      else
			  *(dbf->BufDbf + fld->Offset + 1) = 'Y';
		  }
		break;
	    case 'D':
		memset (dbf->BufDbf + fld->Offset + 1, '0', 8);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_TEXT_VALUE)
			{
			    if (strlen (fld->Value->TxtValue) == 8)
				memcpy (dbf->BufDbf + fld->Offset + 1,
					fld->Value->TxtValue, 8);
			}
		  }
		break;
	    case 'C':
		memset (dbf->BufDbf + fld->Offset + 1, ' ', fld->Length);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_TEXT_VALUE)
			{
			    len = strlen (fld->Value->TxtValue);
			    dynbuf = malloc (len + 1);
			    strcpy (dynbuf, fld->Value->TxtValue);
			    if (len > 512)
			      {
				  dynbuf[512] = '\0';
				  len = strlen (dynbuf);
			      }
			    utf8len = 2048;
			    pBuf = dynbuf;
			    pUtf8buf = utf8buf;
			    if (iconv
				((iconv_t) (dbf->IconvObj), &pBuf, &len,
				 &pUtf8buf, &utf8len) == (size_t) (-1))
			      {
				  free (dynbuf);
				  goto conversion_error;
			      }
			    memcpy (dynbuf, utf8buf, 2048 - utf8len);
			    dynbuf[2048 - utf8len] = '\0';
			    if (strlen (dynbuf) < fld->Length)
				memcpy (dbf->BufDbf + fld->Offset + 1, dynbuf,
					strlen (dynbuf));
			    else
				memcpy (dbf->BufDbf + fld->Offset + 1, dynbuf,
					fld->Length);
			    free (dynbuf);
			}
		  }
		break;
	    case 'N':
		memset (dbf->BufDbf + fld->Offset + 1, '\0', fld->Length);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_INT_VALUE)
			{
			    sprintf (dummy, FRMT64, fld->Value->IntValue);
			    if (strlen (dummy) <= fld->Length)
				memcpy (dbf->BufDbf + fld->Offset + 1,
					dummy, strlen (dummy));
			}
		      if (fld->Value->Type == GAIA_DOUBLE_VALUE)
			{
			    sprintf (fmt, "%%1.%df", fld->Decimals);
			    sprintf (dummy, fmt, fld->Value->DblValue);
			    if (strlen (dummy) <= fld->Length)
				memcpy (dbf->BufDbf + fld->Offset + 1,
					dummy, strlen (dummy));
			}
		  }
		break;
	    };
	  fld = fld->Next;
      }
/* inserting entity in DBF file */
    fwrite (dbf->BufDbf, 1, dbf->DbfReclen, dbf->flDbf);
    (dbf->DbfRecno)++;
    return 1;
  conversion_error:
    if (dbf->LastError)
	free (dbf->LastError);
    sprintf (dummy, "Invalid character sequence");
    len = strlen (dummy);
    dbf->LastError = malloc (len + 1);
    strcpy (dbf->LastError, dummy);
    return 0;
}

GAIAGEO_DECLARE void
gaiaFlushDbfHeader (gaiaDbfPtr dbf)
{
/* updates the DBF file header */
    FILE *fl_dbf = dbf->flDbf;
    int dbf_size = dbf->DbfSize;
    int dbf_reclen = dbf->DbfReclen;
    int dbf_recno = dbf->DbfRecno;
    int endian_arch = dbf->endian_arch;
    unsigned char bf[64];
/* writing the DBF file header */
    *bf = 0x1a;			/* DBF - this is theEOF marker */
    fwrite (bf, 1, 1, fl_dbf);
    fseek (fl_dbf, 0, SEEK_SET);	/* repositioning at DBF file start */
    memset (bf, '\0', 32);
    *bf = 0x03;			/* DBF magic number */
    *(bf + 1) = 1;		/* this is supposed to be the last update date [Year, Month, Day], but we ignore it at all */
    *(bf + 2) = 1;
    *(bf + 3) = 1;
    gaiaExport32 (bf + 4, dbf_recno, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # records in this DBF */
    gaiaExport16 (bf + 8, (short) dbf_size, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the file header size */
    gaiaExport16 (bf + 10, (short) dbf_reclen, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the record length */
    fwrite (bf, 1, 32, fl_dbf);
}

GAIAGEO_DECLARE int
gaiaReadDbfEntity (gaiaDbfPtr dbf, int current_row, int *deleted)
{
    return gaiaReadDbfEntity_ex (dbf, current_row, deleted, 0);
}

GAIAGEO_DECLARE int
gaiaReadDbfEntity_ex (gaiaDbfPtr dbf, int current_row, int *deleted,
		      int text_dates)
{
/* trying to read an entity from DBF */
    int rd;
    int skpos;
    int offset;
    int len;
    char errMsg[1024];
    gaiaDbfFieldPtr pFld;
/* positioning and reading the DBF file */
    offset = dbf->DbfHdsz + (current_row * dbf->DbfReclen);
    skpos = fseek (dbf->flDbf, offset, SEEK_SET);
    if (skpos != 0)
	goto eof;
    rd = fread (dbf->BufDbf, sizeof (unsigned char), dbf->DbfReclen,
		dbf->flDbf);
    if (rd != dbf->DbfReclen)
	goto eof;
/* setting up the current DBF ENTITY */
    gaiaResetDbfEntity (dbf->Dbf);
    dbf->Dbf->RowId = current_row;
    if (*(dbf->BufDbf) == '*')
      {
	  /* deleted row */
	  *deleted = 1;
	  if (dbf->LastError)
	      free (dbf->LastError);
	  dbf->LastError = NULL;
	  return 1;
      }
/* fetching the DBF values */
    pFld = dbf->Dbf->First;
    while (pFld)
      {
	  if (!parseDbfField (dbf->BufDbf, dbf->IconvObj, pFld, text_dates))
	      goto conversion_error;
	  pFld = pFld->Next;
      }
    if (dbf->LastError)
	free (dbf->LastError);
    dbf->LastError = NULL;
    *deleted = 0;
    return 1;
  eof:
    if (dbf->LastError)
	free (dbf->LastError);
    dbf->LastError = NULL;
    return 0;
  conversion_error:
    if (dbf->LastError)
	free (dbf->LastError);
    sprintf (errMsg, "Invalid character sequence");
    len = strlen (errMsg);
    dbf->LastError = malloc (len + 1);
    strcpy (dbf->LastError, errMsg);
    return 0;
}

#endif /* ICONV enabled/disabled */
